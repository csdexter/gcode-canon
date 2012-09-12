/*
 * gcode-machine.c
 *
 *  Created on: Aug 11, 2012
 *      Author: csdexter
 */

#include <math.h>
#include <stdio.h>

#include "gcode-commons.h"
#include "gcode-machine.h"
#include "gcode-debugcon.h"
#include "gcode-parameters.h"


static double machineX, machineY, machineZ;
static uint16_t machineA, machineB, machineC;
static uint32_t spindleSpeed;
static TGCodeMachineState currentMachineState;

bool init_machine(void *data) {
	machineX = machineY = machineZ = 0.0;
	machineA = machineB = machineC = 0;
	currentMachineState.flags = 0x00;
	spindleSpeed = GCODE_MACHINE_LOWEST_RPM;
	currentMachineState.overridesEnabled = true;

	GCODE_DEBUG("Machine is up");

	return true;
}

bool move_machine_line(double X, double Y, double Z, uint16_t A, uint16_t B, uint16_t C, TGCodeFeedMode feedMode, uint16_t F) {
	machineX = X;
	machineY = Y;
	machineZ = Z;
	machineA = A;
	machineB = B;
	machineC = C;
	GCODE_DEBUG("Linear move V(%4.2fmm, %4.2fmm, %4.2fmm, %3ddeg, %3ddeg, %3ddeg) at %4dmm/min", X, Y, Z, A, B, C, F);

	return true;
}

bool move_machine_arc(double X, double Y, double Z, double I, double J, double K, double R, bool ccw, TGCodePlaneMode plane, TGCodeFeedMode feedMode, uint16_t F) {
  if(isnan(R)) R = hypot(machineX - I, machineY - J);
  else if(isnan(I) && isnan(J)) {
    double d = hypot(machineX - X, machineY - Y);
    I = machineX + (X - machineX) / 2 + (ccw ? 1 : -1) * sqrt(R * R - d * d / 4) * (Y - machineY) / d;
    J = machineY + (Y - machineY) / 2 + (ccw ? -1 : 1) * sqrt(R * R - d * d / 4) * (X - machineX) / d;
  }
  GCODE_DEBUG("Circular move C(%4.2fmm, %4.2fmm, %4.2fmm) of radius %4.2fmm", I, J, K, R);

  double As = atan2(machineY - J, machineX - I);
  double Af = atan2(Y - J, X - I);
  double time = fabs(Af - As) * R / F; // time in same units as F
  double quanta = time / 100;
  double t = 0.0;

  while(t < time) {
   machineX = I + R * cos(As + (ccw ? 1 : -1) * ((F / R) * t));
   machineY = J + R * sin(As + (ccw ? 1 : -1) * ((F / R) * t));
   t += quanta;
   GCODE_DEBUG("%4.2fmm %4.2fmm", machineX, machineY);
  }
	(void) plane;

	return true;
}

bool move_machine_cycle(TGCodeCycleMode mode, double X, double Y, double Z, TGCodeRetractMode retract, uint16_t L, double P, double Q, double R, TGCodeFeedMode feedMode, uint16_t F) {
	GCODE_DEBUG("Canned cycle %d", mode);

	return true;
}

bool move_machine_home(TGCodeCycleMode mode, double X, double Y, double Z) {
	move_machine_line(X, Y, Z, machineA, machineB, machineC, GCODE_FEED_PERMINUTE, GCODE_MOVE_FEED);

	switch(mode) {
		case GCODE_CYCLE_HOME:
			GCODE_DEBUG("Home and recalibrate cycle for axes: %s%s%s", (X == machineX) ? "" : "X", (Y == machineY) ? "" : "Y", (Z == machineZ) ? "" : "Z");
			break;
		case GCODE_CYCLE_RETURN:
			GCODE_DEBUG("Return from reference point cycle for axes: %s%s%s", (X == machineX) ? "" : "X", (Y == machineY) ? "" : "Y", (Z == machineZ) ? "" : "Z");
			break;
		case GCODE_CYCLE_ZERO:
			GCODE_DEBUG("Go to zero cycle for axes: %s%s%s", (X == machineX) ? "" : "X", (Y == machineY) ? "" : "Y", (Z == machineZ) ? "" : "Z");
			break;
		default:
			return false;
	}

	return true;
}

bool start_spindle_machine(TGCodeSpindleMode direction) {
	if((currentMachineState.spindleCW && direction == GCODE_SPINDLE_CW) ||
		(currentMachineState.spindleCCW && direction == GCODE_SPINDLE_CCW)) return true;
	if(!(currentMachineState.spindleCW || currentMachineState.spindleCCW)) {
		if(direction == GCODE_SPINDLE_CW) currentMachineState.spindleCW = true;
		else currentMachineState.spindleCCW = true;
		GCODE_DEBUG("Spindle started %s at %5drpm", (direction == GCODE_SPINDLE_CW) ? "clockwise" : "counterclockwise",
					spindleSpeed);
	} else if(direction == GCODE_SPINDLE_STOP) {
		currentMachineState.spindleCW = currentMachineState.spindleCCW = false;
		GCODE_DEBUG("Spindle stopped");
	} else return false; /* Won't switch direction while running */

	return true;
}

bool set_spindle_speed_machine(uint32_t speed) {
	spindleSpeed = speed;

	if(currentMachineState.spindleCW || currentMachineState.spindleCCW) GCODE_DEBUG("Spindle speed set to %5drpm", spindleSpeed);

	return true;
}

bool orient_spindle_machine(uint16_t orientation) {
	GCODE_DEBUG("Oriented spindle at %3ddeg", orientation);

	return true;
}

void display_machine_message(char *message) {
	printf("MSG: %s\n", message);
}

bool block_delete_machine(void) {
	return true; /* Always on, for now */
}

uint16_t override_feed_machine(uint16_t feed) {
	if(currentMachineState.overridesEnabled) return feed * 0.90; /* Simulate 90% setting */
	else return feed;
}

uint32_t override_speed_machine(uint32_t speed) {
	if(currentMachineState.overridesEnabled) return speed * 0.90; /* Simulate 90% setting */
	else return speed;
}

bool preselect_tool_machine(uint8_t tool) {
	GCODE_DEBUG("Moving tool carousel to tool %d", tool);

	return tool;
}

bool change_tool_machine(uint8_t tool) {
	if(tool) {
		GCODE_DEBUG("Performing ATC to tool %d", tool);
	} else GCODE_DEBUG("Unloading spindle");

	return true;
}

bool start_coolant_machine(TGCodeCoolantMode mode) {
	switch(mode) {
		case GCODE_COOL_MIST:
			GCODE_DEBUG("Activating mist coolant");
			break;
		case GCODE_COOL_FLOOD:
			GCODE_DEBUG("Activating flood coolant");
			break;
		case GCODE_COOL_OFF_MF:
			GCODE_DEBUG("Turning flood and mist coolant off");
			break;
		case GCODE_COOL_SHOWER:
			GCODE_DEBUG("Activating washdown coolant");
			break;
		case GCODE_COOL_OFF_S:
			GCODE_DEBUG("Turning washdown coolant off");
			break;
	}

	return true;
}

bool enable_override_machine(TGCodeOverrideMode mode) {
	currentMachineState.overridesEnabled = (mode == GCODE_OVERRIDE_ON) ? true : false;

	GCODE_DEBUG("Feed and speed override switches %s", (mode == GCODE_OVERRIDE_ON ? "enabled" : "disabled"));

	return true;
}

bool select_probeinput_machine(TGCodeProbeInput input) {
	currentMachineState.probeSource = (input == GCODE_PROBE_TOOL) ? true : false;

	GCODE_DEBUG("Probe signal source set to %s", (input == GCODE_PROBE_PART) ? "probe tool" : "tool height sensor");

	return true;
}

bool select_probemode_machine(TGCodeProbeMode mode) {
	currentMachineState.probeMode = (mode == GCODE_PROBE_TWOTOUCH) ? true : false;

	GCODE_DEBUG("Probing mode set to %s stroke", (mode == GCODE_PROBE_ONETOUCH) ? "single" : "double");

	return true;
}

bool enable_mirror_machine(TGCodeMirrorMachine mode) {
	if(mode == GCODE_MIRROR_X) currentMachineState.mirrorX = true;
	else if(mode == GCODE_MIRROR_Y) currentMachineState.mirrorY = true;
	else {
		currentMachineState.mirrorX = false;
		currentMachineState.mirrorY = false;
	}

	GCODE_DEBUG("Machine mirroring %s%s%s", (mode == GCODE_MIRROR_OFF_M) ? "disabled" : "enabled for axes: ",
		currentMachineState.mirrorX ? "X" : "", currentMachineState.mirrorY ? "Y" : "");
	return true;
}

bool select_pathmode_machine(TGCodePathControl mode) {
	currentMachineState.exactStopCheck = (mode == GCODE_EXACTSTOPCHECK_ON);

	GCODE_DEBUG("Exact stop check (path control) %s", currentMachineState.exactStopCheck ? "on" : "off");

	return true;
}

bool done_machine(void) {
	GCODE_DEBUG("Machine shutdown");

	return true;
}

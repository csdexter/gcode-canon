/*
 ============================================================================
 Name        : gcode-machine.c
 Author      : Radu - Eosif Mihailescu
 Version     : 1.0 (2012-08-11)
 Copyright   : (C) 2012 Radu - Eosif Mihailescu <radu.mihailescu@linux360.ro>
 Description : G-Code Machine API and Implementation Code
 ============================================================================
 */

#include <math.h>
#include <stdbool.h>
#include <stdio.h>

#include "gcode-commons.h"
#include "gcode-machine.h"
#include "gcode-debugcon.h"
#include "gcode-parameters.h"
#include "gcode-tools.h"
#include "gcode-queue.h"
#include "gcode-math.h"


static double noMirrorX, noMirrorY;
static TGCodeOffsetSpec old, current, beforeHome;
static uint32_t spindleSpeed;
static TGCodeMachineState currentMachineState;
static bool stillRunning, servoPower;


double _adjust_feed(TGCodeFeedMode mode, double F, double toGo) {
  switch(mode) {
    case GCODE_FEED_INVTIME:
      /* Whole distance in 1/F minutes, i.e. move at (distance * F) mm/min */
      F *= toGo;
      break;
    case GCODE_FEED_PERREVOLUTION:
      /* F mm per revolution, i.e. move at (S * F) mm/min */
      F *= spindleSpeed;
      break;
    case GCODE_FEED_PERMINUTE:
      /* F mm/min, i.e. already in the right format */
      /* NOP */
      break;
  }

  return F;
}

bool init_machine(void *data) {
  current.X = current.Y = current.Z = noMirrorX = noMirrorY = beforeHome.X =
      beforeHome.Y = beforeHome.Z = old.X = old.Y = old.Z = 0.0;
  currentMachineState.flags = 0x00;
  set_spindle_speed_machine(GCODE_MACHINE_LOWEST_RPM);
  enable_override_machine(GCODE_OVERRIDE_ON);
  stillRunning = true;
  enable_power_machine(GCODE_SERVO_ON);
  set_parameter(GCODE_PARM_CURRENT_PALLET, 1);
  /* By default our home and zero positions are at (0, 0, 0) */
  set_parameter(GCODE_PARM_FIRST_HOME + GCODE_AXIS_X, current.X);
  set_parameter(GCODE_PARM_FIRST_HOME + GCODE_AXIS_Y, current.Y);
  set_parameter(GCODE_PARM_FIRST_HOME + GCODE_AXIS_Z, current.Z);
  set_parameter(GCODE_PARM_FIRST_ZERO + GCODE_AXIS_X, current.X);
  set_parameter(GCODE_PARM_FIRST_ZERO + GCODE_AXIS_Y, current.Y);
  set_parameter(GCODE_PARM_FIRST_ZERO + GCODE_AXIS_Z, current.Z);

  GCODE_DEBUG("Machine is up");

  return true;
}

bool move_machine_queue(void) {
  TGCodeMoveSpec movec;

  if(!queue_size() || !servoPower) return false;
  else {
    dequeue_move(&movec);
    current.X = movec.target.X;
    current.Y = movec.target.Y;
    current.Z = movec.target.Z;

    GCODE_MACHINE_POSITION(current);

    return true;
  }
}

bool move_machine_line(double X, double Y, double Z, TGCodeFeedMode feedMode,
    double F, TGCodeCompSpec radComp, TGCodeCornerMode corner) {
  TGCodeMoveSpec move;

  /* Check for and apply machine mirroring */
  X = mirroring_math(X, current.X, &noMirrorX, currentMachineState.mirrorX);
  Y = mirroring_math(Y, current.Y, &noMirrorY, currentMachineState.mirrorY);

  move.isArc = false;

  move.target.X = X;
  move.target.Y = Y;
  move.target.Z = Z;
  move.axesMoving.X = moving_axis_math(old.X, move.target.X);
  move.axesMoving.Y = moving_axis_math(old.Y, move.target.Y);
  move.axesMoving.Z = moving_axis_math(old.Z, move.target.Z);
  old = move.target;
  move.feedValue = _adjust_feed(feedMode, F, sqrt(pow(current.X - X, 2) +
                                                  pow(current.Y - Y, 2) +
                                                  pow(current.Z - Z, 2)));
  move.radComp = radComp;
  move.corner = corner;
  /* Fully initialize the struct, keeps bugs away ;-) */
  move.ccw = false;

  if(F == GCODE_MACHINE_FEED_TRAVERSE)
    GCODE_DEBUG("Traverse move to V(%4.2fmm, %4.2fmm, %4.2fmm)", X, Y, Z)
  else
    GCODE_DEBUG("Linear move to V(%4.2fmm, %4.2fmm, %4.2fmm) at %4.0fmm/min",
                X, Y, Z, move.feedValue);

  return enqueue_move(move);
}

bool move_machine_arc(double X, double Y, double Z, double I, double J,
    double K, double R, bool ccw, TGCodePlaneMode plane,
    TGCodeFeedMode feedMode, double F, TGCodeCompSpec radComp,
    TGCodeCornerMode corner) {
  bool theLongWay = false;
  TGCodeMoveSpec move;
  double arclen;

  /* Use the >180deg arc on user request */
  if(!isnan(R) && signbit(R)) {
    theLongWay = true;
    R *= -1;
  }

  switch(plane) {
    case GCODE_PLANE_XY:
      X = mirroring_math(X, current.X, &noMirrorX, currentMachineState.mirrorX);
      Y = mirroring_math(Y, current.Y, &noMirrorY, currentMachineState.mirrorY);
      if(currentMachineState.mirrorX ^ currentMachineState.mirrorY) ccw = !ccw;
      arclen = arc_math(X, Y, old.X, old.Y, &R, &I, &J, &K, ccw ^ theLongWay);
      if(Z != current.Z) arclen = hypot(arclen, current.Z - Z);
      break;
    case GCODE_PLANE_ZX:
      X = mirroring_math(X, current.X, &noMirrorX, currentMachineState.mirrorX);
      if(currentMachineState.mirrorX) ccw = !ccw;
      arclen = arc_math(Z, X, old.Z, old.X, &R, &K, &I, &J, ccw ^ theLongWay);
      if(Y != current.Y) arclen = hypot(arclen, current.Y - Y);
      break;
    case GCODE_PLANE_YZ:
      Y = mirroring_math(Y, current.Y, &noMirrorY, currentMachineState.mirrorY);
      if(currentMachineState.mirrorY) ccw = !ccw;
      arclen = arc_math(Y, Z, old.Y, old.Z, &R, &J, &K, &I, ccw ^ theLongWay);
      if(X != current.X) arclen = hypot(arclen, current.X - X);
      break;
  }

  move.isArc = true;
  move.center.X = old.X + I;
  move.center.Y = old.Y + J;
  move.center.Z = old.Z + K;
  move.ccw = ccw;
  move.target.X = X;
  move.target.Y = Y;
  move.target.Z = Z;
  move.axesMoving.X = moving_axis_math(old.X, move.target.X);
  move.axesMoving.Y = moving_axis_math(old.Y, move.target.Y);
  move.axesMoving.Z = moving_axis_math(old.Z, move.target.Z);
  old = move.target;
  move.feedValue = _adjust_feed(feedMode, F, arclen);
  move.radComp = radComp;
  move.corner = corner;

  GCODE_DEBUG("Circular move around C(%4.2fmm, %4.2fmm, %4.2fmm) of radius %4.2fmm in plane %s %s ending at V(%4.2fmm, %4.2fmm, %4.2fmm) at %4.0fmm/min",
              move.center.X, move.center.Y, move.center.Z, R,
              (plane == GCODE_PLANE_XY ? "XY" :
                  (plane == GCODE_PLANE_ZX ? "ZX" : "YZ")),
              (ccw ? "counter-clockwise" : "clockwise"), X, Y, Z, move.feedValue);

  return enqueue_move(move);
}

bool move_machine_home(TGCodeCycleMode mode, double X, double Y, double Z) {
  TGCodeCompSpec noComp;

  if(!servoPower) return false;

  switch(mode) {
    case GCODE_CYCLE_HOME:
      GCODE_DEBUG("Home and recalibrate cycle for axes: %s%s%s",
                  (X == current.X) ? "" : "X", (Y == current.Y) ? "" : "Y",
                  (Z == current.Z) ? "" : "Z");
      beforeHome = current;
      break;
    case GCODE_CYCLE_RETURN:
      GCODE_DEBUG("Return from reference point cycle for axes: %s%s%s",
                  (X == current.X) ? "" : "X", (Y == current.Y) ? "" : "Y",
                  (Z == current.Z) ? "" : "Z");
      break;
    case GCODE_CYCLE_ZERO:
      GCODE_DEBUG("Go to zero cycle for axes: %s%s%s",
                  (X == current.X) ? "" : "X",
                  (Y == current.Y) ? "" : "Y",
                  (Z == current.Z) ? "" : "Z");
      beforeHome = current;
      break;
    default:
      return false;
  }
  noComp.mode = GCODE_COMP_RAD_OFF;
  /* Fully initialize the struct, keeps bugs away ;-) */
  noComp.offset = 0.0;

  move_machine_line(X, Y, Z, GCODE_FEED_PERMINUTE, GCODE_MACHINE_FEED_TRAVERSE,
                    noComp, GCODE_CORNER_CHAMFER);

  switch(mode) {
    case GCODE_CYCLE_HOME:
      //TODO: obey per axis home speed
      move_machine_line(fetch_parameter(GCODE_PARM_FIRST_HOME + GCODE_AXIS_X),
                        fetch_parameter(GCODE_PARM_FIRST_HOME + GCODE_AXIS_Y),
                        fetch_parameter(GCODE_PARM_FIRST_HOME + GCODE_AXIS_Z),
                        GCODE_FEED_PERMINUTE,
                        fetch_parameter(GCODE_PARM_FEED_HOME_X), noComp,
                        GCODE_CORNER_CHAMFER);
      break;
    case GCODE_CYCLE_RETURN:
      move_machine_line(beforeHome.X, beforeHome.Y, beforeHome.Z,
                        GCODE_FEED_PERMINUTE, GCODE_MACHINE_FEED_TRAVERSE,
                        noComp, GCODE_CORNER_CHAMFER);
      break;
    case GCODE_CYCLE_ZERO:
      move_machine_line(fetch_parameter(GCODE_PARM_FIRST_ZERO + GCODE_AXIS_X),
                        fetch_parameter(GCODE_PARM_FIRST_ZERO + GCODE_AXIS_Y),
                        fetch_parameter(GCODE_PARM_FIRST_ZERO + GCODE_AXIS_Z),
                        GCODE_FEED_PERMINUTE, GCODE_MACHINE_FEED_TRAVERSE,
                        noComp, GCODE_CORNER_CHAMFER);
      break;
    default:
      return false;
  }

  return true;
}

bool move_machine_aux(TGCodeAuxiliaryMachine mode, uint32_t P) {
  if(!servoPower) return false;

  switch(mode) {
    case GCODE_SPINDLE_ORIENTATION:
      if(P == UINT32_MAX) orient_spindle_machine(0);
      else orient_spindle_machine(P);
      break;
    case GCODE_INDEXER_STEP:
      GCODE_DEBUG("Would advance indexer %d steps", (P == UINT32_MAX ? 1 : P));
      break;
    case GCODE_RETRACT_Z:
      //TODO: this is wrong, should be Zmax instead or thereabouts
      current.Z = 0;
      GCODE_DEBUG("Z-axis retracted/parked");
      break;
    case GCODE_APC_1:
    case GCODE_APC_2:
      set_parameter(GCODE_PARM_CURRENT_PALLET, mode - GCODE_APC_1 + 1);
      GCODE_DEBUG("Performed APC to pallet %d", mode - GCODE_APC_1 + 1);
      break;
    case GCODE_APC_SWAP:
      set_parameter(GCODE_PARM_CURRENT_PALLET,
          (fetch_parameter(GCODE_PARM_CURRENT_PALLET) == 1 ? 2 : 1));
      GCODE_DEBUG("Would swap pallets")
      break;
    default:
      return false;
  }

  return true;
}

bool start_spindle_machine(TGCodeSpindleMode direction) {
  if(!servoPower) return false;

  if((currentMachineState.spindleCW && direction == GCODE_SPINDLE_CW) ||
      (currentMachineState.spindleCCW && direction == GCODE_SPINDLE_CCW))
    return true;
  if(direction == GCODE_SPINDLE_STOP) {
    currentMachineState.spindleCW = currentMachineState.spindleCCW = false;
    GCODE_DEBUG("Spindle stopped");
  } else if(!(currentMachineState.spindleCW ||
      currentMachineState.spindleCCW)) {
    if(direction == GCODE_SPINDLE_CW) currentMachineState.spindleCW = true;
    else currentMachineState.spindleCCW = true;
    GCODE_DEBUG("Spindle started %s at %5drpm",
        (direction == GCODE_SPINDLE_CW) ? "clockwise" : "counterclockwise",
        spindleSpeed);
  } else return false; /* Won't switch direction while running */

  return true;
}

bool set_spindle_speed_machine(uint32_t speed) {
  spindleSpeed = speed;

  if(currentMachineState.spindleCW || currentMachineState.spindleCCW)
    GCODE_DEBUG("Spindle now rotating at %5drpm", spindleSpeed)
  else GCODE_DEBUG("Spindle speed preset at %5drpm", spindleSpeed);

  return true;
}

bool orient_spindle_machine(uint16_t orientation) {
  if(!servoPower) return false;

  if(currentMachineState.spindleCW || currentMachineState.spindleCCW)
    GCODE_DEBUG("Spindle currently running, cannot orient!")
  else GCODE_DEBUG("Oriented spindle at %3ddeg", orientation);

  return true;
}

void display_machine_message(char *message) {
  printf("MSG: %s\n", message);
}

bool block_delete_machine(void) {
  return true; /* Always on, for now */
}

bool optional_stop_machine(void) {
  return false; /* Always off for now */
}

uint16_t override_feed_machine(uint16_t feed) {
  /* Simulate 90% setting */
  if(currentMachineState.overridesEnabled) return feed * 0.90;
  else return feed;
}

uint32_t override_speed_machine(uint32_t speed) {
  /* Simulate 90% setting */
  if(currentMachineState.overridesEnabled) return speed * 0.90;
  else return speed;
}

bool preselect_tool_machine(uint8_t tool) {
  if(!servoPower) return false;

  GCODE_DEBUG("Moving tool carousel to tool %d", tool);

  return tool;
}

bool change_tool_machine(uint8_t tool) {
  if(!servoPower) return false;

  if(tool) {
    GCODE_DEBUG("Performing ATC to tool %d", tool)
    fetch_tool(tool);
    set_parameter(GCODE_PARM_CURRENT_TOOL, tool);
  }
  else GCODE_DEBUG("Unloading spindle");

  return true;
}

bool start_coolant_machine(TGCodeCoolantMode mode) {
  if(!servoPower) return false;

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
  currentMachineState.overridesEnabled = (mode == GCODE_OVERRIDE_ON);
  set_parameter(
      GCODE_PARM_BITFIELD1,
      ((uint8_t)fetch_parameter(GCODE_PARM_BITFIELD1) &
          ~GCODE_MACHINE_PF_OVERRIDES) |
      (currentMachineState.overridesEnabled ? GCODE_MACHINE_PF_OVERRIDES : 0x00));

  GCODE_DEBUG("Feed and speed override switches %s",
              (currentMachineState.overridesEnabled ? "enabled" : "disabled"));

  return true;
}

bool select_probeinput_machine(TGCodeProbeInput input) {
  currentMachineState.probeSource = (input == GCODE_PROBE_TOOL);

  GCODE_DEBUG("Probe signal source set to %s",
              currentMachineState.probeSource ? "probe tool" : "tool height sensor");

  return true;
}

bool select_probemode_machine(TGCodeProbeMode mode) {
  currentMachineState.probeMode = (mode == GCODE_PROBE_TWOTOUCH);

  GCODE_DEBUG("Probing mode set to %s stroke",
              currentMachineState.probeMode ? "single" : "double");

  return true;
}

bool enable_mirror_machine(TGCodeMirrorMachine mode) {
  if(mode == GCODE_MIRROR_X) currentMachineState.mirrorX = true;
  else if(mode == GCODE_MIRROR_Y) currentMachineState.mirrorY = true;
  else {
    currentMachineState.mirrorX = false;
    currentMachineState.mirrorY = false;
  }

  noMirrorX = current.X;
  noMirrorY = current.Y;

  set_parameter(
      GCODE_PARM_BITFIELD2,
      ((uint8_t)fetch_parameter(GCODE_PARM_BITFIELD2) &
          ~(GCODE_MACHINE_PF_MIRROR_X | GCODE_MACHINE_PF_MIRROR_Y)) |
      (currentMachineState.mirrorX ? GCODE_MACHINE_PF_MIRROR_X : 0x00) |
      (currentMachineState.mirrorY ? GCODE_MACHINE_PF_MIRROR_Y : 0x00));

  GCODE_DEBUG("Machine mirroring %s%s%s",
              (mode == GCODE_MIRROR_OFF_M) ? "disabled" : "enabled for axis(es): ",
              currentMachineState.mirrorX ? "X" : "",
              currentMachineState.mirrorY ? "Y" : "");
  return true;
}

bool select_pathmode_machine(TGCodePathControl mode) {
  currentMachineState.exactStopCheck = (mode == GCODE_EXACTSTOPCHECK_ON);
  set_parameter(
      GCODE_PARM_BITFIELD1,
      ((uint8_t)fetch_parameter(GCODE_PARM_BITFIELD1) &
          ~GCODE_MACHINE_PF_EXACTSTOP) |
      (currentMachineState.exactStopCheck ? GCODE_MACHINE_PF_EXACTSTOP : 0x00));

  GCODE_DEBUG("Exact stop check (path control) %s",
              currentMachineState.exactStopCheck ? "on" : "off");

  return true;
}

bool do_stop_machine(TGCodeStopMode mode) {
  switch(mode) {
    case GCODE_STOP_E:
      display_machine_message("STA: Machine in E-Stop");
      break;
    case GCODE_STOP_COMPULSORY_AS_RETURNED: /* Returned as 100 by have_gcode_word() */
      display_machine_message("STA: Machine in compulsory stop");
      break;
    case GCODE_STOP_OPTIONAL:
      if(optional_stop_machine())
        display_machine_message("STA: Machine in optional stop");
      else return false;
      break;
    default:
      break;
  }
  GCODE_DEBUG("Machine stopped, send EOF to abort or newline to continue ...");
  if(fgetc(stdin) == EOF) {
    GCODE_DEBUG("User-requested abort, exiting ...")
    stillRunning = false;
  }
  return true;
}

bool machine_running(void) {
  return stillRunning;
}

bool enable_power_machine(TGCodeStopMode mode) {
  switch(mode) {
    case GCODE_SERVO_ON:
      servoPower = true;
      display_machine_message("WAR: Machine servos activated!");
      break;
    case GCODE_SERVO_OFF:
      servoPower = false;
      display_machine_message("STA: Machine servos inactive");
      break;
    default:
      break;
  }

  return true;
}

bool done_machine(void) {
  GCODE_DEBUG("Machine shutdown");

  return true;
}

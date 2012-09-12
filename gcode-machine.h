/*
 * gcode-machine.h
 *
 *  Created on: Aug 11, 2012
 *      Author: csdexter
 */

#ifndef GCODE_MACHINE_H_
#define GCODE_MACHINE_H_

#include "gcode-commons.h"
#include "gcode-state.h"


#define GCODE_MACHINE_LOWEST_RPM 60UL /* 1 revolution per second, fast enough to catch attention and slow enough to be safe */
#define GCODE_MACHINE_NO_TOOL 0
#define GCODE_MACHINE_FEED_TRAVERSE 0xFFFFU /* 1092.25mm/sec, none of the hardware we're targeting is that fast so safe to use as flag */

typedef union {
	uint8_t flags;
	struct {
		uint8_t probeSource:1;
		uint8_t probeMode:1;
		uint8_t mirrorX:1;
		uint8_t mirrorY:1;
		uint8_t overridesEnabled:1;
		uint8_t spindleCW:1;
		uint8_t spindleCCW:1;
		uint8_t exactStopCheck:1;
	};
} TGCodeMachineState;

bool init_machine(void *data);
/* Move to X,Y,Z-A,B,C at speed F. All axes move simultaneously for linear interpolation */
bool move_machine_line(double X, double Y, double Z, uint16_t A, uint16_t B, uint16_t C, TGCodeFeedMode feedMode, uint16_t F);
/* Move to X,Y,Z following an arc with the center at I,J,K, radius R and running
 * CCW if ccw is true, CW otherwise. Usually, one of I,J,K or R will be NaN so
 * the other will be extrapolated. CW/CCW is decided looking towards the
 * negative end of the axis perpendicular on plane */
bool move_machine_arc(double X, double Y, double Z, double I, double J, double K, double R, bool ccw, TGCodePlaneMode plane, TGCodeFeedMode feedMode, uint16_t F);
/* Moves the machine according to the canned cycle defined by mode */
bool move_machine_cycle(TGCodeCycleMode mode, double X, double Y, double Z, TGCodeRetractMode retract, uint16_t L, double P, double Q, double R, TGCodeFeedMode feedMode, uint16_t F);
/* Executes either home & recalibrate, go to zero or return from zero going through the point specified if any */
bool move_machine_home(TGCodeCycleMode mode, double X, double Y, double Z);
/* Start the spindle in the indicated direction or stop it. If there was no previous call to set_spindle_speed_machine(),
 * spindle will run at the lowest achievable speed. Returns false if spindle is already started. */
bool start_spindle_machine(TGCodeSpindleMode direction);
/* Run the spindle at speed rpm */
bool set_spindle_speed_machine(uint32_t speed);
/* Position the spindle oriented at orientation degrees */
bool orient_spindle_machine(uint16_t orientation);
/* Display operator messages */
void display_machine_message(char *message);
/* Returns true if the "Skip Deleted Blocks" switch is on */
bool block_delete_machine(void);
/* Returns feed reduced by the amount set on the Feed Rate Override control or feed if same is disabled */
uint16_t override_feed_machine(uint16_t feed);
/* Returns speed reduced by the amount set on the Spindle Speed Override control or speed if same is disabled */
uint32_t override_speed_machine(uint32_t speed);
/* Turns the carousel/scrolls the magazine to given tool slot in preparation for ATC. On machines with a fixed tool store,
 * opens the given compartment/slot in preparation for ATC. On machines without any tool store movement, does nothing */
bool preselect_tool_machine(uint8_t tool);
/* Performs ATC to given tool. If called with zero, unloads current tool in spindle if any */
bool change_tool_machine(uint8_t tool);
/* Starts or stops various kinds of coolant based on mode */
bool start_coolant_machine(TGCodeCoolantMode mode);
/* Enables or disables the feed & speed overrides */
bool enable_override_machine(TGCodeOverrideMode mode);
/* Selects probe signal input from either the tool height sensor on the machine bed or the spindle-mounted probe */
bool select_probeinput_machine(TGCodeProbeInput input);
/* Selects probe cycle type: single or double touch */
bool select_probemode_machine(TGCodeProbeMode mode);
/* Enables full axis-mirroring (inversion) */
bool enable_mirror_machine(TGCodeMirrorMachine mode);
/* Selects path control mode (exact stop check) */
bool select_pathmode_machine(TGCodePathControl mode);
bool done_machine(void);

#endif /* GCODE_MACHINE_H_ */

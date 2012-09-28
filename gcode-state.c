/*
 * gcode-state.c
 *
 *  Created on: Aug 12, 2012
 *      Author: csdexter
 */

#include <limits.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gcode-commons.h"
#include "gcode-state.h"
#include "gcode-debugcon.h"
#include "gcode-input.h"
#include "gcode-parameters.h"
#include "gcode-machine.h"
#include "gcode-tools.h"
#include "gcode-math.h"
#include "gcode-stacks.h"

static TGCodeWordCache parseCache;

const TGCodeState defaultGCodeState = {
  GCODE_FEED_PERMINUTE,
  {
    GCODE_PLANE_XY,
    GCODE_UNITS_METRIC,
    {
      GCODE_COMP_RAD_OFF,
      0.0
    },
    {
      GCODE_COMP_LEN_OFF,
      0.0
    },
    GCODE_WCS_1,
    {
      GCODE_MIRROR_OFF_S,
      0.0,
      0.0,
      0.0
    },
    {
      GCODE_ROTATION_OFF,
      0.0,
      0.0,
      0
    },
    GCODE_ABSOLUTE,
    GCODE_CARTESIAN,
    {
      GCODE_SCALING_OFF,
      0.0,
      0.0,
      0.0,
      0.0,
      0.0,
      0.0,
    },
    {
      0.0,
      0.0,
      0.0
    },
    0.0,
    0.0,
    0.0,
    0.0
  },
  GCODE_RETRACT_LAST,
  OFF,
  OFF,
  GCODE_EXACTSTOPCHECK_OFF,
  false,
  GCODE_CYCLE_CANCEL,
  false,
  false,
  false,
  0.0,
  0.0,
  0.0,
  0.0,
  0.0,
  0.0,
  0,
  0,
  0
};

static TGCodeState currentGCodeState;

bool init_gcode_state(void *data) {
  currentGCodeState = defaultGCodeState;

  GCODE_DEBUG("G-Code state machine up, defaults loaded");

  return true;
}

static TGCodeMotionMode _map_move_to_motion(TGCodeMoveMode mode, bool *ccw) {
  switch(mode) {
    case GCODE_MOVE_RAPID + 100: /* Returned as 100 by have_gcode_word() */
      return RAPID;
    case GCODE_MOVE_FEED:
      return LINEAR;
    case GCODE_MODE_ARC_CW:
    case GCODE_MODE_CIRCLE_CW:
      *ccw = false;
      return ARC;
      break;
    case GCODE_MODE_ARC_CCW:
    case GCODE_MODE_CIRCLE_CCW:
      *ccw = true;
      return ARC;
      break;
    default:
      return OFF;
  }
}

bool update_gcode_state(char *line) {
  uint8_t arg;

  parseCache.line = line;
  parseCache.word = ' '; /* because space is not a G-Code word and all spaces have been stripped from line */
  parseCache.at = NULL;

  if((arg = have_gcode_word('G', 2, GCODE_FEED_INVTIME, GCODE_FEED_PERMINUTE))) currentGCodeState.feedMode = arg;
  // We have integer precision for F, but people would write it as a real.
  if(have_gcode_word('F', 0)) currentGCodeState.F = override_feed_machine(get_gcode_word_real('F'));
  if(have_gcode_word('S', 0)) set_spindle_speed_machine(override_speed_machine(get_gcode_word_integer('S')));
  if(have_gcode_word('T', 0)) {
    currentGCodeState.T = get_gcode_word_integer('T');
    preselect_tool_machine(currentGCodeState.T);
  }
  if(have_gcode_word('M', 1, 6)) change_tool_machine(currentGCodeState.T);
  if(have_gcode_word('M', 1, 52)) change_tool_machine(GCODE_MACHINE_NO_TOOL);
  if((arg = have_gcode_word('M', 2, GCODE_PROBE_PART, GCODE_PROBE_TOOL))) select_probeinput_machine(arg);
  if((arg = have_gcode_word('M', 2, GCODE_PROBE_ONETOUCH, GCODE_PROBE_TWOTOUCH))) select_probemode_machine(arg);
  if((arg = have_gcode_word('M', 3, GCODE_SPINDLE_CW, GCODE_SPINDLE_CW, GCODE_SPINDLE_STOP))) start_spindle_machine(arg);
  if((arg = have_gcode_word('M', 5, GCODE_COOL_MIST, GCODE_COOL_FLOOD, GCODE_COOL_OFF_MF, GCODE_COOL_SHOWER, GCODE_COOL_OFF_S))) start_coolant_machine(arg);
  if((arg = have_gcode_word('M', 2, GCODE_COOLSPIN_CW, GCODE_COOLSPIN_CCW))) {
    start_coolant_machine(GCODE_COOL_FLOOD);
    start_spindle_machine((arg == GCODE_COOLSPIN_CW) ? GCODE_SPINDLE_CW : GCODE_SPINDLE_CCW);
  }
  if((arg = have_gcode_word('M', 2, GCODE_OVERRIDE_ON, GCODE_OVERRIDE_OFF))) enable_override_machine(arg);
  if(have_gcode_word('G', 1, 4)) GCODE_DEBUG("Would dwell for %4.2f seconds.", get_gcode_word_real('P'));
  if((arg = have_gcode_word('G', 3, GCODE_PLANE_XY, GCODE_PLANE_ZX, GCODE_PLANE_YZ))) currentGCodeState.system.plane = arg;
  if((arg = have_gcode_word('G', 2, GCODE_UNITS_INCH, GCODE_UNITS_METRIC))) currentGCodeState.system.units = arg;
  if((arg = have_gcode_word('G', GCODE_COMP_RAD_OFF, GCODE_COMP_RAD_L, GCODE_COMP_RAD_R))) {
    currentGCodeState.system.radComp.mode = arg;
    if(arg != GCODE_COMP_RAD_OFF) {
      if(have_gcode_word('D', 0)) currentGCodeState.system.radComp.offset = radiusof_tool(get_gcode_word_integer('D'));
      else currentGCodeState.system.radComp.offset = radiusof_tool(currentGCodeState.T);
    }
  }
  if((arg = have_gcode_word('G', GCODE_COMP_LEN_OFF, GCODE_COMP_LEN_N, GCODE_COMP_LEN_P))) {
    currentGCodeState.system.lenComp.mode = arg;
    if(arg != GCODE_COMP_LEN_OFF) {
      if(have_gcode_word('H', 0)) currentGCodeState.system.lenComp.offset = lengthof_tool(get_gcode_word_integer('H'));
      else currentGCodeState.system.lenComp.offset = lengthof_tool(currentGCodeState.T);
    }
  }
  if((arg = have_gcode_word('G', GCODE_WCS_1, GCODE_WCS_2, GCODE_WCS_3, GCODE_WCS_4, GCODE_WCS_5, GCODE_WCS_6))) currentGCodeState.system.current = arg;
  if((arg = have_gcode_word('M', 3, GCODE_MIRROR_X, GCODE_MIRROR_Y, GCODE_MIRROR_OFF_M))) enable_mirror_machine(arg);
  if((arg = have_gcode_word('G', 2, GCODE_MIRROR_ON, GCODE_MIRROR_OFF_S))) {
    currentGCodeState.system.mirror.mode = arg;
    currentGCodeState.system.mirror.X = get_gcode_word_real('X');
    currentGCodeState.system.mirror.Y = get_gcode_word_real('Y');
    currentGCodeState.system.mirror.Z = get_gcode_word_real('Z');
    currentGCodeState.axisWordsConsumed = true;
  }
  if((arg = have_gcode_word('G', 2, GCODE_ROTATION_ON, GCODE_ROTATION_OFF))) {
    currentGCodeState.system.rotation.mode = arg;
    currentGCodeState.system.rotation.X = get_gcode_word_real('X');
    currentGCodeState.system.rotation.Y = get_gcode_word_real('Y');
    currentGCodeState.system.rotation.R = get_gcode_word_integer('R');
    currentGCodeState.axisWordsConsumed = true;
  }
  if((arg = have_gcode_word('G', 2, GCODE_EXACTSTOPCHECK_ON, GCODE_EXACTSTOPCHECK_OFF))) {
    currentGCodeState.oldPathMode = arg;
    select_pathmode_machine(currentGCodeState.oldPathMode);
  }
  if(have_gcode_word('G', 1, 9)) {
    currentGCodeState.nonModalPathMode = true;
    select_pathmode_machine(GCODE_EXACTSTOPCHECK_ON);
  }
  if((arg = have_gcode_word('G', 2, GCODE_ABSOLUTE, GCODE_RELATIVE))) currentGCodeState.system.absolute = arg;
  if((arg = have_gcode_word('G', 2, GCODE_CARTESIAN, GCODE_POLAR))) currentGCodeState.system.cartesian = arg;
  if((arg = have_gcode_word('G', 2, GCODE_SCALING_ON, GCODE_SCALING_OFF))) {
    currentGCodeState.system.scaling.mode = arg;
    currentGCodeState.system.scaling.X = get_gcode_word_real('X');
    currentGCodeState.system.scaling.Y = get_gcode_word_real('Y');
    currentGCodeState.system.scaling.Z = get_gcode_word_real('Z');
    if(!(isnan(currentGCodeState.system.scaling.X) && isnan(currentGCodeState.system.scaling.Y) && isnan(currentGCodeState.system.scaling.Z))) currentGCodeState.axisWordsConsumed = true;
    currentGCodeState.system.scaling.I = get_gcode_word_real('P');
    if(isnan(currentGCodeState.system.scaling.I)) { /* No P word */
      currentGCodeState.system.scaling.I = get_gcode_word_real('I');
      currentGCodeState.system.scaling.J = get_gcode_word_real('J');
      currentGCodeState.system.scaling.K = get_gcode_word_real('K');
    } else currentGCodeState.system.scaling.J = currentGCodeState.system.scaling.K = currentGCodeState.system.scaling.I;
  }
  if((arg = have_gcode_word('G', 2, GCODE_RETRACT_LAST, GCODE_RETRACT_R))) currentGCodeState.retractMode = arg;
  if((arg = have_gcode_word('G', 4, GCODE_CYCLE_HOME, GCODE_CYCLE_RETURN, GCODE_CYCLE_ZERO, GCODE_CYCLE_CANCEL))) {
    currentGCodeState.motionMode = OFF;
    if(arg != GCODE_CYCLE_CANCEL) {
      do_WCS_move_math(&currentGCodeState.system, get_gcode_word_real('X'), get_gcode_word_real('Y'), get_gcode_word_real('Z'));
      move_machine_home(arg, currentGCodeState.system.X, currentGCodeState.system.Y, currentGCodeState.system.Z);
      currentGCodeState.axisWordsConsumed = true;
    }
  }
  if((arg = have_gcode_word('G', 2, GCODE_DATA_ON, GCODE_DATA_OFF))) {
    if(arg == GCODE_DATA_ON) {
      currentGCodeState.oldMotionMode = currentGCodeState.motionMode;
      currentGCodeState.motionMode = STORE;
    } else currentGCodeState.motionMode = currentGCodeState.oldMotionMode;
  }
  if(have_gcode_word('G', 1, 92)) {
    currentGCodeState.system.offset.X = get_gcode_word_real('X');
    currentGCodeState.system.offset.Y = get_gcode_word_real('Y');
    currentGCodeState.system.offset.Z = get_gcode_word_real('Z');
    currentGCodeState.axisWordsConsumed = true;
  }
  if((arg = have_gcode_word('G', 6, GCODE_MOVE_RAPID, GCODE_MOVE_FEED, GCODE_MODE_ARC_CW, GCODE_MODE_ARC_CCW, GCODE_MODE_CIRCLE_CW, GCODE_MODE_CIRCLE_CCW))) {
    currentGCodeState.motionMode = _map_move_to_motion(arg, &currentGCodeState.ccw);
    if(!(arg == GCODE_MOVE_RAPID + 100 || arg == GCODE_MOVE_FEED)) { /* It's an arc, fetch I,J,K,R now for later */
      currentGCodeState.I = get_gcode_word_real('I');
      currentGCodeState.J = get_gcode_word_real('J');
      currentGCodeState.K = get_gcode_word_real('K');
      currentGCodeState.R = get_gcode_word_real('R');
    }
  }
  if((arg = have_gcode_word('G', 13, GCODE_CYCLE_PROBE_IN, GCODE_CYCLE_PROBE_OUT, GCODE_CYCLE_DRILL_PP, GCODE_CYCLE_TAP_LH, GCODE_CYCLE_DRILL_ND,  GCODE_CYCLE_DRILL_WD, GCODE_CYCLE_DRILL_PF, GCODE_CYCLE_TAP_RH, GCODE_CYCLE_BORING_ND_NS, GCODE_CYCLE_BORING_WD_WS, GCODE_CYCLE_BORING_BACK, GCODE_CYCLE_BORING_MANUAL, GCODE_CYCLE_BORING_WD_NS))) {
    currentGCodeState.motionMode = CYCLE;
    currentGCodeState.cycle = arg;
    if(!(arg == GCODE_CYCLE_PROBE_IN || arg == GCODE_CYCLE_PROBE_OUT)) { /* It's a canned cycle, fetch I,J,L,P,Q,R now for later */
      currentGCodeState.I = get_gcode_word_real('I');
      currentGCodeState.J = get_gcode_word_real('J');
      currentGCodeState.L = get_gcode_word_integer('L');
      currentGCodeState.P = get_gcode_word_real('P');
      currentGCodeState.Q = get_gcode_word_real('Q');
      currentGCodeState.R = get_gcode_word_real('R');
    }
  }
  if((arg = have_gcode_word('M', 3, GCODE_SPINDLE_ORIENTATION, GCODE_INDEXER_STEP, GCODE_RETRACT_Z))) move_machine_aux(arg, get_gcode_word_integer('P'));
  if(have_gcode_word('G', 1, 65)) {
    currentGCodeState.motionMode = MACRO;
    currentGCodeState.macroCall = true;
  }
  /* Sequence point: we read the axis words here and do the WCS math. All axis-word-eating commands MUST be above this line and set axisWordsConsumed to true.
   * Everything below this line will use whatever results from pushing the axis word arguments through the current coordinate transformation. */
  if(!currentGCodeState.axisWordsConsumed) {
    /* if a cycle, then X,Y,Z,R must be translated. If absolute, translate as for normal moves; if incremental, translate X,Y as absolute and Z,R as incremental */
    switch(currentGCodeState.motionMode) {
      case CYCLE:
        do_WCS_cycle_math(&currentGCodeState.system, get_gcode_word_real('X'), get_gcode_word_real('Y'), get_gcode_word_real('Z'), currentGCodeState.R);
        break;
      case STORE:
        switch(get_gcode_word_integer('L')) {
          case 2: {
            uint16_t wcs = (get_gcode_word_integer('P') - 1) * GCODE_PARM_WCS_SIZE;
            double offset;

            offset = get_gcode_word_real('X');
            update_parameter(GCODE_PARM_FIRST_WCS + wcs + 0, (isnan(offset) ? currentGCodeState.system.X : offset));
            offset = get_gcode_word_real('Y');
            update_parameter(GCODE_PARM_FIRST_WCS + wcs + 1, (isnan(offset) ? currentGCodeState.system.Y : offset));
            offset = get_gcode_word_real('Z');
            update_parameter(GCODE_PARM_FIRST_WCS + wcs + 2, (isnan(offset) ? currentGCodeState.system.Z : offset));
            commit_parameters();
          } break;
          case 3: {
            uint8_t tool = get_gcode_word_integer('P');

            if(have_gcode_word('H', 0)) update_parameter(GCODE_TOOL_LEN_BASE + tool, get_gcode_word_real('H'));
            if(have_gcode_word('D', 0)) update_parameter(GCODE_TOOL_DIAM_BASE + tool, get_gcode_word_real('D'));
            commit_parameters();
          } break;
          default:
            break;
        }
        break;
      case MACRO:
        stacks_push_parameters();
        update_parameter(1, get_gcode_word_real('A'));
        update_parameter(2, get_gcode_word_real('B'));
        update_parameter(3, get_gcode_word_real('C'));
        update_parameter(4, get_gcode_word_real('I'));
        update_parameter(5, get_gcode_word_real('J'));
        update_parameter(6, get_gcode_word_real('K'));
        update_parameter(7, get_gcode_word_real('D'));
        update_parameter(11, get_gcode_word_real('H'));
        update_parameter(12, get_gcode_word_real('L'));
        update_parameter(16, get_gcode_word_real('P'));
        update_parameter(17, get_gcode_word_real('Q'));
        update_parameter(18, get_gcode_word_real('R'));
        update_parameter(21, get_gcode_word_real('U'));
        update_parameter(22, get_gcode_word_real('V'));
        update_parameter(23, get_gcode_word_real('W'));
        update_parameter(24, get_gcode_word_real('X'));
        update_parameter(25, get_gcode_word_real('Y'));
        update_parameter(26, get_gcode_word_real('Z'));
        commit_parameters();
        break;
      case OFF:
        break;
      default:
        do_WCS_move_math(&currentGCodeState.system, get_gcode_word_real('X'), get_gcode_word_real('Y'), get_gcode_word_real('Z'));
        break;
    }
  } else currentGCodeState.axisWordsConsumed = false;
  switch(currentGCodeState.motionMode) {
    case RAPID:
      move_machine_line(currentGCodeState.system.X, currentGCodeState.system.Y, currentGCodeState.system.Z, GCODE_FEED_PERMINUTE, GCODE_MACHINE_FEED_TRAVERSE);
      break;
    case LINEAR:
      move_machine_line(currentGCodeState.system.X, currentGCodeState.system.Y, currentGCodeState.system.Z, currentGCodeState.feedMode, currentGCodeState.F);
      break;
    case ARC:
      //TODO: investigate whether to treat full-circle feed as a cycle and add repeat count
      move_machine_arc(currentGCodeState.system.X, currentGCodeState.system.Y, currentGCodeState.system.Z, currentGCodeState.I, currentGCodeState.J, currentGCodeState.K, currentGCodeState.R, currentGCodeState.ccw, currentGCodeState.system.plane, currentGCodeState.feedMode, currentGCodeState.F);
      break;
    case CYCLE:
      move_machine_cycle(currentGCodeState.cycle, currentGCodeState.system.X, currentGCodeState.system.Y, currentGCodeState.system.Z, currentGCodeState.retractMode, currentGCodeState.L, currentGCodeState.P, currentGCodeState.Q, currentGCodeState.R, currentGCodeState.feedMode, currentGCodeState.F);
      break;
    default:
      /*NOP*/;
      break;
  }
  if(currentGCodeState.nonModalPathMode && currentGCodeState.motionMode != OFF) {
    select_pathmode_machine(currentGCodeState.oldPathMode);
    currentGCodeState.nonModalPathMode = false;
  }
  if((arg = have_gcode_word('M', 10, GCODE_STOP_COMPULSORY, GCODE_STOP_OPTIONAL, GCODE_STOP_END, GCODE_SERVO_ON, GCODE_SERVO_OFF, GCODE_STOP_RESET, GCODE_STOP_E, GCODE_APC_1, GCODE_APC_2, GCODE_APC_SWAP)))
    /* Machine stop comes here */;
  /* Read, update and commit parameters comes here */
  if(have_gcode_word('M', 1, 47)) rewind_input();
  if(have_gcode_word('M', 1, 98)) {
    TProgramPointer programState;

    // Set current line
    programState.programCounter = tell_input();
    // Set current status
    programState.macroCall = currentGCodeState.macroCall;
    // We don't care about this repeatCount, the next one is checked
    stacks_push_program(&programState);
    seek_input(get_program_input(get_gcode_word_integer('P')));
    // Reset our status
    currentGCodeState.macroCall = false;
    // Set the repeat count, note that we're still working on the original line
    // even if the input has been fseek()-ed elsewhere.
    programState.repeatCount = (have_gcode_word('L', 0) ? get_gcode_word_integer('L'): 1);
    // Set current line for a possible repeat
    programState.programCounter = tell_input();
    stacks_push_program(&programState);
  }
  if(have_gcode_word('M', 1, 99)) {
    TProgramPointer programState;

    // Either way, we have to look
    stacks_pop_program(&programState);
    if(--programState.repeatCount) {
      // We still have iterations to go, push updated repeatCount back ...
      stacks_push_program(&programState);
      // ... and jump
      seek_input(programState.programCounter);
    } else {
      // Done looping, pop previous status
      stacks_pop_program(&programState);
      // Return to caller
      seek_input(programState.programCounter);
      // ... and since we restore #1-33 here, we don't care about restoring
      // currenGCodeState.macroCall as well
      if(programState.macroCall) stacks_pop_parameters();
    }
  }

/**
 * - echo (MSG, [already done first by input]
 * - if(seen_G && arg = have_G(2, 93, 94)) state.feedrateMode = arg
 * - if(seen_F) state.feedrate = get_override_feed_machine(get_F()) [returns 100% if disabled]
 * - if(seen_S) state.spindleSpeed = get_override_speed_machine(get_S()), set_spindle_speed_machine(state.spindleSpeed)
 * - if(seen_T) state.tool = get_T(), preselect_tool_machine(state.tool) [moves carousel]
 * - if(seen_M && have_M(1, 6)) change_tool_machine(state.tool) [actually changes tool]
 * - if(seen_M && have_M(1, 52)) change_tool_machine(TOOL_EMPTY) [unload spindle]
 * - if(seen_M && arg = have_M(2, 26, 27)) select_probe_machine(arg)
 * - if(seen_M && arg = have_M(2, 41, 42)) set_probemode_machine(arg)
 * - if(seen_M && arg = have_M(3, 3, 4, 5)) start_spindle_machine(arg)
 * - if(seen_M && arg = have_M(5, 7, 8, 9, 68, 69)) start_coolant_machine(arg)
 * - if(seen_M && arg = have_M(2, 13, 14)) start_spindle_and_coolant_machine(arg))
 * - if(seen_M && arg = have_M(2, 48, 49)) enable_override_machine(arg)
 * - if(seen_G && have_G(1, 4)) dwell_machine(get_P())
 * - if(seen_G && arg = have_G(3, 17, 18, 19)) state.systemPlane = arg
 * - if(seen_G && arg = have_G(2, 20, 21)) state.systemUnits = arg
 * - if(seen_G && arg = have_G(3, 40, 41, 42)) state.systemRadComp = arg
 * - if(seen_G && arg = have_G(3, 43, 44, 49)) state.systemLenComp = arg
 * - if(seen_G && arg = have_G(6, 53, 54, 55, 56, 57, 58, 59)) state.systemCurrent = arg
 * - if(seen_M && arg = have_M(3, 21, 22, 23)) enable_mirror_machine(arg)
 * - if(seen_G && arg = have_G(2, 22, 23)) state.systemMirror = (arg, get_X(), get_Y(), get_Z())
 * - if(seen_G && arg = have_G(2, 68, 69)) state.systemRotation = (arg, get_X(), get_Y(), get_R())
 * - if(seen_G && arg = have_G(2, 61, 64)) set_pathcontrol_machine(arg)
 * - if(seen_G && have_G(1, 9)) nmPC = true, oldPC = get_pathcontrol_machine(), set_pathcontrol_machine(EXACT)
 * - if(seen_G && arg = have_G(2, 90, 91)) state.systemAbsolute = arg
 * - if(seen_G && arg = have_G(2, 15, 16)) state.systemCartesian = arg
 * - if(seen_G && arg = have_G(2, 50, 51)) state.systemScaling = (arg, get_I(), get_J(), get_K(), get_P())
 * - if(seen_G && arg = have_G(2, 98, 99)) state.retractMode = arg
 * - if(seen_G)
 *     if(arg = have_G(4, 28, 29, 30, 80))
 *       state.motionMode = OFF
 *       if(arg != 80) move_machine_home(arg, get_X(), get_Y(), get_Z())
 *     if(arg = have_G(2, 10, 11)) oldMode = state.motionMode, state.motionMode = store_parameter_mode(arg) [which means STORE or oldMode]
 *     if(have_G(1, 92)) state.systemOffset = (get_X(), get_Y(), get_Z())
 * - if(seen_G)
 *     if(arg = have_G(6, 0, 1, 2, 3, 12, 13)) state.motionMode = arg
 *     if(arg = have_G(13, 31, 38, 73, 74, 81, 82, 83, 84, 85, 86, 87, 88, 89)) state.motionMode = CYCLE, state.cycle = arg
 * - if(seen_M && arg = have_M(3, 19, 20, 25)) move_machine_aux(arg)
 * - switch(state.motionMode)
 *     RAPID, LINEAR: move_machine_line(do_WCS_math(get_X(), get_Y(), get_Z(), state.system*), state.feedrate, state.feedrateMode)
 *     ARC, CIRCLE: move_machine_arc(do_WCS_math(get_X(), get_Y(), get_Z(), state.system*), get_I(), get_J(), get_K(), get_R(), get_P(), state.feedrate, state.feedrateMode)
 *     STORE: do_data_input(get_L(), get_P(), ...), commit_data()
 *     CYCLE: move_machine_cycle(state.cycle, do_WCS_math(get_X(), get_Y(), get_Z(), state.system*), state.retractMode, get_L(), get_P(), get_Q(), get_R(), state.feedrate, state.feedrateMode)
 *     OFF: <NOP>
 * - if(nmPC && state.motionMode != OFF) set_pathcontrol_machine(oldPC), nmPC = false
 * - if(seen_G && have_G(1, 65)) cmsc = true, push_parameters(), update_parameters(map_words_to_parameters(get_ALL()))
 * - if(seen_M && arg = have_M(8, 0, 1, 2, 17, 18, 30, 36, 60)) stop_machine(arg) [which reads Optional Stop and/or terminates us accordingly]
 * - update_parameters(parse_parameters(line)) [takes care of assignments #<idx>=<value OR #<param>>]
 * - commit_parameters() [always last on the line, according to the standard]
 * - if(seen_M) [since these change the line, they're conceptually after the end]
 *     if(have_M(1, 47)) rewind_input()
 *     if(have_M(1, 98)) push(tell_input()), seek_line_input(programs[get_P()]), push(tell_input()), repeat = get_L()
 *     if(have_M(1, 99))
 *       if(repeat) repeat--, seek_line_input(peek())
 *       if(!repeat) pop(), seek_line_input(pop())
 *         if(cmsc) cmsc = false, pop_parameters() [pop_parameters() calls commit internally]
 */

  return true;
}

uint32_t read_gcode_integer(char *line) {
  if(line[0] == '#') return (uint32_t)fetch_parameter(read_gcode_integer(&line[1]));
  else return strtol(line, (char **)NULL, 10);
}

double read_gcode_real(char *line) {
  if(line[0] == '#') return fetch_parameter(read_gcode_integer(&line[1]));
  else return strtod(line, (char **)NULL);
}

static bool _refresh_gcode_parse_cache(char word) {
  if(parseCache.word != word) { /* Not in cache */
    parseCache.word = word;
    parseCache.at = strchr(parseCache.line, word); /* Position at first occurrence */
  }

  return parseCache.at;
}

uint8_t have_gcode_word(char word, uint8_t argc, ...) {
  va_list argv;
  uint8_t result = false;
  char *last = NULL;

  if(!_refresh_gcode_parse_cache(word)) return false; /* No such word */
  else if(!argc) return true; /* We were only testing presence */
  else if(argc == 1) { /* Special case: single target, no need to loop */
    va_start(argv, argc);
    result = ((uint8_t)read_gcode_integer(&parseCache.at[1]) == (uint8_t)va_arg(argv, int));
    va_end(argv);

    return result;
  } else { /* Generic case: loop through targets */
    va_start(argv, argc);

    while(!result && argc--) {
      last = parseCache.at;
      while(last){
        result = (uint8_t)va_arg(argv, int);
        if((uint8_t)read_gcode_integer(&last[1]) == result) {
          if(!result) result = 100; /* Special case: tell 0 apart from false */
          break;
        } else result = 0;
        last = strchr(&last[1], word);
      }
    }

    va_end(argv);
  }

  return result;
}

double get_gcode_word_real(char word) {
  if(!_refresh_gcode_parse_cache(word)) return NAN;
  else return read_gcode_real(&parseCache.at[1]);
}

uint32_t get_gcode_word_integer(char word) {
  if(!_refresh_gcode_parse_cache(word)) return UINT32_MAX;
  else return read_gcode_integer(&parseCache.at[1]);
}

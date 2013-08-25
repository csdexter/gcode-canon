/*
 ============================================================================
 Name        : gcode-cycles.c
 Author      : Radu - Eosif Mihailescu
 Version     : 1.0 (2013-08-06)
 Copyright   : (C) 2012 Radu - Eosif Mihailescu <radu.mihailescu@linux360.ro>
 Description : Canned Cycle Expansion Generator Code
 ============================================================================
 */

#include <math.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gcode-commons.h"
#include "gcode-cycles.h"
#include "gcode-math.h"
#include "gcode-debugcon.h"
#include "gcode-machine.h"
#include "gcode-parameters.h"


static uint8_t curSlice;
static char *slices[GCODE_CYCLE_MAXSLICES];


static bool _add_generated_line(const char *format, ...) {
  va_list ap;

  va_start(ap, format);
  vsnprintf(slices[0], GCODE_CYCLE_BUFSLICE, format, ap);
  va_end(ap);

  if((strlen(slices[curSlice]) + strlen(slices[0])) >= GCODE_CYCLE_BUFSLICE) {
    if(curSlice < GCODE_CYCLE_MAXSLICES)
      slices[curSlice++] = (char *)calloc(GCODE_CYCLE_BUFSLICE, 1);
    else {
      display_machine_message("PER: Canned cycle injection buffer overflow!");
      return false;
    }
  }

  strncat(slices[curSlice], slices[0], GCODE_CYCLE_BUFSLICE);
  slices[0][0] = '\0';

  return true;
}

static void _reset_slices(void) {
  uint8_t i;

  for(i = curSlice; i > 1; i--)
    free((void *)slices[i]);
  curSlice = 1;
  slices[curSlice][0] = '\0';
}

static char *_build_cycle(void) {
  uint8_t i;
  size_t size = 0;
  char *cycle;

  for(i = curSlice; i; i--) size += strlen(slices[i]);
  cycle = (char *)calloc(size + 1, 1);
  for(i = 1; i <= curSlice; i++) strncat(cycle, slices[i], size);

  return cycle;
}

bool init_cycles(void *data) {
  slices[0] = (char *)calloc(GCODE_CYCLE_BUFSLICE, 1);
  slices[1] = (char *)calloc(GCODE_CYCLE_BUFSLICE, 1);
  _reset_slices();

  GCODE_DEBUG("Canned Cycles up, using %d bytes injection buffer",
              GCODE_CYCLE_BUFSLICE * GCODE_CYCLE_MAXSLICES);

  return true;
}

char *generate_cycles(TGCodeState state) {
  double preZ = state.system.Z, pregZ = state.system.gZ, precZ;
  bool feedRetract;
  uint16_t peckSteps;
  double extraMove, howFarDown = 0.0;

  /* Preparatory move (singleton, only if Z below R) */
  do_WCS_move_math(&state.system, NAN, NAN, state.R);
  if(state.system.Z > preZ)
    _add_generated_line("G00 Z" GCODE_REAL_FORMAT "\n", state.R);
  state.system.Z = preZ;
  state.system.gZ = pregZ;

  if(state.system.absolute == GCODE_RELATIVE) {
    //Compute the previous Z relative to R in case we're in G98
    precZ = -state.R;

    //Z is relative to current position, R was relative to the previous
    //position and needs to be made relative to Z
    state.R = -state.system.cZ;
  } else
    //Reverse engineer the previous Z in case we're in G98
    precZ = state.system.gZ - (
        state.system.current == GCODE_MCS ?
            0.0 :
            fetch_parameter(
                GCODE_PARM_FIRST_WCS +
                (state.system.current - GCODE_WCS_1) *
                GCODE_PARM_WCS_SIZE + GCODE_AXIS_Z) + state.system.offset.Z);

  /* Repetitions */
  while(state.L--) {
    /* First preparatory move */
    _add_generated_line("G00 X" GCODE_REAL_FORMAT " Y" GCODE_REAL_FORMAT "\n",
                        state.system.cX, state.system.cY);
    /* Second preparatory move */
    _add_generated_line("G00 Z" GCODE_REAL_FORMAT "\n", state.R);
    /* Actual canned cycle */
    switch(state.cycle) {
      case GCODE_CYCLE_DRILL_ND:
      case GCODE_CYCLE_DRILL_WD:
      case GCODE_CYCLE_BORING_ND_NS:
      case GCODE_CYCLE_BORING_WD_WS:
      case GCODE_CYCLE_BORING_MANUAL:
      case GCODE_CYCLE_BORING_WD_NS:
        _add_generated_line("G01 Z" GCODE_REAL_FORMAT "\n", state.system.cZ);
        if(state.cycle == GCODE_CYCLE_DRILL_WD ||
           state.cycle == GCODE_CYCLE_BORING_WD_WS ||
           state.cycle == GCODE_CYCLE_BORING_MANUAL ||
           state.cycle == GCODE_CYCLE_BORING_WD_NS)
          _add_generated_line("G04 P" GCODE_REAL_FORMAT "\n", state.P);
        if(state.cycle == GCODE_CYCLE_BORING_WD_WS ||
           state.cycle == GCODE_CYCLE_BORING_MANUAL)
          _add_generated_line("M05\n");
        if(state.cycle == GCODE_CYCLE_BORING_MANUAL)
          _add_generated_line("M01\n");
        if(state.cycle == GCODE_CYCLE_BORING_ND_NS ||
           state.cycle == GCODE_CYCLE_BORING_WD_NS)
          feedRetract = true;
        else
          feedRetract = false;
        break;
      case GCODE_CYCLE_TAP_LH:
      case GCODE_CYCLE_TAP_RH:
        /* Stop spindle, disable overrides, per revolution */
        _add_generated_line("M05 M49 G95 F" GCODE_REAL_FORMAT "\n", state.K);
        /* Start spindle, exact stop check, feed in */
        _add_generated_line("M%02d G09 G01 Z" GCODE_REAL_FORMAT "\n",
                            (state.cycle == GCODE_CYCLE_TAP_LH ? 4 : 3),
                            state.system.cZ);
        /* Spindle will stop at end of move, but better safe than sorry */
        _add_generated_line("M05\n");
        /* Reverse spindle, exact stop check, feed out */
        _add_generated_line("M%02d G09 G01 Z" GCODE_REAL_FORMAT "\n",
                            (state.cycle == GCODE_CYCLE_TAP_RH ? 4 : 3),
                            state.R);
        /* Spindle will stop at end of move, but better safe than sorry */
        _add_generated_line("M05\n");
        /* Restore feed mode and value, enable overrides */
        _add_generated_line("G%2d M48 F" GCODE_REAL_FORMAT "\n",
                            state.feedMode, state.F);
        /* Start spindle */
        _add_generated_line("M%02d\n",
                            (state.cycle == GCODE_CYCLE_TAP_LH ? 4 : 3));
        feedRetract = false;
        break;
      case GCODE_CYCLE_BORING_BACK:
        _add_generated_line("G91 G00 X" GCODE_REAL_FORMAT " Y" GCODE_REAL_FORMAT "\n",
                            state.I, state.J);
        _add_generated_line("M05\n");
        _add_generated_line("M19\n");
        _add_generated_line("G%02d G00 Z" GCODE_REAL_FORMAT "\n",
                            state.system.absolute, state.system.cZ);
        _add_generated_line("G00 X" GCODE_REAL_FORMAT " Y" GCODE_REAL_FORMAT "\n",
                            (state.system.absolute == GCODE_ABSOLUTE ?
                                state.system.cX : -state.I),
                            (state.system.absolute == GCODE_ABSOLUTE ?
                                state.system.cY : -state.J));
        //TODO: make it start in the same direction it was turning before
        _add_generated_line("M03\n");
        _add_generated_line("G01 Z" GCODE_REAL_FORMAT "\n", state.K);
        _add_generated_line("G01 Z" GCODE_REAL_FORMAT "\n",
                            (state.system.absolute == GCODE_ABSOLUTE ?
                                state.system.cZ : -state.K));
        _add_generated_line("M05\n");
        _add_generated_line("M19\n");
        _add_generated_line("G91 G00 X" GCODE_REAL_FORMAT " Y" GCODE_REAL_FORMAT "\n",
                            state.I, state.J);
        /* This does the final move here: we need it in order to satisfy the
         * condition that every cycle ends in the same spot it started */
        _add_generated_line("G%02d G00 Z" GCODE_REAL_FORMAT "\n",
                            state.system.absolute, state.R);
        _add_generated_line("G00 X" GCODE_REAL_FORMAT " Y" GCODE_REAL_FORMAT "\n",
                            (state.system.absolute == GCODE_ABSOLUTE ?
                                state.system.cX : -state.I),
                            (state.system.absolute == GCODE_ABSOLUTE ?
                                state.system.cY : -state.J));
        //TODO: make it start in the same direction it was turning before
        _add_generated_line("M03\n");
        break;
      case GCODE_CYCLE_DRILL_PP:
      case GCODE_CYCLE_DRILL_PF:
        /* Calculate how many movements by Q we need to cover Z. The quotient
         * is the number of steps. If there is any remainder, that will be an
         * extra move towards Z proper */
        peckSteps = (uint16_t)trunc(
            state.R -
            (state.system.absolute == GCODE_ABSOLUTE ? state.system.cZ : 0) /
            state.Q);
        extraMove = fmod(
            state.R -
            (state.system.absolute == GCODE_ABSOLUTE ? state.system.cZ : 0),
            state.Q);
        /* Perform the pecks */
        while(peckSteps--) {
          /* Z goes down, so feed by -Q in relative mode */
          _add_generated_line("G91 G01 Z" GCODE_REAL_FORMAT "\n", -state.Q);
          if(state.cycle == GCODE_CYCLE_DRILL_PP)
            /* Partial retract: traverse up by +Q/2 */
            _add_generated_line("G00 Z" GCODE_REAL_FORMAT "\n", state.Q / 2);
          else {
            /* Full retract: traverse up to where we began, traverse down to
             * where we were before less Q/2 */
            howFarDown += state.Q;
            _add_generated_line("G00 Z" GCODE_REAL_FORMAT "\n", howFarDown);
            _add_generated_line("G00 Z" GCODE_REAL_FORMAT "\n",
                                -(howFarDown - state.Q / 2));
          }
          _add_generated_line("G01 Z" GCODE_REAL_FORMAT "\n", -(state.Q / 2));
        }
        /* Still extraMove to go until we reach Z */
        if(fpclassify(extraMove) == FP_NORMAL)
          _add_generated_line("G01 Z" GCODE_REAL_FORMAT "\n", -extraMove);
        /* Restore previous measurement mode */
        _add_generated_line("G%02d\n", state.system.absolute);
        feedRetract = false;
        break;
    }
    /* Final move */
    if(state.cycle != GCODE_CYCLE_BORING_BACK)
      _add_generated_line("G%02d Z" GCODE_REAL_FORMAT "\n", (int)feedRetract,
                          state.R);
  }

  /* Retract to last Z if in G98 */
  if(state.retractMode == GCODE_RETRACT_LAST)
    _add_generated_line("G00 Z" GCODE_REAL_FORMAT "\n", precZ);

  /* Any extras ? */
  if(state.cycle == GCODE_CYCLE_BORING_WD_WS ||
     state.cycle == GCODE_CYCLE_BORING_MANUAL)
    //TODO: make it start in the same direction it was turning before
    _add_generated_line("M03\n");

  return _build_cycle();
}

bool done_cycles(void) {
  _reset_slices();
  free((void *)slices[0]);
  free((void *)slices[1]);

  return true;
}

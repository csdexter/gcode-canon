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


static const char GCODE_CYCLE_PRE_1[] = "G00 Z%4.2f\n";
static const char GCODE_CYCLE_PRE_2[] = "G00 X%4.2f Y%4.2f\n";
static const char GCODE_CYCLE_POST[] = "G53 G%02d Z%4.2f\n";

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
  uint16_t rep;

  /* Preparatory move (singleton, only if Z below R) */
  do_WCS_move_math(&state.system, NAN, NAN, state.R);
  if(state.system.Z > preZ) {
    _add_generated_line(GCODE_CYCLE_PRE_1, state.R);
  }
  state.system.Z = preZ;
  state.system.gZ = pregZ;

  if(state.system.absolute == GCODE_RELATIVE) {
    //Compute the previous Z relative to R in case we're in G98
    precZ = 0 - state.R;

    //Z is relative to current position, R was relative to the current
    //position and needs to be made relative to Z
    state.R = 0 - state.system.cZ;
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
  for(rep = 0; rep < state.L; rep++) {
    /* First preparatory move */
    _add_generated_line(GCODE_CYCLE_PRE_2, state.system.cX, state.system.cY);
    /* Second preparatory move */
    _add_generated_line(GCODE_CYCLE_PRE_1, state.R);
    /* Actual canned cycle */
    switch(state.cycle) {
      case GCODE_CYCLE_DRILL_ND:
      case GCODE_CYCLE_DRILL_WD:
      case GCODE_CYCLE_BORING_ND_NS:
      case GCODE_CYCLE_BORING_WD_WS:
      case GCODE_CYCLE_BORING_MANUAL:
      case GCODE_CYCLE_BORING_WD_NS:
        _add_generated_line("G01 Z%4.2f\n", state.system.cZ);
        if(state.cycle == GCODE_CYCLE_DRILL_WD ||
           state.cycle == GCODE_CYCLE_BORING_WD_WS ||
           state.cycle == GCODE_CYCLE_BORING_MANUAL ||
           state.cycle == GCODE_CYCLE_BORING_WD_NS)
          _add_generated_line("G04 P%4.2f\n", state.P);
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
    }
    /* Final move */
    _add_generated_line(GCODE_CYCLE_POST, (int)feedRetract, state.R);
  }

  /* Retract to last Z if in G98 */
  if(state.retractMode == GCODE_RETRACT_LAST)
    _add_generated_line(GCODE_CYCLE_POST, (int)false, precZ);

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
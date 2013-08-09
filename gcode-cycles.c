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


static const char GCODE_CYCLE_PRE_1[] = "G00 X%4.2f Y%4.2f\n";
static const char GCODE_CYCLE_PRE_2[] = "G00 Z%4.2f\n";
static const char GCODE_CYCLE_POST[] = "G53 G%02d Z%4.2f\n";
static const char GCODE_DRILL_NDWELL[] = "G01 Z%4.2f\n";

static uint8_t curSlice;
static char *slices[GCODE_CYCLE_MAXSLICES];


static bool _add_generated_line(const char *format, ...) {
  va_list ap;

  va_start(ap, format);
  vsnprintf(slices[0], GCODE_CYCLE_BUFSLICE, format, ap);
  va_end(ap);

  if((strlen(slices[curSlice]) + strlen(slices[0])) >= GCODE_CYCLE_BUFSLICE)
    if(curSlice < GCODE_CYCLE_MAXSLICES)
      slices[curSlice++] = (char *)calloc(GCODE_CYCLE_BUFSLICE, 1);
    else {
      display_machine_message("PER: Canned cycle injection buffer overflow!");
      return false;
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
  cycle = (char *)calloc(size, 1);
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
  double preZ = state.system.Z;
  bool feedRetract;

  /* First preparatory move */
  _add_generated_line(GCODE_CYCLE_PRE_1, state.system.cX, state.system.cY);

  /* Second preparatory move */
  do_WCS_move_math(&state.system, NAN, NAN, state.R);
  if(state.system.Z > preZ) _add_generated_line(GCODE_CYCLE_PRE_2, state.R);
  state.system.Z = preZ;

  /* Actual canned cycle */
  //TODO: handle repeats (in both absolute and relative mode)
  switch(state.cycle) {
    case GCODE_CYCLE_DRILL_ND:
      _add_generated_line(GCODE_DRILL_NDWELL, state.system.cZ);
      feedRetract = false;
      break;
  }

  /* Final move */
  _add_generated_line(
      GCODE_CYCLE_POST, (int)feedRetract,
      (state.retractMode == GCODE_RETRACT_LAST ? preZ : state.R));

  return _build_cycle();
}

bool done_cycles(void) {
  _reset_slices();
  free((void *)slices[0]);
  free((void *)slices[1]);

  return true;
}

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
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "gcode-commons.h"
#include "gcode-cycles.h"
#include "gcode-math.h"


static const char GCODE_CYCLE_PRE_1[] = "G00 X%4.2f Y%4.2f\n";
static const char GCODE_CYCLE_PRE_2[] = "G00 Z%4.2f\n";
static const char GCODE_CYCLE_POST[] = "G%02d Z%4.2f\n";

static void _add_generated_line(char **data, uint8_t *index) {
  if((strlen(data[index]) + strlen(data[0])) >= GCODE_CYCLE_BUFSLICE)
    data[index++] = (char *)calloc(GCODE_CYCLE_BUFSLICE, 1);
  strncat(data[index], data[0], GCODE_CYCLE_BUFSLICE);
  data[0][0] = '\0';
}

char *generate_cycle(TGCodeState state) {
  double preZ = state.system.Z;
  char *slices[GCODE_CYCLE_MAXSLICES];
  uint8_t curSlice = 1;
  int written;
  slices[0] = (char *)calloc(GCODE_CYCLE_BUFSLICE, 1);
  slices[curSlice] = (char *)calloc(GCODE_CYCLE_BUFSLICE, 1);

  snprintf(slices[0], GCODE_CYCLE_BUFSLICE, GCODE_CYCLE_PRE_1, state.system.cX,
           state.system.cY);

  _add_generated_line(slices, &curSlice);

  do_WCS_move_math(&state.system, NAN, NAN, state.R);
  if(state.system.Z > preZ) {
    snprintf(slices[0], GCODE_CYCLE_BUFSLICE, GCODE_CYCLE_PRE_2, state.R);
    _add_generated_line(slices, &curSlice);
  }
  state.system.Z = preZ;

  /* the cycle happens here */



  return NULL;
}

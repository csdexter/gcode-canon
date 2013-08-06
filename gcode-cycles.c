/*
 ============================================================================
 Name        : gcode-cycles.c
 Author      : Radu - Eosif Mihailescu
 Version     : 1.0 (2013-08-06)
 Copyright   : (C) 2012 Radu - Eosif Mihailescu <radu.mihailescu@linux360.ro>
 Description : Canned Cycle Expansion Generator Code
 ============================================================================
 */

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "gcode-commons.h"
#include "gcode-cycles.h"


static const char GCODE_CYCLE_PRE_1[] = "G00 X%4.2f Y%4.2f";
static const char GCODE_CYCLE_PRE_2[] = "G00 Z%4.2f";
static const char GCODE_CYCLE_POST[] = "G%02d Z%4.2f";

char *generate_cycle(TGCodeState state, double oldZ, double X, double Y, double Z) {
  double preZ;
  char *slices[GCODE_CYCLE_MAXSLICES];
  uint8_t curSlice = 0;

  slices[curSlice] = (char *)malloc(GCODE_CYCLE_BUFSLICE);

  //snprintf(slices[curSlice], GCODE_CYCLE_BUFSLICE, GCODE_CYCLE_PRE_1, )
  preZ = (oldZ < state.R ? state.R : oldZ);

  return NULL;
}

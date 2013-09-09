/*
 ============================================================================
 Name        : gcode-cycles.c
 Author      : Radu - Eosif Mihailescu
 Version     : 1.0 (2013-08-06)
 Copyright   : (C) 2013 Radu - Eosif Mihailescu <radu.mihailescu@linux360.ro>
 Description : Canned Cycle Expansion Generator API Header
 ============================================================================
 */

#ifndef GCODE_CYCLES_H_
#define GCODE_CYCLES_H_


#include "gcode-state.h"


#define GCODE_CYCLE_BUFSLICE 0xFFU
#define GCODE_CYCLE_MAXSLICES 32

bool init_cycles(void *data);
/* Generates G-Code for the canned cycle in state.cycle. The char * allocated
 * here will be freed in gcode-input, when the '\0' at its end will be read by
 * fetch_char_input */
char *generate_cycles(TGCodeState state, double X, double Y, double Z);
bool done_cycles(void);

#endif /* GCODE_CYCLES_H_ */

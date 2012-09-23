/*
 * gcode-stacks.c
 *
 *  Created on: Sep 23, 2012
 *      Author: csdexter
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "gcode-commons.h"
#include "gcode-stacks.h"
#include "gcode-debugcon.h"
#include "gcode-parameters.h"


static double *parametersStack[GCODE_MACRO_COUNT];
static TProgramPointer *programStack[GCODE_SUBPROGRAM_COUNT];
uint8_t paSP, prSP;

bool init_stacks(void *data) {
  paSP = prSP = 0;

  GCODE_DEBUG("Stacks initialized, %d nested calls (%d of which macro-capable) supported", GCODE_SUBPROGRAM_COUNT, GCODE_MACRO_COUNT);

  return true;
}

bool stacks_push_parameters(void) {
  if(paSP < GCODE_MACRO_COUNT - 1) {
    double *params = (double *)malloc(sizeof(double) * 33);
    uint8_t i;

    for(i = 0; i < 33; i++) params[i] = fetch_parameter(i + 1);
    parametersStack[++paSP] = params;

    return true;
  } else return false;
}

bool stacks_push_program(uint16_t pointer, bool macro) {
  if(prSP < GCODE_SUBPROGRAM_COUNT - 1) {
    TProgramPointer *pptr = (TProgramPointer *)malloc(sizeof(TProgramPointer));

    pptr->programCounter = pointer;
    pptr->macroCall = macro;
    programStack[++prSP] = pptr;

    return true;
  } else return false;
}

bool stacks_pop_parameters(void) {
  if(paSP) {
    double *params = parametersStack[paSP--];
    uint8_t i;

    for(i = 0; i < 33; i++) update_parameter(i + 1, params[i]);
    free(params);
    commit_parameters();

    return true;
  } else return false;
}

uint16_t stacks_pop_program(bool *macro) {
  if(prSP) {
    TProgramPointer *pptr = programStack[prSP--];
    uint16_t ptr = pptr->programCounter;

    *macro = pptr->macroCall;
    free(pptr);

    return ptr;
  } else return -1;
}

uint16_t stacks_peek_program(void) {
  return programStack[prSP]->programCounter;
}

bool done_stacks(void) {
  GCODE_DEBUG("Stacks done");

  return true;
}

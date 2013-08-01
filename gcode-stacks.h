/*
 ============================================================================
 Name        : gcode-stacks.h
 Author      : Radu - Eosif Mihailescu
 Version     : 1.0 (2012-09-23)
 Copyright   : (C) 2012 Radu - Eosif Mihailescu <radu.mihailescu@linux360.ro>
 Description : G-Code Subroutine Parameter Stack API Header
 ============================================================================
 */

#ifndef GCODE_STACKS_H_
#define GCODE_STACKS_H_


#include <stdbool.h>
#include <stdint.h>

#include "gcode-commons.h"


typedef struct {
    long programCounter;
    bool macroCall;
    uint16_t repeatCount;
} TProgramPointer;


bool init_stacks(void *data);
/* Pushes #1-33 on stack for G65 */
bool stacks_push_parameters(void);
/* Pushes current state of program */
bool stacks_push_program(const TProgramPointer *state);
/* Pops #1-33 off the stack on M99 after G65 */
bool stacks_pop_parameters(void);
/* Pops current state of program */
bool stacks_pop_program(TProgramPointer *state);
bool done_stacks(void);


#endif /* GCODE_STACKS_H_ */

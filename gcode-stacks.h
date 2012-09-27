/*
 * gcode-stacks.h
 *
 *  Created on: Sep 23, 2012
 *      Author: csdexter
 */

#ifndef GCODE_STACKS_H_
#define GCODE_STACKS_H_


#include <stdbool.h>
#include <stdint.h>

#include "gcode-commons.h"


typedef struct {
    uint16_t programCounter;
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
/* Peeks current state of program */
void stacks_peek_program(TProgramPointer *state);
bool done_stacks(void);

#endif /* GCODE_STACKS_H_ */

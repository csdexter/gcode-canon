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
} TProgramPointer;

bool init_stacks(void *data);
/* Pushes #1-33 on stack for G65 */
bool stacks_push_parameters(void);
/* Pushes current line of program and macro call flag */
bool stacks_push_program(uint16_t pointer, bool macro);
/* Pops #1-33 off the stack on M99 after G65 */
bool stacks_pop_parameters(void);
/* Pops line we were called from and macro call flag */
uint16_t stacks_pop_program(bool *macro);
/* Peeks line we began at for M98 L */
uint16_t stacks_peek_program(void);
bool done_stacks(void);

#endif /* GCODE_STACKS_H_ */

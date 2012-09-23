/*
 * gcode-input.h
 *
 *  Created on: Aug 11, 2012
 *      Author: csdexter
 */

#ifndef GCODE_INPUT_H_
#define GCODE_INPUT_H_

#include "gcode-commons.h"

typedef struct {
  uint16_t offset;
  uint16_t program;
} TGCodeProgramIndexEntry;


/* Gets the input ready to stream data in, takes opaque pointer to data store */
bool init_input(void *data);
/* Reset input, rewinding it to the top. Next character fetched will be first character of program */
bool rewind_input(void);
/* Seek input to lineNumber. Next character fetched will be the 1st on lineNumber. Note that lineNumber has no relation to N word */
bool seek_input_line(uint16_t lineNumber);
/* Return current position of input as a line number usable for seek_input_line() later on */
uint16_t tell_input(void);
/* Fetch the next character of input or EOF */
char fetch_char_input(void);
/* Fetch a complete line of input, stripped of whitespace, comments and \n; returns false if there's no more input to read */
bool fetch_line_input(char *line);
bool done_input(void);

#endif /* GCODE_INPUT_H_ */

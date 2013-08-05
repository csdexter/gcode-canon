/*
 ============================================================================
 Name        : gcode-input.h
 Author      : Radu - Eosif Mihailescu
 Version     : 1.0 (2012-08-11)
 Copyright   : (C) 2012 Radu - Eosif Mihailescu <radu.mihailescu@linux360.ro>
 Description : G-Code Input API Header
 ============================================================================
 */

#ifndef GCODE_INPUT_H_
#define GCODE_INPUT_H_


#include <stdint.h>

#include "gcode-commons.h"


typedef struct {
  long offset; /* "long" as per man fseek */
  uint16_t program;
} TGCodeProgramIndexEntry;


/* Gets the input ready to stream data in, takes opaque pointer to data store */
bool init_input(void *data);
/* Reset input, rewinding it to the top. Next character fetched will be first
 * character of program */
bool rewind_input(void);
/* Seek input to offset. Next character fetched will be the one at
 * offset. Note that offset has no relation to N word */
bool seek_input(long offset);
/* Return current position of input as an opaque offset usable for
 * seek_input() later on */
long tell_input(void);
/* Fetch the next character of input or EOF */
char fetch_char_input(void);
/* Fetch a complete line of input, stripped of whitespace, comments and \n;
 * returns false if there's no more input to read.
 * Call with NULL if you don't care about the line's program contents and only
 * want to detect syntax errors. */
bool fetch_line_input(char *line);
/* Where does O<n> start? */
long get_program_input(uint16_t program);
/* Splices data into the input stream. After the call, fetch_char_input() will
 * operate on data instead of the input file (which remains otherwise open and
 * unaffected). When '\0' is read from data, input is switched back to the
 * input file and data is free()d.
 * Calling subprograms from spliced code results in undefined behavior.
 * Returns true if successful, false if already spliced. */
bool splice_input(const char *data);
/* Returns true exactly once if the end of the input splice was reached */
bool end_of_spliced_input(void);
bool done_input(void);


#endif /* GCODE_INPUT_H_ */

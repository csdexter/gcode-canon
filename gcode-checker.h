/*
 ============================================================================
 Name        : gcode-checker.h
 Author      : Radu - Eosif Mihailescu
 Version     : 1.0 (2015-11-05)
 Copyright   : (C) 2015 Radu - Eosif Mihailescu <radu.mihailescu@linux360.ro>
 Description : G-Code Syntax Checker API Header
 ============================================================================
 */

#ifndef GCODE_CHECKER_H_
#define GCODE_CHECKER_H_

#include <stdbool.h>

bool init_checker(void *data);
void done_checker(void);
/* Check the validity of the contents of line according to G-Code syntax and
 * current state of the interpreter. Returns true if valid, false otherwise. */
bool gcode_check(char *line);
/* Resets the state of the interpreter so that the next call to gcode_check()
 * behaves as if it were the first G-Code line it ever saw. */
void reset_checker(void);

#endif /* GCODE_CHECKER_H_ */

/*
 ============================================================================
 Name        : gcode-checker.h
 Author      : Radu - Eosif Mihailescu
 Version     : 1.0 (2015-11-05)
 Copyright   : (C) 2015 Radu - Eosif Mihailescu <radu.mihailescu@linux360.ro>
 Description : G-Code Syntax Checker Code
 ============================================================================
 */

#include <stdbool.h>
#include <stdio.h>

#include "gcode-commons.h"
#include "gcode-checker.h"
#include "gcode-debugcon.h"

bool init_checker(void *data) {
  GCODE_DEBUG("G-Code code syntax verifier up, checking against The Book.");

  return true;
}

void done_checker(void) {
  /* NOP */
}

bool gcode_check(char *line) {
  return true;
}

void reset_checker(void) {
  /* NOP */
}

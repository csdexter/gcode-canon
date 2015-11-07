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
#include <string.h>

#include "gcode-commons.h"
#include "gcode-checker.h"
#include "gcode-debugcon.h"
#include "gcode-state.h"

/* G command to modal group number mapping */
static const int8_t G2ModalGroup[] = {
    1, 1, 1, 1, 0, -1, -1, -1, -1, 0, 0, 0, -1, -1, -1, 18, 18, 2, 2, 2, 6, 6,
    4, 4, -1, 24, 24, 0, 0, 0, 0, 0, -1, -1, -1, -1, -1, 0, -1, -1, 7, 7, 7, 8,
    8, 0, 0, 0, 0, 8, 11, 11, 0, 0, 12, 12, 12, 12, 12, 12, 0, 13, 13, 13, 13,
    0, 14, 14, 16, 16, -1, -1, -1, 9, 9, -1, 9, -1, -1, -1, 9, 9, 9, 9, 9, 9, 9,
    9, 9, 9, 3, 3, -1, 5, 5, 5, -1, -1, 10, 10};

/* M command to moda group number mapping */
static const int8_t M2ModalGroup[] = {
    1, 1, 1, 2, 2, 2, 3, 4, 4, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, 2, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, 8, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    9, 9};

static const char allowedWords[] = "ABCDEFGHIJKLMPQRSTUVWXYZZ";

bool init_checker(void *data) {
  GCODE_DEBUG("G-Code code syntax verifier up, checking against The Book.");

  return true;
}

void done_checker(void) {
  /* NOP */
}

bool gcode_check(const char *line) {
  const char *checkp;

  /* No data is bad (our caller didn't get the point) */
  if(!line) return false;
  /* Empty line is good, though it gets stripped before we get here. */
  if(!(*line)) return true;

  checkp = line;

  /* Skip over deleted block marker, a valid block must still follow */
  if(*checkp == '/') checkp++;
  /* If there is a line number, skip over it */
  //TODO: we need a specialized skip_gcode_digits() here that would not accept
  //parameter references.
  if(*checkp == 'N') checkp = skip_gcode_digits(++checkp);
  /* If there is a program number, check it's the last word in block */
  if(*checkp == 'O') {
    checkp = skip_gcode_digits(++checkp);
    if(*checkp != '\0') return false;
  }

  /* From here on we process things word-by-word */
//  while(*checkp) {
    /* Illegal word address given or syntax error (e.g. bare number) */
//    if(!strchr(allowedWords, *checkp)) return false;
//    if(*checkp == 'G') {
      /* Check for modal group violations */
//    } else if(*checkp == 'M') {
      /* Check for modal group violations */
//    } else {
      /* Check if we have seen this word before in this block */
//    }
//  }

  return true;
}

void reset_checker(void) {
  /* NOP */
}

/*
 ============================================================================
 Name        : gcode-input.c
 Author      : Radu - Eosif Mihailescu
 Version     : 1.0 (2012-08-11)
 Copyright   : (C) 2012 Radu - Eosif Mihailescu <radu.mihailescu@linux360.ro>
 Description : G-Code Input API and Implementation Code
 ============================================================================
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <locale.h>

#include "gcode-commons.h"
#include "gcode-input.h"
#include "gcode-debugcon.h"
#include "gcode-machine.h"
#include "gcode-expression.h"


static FILE *input;
static TGCodeProgramIndexEntry programs[GCODE_PROGRAM_CAPACITY];
static uint8_t programCount;
static bool spliced, endOfSplice;
static const char *splice;
static ptrdiff_t splicep;

bool init_input(void *data) {
  input = (FILE *)data;
  memset(&programs, 0x00, sizeof(programs));
  programCount = 0;
  spliced = false;

  GCODE_DEBUG("Input stream up, %d program table entries available",
              GCODE_PROGRAM_CAPACITY);
  /* Protect against numbering system strangeness on other locales. Also makes
   * "upper case" have a very well defined meaning */
  setlocale(LC_ALL, "C");
  display_machine_message("STA: Scanning input for programs (O words)");
  while(fetch_line_input(NULL));
  rewind_input();

  return true;
}

bool rewind_input(void) {
  if(input) {
    rewind(input);
    GCODE_DEBUG("Program reset");

    return true;
  } else {
    display_machine_message("IER: No program to reset, ignoring request!");

    return false;
  }
}

bool seek_input(long offset) {
  int result;

  result = fseek(input, offset, SEEK_SET);

  GCODE_DEBUG("Seek to offset %zd in input file", offset);

  return result ? false : true;
}

long tell_input(void) {
  return ftell(input);
}

char fetch_char_input(void) {
  char result;

  if(spliced) {
    result = splice[splicep++];
    /* If the next char is '\0', act as if it were EOF and revert to the file */
    if(!splice[splicep]) {
      spliced = false;
      endOfSplice = true;
      free((void *)splice);
    }
  } else {
    if(input) result = fgetc(input);
    else {
      display_machine_message("IER: Unable to read input, assuming empty!");
      result = EOF;
    }
  }

  return result;
}

/* NOTE: when in spliced mode, this does not change content retrieved in the
 * future as ungetc() does, only moves the virtual file pointer backwards */
void push_char_input(unsigned char c) {
  if(spliced && splicep) splicep--;
  else ungetc(c, input);
}

bool fetch_line_input(char *line) {
  int c = '\0';
  uint8_t i = 0, j, l;
  bool ignore = false;
  char commsg[0xFF - 2]; /* max line length - 2 parentheses */

  while(c != EOF) {
    c = toupper(fetch_char_input());

    if(c == '\n' || c == '\r') {
      ignore = false;

      if(c == '\r') {
        c = fetch_char_input();
        if(c != '\n') push_char_input(c);
      }

      if(i) break; /* EOL with data: return line */
      else continue; /* EOL without data: strip empty line */
    }

    /* Strip whitespace and deleted blocks */
    if(ignore || c == ' ' || c == '\t') continue;

    if(c == '/' && !i && block_delete_machine()) { /* Strip deleted blocks */
      ignore = true;
      continue;
    }

    if(c == '(') { /* Strip (and maybe display) comments */
      j = 0;

      c = fetch_char_input();
      while(c != ')') {
        commsg[j++] = c;
        c = fetch_char_input();
      }
      commsg[j] = '\0';
      /* We shouldn't output messages if we were called in test mode */
      if(!strncmp(commsg, "MSG,", strlen("MSG,")) && line)
        display_machine_message(&commsg[strlen("MSG,")]);

      continue;
    }

    if((c == 'N' || c == 'O') && !i) {
      int d;

      j = 0;
      /* read the number, which should be a literal integer, since O and N do
       * not support parameter indirection */
      d = fetch_char_input();
      while(isdigit(d)) {
        commsg[j++] = d;
        d = fetch_char_input();
      }
      commsg[j] = '\0';
      push_char_input(d); /* First non-digit character has to go back */

      if(c == 'O') {/* We ignore N and store a bookmark for O for now */
        if(programCount < GCODE_PROGRAM_CAPACITY) {
          /* Better mess with precision than signedness */
          programs[programCount].program = (uint16_t)atol(commsg);
          /* The line immediately after the O word */
          programs[programCount].offset = tell_input();
          programCount++;
        } else display_machine_message("PER: Program table overflow!");
      } else {
        //TODO: support N
      }

      continue;
    }

    /* We don't really do anything with the program separator */
    if(c == '%') continue;

    if(c == '[') { /* Evaluate expressions before any other processing */
      j = 0;
      l = 0;
      c = fetch_char_input();
      while(!(c == ']' && !l)) {
        /* Strip whitespace */
        if(!(c == ' ' || c == '\t')) {
          commsg[j++] = c;
          if(c == '[') l++; /* Handle nested brackets properly */
          if(c == ']') l--;
        }
        c = fetch_char_input();
      }
      commsg[j] = '\0';

      if(line)
        i += snprintf(&line[i], 0xFF - i, GCODE_REAL_FORMAT,
                      evaluate_expression(commsg));

      continue;
    }

    if(line && c != EOF) line[i++] = c; /* Otherwise add to the line buffer */
  }

  if(line) {
    line[i] = '\0';
    evaluate_unary_expression(line);
  }

  return c == EOF ? false : true;
}

long get_program_input(uint16_t program) {
  uint8_t i;
  for(i = 0; i < GCODE_PROGRAM_CAPACITY; i++)
    if(programs[i].program == program) return programs[i].offset;

  return 0;
}

bool splice_input(const char *data) {
  if(!spliced) {
    splice = data;
    spliced = true;
    endOfSplice = false;
    splicep = 0;

    return true;
  } else return false;
}

bool end_of_spliced_input(void) {
  if(endOfSplice) {
    endOfSplice = false;
    return true;
  } else return false;
}

bool done_input(void) {
  if(input) {
    return fclose(input) ? false : true;
  } else {
    display_machine_message("IER: No input to close, ignoring request!");
    return false;
  }

}

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
  rewind(input);

  GCODE_DEBUG("Program reset");

  return true;
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
  if(spliced && splice[splicep]) return splice[splicep++];
  else {
    if(spliced) {
      spliced = false;
      endOfSplice = true;
      free((void *)splice);
    }

    return fgetc(input);
  }
}

bool fetch_line_input(char *line) {
  int c = '\0';
  uint8_t i = 0, j;
  bool ignore = false;
  char commsg[0xFF - 5]; /* max line length - 2 parentheses and "MSG" */

  while(c != EOF) {
    c = toupper(fetch_char_input());

    if(c == '\n' || c == '\r') {
      ignore = false;

      if(c == '\r') {
        c = fetch_char_input();
        if(c != '\n') ungetc(c, input);
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

    if(c == 'N' || c == 'O') {
      int d;

      if(i) display_machine_message("WAR: N or O word not at start of block!");

      j = 0;
      /* read the number, which should be a literal integer */
      d = fetch_char_input();
      while(isdigit(d)) {
        commsg[j++] = d;
        d = fetch_char_input();
      }
      commsg[j] = '\0';
      ungetc(d, input); /* First non-digit character has to go back */

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

    if(line) line[i++] = c; /* Otherwise add to the line buffer */
  }
  if(line) line[i] = '\0';

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
  return fclose(input) ? false : true;
}

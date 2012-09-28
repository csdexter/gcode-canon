/*
 * gcode-input.c
 *
 *  Created on: Aug 11, 2012
 *      Author: csdexter
 */

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
static uint16_t currentLine;
static TGCodeProgramIndexEntry programs[GCODE_PROGRAM_CAPACITY];
static uint8_t programCount;


bool init_input(void *data) {
  char dummy[0xFF];

  input = (FILE *)data;
  memset(&programs, 0x00, sizeof(programs));
  programCount = 0;
  GCODE_DEBUG("Input stream up, %d program table entries available", GCODE_PROGRAM_CAPACITY);
  setlocale(LC_ALL, "C");
  display_machine_message("STA: Scanning input for programs (O words)");
  while(fetch_line_input(dummy));
  rewind_input();

  return true;
}

bool rewind_input(void) {
  rewind(input);
  currentLine = 1;

  GCODE_DEBUG("Program reset");

  return true;
}

bool seek_input(uint16_t lineNumber) {
  int i;
  char *dummy, *result;

  if(lineNumber < currentLine) rewind_input();

  dummy = (char *)malloc(0xFF);

  for(i = currentLine; i < lineNumber && result; i++)
    result = fgets(dummy, 0xFF, input);
  free(dummy);

  GCODE_DEBUG("Seek to line %d", lineNumber);

  return result ? true : false;
}

uint16_t tell_input(void) {
  return currentLine;
}

char fetch_char_input(void) {
  return fgetc(input);
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
      currentLine++;

      if(i) break; /* EOL with data: return line */
      else continue; /* EOL without data: strip empty line */
    }

    if(ignore || c == ' ' || c == '\t') continue; /* Strip whitespace and deleted blocks */

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
      if(!strncmp(commsg, "MSG,", strlen("MSG,"))) display_machine_message(&commsg[3]);

      c = toupper(fetch_char_input()); /* go past the closing parenthesis */

      continue;
    }

    if(c == 'N' || c == 'O') {
      int d;

      if(i) display_machine_message("PER: N or O word not at start of block!");

      j = 0;
      d = fetch_char_input(); /* read the number, which should be a literal integer */
      while(isdigit(d)) {
        commsg[j++] = d;
        d = fetch_char_input();
      }
      commsg[j] = '\0';

      if(c == 'O') /* We ignore N and store a bookmark for O for now */
        if(programCount < GCODE_PROGRAM_CAPACITY) {
          programs[programCount].program = (uint16_t)atol(commsg); /* Better mess with precision than signedness */
          programs[programCount].offset = tell_input() + 1; /* The line immediately after the O word */
          programCount++;
        } else display_machine_message("PER: Program table overflow!");

      c = d; /* First non-digit character has to go back in processing */
    }

    line[i++] = c; /* Otherwise add to the line buffer */
  }
  line[i] = '\0';

  return c == EOF ? false : true;
}

uint16_t get_program_input(uint16_t program) {
  uint8_t i;
  //TODO: if we ever consider storing a big number of these, switch to qsort/bsearch
  for(i = 0; i < GCODE_PROGRAM_CAPACITY; i++)
    if(programs[i].program == program) return programs[i].offset;

  return 0;
}

bool done_input(void) {
  return fclose(input) ? false : true;
}

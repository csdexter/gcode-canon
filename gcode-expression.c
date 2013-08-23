/*
 ============================================================================
 Name        : gcode-expression.c
 Author      : Radu - Eosif Mihailescu
 Version     : 1.0 (2013-08-22)
 Copyright   : (C) 2013 Radu - Eosif Mihailescu <radu.mihailescu@linux360.ro>
 Description : G-Code Expression Evaluator Code
 ============================================================================
 */

#include <ctype.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "gcode-commons.h"
#include "gcode-expression.h"
#include "gcode-debugcon.h"
#include "gcode-state.h"


static double _do_function(char *fname, double arg1, double arg2) {
  if(!strcasecmp(fname, "ABS"))
    return fabs(arg1);
  if(!strcasecmp(fname, "ASIN"))
    return asin(arg1) * GCODE_RAD2DEG;
  if(!strcasecmp(fname, "ACOS"))
    return acos(arg1) * GCODE_RAD2DEG;
  if(!strcasecmp(fname, "ATAN"))
    return atan2(arg1, arg2) * GCODE_RAD2DEG;
  if(!strcasecmp(fname, "COS"))
    return cos(arg1 * GCODE_DEG2RAD);
  if(!strcasecmp(fname, "EXP"))
    return exp(arg1);
  if(!strcasecmp(fname, "FIX"))
    return floor(arg1);
  if(!strcasecmp(fname, "FUP"))
    return ceil(arg1);
  if(!strcasecmp(fname, "LN"))
    return log(arg1);
  if(!strcasecmp(fname, "ROUND"))
    return round(arg1);
  if(!strcasecmp(fname, "SIN"))
    return sin(arg1 * GCODE_DEG2RAD);
  if(!strcasecmp(fname, "SQRT"))
    return sqrt(arg1);
  if(!strcasecmp(fname, "TAN"))
    return tan(arg1 * GCODE_DEG2RAD);

  return 0;
}

double evaluate_expression(const char *expression) {
  //TODO: implement expressions

  return +0.0E+0;
}

void evaluate_unary_expression(char *line) {
  bool seenWord = false;
  char fname[6], *sptr, *buf; /* strlen('ROUND'), longest named function */
  uint8_t fni;
  double arg, sar;

  while(*(line++)) {
    if(isdigit(*line) && seenWord) {
      line = skip_gcode_digits(line);
      seenWord = false;
    }
    if(isalpha(*line)) {
      if(seenWord) {
        sptr = line; /* Remember where it started */
        fni = 0;
        do { fname[fni++] = *line; } while (isalpha(*(++line)));
        fname[fni] = '\0'; /* Function name at fname */
        arg = read_gcode_real(line); /* Function (1st) argument in arg */
        line = skip_gcode_digits(line);
        if(*line == '/') {/* Special case of ATAN */
          sar = read_gcode_real(++line);
          line = skip_gcode_digits(line);
        }

        buf = (char *)calloc(1, strlen(line) + strlen("4.2f") + 1);
        strcpy(buf, "4.2f");
        strcat(buf, line); /* Save the rest of the line */
        line = sptr; /* Rewind to where the function started */
        /* Overwrite with numeric result */
        snprintf(line, strlen(line), buf, _do_function(fname, arg, sar));
        /* And go past it */
        line = skip_gcode_digits(line);
      }
      else seenWord = true;
    }
  }

  return;
}

/*
 ============================================================================
 Name        : gcode-parameters.c
 Author      : Radu - Eosif Mihailescu
 Version     : 1.0 (2012-08-12)
 Copyright   : (C) 2012 Radu - Eosif Mihailescu <radu.mihailescu@linux360.ro>
 Description : G-Code Parameter Address Handling Code
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gcode-commons.h"
#include "gcode-parameters.h"
#include "gcode-debugcon.h"
#include "gcode-machine.h"


typedef struct {
  uint16_t index;
  double value;
} TGCodePendingParameterUpdate;

static FILE *parameterStore;
static double parameters[GCODE_PARAMETER_COUNT] = {0.0};
static TGCodePendingParameterUpdate parameterUpdates[GCODE_PARAMETER_UPDATES];
static uint8_t parameterUpdateCount;


bool init_parameters(void *data) {
  char *line = (char *)malloc(0xFF), *key;
  int i;

  parameterStore = (FILE *)data;

  /* binary 0x00 may not always result in float +0.0E+0, so do it by hand */
  for(i = 0; i < GCODE_PARAMETER_COUNT; i++) parameters[i] = 0.0;
  for(i = 0; i < GCODE_PARAMETER_UPDATES; i++) {
    parameterUpdates[i].index = 0;
    parameterUpdates[i].value = 0.0;
  }

  if(parameterStore) {
    rewind(parameterStore);
    i = 0;
    while(fgets(line, 0xFF, parameterStore)) {
      key = strtok(line, ","); // Force execution order
      parameters[atoi(key)] = atof(strtok(NULL, ","));
      i++;
    }
    free(line);
    GCODE_DEBUG("%d parameters restored from non-volatile storage, %d available",
                i, GCODE_PARAMETER_COUNT);
  } else display_machine_message("WAR: Parameter store void, using defaults!");

  /* Now handle the special cases */
  parameters[0] = +0.0E+0; /* #0 is always zero */
  /* #3004,3007,5161-5169,5181-5189: will be set by init_machine() */
  /* #71,3005,4001-4018,5211-5219,5220: will be set by init_gcode_state() */

  return true;
}

double fetch_parameter(uint16_t index) {
  return parameters[index];
}

bool update_parameter(uint16_t index, double newValue) {
  // #0 is readonly and there's only 5400 of them
  if(!index || index > GCODE_PARAMETER_COUNT) return false;

  if(parameterUpdateCount < GCODE_PARAMETER_UPDATES) {
    parameterUpdateCount++;
    parameterUpdates[parameterUpdateCount - 1].index = index;
    parameterUpdates[parameterUpdateCount - 1].value = newValue;

    return true;
  } else {
    display_machine_message("PER: Parameter update buffer overflow!");

    return false;
  }
}

bool set_parameter(uint16_t index, double newValue) {
  // #0 is readonly and there's only 5400 of them
  if(!index || index > GCODE_PARAMETER_COUNT) return false;

  parameters[index] = newValue;

  return true;
}

bool commit_parameters(void){
  int i;

  for(i = 0; i < parameterUpdateCount; i++) {
    parameters[parameterUpdates[i].index] = parameterUpdates[i].value;
    GCODE_DEBUG("#%d = %4.2f", parameterUpdates[i].index,
                parameterUpdates[i].value);
  }
  parameterUpdateCount = 0;

  return true;
}

bool done_parameters(void) {
  int i, j = 0;

  parameterStore = freopen(GCODE_PARAMETER_STORE, "w", parameterStore);
  for(i = 500; i < GCODE_PARAMETER_COUNT; i++)
    /* Our parameter store defaults to 0.0E+0 on initialization so we wouldn't
     * want to write a file full of zeros. Therefore we extend the standard's
     * convention for integer-float equivalence to the special case of 0.0E+0:
     * if the value stored in a parameter is closer than 0.0001 to 0.0E+0 then
     * we coerce it to 0.0E+0 and subsequently don't save it */
    if(parameters[i] > GCODE_INTEGER_THRESHOLD ||
       parameters[i] < -GCODE_INTEGER_THRESHOLD) {
      fprintf(parameterStore, "%4d," GCODE_REAL_FORMAT "\n", i, parameters[i]);
      j++;
    }
  GCODE_DEBUG("Saved %d non-null parameter values to non-volatile storage for shutdown", j);

  return fclose(parameterStore) ? false : true;
}

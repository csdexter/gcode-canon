/*
 * gcode-parameters.c
 *
 *  Created on: Aug 12, 2012
 *      Author: csdexter
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gcode-commons.h"
#include "gcode-parameters.h"
#include "gcode-debugcon.h"

static FILE *parameterStore;
static double parameters[GCODE_PARAMETER_COUNT] = {0.0};

typedef struct {
  uint16_t index;
  double value;
} TGCodePendingParameterUpdate;

static TGCodePendingParameterUpdate parameterUpdates[GCODE_PARAMETER_UPDATES];
static uint8_t parameterUpdateCount;

bool init_parameters(void *data) {
  char *line = (char *)malloc(0xFF), *key;
  int i;

  parameterStore = (FILE *)data;

  /* binary 0x00 may not result in float +0.0E+1, so do it by hand */
  for(i = 0; i < GCODE_PARAMETER_COUNT; i++) parameters[i] = 0.0;
  for(i = 0; i < GCODE_PARAMETER_UPDATES; i++) {
    parameterUpdates[i].index = 0;
    parameterUpdates[i].value = 0.0;
  }

  rewind(parameterStore);
  i = 0;
  while(fgets(line, 0xFF, parameterStore)) {
    key = strtok(line, ","); // Force execution order
    parameters[atoi(key)] = atof(strtok(NULL, ","));
    i++;
  }
  free(line);
  GCODE_DEBUG("%d parameters restored from non-volatile storage, %d available", i, GCODE_PARAMETER_COUNT);

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
  if(!index || index > 5400) return false; // #0 is readonly and there's only 5400 of them

  if(parameterUpdateCount < GCODE_PARAMETER_UPDATES) {
    parameterUpdateCount++;
    parameterUpdates[parameterUpdateCount - 1].index = index;
    parameterUpdates[parameterUpdateCount - 1].value = newValue;

    return true;
  } else return false;
}

bool set_parameter(uint16_t index, double newValue) {
  if(!index || index > 5400) return false; // #0 is readonly and there's only 5400 of them

  parameters[index] = newValue;

  return true;
}

bool commit_parameters(void){
  int i;

  for(i = 0; i < parameterUpdateCount; i++) {
    parameters[parameterUpdates[i].index] = parameterUpdates[i].value;
    GCODE_DEBUG("#%d = %4.2f", parameterUpdates[i].index, parameterUpdates[i].value);
  }
  parameterUpdateCount = 0;

  return true;
}

bool done_parameters(void) {
  int i, j = 0;

  parameterStore = freopen(GCODE_PARAMETER_STORE, "w", parameterStore);
  for(i = 500 - 1; i < GCODE_PARAMETER_COUNT; i++)
    if(!(parameters[i] < 0.0001) || !(parameters[i] > -0.0001)) {
      fprintf(parameterStore, "%4d,%4.2f\n", i, parameters[i]);
      j++;
    }
  GCODE_DEBUG("Saved %d non-null parameter values to non-volatile storage for shutdown", j);

  return fclose(parameterStore) ? false : true;
}

/*
 * gcode-parameters.h
 *
 *  Created on: Aug 11, 2012
 *      Author: csdexter
 */

#ifndef GCODE_PARAMETERS_H_
#define GCODE_PARAMETERS_H_


#include "gcode-commons.h"

#include <stdbool.h>
#include <stdint.h>

/* Initialize parameter store, takes anonymous pointer to opaque data store, returns true on success */
bool init_parameters(void *data);
/* Fetch parameter index as double (according to the standard, all are double) */
double fetch_parameter(uint16_t index);
/* Queue parameter index for update with newValue, returns true if ok and false if index is readonly */
bool update_parameter(uint16_t index, double newValue);
/* Commit all update_parameter() changes to permanent store, returns false if any error occurred when accessing the data store */
bool commit_parameters(void);
/* Save all persistent parameters to data store in preparation for shutdown, returns false if any errors occur */
bool done_parameters(void);

#endif /* GCODE_PARAMETERS_H_ */

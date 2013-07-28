/*
 ============================================================================
 Name        : gcode-parameters.h
 Author      : Radu - Eosif Mihailescu
 Version     : 1.0 (2012-08-11)
 Copyright   : (C) 2012 Radu - Eosif Mihailescu <radu.mihailescu@linux360.ro>
 Description : G-Code Parameter Address Handling API Header
 ============================================================================
 */

#ifndef GCODE_PARAMETERS_H_
#define GCODE_PARAMETERS_H_


#include "gcode-commons.h"

#include <stdbool.h>
#include <stdint.h>


/* Initialize parameter store, takes anonymous pointer to opaque data store,
 * returns true on success */
bool init_parameters(void *data);
/* Fetch parameter index as double (according to the standard) */
double fetch_parameter(uint16_t index);
/* Queue parameter index for update with newValue, returns true if ok and false
 * if index is readonly */
bool update_parameter(uint16_t index, double newValue);
/* Immediately update parameter index with newValue, returns true if ok and
 * false if index is readonly */
bool set_parameter(uint16_t index, double newValue);
/* Commit all update_parameter() changes to permanent store, returns false if
 * any error occurred when accessing the data store */
bool commit_parameters(void);
/* Save all persistent parameters to data store in preparation for shutdown,
 * returns false if any errors occur */
bool done_parameters(void);


#endif /* GCODE_PARAMETERS_H_ */

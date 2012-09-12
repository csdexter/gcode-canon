/*
 * gcode-parameters.h
 *
 *  Created on: Aug 11, 2012
 *      Author: csdexter
 */

#ifndef GCODE_PARAMETERS_H_
#define GCODE_PARAMETERS_H_


#include "gcode-commons.h"

#define GCODE_PARM_FEED_HOME_X 2001
#define GCODE_PARM_FEED_HOME_Y 2002
#define GCODE_PARM_FEED_HOME_Z 2003
#define GCODE_PARM_FEED_HOME_A 2004
#define GCODE_PARM_FEED_HOME_B 2005
#define GCODE_PARM_FEED_HOME_C 2006


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

/*
 * gcode-commons.h
 *
 *  Created on: Aug 11, 2012
 *      Author: csdexter
 */

#ifndef GCODE_COMMONS_H_
#define GCODE_COMMONS_H_

#include <stdbool.h>
#include <stdint.h>

/* How many discrete O words we support */
#define GCODE_PROGRAM_CAPACITY 16

/* Where is our parameter store */
#define GCODE_PARAMETER_STORE "parameters.csv"
/* How many parameters we support */
#define GCODE_PARAMETER_COUNT 5400
/* How many parameter updates in a single line we support */
#define GCODE_PARAMETER_UPDATES 53

/* How many tools we support */
#define GCODE_TOOL_COUNT 100
/* Where is the tool data stored */
#define GCODE_TOOL_TYPE_BASE 3200
#define GCODE_TOOL_DIAM_BASE 3300
#define GCODE_TOOL_LEN_BASE 3400

/* Well known parameter numbers */
#define GCODE_PARM_FEED_HOME_X 2001
#define GCODE_PARM_FEED_HOME_Y 2002
#define GCODE_PARM_FEED_HOME_Z 2003
#define GCODE_PARM_FIRST_WCS 5221
#define GCODE_PARM_WCS_SIZE 20

/* How many nested macro calls we support */
#define GCODE_MACRO_COUNT 16
/* How many nested subprogram calls we support */
#define GCODE_SUBPROGRAM_COUNT 16


#endif /* GCODE_COMMONS_H_ */

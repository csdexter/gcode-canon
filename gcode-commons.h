/*
 ============================================================================
 Name        : gcode-commons.h
 Author      : Radu - Eosif Mihailescu
 Version     : 1.0 (2012-08-11)
 Copyright   : (C) 2012 Radu - Eosif Mihailescu <radu.mihailescu@linux360.ro>
 Description : Program-wide Generic Use Symbolic Constants
 ============================================================================
 */

#ifndef GCODE_COMMONS_H_
#define GCODE_COMMONS_H_


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
#define GCODE_PARM_SCALING 71
#define GCODE_PARM_FEED_HOME_X 2001
#define GCODE_PARM_FEED_HOME_Y 2002
#define GCODE_PARM_FEED_HOME_Z 2003
#define GCODE_PARM_BITFIELD1 3004
#define GCODE_PARM_BITFIELD2 3005
#define GCODE_PARM_CURRENT_PALLET 3007
#define GCODE_PARM_CURRENT_TOOL 3101
/* Absolute GCode coordinates at end of current block */
#define GCODE_PARM_FIRST_CEOB 5001
#define GCODE_PARM_FIRST_OFFSET 5081
#define GCODE_PARM_FIRST_HOME 5161
#define GCODE_PARM_FIRST_ZERO 5181
#define GCODE_PARM_FIRST_LOCAL 5211
#define GCODE_PARM_CURRENT_WCS 5220
#define GCODE_PARM_FIRST_WCS 5221
#define GCODE_PARM_WCS_SIZE 20

/* How many nested macro calls we support */
#define GCODE_MACRO_COUNT 16
/* How many nested subprogram calls we support */
#define GCODE_SUBPROGRAM_COUNT 16

/* Handy for trigonometry */
#define GCODE_DEG2RAD 0.0174532925199
#define GCODE_RAD2DEG 57.2957795131
/* Handy for measurements */
#define GCODE_INCH2MM 25.4

/* Handy for batch/vector processing */
#define GCODE_AXIS_X 0
#define GCODE_AXIS_Y 1
#define GCODE_AXIS_Z 2


#endif /* GCODE_COMMONS_H_ */

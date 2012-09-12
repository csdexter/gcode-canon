/*
 * gcode-debugcon.h
 *
 *  Created on: Aug 11, 2012
 *      Author: csdexter
 */

#ifndef GCODE_DEBUGCON_H_
#define GCODE_DEBUGCON_H_

#include <libgen.h>

#define GCODE_DEBUG(...) printf("[%s](%s@%d): ", basename(__FILE__), __FUNCTION__, __LINE__); \
	printf(__VA_ARGS__); \
	printf("\n")

#endif /* GCODE_DEBUGCON_H_ */

/*
 * gcode-math.h
 *
 *  Created on: Aug 16, 2012
 *      Author: csdexter
 */

#ifndef GCODE_MATH_H_
#define GCODE_MATH_H_

#include "gcode-commons.h"
#include "gcode-state.h"


/* Coordinate math workhorse. Transforms X,Y,Z according to all information in
 * system and stores the result in system->X, system->Y, system->Z. Returns
 * false if any [math] error occurs */
bool do_WCS_move_math(TGCodeCoordinateInfo *system, double X, double Y, double Z);
/* Same, for cycles */
bool do_WCS_cycle_math(TGCodeCoordinateInfo *system, double X, double Y, double Z, double R);
/* Transform value from imperial to metric if the current state says so */
double to_metric_math(const TGCodeCoordinateInfo system, const double value);

#endif /* GCODE_MATH_H_ */

/*
 ============================================================================
 Name        : gcode-math.h
 Author      : Radu - Eosif Mihailescu
 Version     : 1.0 (2012-08-16)
 Copyright   : (C) 2012 Radu - Eosif Mihailescu <radu.mihailescu@linux360.ro>
 Description : Coordinate System Transformations API Header
 ============================================================================
 */

#ifndef GCODE_MATH_H_
#define GCODE_MATH_H_


#include "gcode-commons.h"
#include "gcode-state.h"


/* Coordinate pre-processing: de-inch, apply WCS and LCS and absolutize */
double do_G_coordinate_math(const TGCodeCoordinateInfo *system, double input,
    const double offset, const double previous, const uint8_t axis);
/* Coordinate math workhorse. Transforms X,Y,Z according to all information in
 * system and stores the result in system->X, system->Y, system->Z. Returns
 * false if any [math] error occurs */
bool do_WCS_move_math(TGCodeCoordinateInfo *system, double X, double Y,
    double Z);
/* Transform value from imperial to metric if the current state says so */
double to_metric_math(const TGCodeCoordinateInfo system, const double value);


#endif /* GCODE_MATH_H_ */

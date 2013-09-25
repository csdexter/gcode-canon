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

#include <stdbool.h>

#include "gcode-commons.h"
#include "gcode-state.h"


/* Coordinate pre-processing: de-inch, apply WCS and LCS and absolutize */
double do_G_coordinate_math(const TGCodeCoordinateInfo *system, double input,
    const double offset, const double previous, const uint8_t axis);
/* NEW STUFF BELOW */
/* Returns input iff input is not NAN, otherwise returns last */
double current_or_last_math(double input, double last);
/* Returns value if !missing, otherwise last if absolute or zero otherwise */
double current_or_zero_math(double value, double last, bool absolute,
    bool missing);
/* Returns input if absolute, input + origin otherwise */
double relative_math(double input, double origin, bool absolute);
/* Performs coordinate system calculations (MCS, WCS, LCS) */
double system_math(double input, bool MCS, double offset, double origin);
/* Performs length compensation on input according to comp */
double length_comp_math(double input, TGCodeCompSpec comp);
/* Translates to metric if inch is true */
double inch_math(double input, bool inch);
/* Transforms from polar to Cartesian coordinates */
void polar_math(double radius, double theta, double *X, double *Y);
/* Transforms (inputX, inputY) by rotation around (originX, originY) of theta
 * degrees */
void rotation_math(double inputX, double inputY, double theta, double originX,
    double originY, double *X, double *Y);
/* Transforms input by scaling around origin by factor */
double scaling_math(double input, double origin, double factor);
/* Coordinate math workhorse. Transforms X,Y,Z according to all information in
 * system and stores the result in system->X, system->Y, system->Z. */
void move_math(TGCodeCoordinateInfo *system, double X, double Y, double Z);


#endif /* GCODE_MATH_H_ */

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
/* Transform value from imperial to metric if the current state says so */
double to_metric_math(const TGCodeCoordinateInfo system, const double value);
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
/* Performs radius compensation on input[XY] according to comp and side */
void radius_comp_math(double inputX, double inputY, TGCodeCompSpec comp,
    bool side, double *X, double *Y);
/* Translates to metric if inch is true */
double inch_math(double input, bool inch);
/* Transforms from polar to Cartesian coordinates */
void polar_math(double radius, double theta, double *X, double *Y);
/* Given the vector (x1, y1) -> (x2, y2) returns true if the vector's right
 * coincides with the coordinate system's right */
bool vector_side_math(double x1, double y1, double x2, double y2);
/* Transforms (inputX, inputY) by rotation around (originX, originY) of theta
 * degrees */
void rotation_math(double inputX, double inputY, double theta, double originX,
    double originY, double *X, double *Y);
/* Transforms input by scaling around origin by factor */
double scaling_math(double input, double origin, double factor);
/* New version of do_WCS_move_math() */
void move_math(TGCodeCoordinateInfo *system, double X, double Y, double Z);


#endif /* GCODE_MATH_H_ */

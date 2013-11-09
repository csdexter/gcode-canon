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
#include <stdint.h>

#include "gcode-commons.h"
#include "gcode-state.h"
#include "gcode-queue.h"


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
/* Transforms changes in input wrt. original by mirroring them if mirrored is
 * true. Saves input in original. */
double mirroring_math(double input, double previous, double *original,
    bool mirrored);
/* Calculates I,J,K from R or R from I,J,K given start and end of arc. Invert
 * is ccw XOR the_other_way_around. Returns linear length of arc. */
double arc_math(double X, double Y, double oldX, double oldY, double *R,
    double *I, double *J, double *K, bool invert);
/* Coordinate math workhorse. Transforms X,Y,Z according to all information in
 * system and stores the result in system->X, system->Y, system->Z. */
void move_math(TGCodeCoordinateInfo *system, double X, double Y, double Z);
/* Returns the side of (x1,y1)->(x2,y2) (x3,y3) is on as a radius compensation
 * mode. */
TGCodeRadCompMode vector_side_math(double x1, double y1, double x2, double y2,
    double x3, double y3);
/* Calculates the offset of thisMove according to the given radius compensation
 * mode specification. Returns the new target in a TGCodeMoveSpec copied from
 * thisMove, the new origin is at (originX,originY) */
TGCodeMoveSpec offset_math(TGCodeMoveSpec prevMove, TGCodeMoveSpec thisMove,
    TGCodeCompSpec radComp, double *originX, double *originY);
/* Calculates the intersection of (opX,opY)->prevMove and (otX,otY)->thisMove
 * closer to prevMove.target */
void intersection_math(double opX, double opY, TGCodeMoveSpec prevMove,
    double otX, double otY, TGCodeMoveSpec thisMove, double *iX, double *iY);
/* Calculates whether prevMove->thisMove represents a corner towards or against
 * the radius compensation side radComp. Returns true for towards. */
bool inside_corner_math(double oX, double oY, TGCodeMoveSpec prevMove,
    TGCodeMoveSpec thisMove, TGCodeCompSpec radComp);

#endif /* GCODE_MATH_H_ */

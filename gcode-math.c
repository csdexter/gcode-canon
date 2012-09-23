/*
 * gcode-math.c
 *
 *  Created on: Aug 16, 2012
 *      Author: csdexter
 */

#include "gcode-math.h"
#include "gcode-commons.h"

#include <math.h>

bool do_WCS_move_math(TGCodeCoordinateInfo *system, double X, double Y, double Z) {
  if(!isnan(X)) system->X = X; /* Dummy implementation */
  if(!isnan(Y)) system->Y = Y;
  if(!isnan(Z)) system->Z = Z;

  return true;
}

bool do_WCS_cycle_math(TGCodeCoordinateInfo *system, double X, double Y, double Z, double R) {
  if(system->absolute == GCODE_ABSOLUTE) do_WCS_move_math(system, X, Y, Z);
  if(!isnan(R)) system->R = R; /* Dummy implementation */

  return true;
}

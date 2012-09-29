/*
 * gcode-math.c
 *
 *  Created on: Aug 16, 2012
 *      Author: csdexter
 */

#include "gcode-math.h"
#include "gcode-commons.h"
#include "gcode-parameters.h"

#include <math.h>

bool do_WCS_move_math(TGCodeCoordinateInfo *system, double X, double Y, double Z) {
  //TODO: establish order in which transformations should apply
  if(!isnan(X)) {
    if(system->units == GCODE_UNITS_INCH) X *= 25.4;
    if(system->absolute == GCODE_ABSOLUTE)
      system->X = fetch_parameter(GCODE_PARM_FIRST_WCS + (system->current - GCODE_WCS_1) * GCODE_PARM_WCS_SIZE + 0) +
          system->offset.X + X;
    else system->X += X;
  }
  if(!isnan(Y)) {
    if(system->units == GCODE_UNITS_INCH) Y *= 25.4;
    if(system->absolute == GCODE_ABSOLUTE)
      system->Y = fetch_parameter(GCODE_PARM_FIRST_WCS + (system->current - GCODE_WCS_1) * GCODE_PARM_WCS_SIZE + 1) +
          system->offset.Y + Y;
    else system->Y += Y;
  }
  if(!isnan(Z)) {
    if(system->units == GCODE_UNITS_INCH) Z *= 25.4;
    if(system->absolute == GCODE_ABSOLUTE)
      system->Z = fetch_parameter(GCODE_PARM_FIRST_WCS + (system->current - GCODE_WCS_1) * GCODE_PARM_WCS_SIZE + 2) +
          system->offset.Z + Z;
    else system->Z += Z;
  }

  return true;
}

bool do_WCS_cycle_math(TGCodeCoordinateInfo *system, double X, double Y, double Z, double R) {
  if(system->absolute == GCODE_ABSOLUTE) do_WCS_move_math(system, X, Y, Z);
  if(!isnan(R)) system->R = R; /* Dummy implementation */

  return true;
}

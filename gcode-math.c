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
  //TODO: also provide for G53 (a.k.a. WCS0, a.k.a. MCS)
  //Apply geometric transformations
  if(system->cartesian == GCODE_CARTESIAN) {
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
  } else {
    //TODO: provide for "use the last value" when using polar coordinates
    system->X += X * cos(Y * 0.0174532925);
    system->Y += X * sin(Y * 0.0174532925);
  }
  if(!isnan(Z)) {
    //Apply length compensation first, by definition only along the spindle axis
    //and by definition that is only Z
    if(system->lenComp.mode != GCODE_COMP_LEN_OFF) {
      if(system->lenComp.mode == GCODE_COMP_LEN_P) Z += system->lenComp.offset;
      else Z -= system->lenComp.offset;
    }
    //Then do the geometric transformations
    if(system->units == GCODE_UNITS_INCH) Z *= 25.4;
    if(system->absolute == GCODE_ABSOLUTE)
      system->Z = fetch_parameter(GCODE_PARM_FIRST_WCS + (system->current - GCODE_WCS_1) * GCODE_PARM_WCS_SIZE + 2) +
          system->offset.Z + Z;
    else system->Z += Z;
  }

  //Apply coordinate system rotation
  //TODO: Make sure we're not overwriting values, duplicate variables as needed
  if(system->rotation.mode == GCODE_ROTATION_ON) {
    //TODO: Support all planes, now assumes XY
    system->X = system->rotation.X +
        hypot(system->X - system->rotation.X, system->Y - system->rotation.Y) *
        cos(system->rotation.R * 0.0174532925 +
            atan2(system->X - system->rotation.X, system->Y - system->rotation.Y));
    system->Y = system->rotation.Y +
        hypot(system->X - system->rotation.X, system->Y - system->rotation.Y) *
        sin(system->rotation.R * 0.0174532925 +
            atan2(system->X - system->rotation.X, system->Y - system->rotation.Y));
  }

  //Apply scaling
  if(system->scaling.mode == GCODE_SCALING_ON) {
    system->X += (system->X - system->scaling.X) * system->scaling.I;
    system->Y += (system->Y - system->scaling.Y) * system->scaling.J;
    system->Z += (system->Z - system->scaling.Z) * system->scaling.K;
  }

  return true;
}

bool do_WCS_cycle_math(TGCodeCoordinateInfo *system, double X, double Y, double Z, double R) {
  if(system->absolute == GCODE_ABSOLUTE) do_WCS_move_math(system, X, Y, Z);
  if(!isnan(R)) system->R = R; /* Dummy implementation */

  return true;
}

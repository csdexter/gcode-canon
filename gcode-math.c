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
        X = fetch_parameter(GCODE_PARM_FIRST_WCS + (system->current - GCODE_WCS_1) * GCODE_PARM_WCS_SIZE + 0) +
            system->offset.X + X;
      else X += system->X;
    } else X = system->X;
    if(!isnan(Y)) {
      if(system->units == GCODE_UNITS_INCH) Y *= 25.4;
      if(system->absolute == GCODE_ABSOLUTE)
        Y = fetch_parameter(GCODE_PARM_FIRST_WCS + (system->current - GCODE_WCS_1) * GCODE_PARM_WCS_SIZE + 1) +
            system->offset.Y + Y;
      else Y += system->Y;
    } else Y = system->Y;
  } else {
    double pX, pY = Y;

    if(!isnan(X) && system->units == GCODE_UNITS_INCH) X *= 25.4;
    pX = X;
    X = system->X + (isnan(X) ? system->pR : X) * cos((isnan(Y) ? system->pT : Y) * 0.0174532925);
    Y = system->Y + (isnan(X) ? system->pR : X) * sin((isnan(Y) ? system->pT : Y) * 0.0174532925);
    if(!isnan(pX)) system->pR = pX;
    if(!isnan(pY)) system->pT = pY;
  }
  if(!isnan(Z)) {
    //Apply length compensation first, by definition only along the spindle axis
    //and by definition that is only Z
    if(system->lenComp.mode != GCODE_COMP_LEN_OFF) {
      if(system->lenComp.mode == GCODE_COMP_LEN_P) Z += system->lenComp.offset;
      else Z -= system->lenComp.offset;
    }
    //Length compensation is measurement system -agnostic, i.e. "3.0" will be
    //in or mm above in both what the user gave us for Z *and* the compensation
    //value currently in effect.
    if(system->units == GCODE_UNITS_INCH) Z *= 25.4;
    //Then do the geometric transformations
    if(system->absolute == GCODE_ABSOLUTE)
      Z = fetch_parameter(GCODE_PARM_FIRST_WCS + (system->current - GCODE_WCS_1) * GCODE_PARM_WCS_SIZE + 2) +
          system->offset.Z + Z;
    else Z += system->Z;
  } else Z = system->Z;

  //Apply coordinate system rotation
  if(system->rotation.mode == GCODE_ROTATION_ON) {
    switch(system->plane) {
      case GCODE_PLANE_XY:
        X = system->rotation.X +
            hypot(X - system->rotation.X, Y - system->rotation.Y) *
            cos(system->rotation.R * 0.0174532925 +
                atan2(X - system->rotation.X, Y - system->rotation.Y));
        Y = system->rotation.Y +
            hypot(X - system->rotation.X, Y - system->rotation.Y) *
            sin(system->rotation.R * 0.0174532925 +
                atan2(X - system->rotation.X, Y - system->rotation.Y));
        break;
      case GCODE_PLANE_YZ:
        Y = system->rotation.Y +
            hypot(Y - system->rotation.Y, Z - system->rotation.Z) *
            cos(system->rotation.R * 0.0174532925 +
                atan2(Y - system->rotation.Y, Z - system->rotation.Z));
        Z = system->rotation.Z +
            hypot(Y - system->rotation.Y, Z - system->rotation.Z) *
            sin(system->rotation.R * 0.0174532925 +
                atan2(Y - system->rotation.Y, Z - system->rotation.Z));
        break;
      case GCODE_PLANE_ZX:
        Z = system->rotation.Z +
            hypot(Z - system->rotation.Z, X - system->rotation.X) *
            cos(system->rotation.R * 0.0174532925 +
                atan2(Z - system->rotation.Z, X - system->rotation.X));
        X = system->rotation.X +
            hypot(Z - system->rotation.Z, X - system->rotation.X) *
            sin(system->rotation.R * 0.0174532925 +
                atan2(Z - system->rotation.Z, X - system->rotation.X));
        break;
    }
  }

  //Apply scaling
  if(system->scaling.mode == GCODE_SCALING_ON) {
    X = (X - system->scaling.X) * system->scaling.I;
    Y = (Y - system->scaling.Y) * system->scaling.J;
    Z = (Z - system->scaling.Z) * system->scaling.K;
  }

  //We're finally done, update the processed coordinates
  system->X = X;
  system->Y = Y;
  system->Z = Z;

  return true;
}

bool do_WCS_cycle_math(TGCodeCoordinateInfo *system, double X, double Y, double Z, double R) {
  if(system->absolute == GCODE_ABSOLUTE) do_WCS_move_math(system, X, Y, Z);
  if(!isnan(R)) system->R = R; /* Dummy implementation */

  return true;
}

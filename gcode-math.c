/*
 ============================================================================
 Name        : gcode-math.c
 Author      : Radu - Eosif Mihailescu
 Version     : 1.0 (2012-08-16)
 Copyright   : (C) 2012 Radu - Eosif Mihailescu <radu.mihailescu@linux360.ro>
 Description : Coordinate System Transformations Code
 ============================================================================
 */

#include "gcode-math.h"
#include "gcode-commons.h"
#include "gcode-parameters.h"

#include <math.h>
#include <stdbool.h>


double do_G_coordinate_math(const TGCodeCoordinateInfo *system, double input,
    const double offset, const double previous, const uint8_t axis) {
  if(!isnan(input)) {
    input = to_metric_math(*system, input);
    if(system->absolute == GCODE_ABSOLUTE) {
      return (
          system->current == GCODE_MCS ?
              0.0 :
              fetch_parameter(
                  GCODE_PARM_FIRST_WCS +
                  (system->current - GCODE_WCS_1) *
                  GCODE_PARM_WCS_SIZE + axis) + offset) +
          input;
    } else return previous + input;
  } else return previous;
}

bool do_WCS_move_math(TGCodeCoordinateInfo *system, double X, double Y,
    double Z) {
  //Apply geometric transformations
  if(system->cartesian == GCODE_CARTESIAN) {
    system->gX = X = do_G_coordinate_math(system, X, system->offset.X,
                                          system->gX, GCODE_AXIS_X);
    system->gY = Y = do_G_coordinate_math(system, Y, system->offset.Y,
                                          system->gY, GCODE_AXIS_Y);
  } else {
    double pX, pY = Y;

    if(!isnan(X) && system->units == GCODE_UNITS_INCH) X *= 25.4;
    pX = X;
    X = system->X + (isnan(pX) ? system->pR : pX) *
        cos((isnan(pY) ? system->pT : pY) * GCODE_DEG2RAD);
    Y = system->Y + (isnan(pX) ? system->pR : pX) *
        sin((isnan(pY) ? system->pT : pY) * GCODE_DEG2RAD);
    if(!isnan(pX)) system->pR = pX;
    if(!isnan(pY)) system->pT = pY;
    system->gX = X;
    system->gY = Y;
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
  }
  system->gZ = Z = do_G_coordinate_math(system, Z, system->offset.Z,
                                        system->gZ, GCODE_AXIS_Z);

  //Apply coordinate system rotation
  if(system->rotation.mode == GCODE_ROTATION_ON) {
    switch(system->plane) {
      /* reusing g[XYZ] instead of [XYZ] so that no temporary variables are needed */
      case GCODE_PLANE_XY:
        X = cos(system->rotation.R * GCODE_DEG2RAD) *
            (system->gX - system->rotation.X) -
            sin(system->rotation.R * GCODE_DEG2RAD) *
            (system->gY - system->rotation.Y) + system->rotation.X;
        Y = sin(system->rotation.R * GCODE_DEG2RAD) *
            (system->gX - system->rotation.X) +
            cos(system->rotation.R * GCODE_DEG2RAD) *
            (system->gY - system->rotation.Y) + system->rotation.Y;
        break;
      case GCODE_PLANE_YZ:
        Y = cos(system->rotation.R * GCODE_DEG2RAD) *
        (system->gY - system->rotation.Y) -
            sin(system->rotation.R * GCODE_DEG2RAD) *
            (system->gZ - system->rotation.Z) + system->rotation.Y;
        Z = sin(system->rotation.R * GCODE_DEG2RAD) *
            (system->gY - system->rotation.Y) +
            cos(system->rotation.R * GCODE_DEG2RAD) *
            (system->gZ - system->rotation.Z) + system->rotation.Z;
        break;
      case GCODE_PLANE_ZX:
        Z = cos(system->rotation.R * GCODE_DEG2RAD) *
        (system->gZ - system->rotation.Z) -
            sin(system->rotation.R * GCODE_DEG2RAD) *
            (system->gX - system->rotation.X) + system->rotation.Z;
        X = sin(system->rotation.R * GCODE_DEG2RAD) *
            (system->gZ - system->rotation.Z) +
            cos(system->rotation.R * GCODE_DEG2RAD) *
            (system->gX - system->rotation.X) + system->rotation.X;
        break;
    }
  }

  //Apply scaling
  if(system->scaling.mode == GCODE_SCALING_ON) {
    X = system->scaling.X + (X - system->scaling.X) * system->scaling.I;
    Y = system->scaling.Y + (Y - system->scaling.Y) * system->scaling.J;
    Z = system->scaling.Z + (Z - system->scaling.Z) * system->scaling.K;
  }

  //We're finally done, update the processed coordinates
  system->X = X;
  system->Y = Y;
  system->Z = Z;

  return true;
}

double to_metric_math(const TGCodeCoordinateInfo system, const double value) {
  if(isnan(value)) return value;
  else return (system.units == GCODE_UNITS_INCH ? value * GCODE_INCH2MM : value);
}

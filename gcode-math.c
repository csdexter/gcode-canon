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

double current_or_last_math(double input, double last) {
  if(isnan(input))
    return last;
  else
    return input;
}

double relative_math(double input, double origin, bool absolute) {
    if(absolute)
      return input;
    else
      return input + origin;
}

double system_math(double input, bool MCS, double offset, double origin) {
  if(MCS)
    return input;
  else
    return origin + offset + input;
}

double length_comp_math(double input, TGCodeCompSpec comp) {
  if(comp.mode == GCODE_COMP_LEN_OFF)
    return input;
  else
    if(comp.mode == GCODE_COMP_LEN_P)
      return input + comp.offset;
    else
      return input - comp.offset;
}

void radius_comp_math(double inputX, double inputY, TGCodeCompSpec comp,
    bool side, double *X, double *Y) {
  //TODO: proper implementation with proper math, this one's a dummy
  if(comp.mode == GCODE_COMP_RAD_OFF) {
    *X = inputX;
    *Y = inputY;
  } else
    if(((comp.mode == GCODE_COMP_RAD_L) && side) ||
       ((comp.mode == GCODE_COMP_RAD_R) && !side)) {
      *X = inputX + comp.offset;
      *Y = inputY + comp.offset;
    } else {
      *X = inputX - comp.offset;
      *Y = inputY - comp.offset;
    }
}

double inch_math(double input, bool inch) {
  if(inch)
    return input * GCODE_INCH2MM;
  else
    return input;
}

void polar_math(double radius, double theta, double *X, double *Y) {
  *X = radius * cos(theta * GCODE_DEG2RAD);
  *Y = radius * sin(theta * GCODE_DEG2RAD);
}

bool vector_side_math(double x1, double y1, double x2, double y2) {
  //TODO: implement this properly, this is only a dummy

  return true;
}

void rotation_math(double inputX, double inputY, double theta, double originX,
    double originY, double *X, double *Y) {
  *X = cos(theta * GCODE_DEG2RAD) * (inputX - originX) -
      sin(theta * GCODE_DEG2RAD) * (inputY - originY) + originX;
  *Y = sin(theta * GCODE_DEG2RAD) * (inputX - originX) +
      cos(theta * GCODE_DEG2RAD) * (inputY - originY) + originY;
}

double scaling_math(double input, double origin, double factor) {
  return origin + (input - origin) * factor;
}

void move_math(TGCodeCoordinateInfo *system, double X, double Y, double Z) {
  double oldcX, oldcY;
  TGCodeAbsoluteMode oldAbsolute;
  double newX, newY, newZ, newgX, newgY, newrX, newrY, newrZ;

  system->cX = current_or_last_math(X, system->cX);
  system->cY = current_or_last_math(Y, system->cY);
  system->cZ = current_or_last_math(Z, system->cZ);
  /* c[XYZ] now all contain non-NAN taken either from the current block or
   * the previous word address value
   *
   * NOTE: this is the end of processing for c[XYZ]: they're meant to contain
   *       the word address values from the last block */

  if(system->cartesian == GCODE_POLAR) {
    oldcX = system->cX;
    oldcY = system->cY;
    polar_math(system->cX, system->cY, &system->cX, &system->cY);
    oldAbsolute = system->absolute;
    system->absolute = GCODE_RELATIVE;
    /* c[XY] now contain the Cartesian equivalent of what was specified in
     * polar coordinates in the current block. Since polar coordinates always
     * work in incremental mode, we temporarily change to that to match. */
  }

  /* we need the old g[XY] preserved to define the movement vector for radius
   * compensation below */
  newgX = relative_math(system->cX, system->gX,
                        (system->absolute == GCODE_ABSOLUTE));
  newgY = relative_math(system->cY, system->gY,
                        (system->absolute == GCODE_ABSOLUTE));
  system->gZ = relative_math(system->cZ, system->gZ,
                             (system->absolute == GCODE_ABSOLUTE));
  /* g[XYZ] now contain the relative-corrected version of c[XYZ] as specified
   * in the current block or inferred from past state */

  if(system->cartesian == GCODE_POLAR) {
    /* Restore previous state and word address contents if we were in polar */
    system->absolute = oldAbsolute;
    system->cX = oldcX;
    system->cY = oldcY;
  }

  newgX = system_math(
      newgX, (system->current == GCODE_MCS), system->offset.X,
      fetch_parameter(GCODE_PARM_FIRST_WCS + (system->current - GCODE_WCS_1) *
                      GCODE_PARM_WCS_SIZE + GCODE_AXIS_X));
  newgY = system_math(
      newgY, (system->current == GCODE_MCS), system->offset.Y,
      fetch_parameter(GCODE_PARM_FIRST_WCS + (system->current - GCODE_WCS_1) *
                      GCODE_PARM_WCS_SIZE + GCODE_AXIS_Y));
  system->gZ = system_math(
      system->gZ, (system->current == GCODE_MCS), system->offset.Z,
      fetch_parameter(GCODE_PARM_FIRST_WCS + (system->current - GCODE_WCS_1) *
                      GCODE_PARM_WCS_SIZE + GCODE_AXIS_Z));
  /* g[XYZ] now contain the MCS-, WCS- and LCS- corrected version of their
   * previous self */

  radius_comp_math(newgX, newgY, system->radComp,
                   vector_side_math(system->gX, system->gY, newgX, newgY),
                   &system->gX, &system->gY);
  system->gZ = length_comp_math(system->gZ, system->lenComp);
  /* g[XYZ] now contain the length- and radius- compensated version of their
   * previous self.
   * NOTE: compensation is dimension-less, as per the standard.
   * NOTE: this is the end of processing for g[XYZ]: they're meant to contain
   *       the G-Code interpreter's idea of the current coordinates */

  newX = inch_math(system->gX, (system->units == GCODE_UNITS_INCH));
  newY = inch_math(system->gY, (system->units == GCODE_UNITS_INCH));
  newZ = inch_math(system->gZ, (system->units == GCODE_UNITS_INCH));
  /* new[XYZ] now contain g[XYZ] in machine units */

  if(system->rotation.mode == GCODE_ROTATION_ON) {
    switch(system->plane) {
      case GCODE_PLANE_XY:
        rotation_math(newX, newY, system->rotation.R, system->rotation.X,
                      system->rotation.Y, &newrX, &newrY);
        newrZ = newZ;
        break;
      case GCODE_PLANE_YZ:
        rotation_math(newY, newZ, system->rotation.R, system->rotation.Y,
                      system->rotation.Z, &newrY, &newrZ);
        newrX = newX;
        break;
      case GCODE_PLANE_ZX:
        rotation_math(newZ, newX, system->rotation.R, system->rotation.Z,
                      system->rotation.X, &newrZ, &newrX);
        newrY = newY;
        break;
    }
  }
  /* newr[XYZ] now contain the rotated version of new[XYZ] according to the
   * current coordinate system rotation mode and parameters and active plane */

  if(system->scaling.mode == GCODE_SCALING_ON) {
    newX = scaling_math(newrX, system->scaling.X, system->scaling.I);
    newY = scaling_math(newrX, system->scaling.Y, system->scaling.J);
    newZ = scaling_math(newrX, system->scaling.Z, system->scaling.K);
  }
  /* new[XYZ] now contain the scaled version of newr[XYZ] according to the
   * current scaling mode and parameters */

  /* done, copy over to machine coordinates */
  system->X = newX;
  system->Y = newY;
  system->Z = newZ;
}

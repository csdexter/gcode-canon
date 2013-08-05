/*
 ============================================================================
 Name        : gcode-state.h
 Author      : Radu - Eosif Mihailescu
 Version     : 1.0 (2012-08-11)
 Copyright   : (C) 2012 Radu - Eosif Mihailescu <radu.mihailescu@linux360.ro>
 Description : G-Code Parser Loop API Header
 ============================================================================
 */

#ifndef GCODE_STATE_H_
#define GCODE_STATE_H_


#include <stdbool.h>
#include <stdint.h>

#include "gcode-commons.h"


#define GCODE_STATE_PF_ABSOLUTE 0x40
#define GCODE_STATE_PF_IMPERIAL 0x10


typedef enum {
  OFF,
  RAPID,
  LINEAR,
  ARC,
  CYCLE,
  STORE,
  MACRO
} TGCodeMotionMode;

typedef enum {
  GCODE_CYCLE_HOME = 28,
  GCODE_CYCLE_RETURN = 29,
  GCODE_CYCLE_ZERO = 30,
  GCODE_CYCLE_DRILL_PP = 73,
  GCODE_CYCLE_TAP_LH = 74,
  GCODE_CYCLE_CANCEL = 80,
  GCODE_CYCLE_DRILL_ND = 81,
  GCODE_CYCLE_DRILL_WD = 82,
  GCODE_CYCLE_DRILL_PF = 83,
  GCODE_CYCLE_TAP_RH = 84,
  GCODE_CYCLE_BORING_ND_NS = 85,
  GCODE_CYCLE_BORING_WD_WS = 86,
  GCODE_CYCLE_BORING_BACK = 87,
  GCODE_CYCLE_BORING_MANUAL = 88,
  GCODE_CYCLE_BORING_WD_NS = 89
} TGCodeCycleMode;

typedef enum {
  GCODE_MOVE_RAPID = 0,
  GCODE_MOVE_FEED = 1,
  GCODE_MODE_ARC_CW = 2,
  GCODE_MODE_ARC_CCW = 3,
  GCODE_MODE_CIRCLE_CW = 12,
  GCODE_MODE_CIRCLE_CCW = 13
} TGCodeMoveMode;

typedef enum {
  GCODE_DATA_ON = 10,
  GCODE_DATA_OFF = 11
} TGCodeDataEntryMode;

typedef enum {
  GCODE_PLANE_XY = 17,
  GCODE_PLANE_ZX = 18,
  GCODE_PLANE_YZ = 19
} TGCodePlaneMode;

typedef enum {
  GCODE_ABSOLUTE = 90,
  GCODE_RELATIVE = 91
} TGCodeAbsoluteMode;

typedef enum {
  GCODE_STOP_COMPULSORY = 0,
  GCODE_STOP_OPTIONAL = 1,
  GCODE_STOP_END = 2,
  GCODE_SERVO_ON = 17,
  GCODE_SERVO_OFF = 18,
  GCODE_STOP_RESET = 30,
  GCODE_STOP_E = 36,
  GCODE_APC_1 = 57,
  GCODE_APC_2 = 58,
  GCODE_APC_SWAP = 60
} TGCodeStopMode;

typedef enum {
  GCODE_FEED_INVTIME = 93,
  GCODE_FEED_PERMINUTE = 94
} TGCodeFeedMode;

typedef enum {
  GCODE_UNITS_INCH = 20,
  GCODE_UNITS_METRIC = 21
} TGCodeUnitsMode;

typedef enum {
  GCODE_COMP_RAD_OFF = 40,
  GCODE_COMP_RAD_L = 41,
  GCODE_COMP_RAD_R = 42
} TGCodeRadCompMode;

typedef enum {
  GCODE_SPINDLE_CW = 3,
  GCODE_SPINDLE_CCW = 4,
  GCODE_SPINDLE_STOP = 5
} TGCodeSpindleMode;

typedef enum {
  GCODE_COMP_LEN_N = 43,
  GCODE_COMP_LEN_P = 44,
  GCODE_COMP_LEN_OFF = 49,
} TGCodeLenCompMode;

typedef enum {
  GCODE_OVERRIDE_ON = 48,
  GCODE_OVERRIDE_OFF = 49
} TGCodeOverrideMode;

typedef enum {
  GCODE_RETRACT_LAST = 98,
  GCODE_RETRACT_R = 99
} TGCodeRetractMode;

typedef enum {
  GCODE_MCS = 53,
  GCODE_WCS_1 = 54,
  GCODE_WCS_2 = 55,
  GCODE_WCS_3 = 56,
  GCODE_WCS_4 = 57,
  GCODE_WCS_5 = 58,
  GCODE_WCS_6 = 59
} TGCodeCurrentSystem;

typedef enum {
  GCODE_EXACTSTOPCHECK_ON = 61,
  GCODE_EXACTSTOPCHECK_OFF = 64
} TGCodePathControl;

typedef enum {
  GCODE_MIRROR_ON = 22,
  GCODE_MIRROR_OFF_S = 23
} TGCodeMirrorSystem;

typedef enum {
  GCODE_ROTATION_ON = 68,
  GCODE_ROTATION_OFF = 69
} TGCodeRotationSystem;

typedef enum {
  GCODE_CARTESIAN = 15,
  GCODE_POLAR = 16
} TGCodePolarMode;

typedef enum {
  GCODE_SCALING_OFF = 50,
  GCODE_SCALING_ON = 51
} TGCodeScalingMode;

typedef enum {
  GCODE_PROBE_TOOL = 26,
  GCODE_PROBE_PART = 27
} TGCodeProbeInput;

typedef enum {
  GCODE_PROBE_ONETOUCH = 41,
  GCODE_PROBE_TWOTOUCH = 42
} TGCodeProbeMode;

typedef enum {
  GCODE_COOL_MIST = 7,
  GCODE_COOL_FLOOD = 8,
  GCODE_COOL_OFF_MF = 9,
  GCODE_COOL_SHOWER = 68,
  GCODE_COOL_OFF_S = 69
} TGCodeCoolantMode;

typedef enum {
  GCODE_COOLSPIN_CW = 13,
  GCODE_COOLSPIN_CCW = 14
} TGCodeCoolantAndSpindle;

typedef enum {
  GCODE_MIRROR_X = 21,
  GCODE_MIRROR_Y = 22,
  GCODE_MIRROR_OFF_M = 23
} TGCodeMirrorMachine;

typedef enum {
  GCODE_SPINDLE_ORIENTATION = 19,
  GCODE_INDEXER_STEP = 20,
  GCODE_RETRACT_Z = 25,
  GCODE_CYCLE_PROBE_IN = 31,
  GCODE_CYCLE_PROBE_OUT = 38
} TGCodeAuxiliaryMachine;

typedef struct {
  TGCodeMirrorSystem mode;
  double X, Y, Z;
} TGCodeMirrorSpec;

typedef struct {
  TGCodeRotationSystem mode;
  double X, Y, Z;
  uint16_t R;
} TGCodeRotationSpec;

typedef struct {
  TGCodeScalingMode mode;
  double X, Y, Z;
  double I, J, K;
} TGCodeScalingSpec;

typedef struct {
  double X, Y, Z;
} TGCodeOffsetSpec;

typedef struct {
  uint8_t mode;
  double offset;
} TGCodeCompSpec;

typedef struct {
  TGCodePlaneMode plane;
  TGCodeUnitsMode units;
  TGCodeCompSpec radComp, lenComp;
  TGCodeCurrentSystem current, oldCurrent;
  TGCodeMirrorSpec mirror;
  TGCodeRotationSpec rotation;
  TGCodeAbsoluteMode absolute;
  TGCodePolarMode cartesian;
  TGCodeScalingSpec scaling;
  TGCodeOffsetSpec offset;
  /* This are what we feed the machine. Always in mm, always Cartesian, always
   * absolute */
  double X, Y, Z;
  /* These are the current/old Radius and Theta while we're in polar mode */
  double pR, pT;
  /* These are GCode's idea of the current coordinates, used mostly when
   * transformations are active. Also always in mm, always Cartesian, always
   * absolute */
  double gX, gY, gZ;
} TGCodeCoordinateInfo;

typedef struct {
  TGCodeFeedMode feedMode;
  TGCodeCoordinateInfo system;
  TGCodeRetractMode retractMode;
  TGCodeMotionMode motionMode, oldMotionMode;
  TGCodePathControl oldPathMode;
  bool nonModalPathMode;
  TGCodeCycleMode cycle;
  bool macroCall;
  bool axisWordsConsumed;
  bool ccw;
  double I, J, K, P, Q, R;
  uint16_t F, L;
  uint8_t T;
} TGCodeState;

typedef struct {
  char *line; /* Object of last analysis */
  char word; /* Last word requested */
  char *at; /* If found, where in line? */
} TGCodeWordCache;


bool init_gcode_state(void *data);
/* Process one line of G-Code. Note that line has already been sanitized by
 * gcode-input. Returns false on error */
bool update_gcode_state(char *line);
/* Read from line and interpret as number transparently handling parameter
 * references; return number */
double read_gcode_real(char *line);
uint32_t read_gcode_integer(char *line);
/* If called with 2 arguments and argc of 0, return true if said word was
 * present in the line.
 * If called with 3 arguments and argc of 1, return true if said word was
 * present in the line and had said argument.
 * If called with more than 3 arguments, return the argument which was
 * found on the line attached to the given word or false if none. A single
 * match is assumed (the first one). If an argument of zero is passed (like
 * when searching for G00 or M00) and found on the line, it will be returned
 * as 100 instead of zero (which would evaluate to false and mis-signal that
 * no match has been found). */
uint8_t have_gcode_word(char word, uint8_t argc, ...);
/* Return the argument of the given word as a real number or NaN if no such
 * word was on the line */
double get_gcode_word_real(char word);
/* Return the argument of the given word as a real number or defVal if no such
 * word was on the line */
double get_gcode_word_real_default(char word, double defVal);
/* Return the argument of the given word as an integer or ULONG_MAX if no such
 * word was on the line */
uint32_t get_gcode_word_integer(char word);
/* Return the argument of the given word as an integer or defVal if no such
 * word was on the line */
uint32_t get_gcode_word_integer_default(char word, uint32_t defVal);
/* Scan line for parameter assignments (#<number>=<expression>) and update
 * parameter store accordingly. */
bool process_gcode_parameters(void);
/* Returns true if we are still (or should be) processing the current part
 * program, false if program flow ended and we should exit or reset */
bool gcode_running(void);


#endif /* GCODE_STATE_H_ */

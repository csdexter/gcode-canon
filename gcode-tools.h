/*
 * gcode-tools.h
 *
 *  Created on: Aug 11, 2012
 *      Author: csdexter
 */

#ifndef GCODE_TOOLS_H_
#define GCODE_TOOLS_H_

#include "gcode-commons.h"

typedef enum {
  GCODE_TOOL_UNDEFINED = -10,
  GCODE_TOOL_DRILL = -11,
  GCODE_TOOL_TAP = -12,
  GCODE_TOOL_BORE = -13,
  GCODE_TOOL_MILL = -14,
  GCODE_TOOL_FACEMILL = -15,
  GCODE_TOOL_BALLEND = -16,
  GCODE_TOOL_BACKSPOTFACE = -17,
  GCODE_TOOL_PROBE = -18,
  GCODE_TOOL_GUNDRILL = -19
} TGCodeToolType;

typedef struct {
  uint8_t index;
  TGCodeToolType type;
  double diameter;
  double length;
  /* Other/extended information would go here */
} TGCodeTool;

/* Initialize the tool engine, takes an opaque pointer to a data store/effector, returns true if all ok */
bool init_tools(void *data);
/* Fetch data for tool index out of the store */
TGCodeTool fetch_tool(uint8_t index);
/* Update data for tool index into the store, returns false if anything bad happened */
bool update_tool(TGCodeTool tool);
/* Returns radius of tool index or zero if zero passed */
double radiusof_tool(uint8_t index);
/* Returns length of tool index or zero if zero passed */
double lengthof_tool(uint8_t index);
bool done_tools(void);

#endif /* GCODE_TOOLS_H_ */

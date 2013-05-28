/*
 * gcode-tools.c
 *
 *  Created on: Aug 12, 2012
 *      Author: csdexter
 */

#include <stdint.h>
#include <stdio.h>

#include "gcode-commons.h"
#include "gcode-tools.h"
#include "gcode-debugcon.h"
#include "gcode-parameters.h"

static TGCodeTool currentTool;

bool init_tools(void *data) {
  int i = 0;
  while(fetch_parameter(GCODE_TOOL_TYPE_BASE + i + 1)) i++;

  GCODE_DEBUG("Tools up, %d installed, %d supported", i, GCODE_TOOL_COUNT);

  return true;
}

TGCodeTool fetch_tool(uint8_t index) {
  if(index != currentTool.index) {
    currentTool.index = index;
    currentTool.type = fetch_parameter(GCODE_TOOL_TYPE_BASE + index);
    currentTool.diameter = fetch_parameter(GCODE_TOOL_DIAM_BASE + index);
    currentTool.length = fetch_parameter(GCODE_TOOL_LEN_BASE + index);
  }

  return currentTool;
}

bool update_tool(TGCodeTool tool) {
  if(tool.index == currentTool.index) currentTool = tool;

  return update_parameter(GCODE_TOOL_TYPE_BASE + tool.index, currentTool.type) &&
    update_parameter(GCODE_TOOL_DIAM_BASE + tool.index, currentTool.diameter) &&
    update_parameter(GCODE_TOOL_LEN_BASE + tool.index, currentTool.length);
}

double radiusof_tool(uint8_t index) {
  if(!index) return 0.0;
  if(index != currentTool.index) return fetch_parameter(GCODE_TOOL_DIAM_BASE + index) / 2.0;
  else return currentTool.diameter / 2.0;
}

double lengthof_tool(uint8_t index) {
  if(!index) return 0.0;
  if(index != currentTool.index) return fetch_parameter(GCODE_TOOL_LEN_BASE + index);
  else return currentTool.length;
}

bool done_tools(void) {
  GCODE_DEBUG("Tools down");

  return true;
}

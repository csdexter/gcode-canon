/*
 ============================================================================
 Name        : gcode-debugcon.h
 Author      : Radu - Eosif Mihailescu
 Version     : 1.0 (2012-08-11)
 Copyright   : (C) 2012 Radu - Eosif Mihailescu <radu.mihailescu@linux360.ro>
 Description : Debug Macros
 ============================================================================
 */

#ifndef GCODE_DEBUGCON_H_
#define GCODE_DEBUGCON_H_


#include <libgen.h>


#define GCODE_DEBUG(...) { \
  printf("[%s](%s@%d): ", basename(__FILE__), __FUNCTION__, __LINE__); \
  printf(__VA_ARGS__); \
  printf("\n"); }
#define GCODE_DEBUG_RAW(...) { \
  printf(__VA_ARGS__); \
  printf("\n"); }


#endif /* GCODE_DEBUGCON_H_ */

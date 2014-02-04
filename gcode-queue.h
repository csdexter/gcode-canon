/*
 ============================================================================
 Name        : gcode-machine.c
 Author      : Radu - Eosif Mihailescu
 Version     : 1.0 (2013-09-25)
 Copyright   : (C) 2013 Radu - Eosif Mihailescu <radu.mihailescu@linux360.ro>
 Description : G-Code Movement Queue API Header
 ============================================================================
 */

#ifndef GCODE_QUEUE_H_
#define GCODE_QUEUE_H_


#include <stdbool.h>

#include "gcode-state.h"


typedef struct {
  bool isArc;
  TGCodeOffsetSpec target;
  TGCodeOffsetSpec center;
  bool ccw;
  double feedValue;
  TGCodeCompSpec radComp;
  TGCodeCornerMode corner;
} TGCodeMoveSpec;

/* Start the show */
void init_queue(void);
/* Adds move to the tail of the queue, returns false if queue is full */
bool enqueue_move(TGCodeMoveSpec move);
/* Pops move from the head of the queue, returns false if queue is empty */
bool dequeue_move(TGCodeMoveSpec *move);
/* Returns current queue size */
uint8_t queue_size(void);
/* Returns move at the head of the queue without modifying queue. If the queue
 * is empty, results are undefined */
TGCodeMoveSpec peek_move(void);

bool done_queue(void);

#endif /* GCODE_QUEUE_H_ */

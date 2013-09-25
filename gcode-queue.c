/*
 ============================================================================
 Name        : gcode-machine.c
 Author      : Radu - Eosif Mihailescu
 Version     : 1.0 (2013-09-25)
 Copyright   : (C) 2013 Radu - Eosif Mihailescu <radu.mihailescu@linux360.ro>
 Description : G-Code Movement Queue API and Implementation Code
 ============================================================================
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "gcode-commons.h"
#include "gcode-queue.h"
#include "gcode-debugcon.h"


static uint8_t qHead, qTail;
static TGCodeMoveSpec queue[GCODE_LOOKAHEAD_DEPTH];


void init_queue(void) {
  qHead = 1;
  qTail = 0;
  queue[qHead].isArc = false;
  queue[qHead].feed = 0.0;
  queue[qHead].target.X = 0.0;
  queue[qHead].target.Y = 0.0;
  queue[qHead].target.Z = 0.0;

  GCODE_DEBUG("Movement queue ready, %d steps deep", GCODE_LOOKAHEAD_DEPTH);
}

bool enqueue_move(TGCodeMoveSpec move) {
  uint8_t nHead = (qHead + 1) == GCODE_LOOKAHEAD_DEPTH ? 0 : (qHead + 1);

  if(nHead != qTail) {
    queue[qHead] = move;
    qHead = nHead;

    return true;
  } else return false;
}

TGCodeMoveSpec peek_move(void) {
  return queue[qTail];
}

bool dequeue_move(TGCodeMoveSpec *move) {
  if(qHead == qTail) return false;
  else {
    *move = peek_move();
    qTail = (qTail + 1) == GCODE_LOOKAHEAD_DEPTH ? 0 : (qTail + 1);

    return true;
  }
}

uint8_t queue_size(void) {
  if(qHead >= qTail)
    return qHead - qTail;
  else
    return qHead + GCODE_LOOKAHEAD_DEPTH - qTail;
}

bool done_queue(void) {
  if(queue_size()) {
    GCODE_DEBUG("Movement queue still has %d steps remaining at shutdown!", queue_size())

    return false;
  } else {
    GCODE_DEBUG("Movement queue shutdown.")

    return true;
  }
}

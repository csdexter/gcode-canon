/*
 ============================================================================
 Name        : gcode-machine.c
 Author      : Radu - Eosif Mihailescu
 Version     : 1.0 (2013-09-25)
 Copyright   : (C) 2013 Radu - Eosif Mihailescu <radu.mihailescu@linux360.ro>
 Description : G-Code Movement Queue API and Implementation Code
 ============================================================================
 */

#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "gcode-commons.h"
#include "gcode-queue.h"
#include "gcode-debugcon.h"
#include "gcode-math.h"
#include "gcode-machine.h"


static uint8_t qHead, qTail;
static TGCodeMoveSpec queue[GCODE_LOOKAHEAD_DEPTH], buffer;
static bool bufferValid;
static TGCodeOffsetSpec lastRawTarget, lastCompTarget;


static bool _enqueue_nonull_move(TGCodeMoveSpec move, uint8_t where) {
  /* Comparing floating point values for equality is asking for trouble,
  * however we're simply treating them as opaque data and actually asking
  * "is THIS equal to THE ONE BEFORE?" as opposed to "is 1.2 equal to 1.2?".
  *
  * Since the data we previously fed in went through the same transformations
  * and obeys the same limitations of the storage type, we can safely conclude
  * we'll always be comparing apples to apples. */
  if(memcmp(&lastCompTarget, &move.target, sizeof(TGCodeOffsetSpec))) {
    queue[qHead] = move;
    qHead = where;

    lastCompTarget = move.target;
    if(move.radComp.mode == GCODE_COMP_RAD_OFF)
      lastRawTarget = move.target;

    return true;
  } else return false;
}

static uint8_t _next_head(void) {
  uint8_t nHead = (qHead + 1) == GCODE_LOOKAHEAD_DEPTH ? 0 : (qHead + 1);

  if(nHead != qTail) return nHead;
  else {
    display_machine_message("QER: Movement queue overflow!");

    return qHead;
  }
}

static void _do_radcomp(TGCodeMoveSpec move) {
  double opX, opY, ocX, ocY;
  TGCodeMoveSpec movep, movec;
  bool arcFlag;

  movep.target = lastRawTarget;
  /* (lastCompTarget -> movep) is the first segment */
  movep = offset_math(movep, buffer, buffer.radComp, &opX, &opY);
  /* (movep -> movec) is the second segment, as if radComp were constant,
   * we only need the starting point anyway. */
  movec = offset_math(buffer, move, buffer.radComp, &ocX, &ocY);

  // TODO: bring currentGCodeState.system.corner here
  arcFlag = /* USE_ARCS && !*/ inside_corner_math(lastRawTarget.X,
                                                  lastRawTarget.Y, buffer,
                                                  move, buffer.radComp);
  if(!arcFlag)
    /* Trim/extend the first move to the intersection point */
    intersection_math(opX, opY, movep, ocX, ocY, movec, &movep.target.X,
                      &movep.target.Y);

  /* Enqueue first (and maybe only) compensated move */
  _enqueue_nonull_move(movep, _next_head());
  /* Save last real target */
  lastRawTarget = buffer.target;

  if(arcFlag) {
    TGCodeMoveSpec arcMove;

    /* Create an arc around the corner */
    arcMove.ccw = buffer.radComp.mode == GCODE_COMP_RAD_R;
    arcMove.center = buffer.target;
    arcMove.feedValue = buffer.feedValue;
    arcMove.isArc = true;
    arcMove.radComp = buffer.radComp;
    arcMove.target.X = ocX;
    arcMove.target.Y = ocY;
    arcMove.target.Z = buffer.target.Z;
    /* Enqueue second compensated move */
    _enqueue_nonull_move(arcMove, _next_head());
    /* We just circled around a single point, no need to update the last
     * un-compensated location */
  }
}

void init_queue(void) {
  qHead = qTail = 0;
  bufferValid = false;
  lastRawTarget.X = lastRawTarget.Y = lastRawTarget.Z = +0.0E+0;
  lastCompTarget.X = lastCompTarget.Y = lastCompTarget.Z = +0.0E+0;

  GCODE_DEBUG("Movement queue ready, %d steps deep", GCODE_LOOKAHEAD_DEPTH);
}

bool enqueue_move(TGCodeMoveSpec move) {
  if(bufferValid) _do_radcomp(move);

  if(move.radComp.mode != GCODE_COMP_RAD_OFF) {
    buffer = move;
    bufferValid = true;

    return true;
  } else {
    if(bufferValid) {
      /* Returning to non-compensated mode */
      bufferValid = false;
    }

    return _enqueue_nonull_move(move, _next_head());
  }
}

TGCodeMoveSpec peek_move(void) {
  return queue[qTail];
}

bool dequeue_move(TGCodeMoveSpec *move) {
  /* Empty queue? */
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

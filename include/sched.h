#ifndef SCHED_H
#define SCHED_H

/* sched.h -- the scheduler's buffer index, apart from boot.h because unit 53B0 declares it without volatile */

#include "common.h"

/* 0 or 1: which of the double-buffered display lists and matrices is built */
// volatile: units 1060, 7250 and C650 read it with lbu and a separate sign extension, once per use (53B0 alone uses lb)
extern volatile s8 sched_bufferIndex;

#endif

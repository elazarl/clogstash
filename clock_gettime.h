#ifndef __H_CLOCK_GETTIME_
#define __H_CLOCK_GETTIME_
#include <sys/time.h>
#include <time.h>

#ifndef CLOCK_MONOTONIC
/* Mac OS X don't have it */
#define CLOCK_MONOTONIC 0
#endif

int clock_gettime(int clk_id, struct timespec *ts);

#endif

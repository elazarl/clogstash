#include "clock_gettime.h"
#include "timespec.h"
#include "helpers.h"

/**
 * Mac OS X doesn't have clock_gettime function,
 * we'll use gettimeofday instead.
 * In production you should probably use mach_monotonous_time or something
 */
int clock_gettime(int UNUSED(_), struct timespec *ts) { 
    struct timeval tv; 

    gettimeofday(&tv, NULL); 
    set_normalized_timespec(ts, tv.tv_sec, tv.tv_usec * 1000);
    return 0; 
} 

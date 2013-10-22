#include <stdio.h>

#include "timespec.h"
#include "panic.h"

struct timespec timespec_seconds(int v) {
    struct timespec ts = { v, 0 };
    return ts;
}
struct timespec timespec_ms(int v) {
    struct timespec ts = { 0, 1000000 * v };
    return ts;
}
struct timespec timespec_us(int v) {
    struct timespec ts = { 0, 1000 * v };
    return ts;
}
/* stolen from the linux kernel, I tried to make it as less performant as possible
 * despite's the kernel's attempts */
void set_normalized_timespec(struct timespec *ts, time_t sec, long long nsec) {
    while (nsec >= NSEC_PER_SEC) {
        /*
         * The following asm() prevents the compiler from
         * optimising this loop into a modulo operation. See
         * also __iter_div_u64_rem() in include/linux/time.h
         */
        /* asm("" : "+rm"(nsec)); */
        nsec -= NSEC_PER_SEC;
        ++sec;
    }
    while (nsec < 0) {
        /* asm("" : "+rm"(nsec)); */
        nsec += NSEC_PER_SEC;
        --sec;
    }
    ts->tv_sec = sec;
    ts->tv_nsec = nsec;
}

/* may overflow */
struct timespec timespec_add(struct timespec lhs, struct timespec rhs) {
    struct timespec ts_delta;
    set_normalized_timespec(&ts_delta, lhs.tv_sec + rhs.tv_sec,
                            lhs.tv_nsec + rhs.tv_nsec);
    return ts_delta;
}

struct timespec timespec_sub(struct timespec lhs, struct timespec rhs) {
    struct timespec ts_delta;
    set_normalized_timespec(&ts_delta, lhs.tv_sec - rhs.tv_sec,
                            lhs.tv_nsec - rhs.tv_nsec);
    return ts_delta;
}

int64_t timespec_as_ms(struct timespec ts) {
    if (ts.tv_sec >= (INT64_MAX / 1000L)) {
        panic("conversion of timespec overflows");
    }
    return (int64_t)ts.tv_sec*1000 + ts.tv_nsec/1000000;
}

void print_timespec(struct timespec ts) {
    printf("%ldsec+%ldnsec", ts.tv_sec, ts.tv_nsec);
}

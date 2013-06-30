#ifndef __H_TIMESPEC_
#define __H_TIMESPEC_
#include <time.h>
#include <stdint.h>

#ifndef NSEC_PER_SEC
#define NSEC_PER_SEC 1000000
#endif

struct timespec timespec_seconds(int v);
struct timespec timespec_ms(int v);
struct timespec timespec_us(int v);
struct timespec timespec_add(struct timespec lhs, struct timespec rhs);
struct timespec timespec_sub(struct timespec lhs, struct timespec rhs);
void set_normalized_timespec(struct timespec *ts, time_t sec, long long nsec);
/**
 * timespec_as_ms converts the timespec to the number of milliseconds
 * In case of overflow - it'll panic.
 */
long long timespec_as_ms(struct timespec ts);

void print_timespec(struct timespec ts);
#endif

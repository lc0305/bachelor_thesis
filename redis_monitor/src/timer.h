#ifndef __TIMER_H__
#define __TIMER_H__

//#define _POSIX_C_SOURCE 199309L

#include <stddef.h>
#include <assert.h>
#include <limits.h>
#include <time.h>
#include "zmalloc.h"

typedef struct high_resolution_timer {
  struct timespec start_time;
} high_resolution_timer;

high_resolution_timer high_resolution_time();
void restart_high_resolution_timer(high_resolution_timer *timer);
long elapsed_high_resolution_time_nanoseconds(
    const high_resolution_timer *timer);
high_resolution_timer *multiple_high_resolution_timer(size_t count);
void free_multiple_high_resolution_timer(high_resolution_timer *timers);
high_resolution_timer *get_high_resolution_timer(high_resolution_timer *timers,
                                                 size_t index);
// long timespec_to_total_nanoseconds(struct timespec timespec);

#endif

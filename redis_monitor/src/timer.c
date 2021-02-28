#include "timer.h"

#define SECOND_IN_NANOSECONDS 1000000000

static struct timespec diff(struct timespec start, struct timespec end) {
  struct timespec temp;
  if ((end.tv_nsec - start.tv_nsec) < 0) {
    temp.tv_sec = end.tv_sec - start.tv_sec - 1;
    temp.tv_nsec = SECOND_IN_NANOSECONDS + end.tv_nsec - start.tv_nsec;
  } else {
    temp.tv_sec = end.tv_sec - start.tv_sec;
    temp.tv_nsec = end.tv_nsec - start.tv_nsec;
  }
  return temp;
}

static long timespec_to_total_nanoseconds(struct timespec timespec) {
  assert((LONG_MAX / SECOND_IN_NANOSECONDS) > timespec.tv_sec);
  long result = timespec.tv_sec * SECOND_IN_NANOSECONDS;
  result += timespec.tv_nsec;
  return result;
}

high_resolution_timer high_resolution_time() {
  struct timespec start;

  clock_gettime(CLOCK_MONOTONIC, &start);

  high_resolution_timer timer = {
      .start_time = start,
  };

  return timer;
}

void restart_high_resolution_timer(high_resolution_timer *timer) {
  clock_gettime(CLOCK_MONOTONIC, &timer->start_time);
}

long elapsed_high_resolution_time_nanoseconds(
    const high_resolution_timer *timer) {
  struct timespec end;

  clock_gettime(CLOCK_MONOTONIC, &end);

  return timespec_to_total_nanoseconds(diff(timer->start_time, end));
}

high_resolution_timer *multiple_high_resolution_timer(size_t count) {
  high_resolution_timer *ptr =
      (high_resolution_timer *)zmalloc(count * sizeof(high_resolution_timer));

  // init memory

  struct timespec start;
  clock_gettime(CLOCK_MONOTONIC, &start);

  high_resolution_timer timer = {
      .start_time = start,
  };

  for (size_t i = 0; i < count; i++) {
    ptr[i] = timer;
  }

  return ptr;
}

void free_multiple_high_resolution_timer(high_resolution_timer *timers) {
  zfree(timers);
}

high_resolution_timer *get_high_resolution_timer(high_resolution_timer *timers,
                                                 size_t index) {
  return &timers[index];
}

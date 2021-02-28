#ifndef __ELMONITOR_H__
#define __ELMONITOR_H__

#include <stddef.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include "vec.h"
#include "zmalloc.h"
#include "spinlock.h"

#define MONITOR 1

extern uint64_t el_current_tick;

typedef struct Monitor {
  const char *
      ident; // should be r-value strings stored in data, so those are NOT freed
  VecMonitor *vec;
  SpinLock lock;
} Monitor;

size_t monitor_count(Monitor *monitor);
MonitorValue *monitor_begin(Monitor *monitor);
MonitorValue *monitor_end(Monitor *monitor);
MonitorValue *monitor_get(Monitor *monitor, size_t index);
const char *monitor_ident(Monitor *monitor);

/**
 * increments tick and returns previous tick as a side effect
 * */
uint64_t increment_tick();

/**
 * returns current tick
 * */
uint64_t get_current_tick();

/**
 * creates eventloop monitor with an identifier for example function name to be
 * monitored identifier should be r-value string or pointer to data segment
 * since those are not freed e.g. elmonitor_create("write_pending_data")
 * */
Monitor *elmonitor_create(const char *ident);

/**
 * add measured time value in nanoseconds to the specified monitor
 * will store it in form of a MonitorValue with the current tick inside of a vec
 * */
void elmonitor_add(Monitor *monitor, nanoseconds_t value);

/**
 * printfs complete monitor data
 * warning: poorly optimized, O(N) write calls
 * */
void elmonitor_print(Monitor *monitor);

void elmonitor_to_csv(Monitor *monitor);

void free_elmonitor(Monitor *moniitor);

#endif

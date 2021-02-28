#ifndef __VEC_H__
#define __VEC_H__

#include <stddef.h>
#include <assert.h>
#include <stdint.h>
#include "zmalloc.h"

#define PAGESIZE 4096
#define DEFAULT_CAPACITY PAGESIZE

typedef long nanoseconds_t;

//(sizeof struct mod DEFAULT_CAPACITY = 0) if not undefined behavior
typedef struct MonitorValue {
  uint64_t tick;
  nanoseconds_t time;
} MonitorValue;

typedef struct VecMonitor {
  size_t len;
  size_t capacity;
  MonitorValue *values;
} VecMonitor;

size_t vecmonitor_len(VecMonitor *vec);
MonitorValue *vecmonitor_begin(VecMonitor *vec);
MonitorValue *vecmonitor_end(VecMonitor *vec);
MonitorValue *vecmonitor_get(VecMonitor *vec, size_t index);

VecMonitor *new_vecmonitor();

void free_vecmonitor();

void vecmonitor_push(VecMonitor *vec, MonitorValue value);

#endif

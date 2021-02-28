#include "vec.h"

size_t vecmonitor_len(VecMonitor *vec) { return vec->len; }

MonitorValue *vecmonitor_begin(VecMonitor *vec) { return vec->values; }

MonitorValue *vecmonitor_end(VecMonitor *vec) {
  return &vec->values[vecmonitor_len(vec)];
}

MonitorValue *vecmonitor_get(VecMonitor *vec, size_t index) {
  assert(index < vecmonitor_len(vec));
  return &vec->values[index];
}

VecMonitor *new_vecmonitor() {
  VecMonitor *ptr = (VecMonitor *)zmalloc(sizeof(VecMonitor));
  ptr->len = 0;
  ptr->capacity = DEFAULT_CAPACITY / sizeof(MonitorValue);
  ptr->values = (MonitorValue *)zmalloc(DEFAULT_CAPACITY);
  return ptr;
}

void vecmonitor_push(VecMonitor *vec, MonitorValue value) {
  if (vec->len >= vec->capacity) {
    vec->capacity <<= 1;
    vec->values = (MonitorValue *)zrealloc(vec->values,
                                          sizeof(MonitorValue) * vec->capacity);
    assert(NULL != vec->values);
  }
  vec->values[vec->len] = value;
  vec->len++;
}

void free_vecmonitor(VecMonitor *vec) {
  zfree(vec->values);
  zfree(vec);
}

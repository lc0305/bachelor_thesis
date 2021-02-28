#include "elmonitor.h"

uint64_t el_current_tick = 0;

size_t monitor_count(Monitor *monitor) { return vecmonitor_len(monitor->vec); }

MonitorValue *monitor_begin(Monitor *monitor) {
  return vecmonitor_begin(monitor->vec);
}

MonitorValue *monitor_end(Monitor *monitor) {
  return vecmonitor_end(monitor->vec);
}

MonitorValue *monitor_get(Monitor *monitor, size_t index) {
  return vecmonitor_get(monitor->vec, index);
}

const char *monitor_ident(Monitor *monitor) { return monitor->ident; }

uint64_t increment_tick() { return el_current_tick++; }

uint64_t get_current_tick() { return el_current_tick; }

Monitor *elmonitor_create(const char *ident) {
  Monitor *ptr = (Monitor *)zmalloc(sizeof(Monitor));
  ptr->ident = ident;
  ptr->vec = new_vecmonitor();
  SpinLock lock = SPINLOCK_INIT;
  ptr->lock = lock;
  return ptr;
}

void elmonitor_add(Monitor *monitor, nanoseconds_t value) {
  MonitorValue mon_val = {
      .tick = get_current_tick(),
      .time = value,
  };
  spinlock_lock(&monitor->lock);
  vecmonitor_push(monitor->vec, mon_val);
  spinlock_unlock(&monitor->lock);
}

void free_elmonitor(Monitor *monitor) {
  free_vecmonitor(monitor->vec);
  zfree(monitor);
}

void elmonitor_print(Monitor *monitor) {
  for (MonitorValue *iter = monitor_begin(monitor);
       iter != monitor_end(monitor); iter++) {
    printf("Ident: %s, Tick: %lu, Time: %ld\n", monitor_ident(monitor),
           iter->tick, iter->time);
  }
}

void elmonitor_to_csv(Monitor *monitor) {
  fflush(stdout);

  const char file_ext[] = ".csv";
  //VLA is safe because only filename size
  char buf[strlen(monitor_ident(monitor)) + sizeof(file_ext)];
  sprintf(buf, "%s%s", monitor_ident(monitor), file_ext);

  int fd_csv = open(buf, O_RDWR | O_CREAT | O_TRUNC, 0777);
  if (fd_csv < 0)
    perror("csv open error\n");

  const char columns[] = "tick;time\n";
  write(fd_csv, columns, sizeof(columns) - 1);

  int save_stdout = dup(STDOUT_FILENO);
  if (save_stdout < 0)
    perror("csv dup error\n");

  if (dup2(fd_csv, STDOUT_FILENO) < 0)
    perror("csv dup2 error\n");

  for (MonitorValue *iter = monitor_begin(monitor);
       iter != monitor_end(monitor); iter++) {
    printf("%lu;%ld\n", iter->tick, iter->time);
  }

  fflush(stdout);
  fsync(fd_csv);

  dup2(save_stdout, STDOUT_FILENO);
  close(save_stdout);

  close(fd_csv);
}

#ifndef __ELMONITORING_H__
#define __ELMONITORING_H__

#include "timer.h"
#include "elmonitor.h"

#define EL_IO_DEBUG 0

/**
 * ---- ELMONITORING GLOBALS ----
 * */

// overall server
extern high_resolution_timer time_server;
extern Monitor *monitor_server;

// pending writes
extern high_resolution_timer *time_pending_writes;
extern Monitor *monitor_pending_writes;

// pending writes using threads per tick
extern high_resolution_timer time_pending_writes_using_threads_per_tick;
extern Monitor *monitor_pending_writes_using_threads_per_tick;

// starting threaded IO
extern high_resolution_timer time_starting_threaded_io;
extern Monitor *monitor_starting_threaded_io;

// stopping threaded IO
extern high_resolution_timer time_stopping_threaded_io;
extern Monitor *monitor_stopping_threaded_io;

//pending reads
extern high_resolution_timer time_pending_reads;
extern Monitor *monitor_pending_reads;

//pending reads using threads
extern high_resolution_timer time_pending_reads_using_threads_per_tick;
extern Monitor *monitor_pending_reads_using_threads_per_tick;

//writes
extern high_resolution_timer time_writes;
extern Monitor *monitor_writes;

//reads
extern high_resolution_timer time_reads;
extern Monitor *monitor_reads;

//tick
extern high_resolution_timer time_tick;
extern Monitor *monitor_tick;

//command processing
extern high_resolution_timer time_command_proc;
extern Monitor *monitor_command_proc;

//command execution
extern high_resolution_timer time_command_exec;
extern Monitor *monitor_command_exec;

extern Monitor *monitor_command;
/**
 * HELPER FUNCTIONS
 * */
extern int isElmonitoringInit;

void initElmonitoring(size_t io_threads);
void quitElmonitoring();
void freeElmonitoring();

#endif

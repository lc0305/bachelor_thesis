#include "elmonitoring.h"

/**
 * ---- ELMONITORING IMPLEMENTATION ----
 * */

// overall server
high_resolution_timer time_server;
Monitor *monitor_server = NULL;

// pending writes
high_resolution_timer *time_pending_writes = NULL;
Monitor *monitor_pending_writes = NULL;

// pending writes using threads per tick
high_resolution_timer time_pending_writes_using_threads_per_tick;
Monitor *monitor_pending_writes_using_threads_per_tick = NULL;

// starting threaded IO
high_resolution_timer time_starting_threaded_io;
Monitor *monitor_starting_threaded_io = NULL;

// stopping threaded IO
high_resolution_timer time_stopping_threaded_io;
Monitor *monitor_stopping_threaded_io = NULL;

//pending reads
high_resolution_timer time_pending_reads;
Monitor *monitor_pending_reads = NULL;

//pending reads using threads
high_resolution_timer time_pending_reads_using_threads_per_tick;
Monitor *monitor_pending_reads_using_threads_per_tick = NULL;

//writes
high_resolution_timer time_writes;
Monitor *monitor_writes = NULL;

//reads
high_resolution_timer time_reads;
Monitor *monitor_reads = NULL;

//tick
high_resolution_timer time_tick;
Monitor *monitor_tick = NULL;

//command processing
high_resolution_timer time_command_proc;
Monitor *monitor_command_proc = NULL;

//command execution
high_resolution_timer time_command_exec;
Monitor *monitor_command_exec = NULL;

Monitor *monitor_command = NULL;

int isElmonitoringInit = 0;

void initElmonitoring(size_t io_threads) {
    time_server = high_resolution_time();
    monitor_server = elmonitor_create("server");

    time_pending_writes = multiple_high_resolution_timer(io_threads);
    monitor_pending_writes = elmonitor_create("pending_writes");

    time_pending_writes_using_threads_per_tick = high_resolution_time();
    monitor_pending_writes_using_threads_per_tick = elmonitor_create("pending_writes_using_threads_per_tick");

    time_starting_threaded_io = high_resolution_time();
    monitor_starting_threaded_io = elmonitor_create("starting_threaded_io");

    time_stopping_threaded_io = high_resolution_time();
    monitor_stopping_threaded_io = elmonitor_create("stopping_threaded_io");

    time_pending_reads = high_resolution_time();
    monitor_pending_reads = elmonitor_create("pending_reads");

    time_pending_reads_using_threads_per_tick = high_resolution_time();
    monitor_pending_reads_using_threads_per_tick = elmonitor_create("pending_reads_using_threads_per_tick");

    time_writes = high_resolution_time();
    monitor_writes = elmonitor_create("writes");

    time_reads = high_resolution_time();
    monitor_reads = elmonitor_create("reads");

    time_tick = high_resolution_time();
    monitor_tick = elmonitor_create("tick");

    time_command_proc = high_resolution_time();
    monitor_command_proc = elmonitor_create("command_proc");

    time_command_exec = high_resolution_time();
    monitor_command_exec = elmonitor_create("command_exec");

    monitor_command = elmonitor_create("command");

    isElmonitoringInit = 1;
}

void quitElmonitoring() {
    if (isElmonitoringInit) { 
        elmonitor_to_csv(monitor_server);
        elmonitor_to_csv(monitor_pending_writes);
        elmonitor_to_csv(monitor_pending_writes_using_threads_per_tick);
        elmonitor_to_csv(monitor_starting_threaded_io);
        elmonitor_to_csv(monitor_stopping_threaded_io);
        elmonitor_to_csv(monitor_pending_reads);
        elmonitor_to_csv(monitor_pending_reads_using_threads_per_tick);
        elmonitor_to_csv(monitor_writes);
        elmonitor_to_csv(monitor_reads);
        elmonitor_to_csv(monitor_tick);
        elmonitor_to_csv(monitor_command_proc);
        elmonitor_to_csv(monitor_command_exec);
        elmonitor_to_csv(monitor_command);
    }
}

void freeElmonitoring() {
    if (isElmonitoringInit) {
        free_multiple_high_resolution_timer(time_pending_writes);
        free_elmonitor(monitor_server);
        free_elmonitor(monitor_pending_writes);
        free_elmonitor(monitor_pending_writes_using_threads_per_tick);
        free_elmonitor(monitor_starting_threaded_io);
        free_elmonitor(monitor_stopping_threaded_io);
        free_elmonitor(monitor_pending_reads);
        free_elmonitor(monitor_pending_reads_using_threads_per_tick);
        free_elmonitor(monitor_writes);
        free_elmonitor(monitor_reads);
        free_elmonitor(monitor_tick);
        free_elmonitor(monitor_command_proc);
        free_elmonitor(monitor_command_exec);
        free_elmonitor(monitor_command);
        isElmonitoringInit = 0;
    }
}

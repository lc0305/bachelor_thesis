#!/bin/bash

MEMTIER=./memtier_benchmark

REMOTE_IP=192.168.188.74
REMOTE_SERVER_PORT=6379

REMOTE_USER=lucas

REMOTE_REDIS_SERVER=/home/lucas/bench_bins/server/redis-server
REMOTE_KEYDB_SERVER=/home/lucas/bench_bins/server/keydb-server
REMOTE_MINI_REDIS_SERVER=/home/lucas/bench_bins/server/mini-redis-server
REMOTE_SERVER_PATH=/home/lucas/bench_bins/server
REMOTE_SAVE_MONITOR=/home/lucas/redis_benchmark_test

SAVE_BENCH_PATH=/Users/lucascraemer/memtier_test

# SAVE_BENCH_REDIS="$SAVE_BENCH_PATH/redis"
# mkdir $SAVE_BENCH_REDIS
# ./memtier_bench.sh $MEMTIER $REMOTE_REDIS_SERVER redis $REMOTE_IP $REMOTE_SERVER_PORT $REMOTE_USER $SAVE_BENCH_REDIS

# SAVE_BENCH_KEYDB="$SAVE_BENCH_PATH/keydb"
# mkdir $SAVE_BENCH_KEYDB
# ./memtier_bench.sh $MEMTIER $REMOTE_KEYDB_SERVER keydb $REMOTE_IP $REMOTE_SERVER_PORT $REMOTE_USER $SAVE_BENCH_KEYDB

# SAVE_BENCH_MINI_REDIS="$SAVE_BENCH_PATH/mini-redis"
# mkdir $SAVE_BENCH_MINI_REDIS
# ./memtier_bench.sh $MEMTIER $REMOTE_MINI_REDIS_SERVER mini-redis $REMOTE_IP $REMOTE_SERVER_PORT $REMOTE_USER $SAVE_BENCH_MINI_REDIS

# SAVE_BENCH_REDIS_CLUSTER="$SAVE_BENCH_PATH/redis-cluster"
# mkdir $SAVE_BENCH_REDIS_CLUSTER
# ./memtier_bench_cluster.sh $MEMTIER $REMOTE_SERVER_PATH $REMOTE_IP $REMOTE_USER $SAVE_BENCH_REDIS_CLUSTER

SAVE_BENCH_MONITOR="$SAVE_BENCH_PATH/redis-monitor"
mkdir $SAVE_BENCH_MONITOR
./redis_benchmark_monitor.sh $REMOTE_SERVER_PATH $REMOTE_SAVE_MONITOR '.' $SAVE_BENCH_MONITOR $REMOTE_IP $REMOTE_USER
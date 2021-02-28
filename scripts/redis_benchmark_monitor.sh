#!/bin/bash

REDIS_SERVER_PATH=$1
SAVE_PATH_SERVER=$2
REDIS_BENCH_PATH=$3
SAVE_PATH_CLIENT=$4
IP=$5
PORT=6379
# remote port
# remote user
USER=$6

remote="$USER@$IP"
REDIS_SERVER_MONITOR=redis-server-monitor

CLIENTS=64
PAYLOADS=(10 100 1000 10000 100000)

for payload in "${PAYLOADS[@]}"
do
    # echo ssh $remote "nohup $REDIS_SERVER_PATH/$REDIS_SERVER_MONITOR --bind 0.0.0.0 --port $PORT --appendonly no --save '' --protected-mode no >$REDIS_SERVER_MONITOR.log 2>&1 </dev/null &"

    ssh $remote "nohup $REDIS_SERVER_PATH/$REDIS_SERVER_MONITOR --bind 0.0.0.0 --port $PORT --appendonly no --save '' --protected-mode no >$REDIS_SERVER_MONITOR.log 2>&1 </dev/null &"

    sleep 5s

    mkdir $SAVE_PATH_CLIENT/$payload
    result=$SAVE_PATH_CLIENT/$payload/redis_benchmark.csv
    echo "COMMAND,REQUEST_COUNT_PER_SECOND" > $result

    # echo "$REDIS_BENCH_PATH/redis-benchmark -h $IP -c $CLIENTS -n 100000 -d $payload -t SET,GET -r 1000 --threads 4 --csv >> $result"

    $REDIS_BENCH_PATH/redis-benchmark -h $IP -c $CLIENTS -n 100000 -d $payload -t SET,GET -r 1000 --threads 4 --csv >> $result

    # ssh stop remote server
    pid=$(ssh $remote "pgrep -f $REDIS_SERVER_MONITOR[^\[]")
    #echo "ssh $remote "kill $pid && wait""
    ssh $remote "kill $pid && wait"

    # ssh stop remote server
    ssh $remote "mkdir $SAVE_PATH_SERVER/$payload && mv *.csv $SAVE_PATH_SERVER/$payload/"
done

ssh $remote "tar -czvf $SAVE_PATH_SERVER.tar.gz $SAVE_PATH_SERVER"


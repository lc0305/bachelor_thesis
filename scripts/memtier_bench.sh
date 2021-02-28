#!/bin/bash

MEMTIER=$1
# remote bin server path
SERVER=$2
# remote server type
TYPE=$3
# remote IP
IP=$4
# remote port
PORT=$5
# remote user
USER=$6
SAVE_DIR=$7

# REQ_PER_IT=10000000
REQ_PER_IT=100000

thread_counts=(1 2 4 8 12)

clients_per_threads=(1 4 8 16 32)

remote="$USER@$IP"

# largest thread count in the array times two
THREADS_MEMTIER=${thread_counts[0]}
for n in "${thread_counts[@]}"
do
    ((n > THREADS_MEMTIER)) && THREADS_MEMTIER=$n
done
let THREADS_MEMTIER="$THREADS_MEMTIER * 2"

# increase max open files for user
# ulimit -n 10000

for threads in "${thread_counts[@]}"
do
    # ssh start remote server with thread count
    if [ "$TYPE" == "redis" ]
    then
        ssh $remote "nohup $SERVER --bind 0.0.0.0 --port $PORT --io-threads $threads --io-threads-do-reads yes --appendonly no --save '' --protected-mode no >$SERVER-$threads.log 2>&1 </dev/null &"
    fi

    if [ "$TYPE" == "keydb" ]
    then
        ssh $remote "nohup $SERVER --bind 0.0.0.0 --port $PORT --server-threads $threads --min-clients-per-thread 1 --appendonly no --save '' --protected-mode no >$SERVER-$threads.log 2>&1 </dev/null &"
    fi

    if [ "$TYPE" == "mini-redis" ]
    then
        ssh $remote "nohup $SERVER-$threads --port $PORT >$SERVER-$threads.log 2>&1 </dev/null &"
    fi
    
    # wait 5 seconds for startup
    sleep 5s

    # quick warm up
    let requests_per_client_warmup="200000 / ($THREADS_MEMTIER * 8)"
    $MEMTIER -s $IP -p $PORT -t $THREADS_MEMTIER -c 8 -n $requests_per_client_warmup -d 10 --ratio=1:2

    for clients_per_thread in "${clients_per_threads[@]}"
    do
        let requests_per_client="$REQ_PER_IT / ($THREADS_MEMTIER * $clients_per_thread)"

        filename="$threads-$clients_per_thread"
        
        $MEMTIER -s $IP -p $PORT -t $THREADS_MEMTIER -c $clients_per_thread -n $requests_per_client --data-size-list=100:99,100000:1 --ratio=1:2 --key-pattern=R:R --distinct-client-seed --json-out-file="$filename.json" --hdr-file-prefix=$filename

        mv $filename* $SAVE_DIR/
    done

    # ssh stop remote server
    pid=$(ssh $remote "pgrep -f $SERVER[^\[]")
    #echo "ssh $remote "kill $pid && wait""
    ssh $remote "kill $pid && wait"
done
#!/bin/bash

MEMTIER=$1
# remote bin path
BIN_PATH=$2
# remote IP
IP=$3
# remote user
USER=$4
SAVE_DIR=$5

# REQ_PER_IT=10000000
REQ_PER_IT=100000

thread_counts=(4 8 12)

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
    ssh $remote "$BIN_PATH/create-cluster start $threads $BIN_PATH && sleep 5s && $BIN_PATH/create-cluster create $threads $BIN_PATH -f"
    # wait 5 seconds for startup
    sleep 5s

    # quick warm up
    let requests_per_client_warmup="200000 / ($THREADS_MEMTIER * 8)"
    $MEMTIER -s $IP -p 3001 -t $THREADS_MEMTIER -c 8 -n $requests_per_client_warmup -d 10 --ratio=1:2 --cluster-mode 

    for clients_per_thread in "${clients_per_threads[@]}"
    do
        let requests_per_client="$REQ_PER_IT / ($THREADS_MEMTIER * $clients_per_thread)"

        filename="$threads-$clients_per_thread"
        
        $MEMTIER -s $IP -p 3001 -t $THREADS_MEMTIER -c $clients_per_thread -n $requests_per_client --cluster-mode --data-size-list=100:99,100000:1 --ratio=1:2 --key-pattern=R:R --distinct-client-seed --json-out-file="$filename.json" --hdr-file-prefix=$filename

        mv $filename* $SAVE_DIR/
    done

    ssh $remote "$BIN_PATH/create-cluster stop $threads $BIN_PATH && sleep 5s && $BIN_PATH/create-cluster clean $threads $BIN_PATH"

    sleep 2s
done
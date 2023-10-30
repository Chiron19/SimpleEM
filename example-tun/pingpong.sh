#!/bin/bash

set -x

ifname='tun_tinyem'
laddr='172.16.0.1'
lmask='255.240.0.0'

./tinyem "${ifname}" "${laddr}" "${lmask}" &
em=$!

ifname2='tun_tinyem2'
laddr2='172.16.0.2'
lmask2='255.240.0.0'
./tinyem "${ifname2}" "${laddr2}" "${lmask2}" 

sleep 0.1

while true; do
    # 172.16.0.2 listens on port 8000
    if nc -lvN 172.16.0.2 8000 & pid1=$! ; then
    echo "Ping" | nc -4 -s 172.16.0.1 172.16.0.2 8000 & pid=$!
    sleep 0.1 ; kill ${pid}; kill ${pid1}; 
    fi

    if nc -lvN 172.16.0.1 8000 & pid1=$! ; then
    echo "Pong" | nc -4 -s 172.16.0.2 172.16.0.1 8000 & pid=$!
    sleep 0.1 ; kill ${pid}; kill ${pid1}; 
    fi

    sleep 1
done

kill ${em}

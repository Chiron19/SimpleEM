#!/bin/bash

set -x

ifname='tun_tinyem'
laddr='172.16.0.1'
lmask='255.240.0.0'

./example-tun/tinyem "${ifname}" "${laddr}" "${lmask}" & em1=$!

ifname2='tun_tinyem2'
laddr2='172.16.0.2'
lmask2='255.240.0.0'
./example-tun/tinyem "${ifname2}" "${laddr2}" "${lmask2}" & em2=$!

# while true; do
# ./src/src/tcp_server &

# ./src/src/tcp_client 

# sleep 1
# done

# sleep 1

# kill ${em2}

# kill ${em1}
#!/bin/bash

set -x

ifname='tun_tinyem'
laddr='172.16.0.1'
lmask='255.240.0.0'

./tinyem "${ifname}" "${laddr}" "${lmask}" &
em=$!

sleep 0.1

if netcat -h 2>&1 | head | grep -q '^OpenBSD' ; then
    echo 'ping!' | netcat -N '172.16.0.2' 8000 & pid=$!
    sleep 0.1 ; kill ${pid}
elif netcat -h 2>&1 | head | grep -q '^GNU' ; then
    echo 'ping!' | netcat --close '172.16.0.2' 8000
fi

kill ${em}

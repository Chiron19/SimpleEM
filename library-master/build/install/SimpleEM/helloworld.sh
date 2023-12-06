#!/bin/bash

echo -n "helloworld"

for arg in "$@"; do
    echo -n " $arg"
done

echo ""
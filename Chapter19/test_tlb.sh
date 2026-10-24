#!/bin/bash

TRIALS=1000000

for pages in 1 2 4 8 16 32 64 128 256 512 1024 2048 4096
do
    echo "Testing $pages pages..."
    ./tlb $pages $TRIALS
    echo "----------------------"
done
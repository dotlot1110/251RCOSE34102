#!/bin/bash

SRC="main.c process.c queue.c scheduler.c simulator.c display.c"

OUT="cpu_scheduler"

gcc -std=c99 -Wall -o $OUT $SRC

if [ $? -eq 0 ]; then
	echo "build success (./cpu_scheduler)"
else
	echo "build failure"
fi

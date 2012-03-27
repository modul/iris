#!/bin/sh

openocd -f openocd.cfg 2>/dev/null &
PID=$!
gdb
kill $PID

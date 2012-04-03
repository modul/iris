#!/bin/sh

openocd -f openocd.cfg 2>/dev/null &
arm-none-eabi-gdb
killall openocd

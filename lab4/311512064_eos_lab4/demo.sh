#!/bin/sh

set -x
# set -e

rmmod -f mydev
insmod mydev.ko

./writer TENGSHUHUAN &
./reader 192.168.3.198 8000 /dev/mydev

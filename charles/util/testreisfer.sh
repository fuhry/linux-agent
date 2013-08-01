#!/bin/sh
echo "Zeroing out both loopbacks"
dd if=/dev/zero of=/dev/loop0 bs=1024M count=1
dd if=/dev/zero of=/dev/loop1 bs=1024M count=1
echo "Running partclone on loop0"
./partclone/partclone.reiserfs -b -d2 -O /dev/loop0 -s /dev/sdb2
echo "Running datto on loop1"
../bin/test /dev/loop1 2

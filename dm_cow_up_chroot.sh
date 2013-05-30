#!/bin/bash

base=$1
loopdev=$2

PATH=/bin

echo "Creating duplicate DM device"
dmsetup table ${base} | dmsetup create ${base}_dup
echo "Suspending device"
if ! dmsetup suspend ${base}; then
	dmsetup remove ${base}_dup
	exit 1
fi
echo "Creating COW snapshot"
if ! echo 0 `blockdev --getsize /dev/mapper/${base}_dup` snapshot /dev/mapper/${base}_dup $loopdev p 32 | dmsetup create ${base}_cow; then
	dmsetup resume ${base}
	exit 1
fi
echo "Creating origin snapshot"
if ! echo 0 `blockdev --getsize /dev/mapper/${base}_dup` snapshot-origin /dev/mapper/${base}_dup | dmsetup create ${base}_origin; then
	dmsetup remove ${base}_cow
	dmsetup resume ${base}
	exit 1
fi
echo "Loading origin in to original device"
if ! dmsetup table ${base}_origin | dmsetup load ${base}; then
	dmsetup remove ${base}_origin
	dmsetup remove ${base}_cow
	dmsetup resume ${base}
	exit 1
fi
echo "Resuming device"
if ! dmsetup resume ${base}; then
	echo "You're probably fucked... sorry bro"
	exit 1
fi

echo "COW is now at /dev/mapper/${base}_cow"

#!/bin/sh
source `dirname $0`/settings.sh

if lsmod | grep loop &>/dev/null; then
	echo "Found loopback module..."
else
	echo "Loading loopback module..."
	modprobe loop
fi

echo "Creating virtual disk $VDISKI ($VDISKSIZE KB)..."
dd if=/dev/zero of=$VDISKI bs=1024 count=$VDISKSIZE &>/dev/null

echo "Creating virtual disk $VDISKO ($VDISKSIZE KB)..."
dd if=/dev/zero of=$VDISKO bs=1024 count=$VDISKSIZE &>/dev/null

echo "Setting up loopback device /dev/loop0 for $VDISKI..."
losetup /dev/loop0 $VDISKI

echo "Setting up loopback device /dev/loop1 for $VDISKO..."
losetup /dev/loop1 $VDISKO

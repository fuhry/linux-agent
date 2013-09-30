#!/bin/bash
source `dirname $0`/settings.sh

if lsmod | grep loop &>/dev/null; then
	echo "Found loopback module..."
else
	echo "Loading loopback module..."
	modprobe loop
fi

echo "Creating virtual disk $VDISKI ($VDISKSIZE KB)..."
dd if=/dev/zero of=$VDISKI bs=1024 seek=$VDISKSIZE count=0 &>/dev/null

echo "Creating virtual disk $VDISKO ($VDISKSIZE KB)..."
dd if=/dev/zero of=$VDISKO bs=1024 seek=$VDISKSIZE count=0 &>/dev/null

echo "Creating virtual disk $VDISKT ($VDISKSIZE KB)..."
dd if=/dev/zero of=$VDISKT bs=1024 seek=$VDISKSIZE count=0 &>/dev/null

echo "Setting up loopback device /dev/loop0 for $VDISKT..."
losetup /dev/loop0 $VDISKT || exit 1

echo "Setting up loopback device /dev/loop1 for $VDISKI..."
losetup /dev/loop1 $VDISKI || exit 1

echo "Setting up loopback device /dev/loop2 for $VDISKO..."
losetup /dev/loop2 $VDISKO || exit 1

echo "Making mount directory..."
mkdir $MNTDIR

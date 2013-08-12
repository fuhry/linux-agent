#!/bin/sh
source `dirname $0`/settings.sh

echo "Removing loopback device /dev/loop0..."
losetup -d /dev/loop0

echo "Removing loopback device /dev/loop1..."
losetup -d /dev/loop1

echo "Removing loopback device /dev/loop2..."
losetup -d /dev/loop2

echo "Removing disk images..."
rm -f $VDISKI $VDISKO $VDISKT

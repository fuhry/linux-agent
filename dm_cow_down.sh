#!/bin/bash

usage()
{
	cat <<EOF
Usage: $0 dm_device

EOF
	exit 1
}

base=$1
if [ ! -b `readlink -f /dev/mapper/$base` ]; then
	usage
fi

if [ ! -b `readlink -f /dev/mapper/${base}_cow` ]; then
	echo "No COW snapshot is active for DM device $base"
	exit 1
fi

loopdev=`losetup -a | fgrep "/backup_cow/${base}.img" | awk '{print $1;}' | tr -d :`
if [ -z "$loopdev" ]; then
	usage
fi

if mount | grep "^/dev/mapper/${base}_cow " > /dev/null; then
	echo "${base}_cow is mounted - refusing to dismount"
	exit 1
fi

# quiesce the disks to make things as sane as possible before doing the dangerous part
sync; sync
chroot /backup_cow/chroot /bin/bash /dm_cow_down_chroot.sh ${base} || exit 1

echo "Detaching loop device"
losetup -d $loopdev || exit 1

echo "Checking to see if any other devices are currently COW'd"
if losetup -a | fgrep /backup_cow/ > /dev/null; then
	echo "	Yes there are - exiting now and not unmounting anything"
	exit 0
fi

echo "Unmounting /backup_cow and cleaning up"
if ! umount /backup_cow/chroot/dev; then
	sleep 1
	umount /backup_cow/chroot/dev || exit 1
fi
umount /backup_cow/chroot/proc || exit 1
umount /backup_cow || exit 1
rm -r /backup_cow

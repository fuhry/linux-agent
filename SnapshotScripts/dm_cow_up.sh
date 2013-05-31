#!/bin/bash

usage()
{
	cat <<EOF
Usage: $0 dm_device

EOF
	exit 1
}

base="$1"
if [ ! -b `readlink -f "/dev/mapper/$base"` ]; then
	usage
fi
if [ -b `readlink -f "/dev/mapper/${base}_cow"` ]; then
	echo "A COW snapshot is already mounted for this device. Refusing to continue."
	exit 1
fi

if ! cat /proc/mounts  | grep -q " /backup_cow tmpfs "; then
	[ ! -d /backup_cow ] && mkdir /backup_cow
	mount -t tmpfs tmpfs_datto_cow /backup_cow || exit 1
fi

if ! lsmod | grep -q dm_snapshot > /dev/null ; then
	modprobe dm_snapshot
fi

dd if=/dev/null bs=1M seek=4096 count=0 of=/backup_cow/$base.img 2>/dev/null || exit 1
loopdev=`losetup -f`
losetup $loopdev /backup_cow/$base.img || exit 1

echo "Building chroot..."
mkdir -p /backup_cow/chroot/bin
if [ ! -d /backup_cow/chroot/dev/mapper ]; then
	mkdir -p /backup_cow/chroot/dev
	mount -o bind /dev /backup_cow/chroot/dev
	mkdir -p /backup_cow/chroot/proc
	mount -o bind /proc /backup_cow/chroot/proc
	cp `which dmsetup` `which blockdev` `which bash` `which sleep` /backup_cow/chroot/bin/
	if [ -d /lib64 -a ! -h /lib64 ]; then
		libdir=/backup_cow/chroot/lib64
		mkdir $libdir
		ln -sf lib64 /backup_cow/chroot/lib
	else
		libdir=/backup_cow/chroot/lib
		mkdir $libdir
		ln -sf lib /backup_cow/chroot/lib64
	fi
	# this is for systems where /lib is a symlink to /usr/lib
	ln -s . /backup_cow/chroot/usr
	ldd `which dmsetup` | awk '{print $3;}' | grep ^/ | xargs cp -Lvt $libdir/
	ldd `which blockdev` | awk '{print $3;}' | grep ^/ | xargs cp -Lvt $libdir/
	ldd `which bash` | awk '{print $3;}' | grep ^/ | xargs cp -Lvt $libdir/
	ldd `which sleep` | awk '{print $3;}' | grep ^/ | xargs cp -Lvt $libdir/
	ldd `which dmsetup` | egrep '^\s*/' | awk '{print $1;}' | xargs cp -Lvt $libdir/
	cp "$(dirname ${BASH_SOURCE[0]})"/dm_cow_{up,down}_chroot.sh /backup_cow/chroot/
fi

exec chroot /backup_cow/chroot /bin/bash /dm_cow_up_chroot.sh "${base}" "${loopdev}"

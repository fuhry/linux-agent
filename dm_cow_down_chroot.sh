#!/bin/bash

base=$1
PATH=/bin

#set -o monitor
dmexit=0

# We have to be SUPER SUPER careful here to handle cases where dmsetup might time out for any reason. This safe_command function
# gives the command 10 seconds, after which we report failure and resume access to the device. This is because we're potentially
# dealing with the system's root filesystem here, and this is the equivalent of a quad bypass surgery where they stop your heart
# ;)
# It also means we need to avoid the use of non-builtin commands, as these most likely do not exist in the chroot.
safe_command()
{
	local desc="$1"
	shift
	local cmd="$@"
	
	echo -n $desc
	#trap 'echo -n c; wait $dmpid; dmexit=$?' CHLD
	# the trap on interrupts and SIGTERM is a last ditch diagnostic option
	trap "dmsetup resume ${base}" INT TERM
	$cmd &
	dmpid=$!
	local finished=n
	for (( i=0; i < 50; i++ )); do
		echo -n .
		if ! kill -0 $dmpid 2>/dev/null ; then
			finished=y
			break
		fi
		sleep 0.2
	done
	if [ "$finished" = "n" ]; then
		echo -e "\nCommand timed out: $cmd"
		trap - INT TERM
		dmsetup resume ${base}
		exit 1
	fi
	echo ""
	if [ $dmexit -ge 1 ]; then
		echo "Command failed: $cmd"
		#dmsetup resume ${base}
		#exit $dmexit
	fi
	trap - INT TERM
	return 0
}

echo "Suspending device"
dmsetup suspend ${base} || exit 1

safe_command "Removing origin snap" dmsetup remove ${base}_origin
safe_command "Removing COW snap" dmsetup remove ${base}_cow

echo "Swapping dupe back in as base device"
if ! dmsetup table ${base}_dup | dmsetup load ${base}; then
	dmsetup resume ${base}
	exit 1
fi
echo "Resuming device"
if ! dmsetup resume ${base}; then
	echo "Whoops... shit."
	exit 1
fi
echo "Removing dupe"
dmsetup remove ${base}_dup

exit 0

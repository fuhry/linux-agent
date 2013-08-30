#!/bin/bash
source `dirname $0`/settings.sh

INPUTDEV=/dev/loop0
PARTCLONEOUT=/dev/loop1
DATTOOUT=/dev/loop2

if [ -e $DATTOEXEC ]; then
	echo "[SETUP]"
	bash setup.sh
	printf "\n"
	echo "Running mkfs on $INPUTDEV..."
	mkfs.xfs $INPUTDEV &>/dev/null
	
	echo "Running partclone on $INPUTDEV to $PARTCLONEOUT..."
	$PARTCLONED.xfs -b -O $PARTCLONEOUT -s $INPUTDEV &>/dev/null
	
	echo "Running datto on $INPUTDEV to $DATTOOUT..."
	$DATTOEXEC -i $INPUTDEV -o $DATTOOUT -t xfs &>/dev/null
	printf "\n"
	
	echo "[TEST]"
	echo -ne "Binary diffing $PARTCLONEOUT and $DATTOOUT... "
	diff $PARTCLONEOUT $DATTOOUT &>/dev/null
	if [ $? -eq 0 ]; then
		echo "SUCCESS"
	else
		echo "FAIL" 
	fi
	
	mount $INPUTDEV $MNTDIR -o ro,noexec
	ORIGTREE=$(cd $MNTDIR && find . | sort)
	INDUMP=$(xfs_info $INPUTDEV 2>/dev/null)
	umount $MNTDIR
	
	mount $DATTOOUT $MNTDIR -o ro,noexec
	DATTOTREE=$(cd $MNTDIR && find . | sort)
	DATTODUMP=$(xfs_info $DATTOOUT 2>/dev/null)	
	umount $MNTDIR
	
	echo -ne "xfs_info diffing $INPUTDEV and $DATTOOUT... "
	if [ "${INDUMP:21}" == "${DATTODUMP:21}" ]; then
		echo "SUCCESS"
	else
		echo "FAIL" 
	fi
	
	echo -ne "Diffing directory tree of $INPUTDEV and $DATTOOUT... "
	if [ "$ORIGTREE" == "$DATTOTREE" ]; then
		echo "SUCCESS"
	else
		echo "FAIL" 
	fi
	
	printf "\n"
	echo "[CLEANUP]"
	bash cleanup.sh
	
else
	echo "Executable does not exist!"
fi

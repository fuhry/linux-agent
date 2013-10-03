#!/bin/bash
set -e
source `dirname $0`/settings.sh

INPUTDEV=/dev/loop0
PARTCLONEOUT=/dev/loop1
DATTOOUT=/dev/loop2

if [ -e $DATTOEXEC ]; then
	echo "[SETUP]"
	bash setup.sh
	printf "\n"
	echo "Running mkfs on $INPUTDEV..."
	mkfs.ext2 $INPUTDEV
	
	echo "Running partclone on $INPUTDEV to $PARTCLONEOUT..."
	$PARTCLONED.extfs -b -O $PARTCLONEOUT -s $INPUTDEV
	
	echo "Running datto on $INPUTDEV to $DATTOOUT..."
	$DATTOEXEC -i $INPUTDEV -o $DATTOOUT -t ext
	printf "\n"
	
	echo "[TEST]"
	echo -ne "Binary diffing $PARTCLONEOUT and $DATTOOUT... "
	diff $PARTCLONEOUT $DATTOOUT
	if [ $? -eq 0 ]; then
		echo "SUCCESS"
	else
		echo "FAIL" 
	fi

	echo -ne "dumpe2fs diffing $INPUTDEV and $DATTOOUT... "
	INDUMP=$(dumpe2fs $INPUTDEV 2>/dev/null)
	DATTODUMP=$(dumpe2fs $DATTOOUT 2>/dev/null)	
	if [ "$INDUMP" == "$DATTODUMP" ]; then
		echo "SUCCESS"
	else
		echo "FAIL" 
	fi
	
	mount $INPUTDEV $MNTDIR -o ro,noexec
	ORIGTREE=$(cd $MNTDIR && find . | sort)
	umount $MNTDIR
	
	mount $DATTOOUT $MNTDIR -o ro,noexec
	DATTOTREE=$(cd $MNTDIR && find . | sort)
	umount $MNTDIR
	
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

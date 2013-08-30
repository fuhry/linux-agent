#!/bin/bash
source `dirname $0`/settings.sh

INPUTDEV=/dev/loop0
PARTCLONEOUT=/dev/loop1
DATTOOUT=/dev/loop2

if [ -e $DATTOEXEC ]; then
	echo "[SETUP]"
	sh setup.sh
	printf "\n"
	echo "Running mkfs on $INPUTDEV..."
	mkfs.reiserfs $INPUTDEV &>/dev/null <<< 'y'
	
	echo "Running partclone on $INPUTDEV to $PARTCLONEOUT..."
	$PARTCLONED.reiserfs -b -O $PARTCLONEOUT -s $INPUTDEV &>/dev/null
	
	echo "Running datto on $INPUTDEV to $DATTOOUT..."
	$DATTOEXEC -i $INPUTDEV -o $DATTOOUT -t reiserfs &>/dev/null
	printf "\n"
	
	echo "[TEST]"
	echo -ne "Binary diffing $PARTCLONEOUT and $DATTOOUT... "
	diff $PARTCLONEOUT $DATTOOUT &>/dev/null
	if [ $? -eq 0 ]; then
		echo "SUCCESS"
	else
		echo "FAIL" 
	fi

	echo -ne "debugreiserfs diffing $INPUTDEV and $DATTOOUT... "
	INDUMP=$(debugreiserfs $INPUTDEV 2>/dev/null | grep -v 'super block in')
	DATTODUMP=$(debugreiserfs $DATTOOUT 2>/dev/null | grep -v 'super block in')
	if [ "$INDUMP" == "$DATTODUMP" ]; then
		echo "SUCCESS"
	else
		echo "FAIL" 
	fi
	
	mount -t reiserfs $INPUTDEV $MNTDIR -o ro,noexec
	ORIGTREE=$(cd $MNTDIR && find . | sort)
	umount $MNTDIR
	
	mount -t reiserfs $DATTOOUT $MNTDIR -o ro,noexec
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
	sh cleanup.sh
	
else
	echo "Executable does not exist!"
fi

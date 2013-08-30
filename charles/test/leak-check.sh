#!/bin/bash
source `dirname $0`/settings.sh

INPUTDEV=/dev/loop0
DATTOOUT=/dev/loop1

if [ -e $DATTOEXEC ]; then
	echo "[SETUP]"
	sh setup.sh
	printf "\n"
	
	echo "[TEST]"
	echo "Running mkfs on $INPUTDEV..."
	mkfs.ext2 $INPUTDEV &>/dev/null
	echo -ne "Running datto on $INPUTDEV to $DATTOOUT through valgrind... "
	LEAKS=`valgrind --leak-check=full $DATTOEXEC -i $INPUTDEV -o $DATTOOUT -t ext 2>&1 | grep 'All heap blocks were freed'`
	if [[ "$LEAKS" =~ .*All.* ]]; then
		echo "SUCCESS"
	else
		echo "FAIL" 
	fi
	printf "\n"
	
	echo "Running mkfs on $INPUTDEV..."
	dd if=/dev/zero of=$INPUTDEV bs=1024 count=$VDISKSIZE &>/dev/null
	mkfs.xfs $INPUTDEV &>/dev/null
	echo -ne "Running datto on $INPUTDEV to $DATTOOUT through valgrind... "
	LEAKS=`valgrind --leak-check=full $DATTOEXEC -i $INPUTDEV -o $DATTOOUT -t xfs 2>&1 | grep 'All heap blocks were freed'`
	if [[ "$LEAKS" =~ .*All.* ]]; then
		echo "SUCCESS"
	else
		echo "FAIL" 
	fi
	printf "\n"
	
	echo "Running mkfs on $INPUTDEV..."
	dd if=/dev/zero of=$INPUTDEV bs=1024 count=$VDISKSIZE &>/dev/null
	mkfs.reiserfs $INPUTDEV &>/dev/null <<< 'y'
	echo -ne "Running datto on $INPUTDEV to $DATTOOUT through valgrind... "
	LEAKS=`valgrind --leak-check=full $DATTOEXEC -i $INPUTDEV -o $DATTOOUT -t reiserfs 2>&1 | grep 'All heap blocks were freed'`
	if [[ "$LEAKS" =~ .*All.* ]]; then
		echo "SUCCESS"
	else
		echo "FAIL" 
	fi
	printf "\n"
	
	echo "Running mkfs on $INPUTDEV..."
	dd if=/dev/zero of=$INPUTDEV bs=1024 count=$VDISKSIZE &>/dev/null
	mkfs.btrfs $INPUTDEV &>/dev/null
	echo -ne "Running datto on $INPUTDEV to $DATTOOUT through valgrind... "
	LEAKS=`valgrind --leak-check=full $DATTOEXEC -i $INPUTDEV -o $DATTOOUT -t btrfs 2>&1 | grep 'All heap blocks were freed'`
	if [[ "$LEAKS" =~ .*All.* ]]; then
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

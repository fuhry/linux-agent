#!/bin/sh
if [ -e "$DATTOEXEC" ]; then

	echo "[SETUP]"
	sh setup.sh
	printf "\n"

	echo "[TEST]"
	echo "Not implemented!"

	printf "\n"
	echo "[CLEANUP]"
	sh cleanup.sh
	
else
	echo "Executable does not exist!"
fi

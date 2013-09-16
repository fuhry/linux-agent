#!/bin/bash
FEATURE="CONFIG_DM_SNAPSHOT"
MODULE="dm_snapshot"

feature_line=$(grep /boot/config-$(uname -r) -e "$FEATURE")
if [ -n "$feature_line" ]; then
    last_char="${feature_line:${#feature_line}-1}"
    if [ $last_char = "m" ]; then
        echo -n "Loading $MODULE... "
        if ! modprobe "$MODULE"; then
            echo "Error loading $MODULE"
            exit 1
        else
            echo "done."
        fi
    elif [ $last_char = "y" ]; then
        echo "$MODULE is compiled into the kernel"
    fi
else
    echo "Problem! Can't find support for $FEATURE"
    exit 1
fi
exit 0

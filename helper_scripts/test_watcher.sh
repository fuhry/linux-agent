#!/bin/bash
watch 'dmsetup udevcookies && echo; clang++ -o tests/table_test -I. device_mapper/dm_table_task.cc device_mapper/dm_task.cc device_mapper/dm_target.cc tests/dm_table_task_test.cc -lglog -ldevmapper && tests/table_test'

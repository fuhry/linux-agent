#!/bin/bash
# This is a temporary file until the build system is ironed out

TEST_LIBS="test/gtest/libgtest_main.a test/gtest/libgtest.a"

if [ ! -r test/gtest/libgtest.a ]; then
    pushd test/gtest

    cmake CMakeLists.txt
    make

    popd
fi

clang++ -std=c++11 $TEST_LIBS unsynced_sector_tracker/unsynced_sector_tracker.cc test/unsynced_block_tracker_test.cc -I. -pthread && ./a.out

clang++ -std=c++11 $TEST_LIBS block_trace/device_tracer.cc test/device_tracer_test.cc -I. -pthread && ./a.out


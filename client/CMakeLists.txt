cmake_minimum_required(VERSION 2.8.0)
set(CMAKE_CXX_FLAGS "-Wall -g -Wshadow -pthread ${CMAKE_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS "-std=c++0x ${CMAKE_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS "-lglog ${CMAKE_CXX_FLAGS}")

include_directories(.)
include(cmake/test_setup.cmake)

add_unit_test(unsynced_sector_tracker_test
              unsynced_sector_tracker/unsynced_sector_tracker.cc)

add_unit_test(device_tracer_test
              test/loop_device.cc
              block_trace/device_tracer.cc
              block_trace/cpu_tracer.cc
              block_trace/trace_handler.cc
              unsynced_sector_tracker/unsynced_sector_tracker.cc)

add_unit_test(nbd_block_device_test
              test/loop_device.cc
              block_device/block_device.cc
              remote_block_device/nbd_client.cc
              remote_block_device/nbd_block_device.cc)

add_unit_test(block_device_test
              test/loop_device.cc
              block_device/block_device.cc)

add_unit_test(mountable_block_device_test
              test/loop_device.cc
              block_device/mountable_block_device.cc
              block_device/block_device.cc)

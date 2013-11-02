cmake_minimum_required(VERSION 2.8.0)
set(CMAKE_CXX_FLAGS "-Wall -Wshadow -pthread ${CMAKE_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS "-std=c++0x ${CMAKE_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS "-lglog ${CMAKE_CXX_FLAGS}")

include_directories(.)

# Take care of building the protobuf files first
find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
file (GLOB PROTO_FILES messages/*.proto)

# C++ protobuf files
PROTOBUF_GENERATE_CPP(PROTO_SRCS PROTO_HDRS ${PROTO_FILES})

# Python protobuf files
add_custom_target(protobuf_python)
add_custom_command(TARGET protobuf_python
                   COMMAND protoc --python_out=${CMAKE_CURRENT_BINARY_DIR}
                           -I${CMAKE_CURRENT_SOURCE_DIR}/messages
                           ${PROTO_FILES})

# dattocli
add_custom_target(dattocli)
add_dependencies(dattocli protobuf_python)
add_custom_command(TARGET dattocli
                   COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/dattocli/dattocli
                              ${CMAKE_CURRENT_SOURCE_DIR}/dattocli/argparse.py
                              ${CMAKE_CURRENT_BINARY_DIR})

# dattod
add_executable(dattod
               backup/backup.cc
               backup/backup_manager.cc
               backup/full_backup.cc
               block_device/block_device.cc
               block_device/mountable_block_device.cc
               block_trace/cpu_tracer.cc
               block_trace/device_tracer.cc
               block_trace/trace_handler.cc
               dattod/dattod.cc
               dattod/dattod.cc
               dattod/flock.cc
               dattod/signal_handler.cc
               device_synchronizer/device_synchronizer.cc
               fs_parsing/ext_file_system.cc
               fs_parsing/ext_mountable_block_device.cc
               remote_block_device/nbd_block_device.cc
               remote_block_device/nbd_client.cc
               request_listener/ipc_request_listener.cc
               request_listener/request_handler.cc
               request_listener/socket_reply_channel.cc
               unsynced_sector_manager/unsynced_sector_manager.cc
               unsynced_sector_manager/unsynced_sector_store.cc
               ${PROTO_SRCS})
target_link_libraries(dattod glog)
target_link_libraries(dattod ext2fs)
target_link_libraries(dattod ${PROTOBUF_LIBRARIES})

# Unit tests
include(cmake/test_setup.cmake)

add_unit_test(unsynced_sector_store_test
              unsynced_sector_manager/unsynced_sector_store.cc)

add_unit_test(flock_test
              dattod/flock.cc)

add_unit_test(signal_handler_test
              dattod/signal_handler.cc)

add_unit_test(device_tracer_test
              test/loop_device.cc
              block_trace/device_tracer.cc
              block_trace/cpu_tracer.cc
              block_trace/trace_handler.cc
              unsynced_sector_manager/unsynced_sector_store.cc)

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

add_unit_test(device_synchronizer_test
              test/loop_device.cc
              block_device/block_device.cc
              block_device/mountable_block_device.cc
              unsynced_sector_manager/unsynced_sector_store.cc
              ${PROTO_SRCS}
              device_synchronizer/device_synchronizer.cc)
target_link_libraries(device_synchronizer_test ${PROTOBUF_LIBRARIES})

add_unit_test(unsynced_sector_manager_test
              block_trace/device_tracer.cc
              block_trace/cpu_tracer.cc
              block_trace/trace_handler.cc
              test/loop_device.cc
              unsynced_sector_manager/unsynced_sector_store.cc
              unsynced_sector_manager/unsynced_sector_manager.cc)

add_unit_test(ipc_request_listener_test
              request_listener/socket_reply_channel.cc
              ${PROTO_SRCS}
              request_listener/ipc_request_listener.cc)
target_link_libraries(ipc_request_listener_test ${PROTOBUF_LIBRARIES})

add_unit_test(extfs_test
              test/loop_device.cc
              fs_parsing/ext_mountable_block_device.cc
              fs_parsing/ext_file_system.cc
              block_device/mountable_block_device.cc
              block_device/block_device.cc)
target_link_libraries(extfs_test ext2fs com_err)

#add_unit_test(xfs_test
#              test/xfs_test.cc
#              fs_parsing/xfs_mountable_block_device.cc
#              block_device/mountable_block_device.cc
#              block_device/block_device.cc)

# dattocli test
#add_test(dattocli_test dattocli_test)
#add_custom_target(ctest_dattocli_test
#                  ${CMAKE_CURRENT_SOURCE_DIR}/test/dattocli_test.py
#                  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

#add_dependencies(ctest_dattocli_test dattocli)
#add_dependencies(ctest_dattocli_test dattocli_test)
#add_dependencies(check ctest_dattocli_test)

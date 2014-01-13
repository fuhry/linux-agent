cmake_minimum_required(VERSION 2.8.0)
set(CMAKE_CXX_FLAGS "-Wall -Wshadow -pthread ${CMAKE_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS "-std=c++0x ${CMAKE_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS "-lglog ${CMAKE_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS "-D_GNU_SOURCE ${CMAKE_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS "-D_LARGEFILE_SOURCE ${CMAKE_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS "-D_FILE_OFFSET_BITS=64 ${CMAKE_CXX_FLAGS}")

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
                              ${CMAKE_CURRENT_SOURCE_DIR}/python_libs/argparse.py
                              ${CMAKE_CURRENT_SOURCE_DIR}/python_libs/dattolib.py
                              ${CMAKE_CURRENT_BINARY_DIR})
# dattonet
add_custom_target(dattonet)
add_dependencies(dattonet protobuf_python)
add_custom_command(TARGET dattonet
                   COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/dattonet/dattonet
                              ${CMAKE_CURRENT_SOURCE_DIR}/python_libs/dattolib.py
                              ${CMAKE_CURRENT_BINARY_DIR})

# Doesn't build currently
## dattod
#add_executable(dattod
#               backup/backup.cc
#               backup/backup_manager.cc
#               backup/full_backup.cc
#               backup/in_progress_path_set.cc
#               backup/incremental_backup.cc
#               backup_event_tracker/backup_event_handler.cc
#               backup_event_tracker/backup_event_tracker.cc
#               block_device/block_device.cc
#               block_device/mountable_block_device.cc
#               block_trace/cpu_tracer.cc
#               block_trace/device_tracer.cc
#               block_trace/trace_handler.cc
#               cancellation/cancellation_token.cc
#               dattod/dattod.cc
#               dattod/dattod.cc
#               dattod/flock.cc
#               dattod/signal_handler.cc
#               device_synchronizer/device_synchronizer.cc
#               fs_parsing/ext_file_system.cc
#               fs_parsing/ext_mountable_block_device.cc
#               remote_block_device/nbd_block_device.cc
#               remote_block_device/nbd_client.cc
#               request_listener/ipc_request_listener.cc
#               request_listener/request_handler.cc
#               request_listener/socket_reply_channel.cc
#               unsynced_sector_manager/unsynced_sector_manager.cc
#               unsynced_sector_manager/unsynced_sector_store.cc
#               ${PROTO_SRCS})
#target_link_libraries(dattod com_err)
#target_link_libraries(dattod ext2fs)
#target_link_libraries(dattod glog)
#target_link_libraries(dattod uuid)
#target_link_libraries(dattod ${PROTOBUF_LIBRARIES})

# Unit tests
include(cmake/test_setup.cmake)

add_unit_test(backup_coordinator_test
              backup/backup_coordinator.cc)

add_unit_test(block_device_test
              test/loop_device.cc
              block_device/block_device.cc)

add_unit_test(backup_status_tracker_test
              backup_status_tracker/backup_event_handler.cc
              ${PROTO_SRCS}
              backup_status_tracker/backup_status_tracker.cc)
target_link_libraries(backup_status_tracker_test ${PROTOBUF_LIBRARIES})

add_unit_test(device_tracer_test
              test/loop_device.cc
              block_trace/device_tracer.cc
              block_trace/cpu_tracer.cc
              block_trace/trace_handler.cc
              unsynced_sector_manager/unsynced_sector_store.cc)

add_unit_test(extfs_test
              test/loop_device.cc
              block_device/ext_file_system.cc
              block_device/mountable_block_device.cc
              block_device/block_device.cc
              block_device/ext_mountable_block_device.cc)
target_link_libraries(extfs_test ext2fs com_err)

add_unit_test(flock_test
              dattod/flock.cc)

# Out until backup_manager.h is fixed up
#add_unit_test(ipc_request_listener_test
#              request_listener/socket_reply_channel.cc
#              ${PROTO_SRCS}
#              request_listener/ipc_request_listener.cc)
#target_link_libraries(ipc_request_listener_test ${PROTOBUF_LIBRARIES})

add_unit_test(mountable_block_device_test
              test/loop_device.cc
              block_device/mountable_block_device.cc
              block_device/block_device.cc)

add_unit_test(nbd_block_device_test
              test/loop_device.cc
              block_device/block_device.cc
              block_device/nbd_client.cc
              block_device/nbd_block_device.cc)

add_unit_test(signal_handler_test
              dattod/signal_handler.cc)

add_unit_test(unsynced_sector_manager_test
              block_device/block_device.cc
              block_trace/cpu_tracer.cc
              block_trace/device_tracer.cc
              block_trace/trace_handler.cc
              test/loop_device.cc
              unsynced_sector_manager/unsynced_sector_store.cc
              unsynced_sector_manager/unsynced_sector_manager.cc)

add_unit_test(unsynced_sector_store_test
              test/loop_device.cc
              unsynced_sector_manager/unsynced_sector_store.cc)

# Packaging
set(CPACK_GENERATOR "DEB;RPM")
set(CPACK_PACKAGE_CONTACT "Nick Garvey ngarvey@dattobackup.com")
set(CPACK_PACKAGE_EXECUTABLES "Nick Garvey ngarvey@dattobackup.com")
# XXX CMACK?
set(CMACK_INSTALL_DIRECTORY "datto")

file(GLOB python_protobufs ${CMAKE_CURRENT_BINARY_DIR}/*_pb2.py)
file(GLOB python_libs ${CMAKE_CURRENT_SOURCE_DIR}/python_libs/*.py)

# Doesn't build currently
#install(TARGETS dattod DESTINATION /usr/sbin)
#install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/dattocli DESTINATION /usr/lib/datto)
#install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/dattonet DESTINATION /usr/lib/datto)
#install(FILES ${python_protobufs} DESTINATION /usr/lib/datto)
#install(FILES ${python_libs} DESTINATION /usr/lib/datto)

include(CPack)

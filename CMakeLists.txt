cmake_minimum_required(VERSION 2.8.0)
set(CMAKE_CXX_FLAGS "-Wall -Wshadow -pthread ${CMAKE_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS "-std=c++0x ${CMAKE_CXX_FLAGS}")
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
                   COMMAND cp -f
                       ${CMAKE_CURRENT_SOURCE_DIR}/dattocli/dattocli
                       ${CMAKE_CURRENT_SOURCE_DIR}/python_libs/argparse.py
                       ${CMAKE_CURRENT_SOURCE_DIR}/python_libs/dattolib.py
                       ${CMAKE_CURRENT_BINARY_DIR})
# dattonet
add_custom_target(dattonet)
add_dependencies(dattonet protobuf_python)
add_custom_command(TARGET dattonet
                   COMMAND cp -f
                       ${CMAKE_CURRENT_SOURCE_DIR}/dattonet/dattonet
                       ${CMAKE_CURRENT_SOURCE_DIR}/python_libs/dattolib.py
                       ${CMAKE_CURRENT_BINARY_DIR})

# Doesn't build currently
## dattod
add_executable(dattod
               backup/backup.cc
               backup/backup_builder.cc
               backup/backup_coordinator.cc
               backup/backup_manager.cc
               backup_status_tracker/backup_event_handler.cc
               backup_status_tracker/backup_status_tracker.cc
               backup_status_tracker/sync_count_handler.cc
               block_device/block_device.cc
               block_device/block_device_factory.cc
               block_device/ext_file_system.cc
               block_device/ext_mountable_block_device.cc
               block_device/mountable_block_device.cc
               block_device/nbd_block_device.cc
               block_device/nbd_client.cc
               block_device/xfs_mountable_block_device.cc
               block_trace/cpu_tracer.cc
               block_trace/device_tracer.cc
               block_trace/trace_handler.cc
               dattod/dattod.cc
               dattod/flock.cc
               dattod/signal_handler.cc
               device_synchronizer/device_synchronizer.cc
               request_listener/ipc_request_listener.cc
               request_listener/request_handler.cc
               request_listener/socket_reply_channel.cc
               unsynced_sector_manager/unsynced_sector_manager.cc
               unsynced_sector_manager/unsynced_sector_store.cc
               ${PROTO_SRCS})
target_link_libraries(dattod com_err ext2fs glog blkid boost_regex uuid
                      ${PROTOBUF_LIBRARIES})

# Unit tests
include(cmake/test_setup.cmake)

add_unit_test(block_device_factory_test
              block_device/block_device.cc
              block_device/ext_file_system.cc
              block_device/ext_mountable_block_device.cc
              block_device/mountable_block_device.cc
              block_device/nbd_block_device.cc
              block_device/nbd_client.cc
              block_device/nbd_server.cc
              block_device/xfs_mountable_block_device.cc
              test/loop_device.cc
              block_device/block_device_factory.cc)
target_link_libraries(block_device_factory_test ext2fs com_err boost_regex
                      blkid uuid)

add_unit_test(backup_test
              backup/backup_coordinator.cc
              backup_status_tracker/backup_event_handler.cc
              backup_status_tracker/sync_count_handler.cc
              block_device/block_device.cc
              block_device/mountable_block_device.cc
              ${PROTO_SRCS}
              backup/backup.cc)
target_link_libraries(backup_test blkid uuid ${PROTOBUF_LIBRARIES})

add_unit_test(backup_coordinator_test
              backup/backup_coordinator.cc)

add_unit_test(backup_manager_test
              ${PROTO_SRCS}
              backup/backup.cc
              backup/backup_builder.cc
              backup/backup_coordinator.cc
              backup_status_tracker/backup_event_handler.cc
              backup_status_tracker/backup_status_tracker.cc
              backup_status_tracker/sync_count_handler.cc
              block_trace/cpu_tracer.cc
              block_trace/device_tracer.cc
              block_trace/trace_handler.cc
              device_synchronizer/device_synchronizer.cc
              unsynced_sector_manager/unsynced_sector_manager.cc
              unsynced_sector_manager/unsynced_sector_store.cc
              backup/backup_manager.cc)
target_link_libraries(backup_manager_test uuid ${PROTOBUF_LIBRARIES})

add_unit_test(backup_status_tracker_test
              backup_status_tracker/backup_event_handler.cc
              backup_status_tracker/sync_count_handler.cc
              ${PROTO_SRCS}
              backup_status_tracker/backup_status_tracker.cc)
target_link_libraries(backup_status_tracker_test ${PROTOBUF_LIBRARIES})

add_unit_test(block_device_test
              test/loop_device.cc
              block_device/block_device.cc)

add_unit_test(device_synchronizer_test
              backup_status_tracker/backup_event_handler.cc
              backup_status_tracker/sync_count_handler.cc
              backup/backup_coordinator.cc
              block_device/block_device.cc
              block_device/mountable_block_device.cc
              block_trace/cpu_tracer.cc
              block_trace/device_tracer.cc
              block_trace/trace_handler.cc
              test/loop_device.cc
              unsynced_sector_manager/unsynced_sector_manager.cc
              unsynced_sector_manager/unsynced_sector_store.cc
              ${PROTO_SRCS}
              device_synchronizer/device_synchronizer.cc)
target_link_libraries(device_synchronizer_test blkid uuid ${PROTOBUF_LIBRARIES})

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
target_link_libraries(extfs_test blkid ext2fs com_err)

add_unit_test(flock_test
              dattod/flock.cc)

add_unit_test(ipc_request_listener_test
              backup/backup.cc
              backup/backup_coordinator.cc
              backup/backup_manager.cc
              backup_status_tracker/backup_event_handler.cc
              backup_status_tracker/backup_status_tracker.cc
              backup_status_tracker/sync_count_handler.cc
              request_listener/request_handler.cc
              request_listener/socket_reply_channel.cc
              ${PROTO_SRCS}
              request_listener/ipc_request_listener.cc)
target_link_libraries(ipc_request_listener_test uuid ${PROTOBUF_LIBRARIES})

add_unit_test(mountable_block_device_test
              test/loop_device.cc
              block_device/mountable_block_device.cc
              block_device/block_device.cc)
target_link_libraries(mountable_block_device_test blkid)

add_unit_test(nbd_block_device_test
              test/loop_device.cc
              block_device/block_device.cc
              block_device/nbd_client.cc
              block_device/nbd_server.cc
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

#add_unit_test(xfs_test
#              test/loop_device.cc
#              block_device/mountable_block_device.cc
#              block_device/block_device.cc
#              block_device/xfs_mountable_block_device.cc)

add_custom_target(build)
add_dependencies(build dattod dattocli dattonet)


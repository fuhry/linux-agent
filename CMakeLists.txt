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
add_custom_target(protobuf_python ALL COMMAND mkdir -p ${CMAKE_CURRENT_BINARY_DIR}/dattolib)
add_custom_command(TARGET protobuf_python
                   COMMAND protoc --python_out=${CMAKE_CURRENT_BINARY_DIR}/dattolib/
                           -I${CMAKE_CURRENT_SOURCE_DIR}/messages
                           ${PROTO_FILES})

# dattolib
add_custom_target(dattolib ALL COMMAND mkdir -p ${CMAKE_CURRENT_BINARY_DIR}/dattolib)
add_dependencies(dattolib protobuf_python)
add_custom_command(TARGET dattolib
                   COMMAND cp -rf
                       ${CMAKE_CURRENT_SOURCE_DIR}/dattolib/
                       ${CMAKE_CURRENT_BINARY_DIR})

# dattocli
add_custom_target(dattocli ALL)
add_dependencies(dattocli dattolib)
add_custom_command(TARGET dattocli
                   COMMAND cp -f
                       ${CMAKE_CURRENT_SOURCE_DIR}/dattocli/dattocli
                       ${CMAKE_CURRENT_BINARY_DIR})
# dattonet
add_custom_target(dattonet ALL)
add_dependencies(dattonet dattolib)
add_custom_command(TARGET dattonet
                   COMMAND cp -f
                       ${CMAKE_CURRENT_SOURCE_DIR}/dattonet/dattonet
                       ${CMAKE_CURRENT_BINARY_DIR})

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
               logging/queuing_log_sink.cc
               request_listener/ipc_request_listener.cc
               request_listener/request_handler.cc
               request_listener/socket_reply_channel.cc
               unsynced_sector_manager/unsynced_sector_manager.cc
               unsynced_sector_manager/unsynced_sector_store.cc
               ${PROTO_SRCS})
target_link_libraries(dattod com_err ext2fs glog gflags blkid boost_regex uuid
                      ${PROTOBUF_LIBRARIES})

## fsawarebdcopy
add_executable(fsawarebdcopy
               backup/backup_coordinator.cc
               backup_status_tracker/backup_event_handler.cc
               backup_status_tracker/backup_status_tracker.cc
               backup_status_tracker/printing_sync_count_handler.cc
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
               device_synchronizer/device_synchronizer.cc
               fsawarebdcopy/fsawarebdcopy.cc
               unsynced_sector_manager/unsynced_sector_manager.cc
               unsynced_sector_manager/unsynced_sector_store.cc
               ${PROTO_SRCS})
target_link_libraries(fsawarebdcopy com_err ext2fs glog blkid boost_regex uuid
                      ${PROTOBUF_LIBRARIES})

# Need to use an environment variable here so tests don't run during packaging
# While it might seem like we would want to run tests during packaging, enough
# of them require root access that it makes more sense to just not run any of
# them during the packaging process.
if (DEFINED ENV{BUILD_UNIT_TESTS})
    include(cmake/test_setup.cmake)
endif()

add_custom_target(build DEPENDS dattod dattonet dattocli)

configure_file("setup.py.in" "setup.py")
install(TARGETS dattod DESTINATION /usr/sbin)
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/dattocli DESTINATION /usr/sbin)
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/dattonet DESTINATION /usr/sbin)
install(CODE "execute_process(COMMAND python setup.py install --no-compile --prefix=\$ENV{DESTDIR}/usr --install-layout=deb)")

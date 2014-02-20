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

# Need to use an environment variable here so tests don't run during packaging
# While it might seem like we would want to run tests during packaging, enough
# of them require root access that it makes more sense to just not run any of
# them during the packaging process.
if (DEFINED ENV{BUILD_UNIT_TESTS})
    include(cmake/test_setup.cmake)
endif()

add_custom_target(build)
add_dependencies(build dattod dattocli dattonet)

file(GLOB python_protobufs ${CMAKE_CURRENT_BINARY_DIR}/*_pb2.py)
file(GLOB python_libs ${CMAKE_CURRENT_SOURCE_DIR}/python_libs/*.py)

# Doesn't build currently
install(TARGETS dattod DESTINATION /usr/sbin)
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/dattocli DESTINATION /usr/lib/datto)
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/dattonet DESTINATION /usr/lib/datto)
install(FILES ${python_protobufs} DESTINATION /usr/lib/datto)
install(FILES ${python_libs} DESTINATION /usr/lib/datto)

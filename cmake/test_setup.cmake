# This builds gtest and gmock
# Additionally, this defines the add_unit_test macro

enable_testing()

set(GMOCK_DIR "${CMAKE_CURRENT_BINARY_DIR}/gmock/")
set(GTEST_DIR "${CMAKE_CURRENT_BINARY_DIR}/gmock/gtest/")
# On 12.04: We need r403, but r433 breaks the build, so use r432
add_custom_command(OUTPUT ${GMOCK_DIR}/src/gmock-all.cc
                          ${GMOCK_DIR}/src/gmock_main.cc
                          ${GTEST_DIR}/src/gtest-all.cc
                   COMMAND svn checkout -r 432
                           http://googlemock.googlecode.com/svn/trunk gmock)

include_directories(${GTEST_DIR})
include_directories(${GTEST_DIR}/include)
include_directories(${GMOCK_DIR})
include_directories(${GMOCK_DIR}/include)
add_library(gtest ${GTEST_DIR}/src/gtest-all.cc)
add_library(gmock_main ${GMOCK_DIR}/src/gmock-all.cc
                       ${GMOCK_DIR}/src/gmock_main.cc)
add_dependencies(gtest gmock_main)
add_dependencies(gmock_main testing_tools)
add_custom_target(check)

# usage: add_unit_test(test_name test_sources...)
# Note that add_unit_test automatically finds test/${test_name}.cc
# find_library(Protobuf) must have been called before calling this
macro(add_unit_test test_name)
    add_executable(${test_name} EXCLUDE_FROM_ALL test/${test_name}.cc ${ARGN})
    add_dependencies(${test_name} gtest)

    include_directories(${CMAKE_CURRENT_BINARY_DIR})
    target_link_libraries(${test_name} glog)
    target_link_libraries(${test_name} gtest)
    target_link_libraries(${test_name} gmock_main)

    # TODO set_tests_properties should do this so we don't need to do the
    # bash -c hack here, but can't seem to get it to work
    #
    # set_tests_properties(${test_name} PROPERTIES ENVIRONMENT
    #                      GLOG_minloglevel=2)
    add_custom_target(ctest_${test_name} 
                      ${CMAKE_CURRENT_BINARY_DIR}/${test_name}
                      WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

    add_test(${test_name} ${test_name})

    add_dependencies(ctest_${test_name} ${test_name})
    add_dependencies(check ctest_${test_name})
endmacro()

add_unit_test(unsynced_sector_store_test
              unsynced_sector_manager/unsynced_sector_store.cc)

add_unit_test(block_device_factory_test
              block_device/block_device.cc
              block_device/ext_file_system.cc
              block_device/ext_mountable_block_device.cc
              block_device/mountable_block_device.cc
              block_device/nbd_block_device.cc
              block_device/nbd_client.cc
              block_device/nbd_server.cc
              block_device/xfs_mountable_block_device.cc
              freeze_helper/freeze_helper.cc
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
              freeze_helper/freeze_helper.cc
              unsynced_sector_manager/unsynced_sector_manager.cc
              unsynced_sector_manager/unsynced_sector_store.cc
              ${PROTO_SRCS}
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
              freeze_helper/freeze_helper.cc
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
              block_device/block_device.cc
              block_device/ext_file_system.cc
              block_device/mountable_block_device.cc
              freeze_helper/freeze_helper.cc
              test/loop_device.cc
              block_device/ext_mountable_block_device.cc)
target_link_libraries(extfs_test blkid ext2fs com_err)

add_unit_test(flock_test
              dattod/flock.cc)

add_unit_test(freeze_helper_test
              backup/backup.cc
              block_device/block_device.cc
              block_device/mountable_block_device.cc
              ${PROTO_SRCS}
              freeze_helper/freeze_helper.cc)
target_link_libraries(freeze_helper_test blkid uuid ${PROTOBUF_LIBRARIES})

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

#add_unit_test(xfs_test
#              test/loop_device.cc
#              block_device/mountable_block_device.cc
#              block_device/block_device.cc
#              block_device/xfs_mountable_block_device.cc)

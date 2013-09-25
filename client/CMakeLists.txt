cmake_minimum_required(VERSION 2.8.0)
set(CMAKE_CXX_FLAGS "-Wall -Wshadow -Wextra -pthread ${CMAKE_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS "-std=c++0x ${CMAKE_CXX_FLAGS}")

include_directories(.)

## Testing
enable_testing()

### http://stackoverflow.com/a/9695234/965648
include(ExternalProject)
# Add gtest
externalproject_add(
    gtest
    SVN_REPOSITORY http://googletest.googlecode.com/svn/trunk/
    SVN_REVISION -r 660
    TIMEOUT 10
    PREFIX "${CMAKE_CURRENT_BINARY_DIR}/gtest"
    # Disable install step
    INSTALL_COMMAND ""
    # Wrap download, configure and build steps in a script to log output
    LOG_DOWNLOAD ON
    LOG_CONFIGURE ON
    LOG_BUILD ON)

externalproject_get_property(gtest source_dir)
externalproject_get_property(gtest binary_dir)
set(GTEST_INCLUDES "${source_dir}/include")
set(GTEST_BINARIES "${binary_dir}")
set(GTEST_LIBRARIES "${binary_dir}/libgtest.a ${binary_dir}/libgtest-main.a")

include_directories("${GTEST_INCLUDES}")

add_executable(unsynced_sector_tracker_test
               test/unsynced_sector_tracker_test.cc
               unsynced_sector_tracker/unsynced_sector_tracker.cc)
add_dependencies(unsynced_sector_tracker_test gtest)
target_link_libraries(unsynced_sector_tracker_test
                      ${GTEST_BINARIES}/libgtest.a)
target_link_libraries(unsynced_sector_tracker_test
                      ${GTEST_BINARIES}/libgtest_main.a)
add_test(NAME unsynced_sector_tracker_test
         COMMAND unsynced_sector_tracker_test)



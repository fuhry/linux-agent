# This downloads and builds gtest
# Additionally, this defines the add_unit_test macro

enable_testing()

add_custom_target(check)
# usage: add_unit_test(test_name test_sources...)
# Note that add_unit_test automatically finds test/${test_name}.cc
# find_library(Protobuf) must have been called before calling this
macro(add_unit_test test_name)
    add_executable(${test_name} EXCLUDE_FROM_ALL test/${test_name}.cc ${ARGN})
    add_dependencies(${test_name} gtest)

    target_link_libraries(${test_name} glog)
    target_link_libraries(${test_name} ${GTEST_BINARIES}/libgtest.a)
    target_link_libraries(${test_name} ${GTEST_BINARIES}/libgtest_main.a)
    target_link_libraries(${test_name} ${PROTOBUF_LIBRARIES})

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

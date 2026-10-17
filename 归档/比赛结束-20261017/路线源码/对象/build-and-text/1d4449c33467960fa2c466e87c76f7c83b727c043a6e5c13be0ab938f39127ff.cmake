cmake_minimum_required(VERSION 3.16)
project(r31a_paired_probe_host_tests LANGUAGES CXX)

include(CTest)

get_filename_component(R31A_SUPPORT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/.." ABSOLUTE)
find_path(R31A_ACL_INCLUDE_DIR acl/acl.h
    HINTS "$ENV{ASCEND_HOME_PATH}/aarch64-linux/include"
          "/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/include")
if(NOT R31A_ACL_INCLUDE_DIR)
    message(FATAL_ERROR "Cannot locate acl/acl.h for bridge host tests")
endif()

set(R31A_HOST_TEST_ROOT "${CMAKE_CURRENT_BINARY_DIR}/cases")
set(R31A_MISSING_LIBRARY_DIR "${R31A_HOST_TEST_ROOT}/missing-library")
set(R31A_MISSING_SYMBOL_DIR "${R31A_HOST_TEST_ROOT}/missing-symbol")
set(R31A_DISPATCH_DIR "${R31A_HOST_TEST_ROOT}/dispatch")
file(MAKE_DIRECTORY
    "${R31A_MISSING_LIBRARY_DIR}"
    "${R31A_MISSING_SYMBOL_DIR}"
    "${R31A_DISPATCH_DIR}")

add_executable(r31a_paired_probe_host_test
    "${R31A_SUPPORT_DIR}/paired_probe.cpp"
    "${R31A_SUPPORT_DIR}/paired_probe_host_test.cpp")
target_compile_definitions(r31a_paired_probe_host_test PRIVATE R31A_PAIRED_PROBE_BRIDGE_ONLY)
target_include_directories(r31a_paired_probe_host_test PRIVATE
    "${R31A_SUPPORT_DIR}" "${R31A_ACL_INCLUDE_DIR}")
target_link_libraries(r31a_paired_probe_host_test PRIVATE ${CMAKE_DL_LIBS})
target_compile_features(r31a_paired_probe_host_test PRIVATE cxx_std_17)

add_library(r31a_paired_probe_host_test_entry SHARED
    "${R31A_SUPPORT_DIR}/paired_probe_host_test_entry.cpp")
target_include_directories(r31a_paired_probe_host_test_entry PRIVATE
    "${R31A_SUPPORT_DIR}" "${R31A_ACL_INCLUDE_DIR}")
target_compile_features(r31a_paired_probe_host_test_entry PRIVATE cxx_std_17)
add_custom_command(TARGET r31a_paired_probe_host_test_entry POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:r31a_paired_probe_host_test_entry>"
        "${R31A_DISPATCH_DIR}/libr31a_paired_v016.so"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:r31a_paired_probe_host_test_entry>"
        "${R31A_DISPATCH_DIR}/libr31a_paired_v021.so")

add_library(r31a_paired_probe_host_test_missing_symbol SHARED
    "${R31A_SUPPORT_DIR}/paired_probe_host_test_missing_symbol.cpp")
target_include_directories(r31a_paired_probe_host_test_missing_symbol PRIVATE
    "${R31A_SUPPORT_DIR}" "${R31A_ACL_INCLUDE_DIR}")
target_compile_features(r31a_paired_probe_host_test_missing_symbol PRIVATE cxx_std_17)
add_custom_command(TARGET r31a_paired_probe_host_test_missing_symbol POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:r31a_paired_probe_host_test_missing_symbol>"
        "${R31A_MISSING_SYMBOL_DIR}/libr31a_paired_v021.so")

add_test(NAME r31a_bridge_missing_library
    COMMAND r31a_paired_probe_host_test V016 0 -)
set_tests_properties(r31a_bridge_missing_library PROPERTIES
    WORKING_DIRECTORY "${R31A_MISSING_LIBRARY_DIR}")
add_test(NAME r31a_bridge_missing_symbol
    COMMAND r31a_paired_probe_host_test V021 0 -)
set_tests_properties(r31a_bridge_missing_symbol PROPERTIES
    WORKING_DIRECTORY "${R31A_MISSING_SYMBOL_DIR}")
add_test(NAME r31a_bridge_dispatch_v016
    COMMAND r31a_paired_probe_host_test V016 1
        "${R31A_DISPATCH_DIR}/dispatch-v016.marker")
set_tests_properties(r31a_bridge_dispatch_v016 PROPERTIES
    WORKING_DIRECTORY "${R31A_DISPATCH_DIR}")
add_test(NAME r31a_bridge_dispatch_v021
    COMMAND r31a_paired_probe_host_test V021 1
        "${R31A_DISPATCH_DIR}/dispatch-v021.marker")
set_tests_properties(r31a_bridge_dispatch_v021 PROPERTIES
    WORKING_DIRECTORY "${R31A_DISPATCH_DIR}")

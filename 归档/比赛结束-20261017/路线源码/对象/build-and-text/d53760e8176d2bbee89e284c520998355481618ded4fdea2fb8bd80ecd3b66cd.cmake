cmake_minimum_required(VERSION 3.16)

set(ASCEND_HOME_PATH "$ENV{ASCEND_HOME_PATH}" CACHE PATH "CANN toolkit root")
if(NOT ASCEND_HOME_PATH)
    message(FATAL_ERROR "ASCEND_HOME_PATH must be set")
endif()

set(ASCENDC_CMAKE_DIR "${ASCEND_HOME_PATH}/aarch64-linux/tikcpp/ascendc_kernel_cmake")
list(APPEND CMAKE_MODULE_PATH "${ASCENDC_CMAKE_DIR}")
list(APPEND CMAKE_PREFIX_PATH "${ASCENDC_CMAKE_DIR}")
set(CMAKE_ASC_COMPILER
    "${ASCEND_HOME_PATH}/aarch64-linux/ccec_compiler/bin/bisheng"
    CACHE FILEPATH "Ascend C compiler")
set(ASCEND_PRODUCT_TYPE "ascend910b3" CACHE STRING "Ascend product type")
find_package(ASC REQUIRED)
project(hotloop_branch_v002 LANGUAGES ASC CXX)

set(ROUTE_ROOT "${CMAKE_CURRENT_LIST_DIR}/../..")
set(PARENT_SOURCE "${CMAKE_CURRENT_LIST_DIR}/parent_wrapper.asc")
set(CANDIDATE_SOURCE "${CMAKE_CURRENT_LIST_DIR}/candidate_wrapper.asc")

ascendc_library(hotloop_branch_v002_parent SHARED "${PARENT_SOURCE}")
ascendc_library(hotloop_branch_v002_candidate SHARED "${CANDIDATE_SOURCE}")

foreach(target_name IN ITEMS hotloop_branch_v002_parent hotloop_branch_v002_candidate)
    target_include_directories(${target_name} PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}"
        "${ASCEND_HOME_PATH}/aarch64-linux/include")
    target_compile_options(${target_name} PRIVATE
        $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=dav-2201>)
    target_link_directories(${target_name} PRIVATE
        "${ASCEND_HOME_PATH}/aarch64-linux/lib64"
        "${ASCEND_HOME_PATH}/lib64")
    set_target_properties(${target_name} PROPERTIES
        BUILD_RPATH "${ASCEND_HOME_PATH}/aarch64-linux/lib64;${ASCEND_HOME_PATH}/lib64")
endforeach()

ascendc_compile_definitions(hotloop_branch_v002_parent PRIVATE run_kernel=run_kernel_parent)
ascendc_compile_definitions(hotloop_branch_v002_candidate PRIVATE run_kernel=run_kernel_candidate)

add_executable(hotloop_branch_v002_correctness correctness_main.cpp)
target_include_directories(hotloop_branch_v002_correctness PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}"
    "${ASCEND_HOME_PATH}/aarch64-linux/include")
add_dependencies(hotloop_branch_v002_correctness
    hotloop_branch_v002_parent hotloop_branch_v002_candidate)
target_link_libraries(hotloop_branch_v002_correctness PRIVATE
    hotloop_branch_v002_parent
    hotloop_branch_v002_candidate
    "${ASCEND_HOME_PATH}/aarch64-linux/lib64/libascendcl.so"
    dl m)
set_target_properties(hotloop_branch_v002_correctness PROPERTIES
    BUILD_RPATH "${ASCEND_HOME_PATH}/aarch64-linux/lib64;${ASCEND_HOME_PATH}/lib64;${CMAKE_CURRENT_BINARY_DIR}")

add_executable(hotloop_branch_v002_performance performance_main.cpp)
target_include_directories(hotloop_branch_v002_performance PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}"
    "${ASCEND_HOME_PATH}/aarch64-linux/include")
add_dependencies(hotloop_branch_v002_performance
    hotloop_branch_v002_parent hotloop_branch_v002_candidate)
target_link_libraries(hotloop_branch_v002_performance PRIVATE
    hotloop_branch_v002_parent
    hotloop_branch_v002_candidate
    "${ASCEND_HOME_PATH}/aarch64-linux/lib64/libascendcl.so"
    dl m)
set_target_properties(hotloop_branch_v002_performance PROPERTIES
    BUILD_RPATH "${ASCEND_HOME_PATH}/aarch64-linux/lib64;${ASCEND_HOME_PATH}/lib64;${CMAKE_CURRENT_BINARY_DIR}")

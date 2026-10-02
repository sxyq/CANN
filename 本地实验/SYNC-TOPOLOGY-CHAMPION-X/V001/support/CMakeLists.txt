cmake_minimum_required(VERSION 3.16)

if(NOT ASCEND_CANN_PACKAGE_PATH)
    set(ASCEND_CANN_PACKAGE_PATH "$ENV{ASCEND_HOME_PATH}" CACHE PATH "CANN toolkit root")
endif()
if(NOT ASCEND_CANN_PACKAGE_PATH)
    message(FATAL_ERROR "Set ASCEND_CANN_PACKAGE_PATH or source the CANN set_env.sh first")
endif()

set(ENV{ASCEND_HOME_PATH} "${ASCEND_CANN_PACKAGE_PATH}")
list(PREPEND CMAKE_PREFIX_PATH
    "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/tikcpp/ascendc_kernel_cmake")
find_package(ASC REQUIRED)
project(sync_topology_v001 LANGUAGES ASC CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(SOC_VERSION Ascend910B3)

set(SYNC_V001_SUPPORT_DIR "${CMAKE_CURRENT_LIST_DIR}")
set(SYNC_V001_SOURCE_DIR "${SYNC_V001_SUPPORT_DIR}/..")
set(SYNC_ACL_INCLUDE_DIR "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/include")
set(SYNC_ACL_LIB_DIR "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/lib64")
set(SYNC_DRIVER_LIB_DIR "/usr/local/Ascend/driver/lib64/driver")
set(SYNC_DRIVER_COMMON_LIB_DIR "/usr/local/Ascend/driver/lib64/common")
set(HCC_ROOT "${ASCEND_CANN_PACKAGE_PATH}/toolkit/toolchain/hcc")
set(SYNC_ASC_ARCH_OPTIONS
    "--cce-aicore-arch=dav-c220")
string(JOIN " " SYNC_ASC_ARCH_OPTIONS_STR ${SYNC_ASC_ARCH_OPTIONS})
set(SYNC_ASC_KERNEL_OPTIONS
    "-include"
    "${SYNC_V001_SUPPORT_DIR}/local_tensor_metadata.h"
    "-I${HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0"
    "-I${HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0/aarch64-target-linux-gnu"
    "-I${HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0/backward")
string(JOIN " " SYNC_ASC_KERNEL_OPTIONS_STR ${SYNC_ASC_KERNEL_OPTIONS})
set(SYNC_ASC_HOST_OPTIONS
    "-include"
    "${SYNC_V001_SUPPORT_DIR}/local_tensor_metadata.h"
    "-include"
    "${SYNC_V001_SUPPORT_DIR}/local_abi_shim.h"
    "-I/usr/include/c++/11"
    "-I/usr/include/aarch64-linux-gnu/c++/11"
    "-I/usr/include/c++/11/backward"
    "-I${HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0"
    "-I${HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0/aarch64-target-linux-gnu"
    "-I${HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0/backward")
string(JOIN " " SYNC_ASC_HOST_OPTIONS_STR ${SYNC_ASC_HOST_OPTIONS})
set(CMAKE_ASC_COMPILE_OBJECT
    "<CMAKE_ASC_COMPILER> ${CMAKE_COMPILE_AS_ASC_FLAG} <DEFINES> <INCLUDES> ${SYNC_ASC_ARCH_OPTIONS_STR} -Xaicore-start ${SYNC_ASC_KERNEL_OPTIONS_STR} -O3 -std=c++17 -Xaicore-end -Xhost-start ${SYNC_ASC_HOST_OPTIONS_STR} -fPIC -O2 -Xhost-end <FLAGS> -o <OBJECT> -c -x asc <SOURCE>")

set(SYNC_HOST_CXX_INCLUDE_PATHS
    "/usr/include/c++/11:/usr/include/aarch64-linux-gnu/c++/11:/usr/include/c++/11/backward")
set(SYNC_COMMON_INCLUDE_DIRS
    "${SYNC_V001_SUPPORT_DIR}"
    "${SYNC_V001_SOURCE_DIR}"
    "${SYNC_ACL_INCLUDE_DIR}"
    "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/ascendc/include"
    "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/ascendc/include/basic_api/impl"
    "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/asc/include/basic_api"
    "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/asc/include"
    "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux"
    "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/asc"
    "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/asc/include/interface"
    "${ASCEND_CANN_PACKAGE_PATH}/include"
    "${ASCEND_CANN_PACKAGE_PATH}/compiler/tikcpp/tikcfw"
    "${HCC_ROOT}/aarch64-target-linux-gnu/include"
    "${HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0"
    "${HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0/aarch64-target-linux-gnu"
    "${HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0/backward"
    /usr/include/c++/11
    /usr/include/aarch64-linux-gnu/c++/11
    /usr/include/aarch64-linux-gnu)

function(configure_sync_v001_asc_target target)
    set_property(TARGET ${target} PROPERTY RULE_LAUNCH_COMPILE
        "${CMAKE_COMMAND} -E env CPATH=${SYNC_HOST_CXX_INCLUDE_PATHS}")
    target_include_directories(${target} PRIVATE ${SYNC_COMMON_INCLUDE_DIRS})
    target_compile_options(${target} PRIVATE
        $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=dav-2201>)
    target_link_directories(${target} PRIVATE
        "${SYNC_ACL_LIB_DIR}"
        "${ASCEND_CANN_PACKAGE_PATH}/lib64"
        /usr/lib/gcc/aarch64-linux-gnu/11
        /usr/lib/aarch64-linux-gnu)
    target_link_options(${target} PRIVATE "-Wl,-Bsymbolic")
    set_target_properties(${target} PROPERTIES
        BUILD_RPATH "${SYNC_ACL_LIB_DIR};${ASCEND_CANN_PACKAGE_PATH}/lib64;${SYNC_DRIVER_LIB_DIR};${SYNC_DRIVER_COMMON_LIB_DIR};$ORIGIN")
endfunction()

function(configure_sync_v001_host_target target)
    set_property(TARGET ${target} PROPERTY RULE_LAUNCH_COMPILE
        "${CMAKE_COMMAND} -E env CPATH=${SYNC_HOST_CXX_INCLUDE_PATHS}")
    target_include_directories(${target} PRIVATE ${SYNC_COMMON_INCLUDE_DIRS})
    target_link_directories(${target} PRIVATE
        "${SYNC_ACL_LIB_DIR}"
        "${ASCEND_CANN_PACKAGE_PATH}/lib64"
        /usr/lib/gcc/aarch64-linux-gnu/11
        /usr/lib/aarch64-linux-gnu)
    set_target_properties(${target} PROPERTIES
        BUILD_RPATH "${SYNC_ACL_LIB_DIR};${ASCEND_CANN_PACKAGE_PATH}/lib64;${SYNC_DRIVER_LIB_DIR};${SYNC_DRIVER_COMMON_LIB_DIR};$ORIGIN")
endfunction()

set_source_files_properties(npu_correctness.asc PROPERTIES LANGUAGE ASC)
add_executable(sync_topology_v001_correctness npu_correctness.asc)
configure_sync_v001_asc_target(sync_topology_v001_correctness)
target_link_libraries(sync_topology_v001_correctness PRIVATE
    ascendcl tiling_api register platform unified_dlog dl m graph_base)

set(SYNC_PARENT_SOURCE
    "${SYNC_V001_SUPPORT_DIR}/../../../../线上结果/R31B/V011/submission.asc"
    CACHE FILEPATH "Exact Direct Parent source")
if(NOT EXISTS "${SYNC_PARENT_SOURCE}")
    message(FATAL_ERROR "SYNC_PARENT_SOURCE is missing: ${SYNC_PARENT_SOURCE}")
endif()
set(SYNC_CANDIDATE_SOURCE "${SYNC_V001_SOURCE_DIR}/submission.asc")
if(NOT EXISTS "${SYNC_CANDIDATE_SOURCE}")
    message(FATAL_ERROR "SYNC_CANDIDATE_SOURCE is missing: ${SYNC_CANDIDATE_SOURCE}")
endif()

ascendc_library(sync_topology_v001_parent SHARED "${SYNC_PARENT_SOURCE}")
ascendc_compile_definitions(sync_topology_v001_parent PRIVATE
    run_kernel=sync_parent_run_kernel)
configure_sync_v001_asc_target(sync_topology_v001_parent)

ascendc_library(sync_topology_v001_candidate SHARED "${SYNC_CANDIDATE_SOURCE}")
ascendc_compile_definitions(sync_topology_v001_candidate PRIVATE
    run_kernel=sync_candidate_run_kernel)
configure_sync_v001_asc_target(sync_topology_v001_candidate)

add_executable(sync_topology_v001_timing timing_runner.cpp)
add_dependencies(sync_topology_v001_timing
    sync_topology_v001_parent sync_topology_v001_candidate)
target_link_libraries(sync_topology_v001_timing PRIVATE
    "$<TARGET_FILE:sync_topology_v001_parent>"
    "$<TARGET_FILE:sync_topology_v001_candidate>"
    "${SYNC_ACL_LIB_DIR}/libascendcl.so"
    dl m)
configure_sync_v001_host_target(sync_topology_v001_timing)

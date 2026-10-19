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
project(sync_topology_v001 LANGUAGES ASC)

set(CMAKE_CXX_STANDARD 17)
set(SOC_VERSION Ascend910B3)
set(HCC_ROOT "${ASCEND_CANN_PACKAGE_PATH}/toolkit/toolchain/hcc")
set(SYNC_ASC_KERNEL_OPTIONS
    "--cce-aicore-arch=dav-c220"
    "-I${HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0"
    "-I${HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0/aarch64-target-linux-gnu"
    "-I${HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0/backward")
string(JOIN " " SYNC_ASC_KERNEL_OPTIONS_STR ${SYNC_ASC_KERNEL_OPTIONS})
set(SYNC_ASC_HOST_OPTIONS
    "-I/usr/include/c++/11"
    "-I/usr/include/aarch64-linux-gnu/c++/11"
    "-I/usr/include/c++/11/backward"
    "-I${HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0"
    "-I${HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0/aarch64-target-linux-gnu"
    "-I${HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0/backward")
string(JOIN " " SYNC_ASC_HOST_OPTIONS_STR ${SYNC_ASC_HOST_OPTIONS})
set(CMAKE_ASC_COMPILE_OBJECT
    "<CMAKE_ASC_COMPILER> ${CMAKE_COMPILE_AS_ASC_FLAG} <DEFINES> <INCLUDES> ${SYNC_ASC_KERNEL_OPTIONS_STR} -Xaicore-start -O3 -std=c++17 -Xaicore-end -Xhost-start ${SYNC_ASC_HOST_OPTIONS_STR} -fPIC -O2 -Xhost-end <FLAGS> -o <OBJECT> -c -x asc <SOURCE>")

set_source_files_properties(npu_correctness.asc PROPERTIES LANGUAGE ASC)
add_executable(sync_topology_v001_correctness npu_correctness.asc)

set(SYNC_HOST_CXX_INCLUDE_PATHS
    "/usr/include/c++/11:/usr/include/aarch64-linux-gnu/c++/11:/usr/include/c++/11/backward")
set(SYNC_V001_SUPPORT_DIR "${CMAKE_CURRENT_LIST_DIR}")
function(configure_sync_v001_target target)
    set_property(TARGET ${target} PROPERTY RULE_LAUNCH_COMPILE
        "${CMAKE_COMMAND} -E env CPATH=${SYNC_HOST_CXX_INCLUDE_PATHS}")
    target_include_directories(${target} PRIVATE
        "${SYNC_V001_SUPPORT_DIR}"
        "${SYNC_V001_SUPPORT_DIR}/.."
        "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/include"
        "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/ascendc/include"
        "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/ascendc/include/basic_api/impl"
        "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/asc/include/basic_api"
        "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/asc/include"
        "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux"
        "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/asc"
        "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/asc/include/interface"
        "${HCC_ROOT}/aarch64-target-linux-gnu/include"
        "${HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0"
        "${HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0/aarch64-target-linux-gnu"
        "${HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0/backward"
        /usr/include/c++/11
        /usr/include/aarch64-linux-gnu/c++/11
        /usr/include/aarch64-linux-gnu)
    target_compile_options(${target} PRIVATE
        $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=dav-2201>)
    target_link_directories(${target} PRIVATE
        "${ASCEND_CANN_PACKAGE_PATH}/lib64"
        "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/lib64"
        /usr/lib/gcc/aarch64-linux-gnu/11
        /usr/lib/aarch64-linux-gnu)
    target_link_libraries(${target} PRIVATE
        ascendcl tiling_api register platform unified_dlog dl m graph_base)
endfunction()

configure_sync_v001_target(sync_topology_v001_correctness)

set(SYNC_PARENT_SOURCE
    "${SYNC_V001_SUPPORT_DIR}/../../../../线上结果/R31B/V011/submission.asc"
    CACHE FILEPATH "Exact Direct Parent source")
if(NOT EXISTS "${SYNC_PARENT_SOURCE}")
    message(FATAL_ERROR "SYNC_PARENT_SOURCE is missing: ${SYNC_PARENT_SOURCE}")
endif()

set_source_files_properties(timing_runner.asc PROPERTIES LANGUAGE ASC)
add_executable(sync_topology_v001_timing timing_runner.asc)
target_compile_definitions(sync_topology_v001_timing PRIVATE
    SYNC_PARENT_SOURCE="${SYNC_PARENT_SOURCE}")
configure_sync_v001_target(sync_topology_v001_timing)

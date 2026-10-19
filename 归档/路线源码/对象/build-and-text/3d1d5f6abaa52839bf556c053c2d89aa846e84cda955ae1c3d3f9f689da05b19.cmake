cmake_minimum_required(VERSION 3.16)

set(ASCEND_CANN_PACKAGE_PATH
    "/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002"
    CACHE PATH "Verified CANN toolkit")
set(ASCEND_HOME_PATH "${ASCEND_CANN_PACKAGE_PATH}")
set(ASC_DIR
    "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/tikcpp/ascendc_kernel_cmake"
    CACHE PATH "Installed Ascend C CMake package")
set(SOC_VERSION Ascend910B3 CACHE STRING "Target SoC")
set(CMAKE_ASC_COMPILER
    "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/ccec_compiler/bin/bisheng"
    CACHE FILEPATH "Ascend C compiler" FORCE)
set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/install"
    CACHE PATH "Route-local runner install staging" FORCE)

find_package(ASC REQUIRED CONFIG PATHS "${ASC_DIR}" NO_DEFAULT_PATH)
project(staging_liveness_v034_runner LANGUAGES ASC CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(KERNEL_SOURCE
    "${CMAKE_CURRENT_LIST_DIR}/../../src/submission.asc"
    CACHE FILEPATH "Exact candidate source compiled into the runner")
set(EXPECTED_SOURCE_SHA256
    "75131108d32bc587c3c289d53ded04a15c5224417c675cb37a46df032e104d33"
    CACHE STRING "Required exact source SHA-256")
get_filename_component(KERNEL_SOURCE "${KERNEL_SOURCE}" REALPATH)
if(NOT EXISTS "${KERNEL_SOURCE}")
    message(FATAL_ERROR "Kernel source does not exist: ${KERNEL_SOURCE}")
endif()
file(SHA256 "${KERNEL_SOURCE}" ACTUAL_SOURCE_SHA256)
if(NOT ACTUAL_SOURCE_SHA256 STREQUAL EXPECTED_SOURCE_SHA256)
    message(FATAL_ERROR
        "Kernel source SHA mismatch: expected ${EXPECTED_SOURCE_SHA256}, got ${ACTUAL_SOURCE_SHA256}")
endif()

set(EXACT_KERNEL_ENTRY "${CMAKE_CURRENT_BINARY_DIR}/exact_kernel_entry.asc")
configure_file(
    "${CMAKE_CURRENT_LIST_DIR}/exact_kernel_entry.asc.in"
    "${EXACT_KERNEL_ENTRY}"
    @ONLY)
set_source_files_properties("${EXACT_KERNEL_ENTRY}" PROPERTIES LANGUAGE ASC)

add_executable(route_runner "${EXACT_KERNEL_ENTRY}")

set(ASCEND_ROOT "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux")
target_include_directories(route_runner PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}"
    "${ASCEND_ROOT}/include"
    "${ASCEND_CANN_PACKAGE_PATH}/include"
    "${ASCEND_ROOT}/ascendc/include"
    "${ASCEND_ROOT}/ascendc/include/basic_api/impl"
    "${ASCEND_ROOT}/asc/include"
    "${ASCEND_ROOT}/asc/include/basic_api"
    "${ASCEND_ROOT}/asc/include/interface"
    "${ASCEND_ROOT}/asc"
    "${ASCEND_ROOT}/asc/impl/basic_api"
    "${ASCEND_CANN_PACKAGE_PATH}/compiler/tikcpp/tikcfw")
target_compile_definitions(route_runner PRIVATE
    "KERNEL_SOURCE_SHA256=\"${ACTUAL_SOURCE_SHA256}\""
    "RUNNER_EXECUTABLE_PATH=\"${CMAKE_CURRENT_BINARY_DIR}/route_runner\"")
target_link_directories(route_runner PRIVATE
    "${ASCEND_CANN_PACKAGE_PATH}/lib64"
    "${ASCEND_ROOT}/lib64")
target_link_options(route_runner PRIVATE
    -L/usr/lib/gcc/aarch64-linux-gnu/11
    -L/usr/lib/aarch64-linux-gnu)
target_link_libraries(route_runner PRIVATE
    ascendcl
    tiling_api
    register
    runtime
    error_manager
    profapi
    ge_common_base
    ascendalog
    mmpa
    platform
    unified_dlog
    ascend_dump
    graph_base
    c_sec
    dl
    m)

message(STATUS "Pinned exact source: ${KERNEL_SOURCE}")
message(STATUS "Pinned source SHA-256: ${ACTUAL_SOURCE_SHA256}")
message(STATUS "Kernel build: direct-invocation ASC translation unit")
message(STATUS "Host runner entry: route-local ACL runner")

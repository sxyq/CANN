cmake_minimum_required(VERSION 3.16)

set(ASCEND_CANN_PACKAGE_PATH "$ENV{ASCEND_HOME_PATH}" CACHE PATH "CANN package root")
set(SOC_VERSION "Ascend910B3" CACHE STRING "Target SoC")
list(APPEND CMAKE_PREFIX_PATH
    "${ASCEND_CANN_PACKAGE_PATH}/compiler/tikcpp/ascendc_kernel_cmake")

find_package(ASC REQUIRED)
project(aligned_tail_datacopy_x LANGUAGES ASC CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PARENT_SOURCE
    "${CMAKE_CURRENT_SOURCE_DIR}/../submission.asc"
    CACHE FILEPATH "Exact unchanged parent source")
set(CANDIDATE_SOURCE
    "${CMAKE_CURRENT_SOURCE_DIR}/../submission-datacopy.asc"
    CACHE FILEPATH "Single-change candidate source")
option(ROUTE_BUILD_CANDIDATE_RUNNER "Build the route Candidate runner" OFF)
set(ROUTE_EXPECTED_PARENT_SHA256 "" CACHE STRING "Required Parent source SHA256")

if(NOT EXISTS "${PARENT_SOURCE}")
    message(FATAL_ERROR "Parent source does not exist: ${PARENT_SOURCE}")
endif()
file(SHA256 "${PARENT_SOURCE}" PARENT_SOURCE_SHA256)
if(ROUTE_EXPECTED_PARENT_SHA256 AND
   NOT PARENT_SOURCE_SHA256 STREQUAL ROUTE_EXPECTED_PARENT_SHA256)
    message(FATAL_ERROR
        "Parent SHA256 mismatch: expected ${ROUTE_EXPECTED_PARENT_SHA256}, got ${PARENT_SOURCE_SHA256}")
endif()

function(add_route_runner target source)
    if(NOT EXISTS "${source}")
        message(FATAL_ERROR "${target} source does not exist: ${source}")
    endif()
    set(KERNEL_SOURCE "${source}")
    file(SHA256 "${source}" KERNEL_SOURCE_SHA256)
    configure_file(runner.asc.in "${CMAKE_CURRENT_BINARY_DIR}/${target}.asc" @ONLY)
    add_executable(${target} "${CMAKE_CURRENT_BINARY_DIR}/${target}.asc")
    target_include_directories(${target} PRIVATE
        "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/include"
        "${CMAKE_CURRENT_SOURCE_DIR}"
        "/usr/include/c++/11"
        "/usr/include/aarch64-linux-gnu/c++/11")
    target_compile_options(${target} PRIVATE
        $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=dav-2201>)
target_link_directories(${target} PRIVATE
        "/usr/lib/gcc/aarch64-linux-gnu/11"
        "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/lib64"
        "${ASCEND_CANN_PACKAGE_PATH}/lib64")
target_link_libraries(${target} PRIVATE
        tiling_api register platform unified_dlog graph_base
        ascendcl ascendc_runtime runtime error_manager profapi
        ge_common_base ascendalog mmpa ascend_dump c_sec dl m)
endfunction()

add_route_runner(parent_runner "${PARENT_SOURCE}")
if(ROUTE_BUILD_CANDIDATE_RUNNER)
    add_route_runner(candidate_runner "${CANDIDATE_SOURCE}")
endif()

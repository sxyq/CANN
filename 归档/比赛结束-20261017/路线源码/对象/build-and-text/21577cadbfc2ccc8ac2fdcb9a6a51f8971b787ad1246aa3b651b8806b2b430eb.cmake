cmake_minimum_required(VERSION 3.16)

find_package(ASC REQUIRED)
project(case47_v001 LANGUAGES ASC CXX)

set(CMAKE_CXX_STANDARD 17)
set(SERVER_AUDIT_ROOT "/home/data4t2/lelinfeng/server_full_audit" CACHE PATH "")
set(TOOLKIT "$ENV{ASCEND_HOME_PATH}")
set(CASE47_SOURCE_FILE "${CMAKE_CURRENT_SOURCE_DIR}/submission.asc" CACHE FILEPATH "")
set(CASE47_KERNEL_TARGET "case47_kernel" CACHE STRING "")

ascendc_library(${CASE47_KERNEL_TARGET} SHARED "${CASE47_SOURCE_FILE}")
target_compile_options(${CASE47_KERNEL_TARGET} PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=dav-2201>
    $<$<COMPILE_LANGUAGE:ASC>:--npu-soc=Ascend910B3>
    $<$<COMPILE_LANGUAGE:ASC>:-include>
    $<$<COMPILE_LANGUAGE:ASC>:${SERVER_AUDIT_ROOT}/judge_abi.h>
)
target_include_directories(${CASE47_KERNEL_TARGET} PRIVATE
    # bisheng does not inherit the host C++ compiler's standard include paths.
    ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES}
    "${SERVER_AUDIT_ROOT}/src"
    "${SERVER_AUDIT_ROOT}/inc"
    "${TOOLKIT}/include"
    "${TOOLKIT}/aarch64-linux/include"
    "${CMAKE_CURRENT_SOURCE_DIR}"
)
target_link_directories(${CASE47_KERNEL_TARGET} PRIVATE
    "${TOOLKIT}/lib64"
    "${TOOLKIT}/aarch64-linux/lib64"
    /usr/lib/gcc/aarch64-linux-gnu/11
    /usr/lib/aarch64-linux-gnu
    /lib/aarch64-linux-gnu
    "${TOOLKIT}/toolkit/toolchain/hcc/aarch64-target-linux-gnu/lib64"
)
target_link_libraries(${CASE47_KERNEL_TARGET} PRIVATE ascendcl)
target_link_options(${CASE47_KERNEL_TARGET} PRIVATE
    -Wl,-rpath-link,/usr/lib/gcc/aarch64-linux-gnu/11
)

add_executable(case47_v001_correctness correctness_runner.cpp)
target_include_directories(case47_v001_correctness PRIVATE
    "${SERVER_AUDIT_ROOT}"
    "${TOOLKIT}/include"
    "${TOOLKIT}/aarch64-linux/include"
)
target_link_directories(case47_v001_correctness PRIVATE
    "${TOOLKIT}/lib64"
    "${TOOLKIT}/aarch64-linux/lib64"
)
target_link_libraries(case47_v001_correctness PRIVATE ${CASE47_KERNEL_TARGET} ascendcl dl m)
target_compile_options(case47_v001_correctness PRIVATE -ffp-contract=off)
set_target_properties(case47_v001_correctness PROPERTIES
    BUILD_RPATH "${TOOLKIT}/lib64;${TOOLKIT}/aarch64-linux/lib64"
)

add_executable(case47_timing_runner timing_runner.cpp)
target_include_directories(case47_timing_runner PRIVATE
    "${SERVER_AUDIT_ROOT}"
    "${TOOLKIT}/include"
    "${TOOLKIT}/aarch64-linux/include"
)
target_link_directories(case47_timing_runner PRIVATE
    "${TOOLKIT}/lib64"
    "${TOOLKIT}/aarch64-linux/lib64"
)
target_link_libraries(case47_timing_runner PRIVATE ascendcl dl m)
target_compile_options(case47_timing_runner PRIVATE -ffp-contract=off)
set_target_properties(case47_timing_runner PROPERTIES
    BUILD_RPATH "${TOOLKIT}/lib64;${TOOLKIT}/aarch64-linux/lib64"
)

cmake_minimum_required(VERSION 3.16)

find_package(ASC REQUIRED)
project(case47_v001 LANGUAGES ASC CXX)

set(CMAKE_CXX_STANDARD 17)
set(SERVER_AUDIT_ROOT "/home/data4t2/lelinfeng/server_full_audit")
set(TOOLKIT "$ENV{ASCEND_HOME_PATH}")

ascendc_library(case47_kernel SHARED "${CMAKE_CURRENT_SOURCE_DIR}/submission.asc")
target_compile_options(case47_kernel PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=dav-2201>
    $<$<COMPILE_LANGUAGE:ASC>:--npu-soc=Ascend910B3>
    $<$<COMPILE_LANGUAGE:ASC>:-include>
    $<$<COMPILE_LANGUAGE:ASC>:${SERVER_AUDIT_ROOT}/judge_abi.h>
)
target_include_directories(case47_kernel PRIVATE
    "${SERVER_AUDIT_ROOT}/src"
    "${SERVER_AUDIT_ROOT}/inc"
    "${TOOLKIT}/include"
    "${TOOLKIT}/aarch64-linux/include"
    "${CMAKE_CURRENT_SOURCE_DIR}"
)
target_link_directories(case47_kernel PRIVATE
    "${TOOLKIT}/lib64"
    "${TOOLKIT}/aarch64-linux/lib64"
    /usr/lib/gcc/aarch64-linux-gnu/11
    /usr/lib/aarch64-linux-gnu
    /lib/aarch64-linux-gnu
    "${TOOLKIT}/toolkit/toolchain/hcc/aarch64-target-linux-gnu/lib64"
)
target_link_libraries(case47_kernel PRIVATE ascendcl)
target_link_options(case47_kernel PRIVATE
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
target_link_libraries(case47_v001_correctness PRIVATE case47_kernel ascendcl dl m)
target_compile_options(case47_v001_correctness PRIVATE -ffp-contract=off)
set_target_properties(case47_v001_correctness PROPERTIES
    BUILD_RPATH "${TOOLKIT}/lib64;${TOOLKIT}/aarch64-linux/lib64"
)

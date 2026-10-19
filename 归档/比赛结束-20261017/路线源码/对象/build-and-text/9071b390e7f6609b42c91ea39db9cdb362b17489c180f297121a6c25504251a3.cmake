cmake_minimum_required(VERSION 3.16)

find_package(ASC REQUIRED)
project(r14_param_mte2_fingerprint LANGUAGES ASC)

set(CMAKE_CXX_STANDARD 17)
set(R14_HCC_ROOT "$ENV{ASCEND_HOME_PATH}/toolkit/toolchain/hcc")
set(R14_CXX_INCLUDE "${R14_HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0")
set(R14_CXX_TARGET_INCLUDE "${R14_CXX_INCLUDE}/aarch64-target-linux-gnu")
set(R14_HCC_LIBRARY "${R14_HCC_ROOT}/aarch64-target-linux-gnu/lib64")
add_executable(r14_param_mte2_fingerprint main.asc)
target_compile_options(r14_param_mte2_fingerprint PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:-isystem${R14_CXX_INCLUDE}>
    $<$<COMPILE_LANGUAGE:ASC>:-isystem${R14_CXX_TARGET_INCLUDE}>
)
target_link_directories(r14_param_mte2_fingerprint PRIVATE
    "${R14_HCC_LIBRARY}"
    /usr/lib/gcc/aarch64-linux-gnu/11
    /usr/lib/aarch64-linux-gnu
)
target_include_directories(r14_param_mte2_fingerprint PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    "$ENV{ASCEND_HOME_PATH}/include"
    "$ENV{ASCEND_HOME_PATH}/aarch64-linux/include"
)
target_link_directories(r14_param_mte2_fingerprint PRIVATE
    "$ENV{ASCEND_HOME_PATH}/lib64"
    "$ENV{ASCEND_HOME_PATH}/aarch64-linux/lib64"
)
target_link_libraries(r14_param_mte2_fingerprint PRIVATE
    ascendcl
    tiling_api
    register
    platform
    unified_dlog
    graph_base
    dl
    m
)
target_compile_options(r14_param_mte2_fingerprint PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=dav-2201>
)

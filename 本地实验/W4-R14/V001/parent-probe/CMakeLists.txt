cmake_minimum_required(VERSION 3.16)
find_package(ASC REQUIRED)
project(r14_parent_probe LANGUAGES ASC)
set(CMAKE_CXX_STANDARD 17)
set(R14_SUPPORT_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../support/param-mte2-fingerprint" CACHE PATH "Existing Parent support source")
set(R14_HCC_ROOT "$ENV{ASCEND_HOME_PATH}/toolkit/toolchain/hcc")
set(R14_CXX_INCLUDE "${R14_HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0")
add_executable(r14_parent_probe parent_probe.asc)
target_compile_options(r14_parent_probe PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:-isystem${R14_CXX_INCLUDE}>
    $<$<COMPILE_LANGUAGE:ASC>:-isystem${R14_CXX_INCLUDE}/aarch64-target-linux-gnu>
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=dav-2201>
)
target_include_directories(r14_parent_probe PRIVATE
    "${R14_SUPPORT_ROOT}"
    "$ENV{ASCEND_HOME_PATH}/include"
    "$ENV{ASCEND_HOME_PATH}/aarch64-linux/include"
)
target_link_directories(r14_parent_probe PRIVATE
    "${R14_HCC_ROOT}/aarch64-target-linux-gnu/lib64"
    /usr/lib/gcc/aarch64-linux-gnu/11
    /usr/lib/aarch64-linux-gnu
    "$ENV{ASCEND_HOME_PATH}/lib64"
    "$ENV{ASCEND_HOME_PATH}/aarch64-linux/lib64"
)
target_link_libraries(r14_parent_probe PRIVATE ascendcl tiling_api register platform unified_dlog graph_base dl m)

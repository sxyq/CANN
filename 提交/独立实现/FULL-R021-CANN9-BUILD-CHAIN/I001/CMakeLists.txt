cmake_minimum_required(VERSION 3.16)

find_package(ASC REQUIRED)
project(add_rms_norm_bias_custom LANGUAGES ASC CXX)

set(SOC_ARCH "dav-2201" CACHE STRING "Ascend C target architecture")

add_executable(add_rms_norm_bias_custom main.asc)
set(HCC_LIB_DIR "$ENV{ASCEND_HOME_PATH}/tools/hcc/aarch64-target-linux-gnu/lib64")
target_compile_options(add_rms_norm_bias_custom PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=${SOC_ARCH}>
    $<$<COMPILE_LANGUAGE:ASC>:-I/usr/include/c++/11>
    $<$<COMPILE_LANGUAGE:ASC>:-I/usr/include/aarch64-linux-gnu/c++/11>
)
target_link_options(add_rms_norm_bias_custom PRIVATE "-L${HCC_LIB_DIR}")
target_link_libraries(add_rms_norm_bias_custom PRIVATE
    tiling_api
    register
    platform
    unified_dlog
    dl
    m
    graph_base
)

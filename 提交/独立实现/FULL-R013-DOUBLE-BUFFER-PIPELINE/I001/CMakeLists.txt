cmake_minimum_required(VERSION 3.16)

find_package(ASC REQUIRED)

project(add_rms_norm_bias_full_r013 LANGUAGES ASC CXX)

set(CMAKE_CXX_STANDARD 14)

if(DEFINED NPU_ARCH)
    set(SOC_ARCH ${NPU_ARCH})
else()
    set(SOC_ARCH "dav-2201")
endif()

set(HCC_TARGET_INCLUDE "$ENV{ASCEND_HOME_PATH}/toolkit/toolchain/hcc/aarch64-target-linux-gnu/include")
set(HCC_CXX_INCLUDE "$ENV{ASCEND_HOME_PATH}/toolkit/toolchain/hcc/aarch64-target-linux-gnu/include/c++/7.3.0")
set(HCC_CXX_TARGET_INCLUDE "$ENV{ASCEND_HOME_PATH}/toolkit/toolchain/hcc/aarch64-target-linux-gnu/include/c++/7.3.0/aarch64-target-linux-gnu")
set(HCC_CXX_BACKWARD_INCLUDE "$ENV{ASCEND_HOME_PATH}/toolkit/toolchain/hcc/aarch64-target-linux-gnu/include/c++/7.3.0/backward")
set(HCC_TARGET_LIB "$ENV{ASCEND_HOME_PATH}/toolkit/toolchain/hcc/aarch64-target-linux-gnu/lib64")

add_executable(add_rms_norm_bias_full_r013 main.asc)

target_link_libraries(add_rms_norm_bias_full_r013 PRIVATE
    tiling_api
    register
    platform
    unified_dlog
    dl
    m
    graph_base
)

target_include_directories(add_rms_norm_bias_full_r013 PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_directories(add_rms_norm_bias_full_r013 PRIVATE
    ${HCC_TARGET_LIB}
)

target_compile_options(add_rms_norm_bias_full_r013 PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=${SOC_ARCH}>
    $<$<COMPILE_LANGUAGE:ASC>:-isystem${HCC_TARGET_INCLUDE}>
    $<$<COMPILE_LANGUAGE:ASC>:-isystem${HCC_CXX_INCLUDE}>
    $<$<COMPILE_LANGUAGE:ASC>:-isystem${HCC_CXX_TARGET_INCLUDE}>
    $<$<COMPILE_LANGUAGE:ASC>:-isystem${HCC_CXX_BACKWARD_INCLUDE}>
)

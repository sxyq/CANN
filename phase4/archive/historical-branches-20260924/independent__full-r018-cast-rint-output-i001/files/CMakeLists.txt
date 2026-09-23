cmake_minimum_required(VERSION 3.16)

find_package(ASC REQUIRED)

project(add_rms_norm_bias_custom LANGUAGES ASC CXX)

if(DEFINED NPU_ARCH)
    set(SOC_ARCH ${NPU_ARCH})
else()
    set(SOC_ARCH "dav-2201")
endif()

add_executable(add_rms_norm_bias_custom main.asc)

target_link_libraries(add_rms_norm_bias_custom PRIVATE
    tiling_api
    register
    platform
    unified_dlog
    dl
    m
    graph_base
)

target_include_directories(add_rms_norm_bias_custom PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_directories(add_rms_norm_bias_custom PRIVATE
    "$ENV{ASCEND_HOME_PATH}/tools/hcc/aarch64-target-linux-gnu/lib64"
)

target_compile_options(add_rms_norm_bias_custom PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=${SOC_ARCH}>
    $<$<COMPILE_LANGUAGE:ASC>:-isystem$ENV{ASCEND_HOME_PATH}/tools/hcc/aarch64-target-linux-gnu/include/c++/7.3.0>
    $<$<COMPILE_LANGUAGE:ASC>:-isystem$ENV{ASCEND_HOME_PATH}/tools/hcc/aarch64-target-linux-gnu/include/c++/7.3.0/aarch64-target-linux-gnu>
    $<$<COMPILE_LANGUAGE:ASC>:-isystem$ENV{ASCEND_HOME_PATH}/tools/hcc/aarch64-target-linux-gnu/include/c++/7.3.0/backward>
)

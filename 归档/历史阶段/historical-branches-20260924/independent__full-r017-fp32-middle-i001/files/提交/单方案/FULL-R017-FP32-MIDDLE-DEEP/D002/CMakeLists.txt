cmake_minimum_required(VERSION 3.16)

find_package(ASC REQUIRED)

project(add_rms_norm_bias_custom LANGUAGES ASC CXX)

set(CMAKE_CXX_STANDARD 14)

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
    /usr/include/c++/11
    /usr/include/aarch64-linux-gnu/c++/11
)

target_link_directories(add_rms_norm_bias_custom PRIVATE
    /home/data4t2/lelinfeng/cann_game/.toolchains/cann-9.0/cann-9.0.0/tools/hcc/aarch64-target-linux-gnu/lib64
)

target_compile_options(add_rms_norm_bias_custom PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=${SOC_ARCH}>
)

cmake_minimum_required(VERSION 3.16)

find_package(ASC REQUIRED)

project(full_r002_retained_y_i001 LANGUAGES ASC CXX)

set(CMAKE_CXX_STANDARD 14)

if(DEFINED NPU_ARCH)
    set(SOC_ARCH ${NPU_ARCH})
else()
    set(SOC_ARCH "dav-2201")
endif()

add_executable(full_r002_retained_y_i001 compile_main.asc)

target_link_libraries(full_r002_retained_y_i001 PRIVATE
    tiling_api
    register
    platform
    unified_dlog
    dl
    m
    graph_base
)

target_link_directories(full_r002_retained_y_i001 PRIVATE
    "$ENV{ASCEND_HOME_PATH}/tools/hcc/aarch64-target-linux-gnu/lib64"
)

target_include_directories(full_r002_retained_y_i001 PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    "$ENV{ASCEND_HOME_PATH}/tools/hcc/aarch64-target-linux-gnu/include/c++/7.3.0"
    "$ENV{ASCEND_HOME_PATH}/tools/hcc/aarch64-target-linux-gnu/include/c++/7.3.0/aarch64-target-linux-gnu"
    "$ENV{ASCEND_HOME_PATH}/tools/hcc/aarch64-target-linux-gnu/include"
)

target_compile_options(full_r002_retained_y_i001 PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=${SOC_ARCH}>
)

cmake_minimum_required(VERSION 3.16)

find_package(ASC REQUIRED)

project(full_r007_reduce_sum LANGUAGES ASC CXX)

set(CMAKE_CXX_STANDARD 14)

if(DEFINED NPU_ARCH)
    set(SOC_ARCH ${NPU_ARCH})
else()
    set(SOC_ARCH "dav-2201")
endif()

add_executable(full_r007_reduce_sum main.asc)

target_link_libraries(full_r007_reduce_sum PRIVATE
    tiling_api
    register
    platform
    unified_dlog
    dl
    m
    graph_base
)

target_link_directories(full_r007_reduce_sum PRIVATE
    /usr/lib/gcc/aarch64-linux-gnu/11
    /usr/lib/aarch64-linux-gnu
)

target_include_directories(full_r007_reduce_sum PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    /usr/include/c++/11
    /usr/include/aarch64-linux-gnu/c++/11
)

target_compile_options(full_r007_reduce_sum PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=${SOC_ARCH}>
)

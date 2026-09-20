cmake_minimum_required(VERSION 3.16)

find_package(ASC REQUIRED)

project(b001_panel LANGUAGES ASC CXX)
set(CMAKE_CXX_STANDARD 14)

# server3's bisheng wrapper selects GCC 12 while the installed C++ headers
# are GCC 11; expose the installed standard library explicitly.
include_directories(SYSTEM
    /usr/include/c++/11
    /usr/include/aarch64-linux-gnu/c++/11
    /usr/include/c++/11/backward
)
link_directories(
    /usr/lib/gcc/aarch64-linux-gnu/11
    /usr/lib/aarch64-linux-gnu
)

if(DEFINED NPU_ARCH)
    set(SOC_ARCH ${NPU_ARCH})
else()
    set(SOC_ARCH "dav-2201")
endif()

add_executable(b001_panel main.asc)
target_include_directories(b001_panel PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(b001_panel PRIVATE
    tiling_api
    register
    platform
    unified_dlog
    dl
    m
    graph_base
)
target_compile_options(b001_panel PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=${SOC_ARCH}>
)

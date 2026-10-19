cmake_minimum_required(VERSION 3.16)

find_package(ASC REQUIRED)

project(multimode_r031_d004 LANGUAGES ASC CXX)

set(CMAKE_CXX_STANDARD 14)

if(DEFINED NPU_ARCH)
    set(SOC_ARCH ${NPU_ARCH})
else()
    set(SOC_ARCH "dav-2201")
endif()

add_executable(multimode_r031_d004 main.asc)

target_link_libraries(multimode_r031_d004 PRIVATE
    tiling_api
    register
    platform
    unified_dlog
    dl
    m
    graph_base
)

target_link_directories(multimode_r031_d004 PRIVATE
    /usr/lib/gcc/aarch64-linux-gnu/11
)

target_include_directories(multimode_r031_d004 PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_include_directories(multimode_r031_d004 SYSTEM PRIVATE
    /usr/include/c++/11
    /usr/include/aarch64-linux-gnu/c++/11
)

target_compile_options(multimode_r031_d004 PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=${SOC_ARCH}>
)

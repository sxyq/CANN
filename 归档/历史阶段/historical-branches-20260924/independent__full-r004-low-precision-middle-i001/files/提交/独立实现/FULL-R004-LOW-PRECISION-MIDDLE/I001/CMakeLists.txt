cmake_minimum_required(VERSION 3.16)

find_package(ASC REQUIRED)

project(add_rms_norm_bias_independent_i001 LANGUAGES ASC CXX)

set(CMAKE_CXX_STANDARD 14)

add_executable(add_rms_norm_bias_independent_i001 main.asc)

target_link_libraries(add_rms_norm_bias_independent_i001 PRIVATE
    tiling_api
    register
    platform
    unified_dlog
    dl
    m
    graph_base
)

target_include_directories(add_rms_norm_bias_independent_i001 PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_directories(add_rms_norm_bias_independent_i001 PRIVATE
    /usr/lib/gcc/aarch64-linux-gnu/11
    /usr/lib/aarch64-linux-gnu
)

target_compile_options(add_rms_norm_bias_independent_i001 PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:-isystem/usr/include/c++/11>
    $<$<COMPILE_LANGUAGE:ASC>:-isystem/usr/include/aarch64-linux-gnu/c++/11>
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=dav-2201>
)

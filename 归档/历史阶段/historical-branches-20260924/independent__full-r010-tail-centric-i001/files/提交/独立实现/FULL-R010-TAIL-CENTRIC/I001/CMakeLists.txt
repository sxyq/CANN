cmake_minimum_required(VERSION 3.16)
find_package(ASC REQUIRED)
project(tail_centric LANGUAGES ASC CXX)
set(CMAKE_CXX_STANDARD 17)

add_library(tail_centric_kernel OBJECT compile_main.asc)
set_source_files_properties(compile_main.asc PROPERTIES LANGUAGE ASC)
add_custom_target(tail_centric ALL DEPENDS tail_centric_kernel)
target_compile_options(tail_centric_kernel PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=dav-2201>
    $<$<COMPILE_LANGUAGE:ASC>:-isystem/usr/include/c++/11>
    $<$<COMPILE_LANGUAGE:ASC>:-isystem/usr/include/aarch64-linux-gnu/c++/11>)

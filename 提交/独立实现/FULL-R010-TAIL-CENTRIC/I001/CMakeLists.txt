cmake_minimum_required(VERSION 3.16)
find_package(ASC REQUIRED)
project(tail_centric LANGUAGES ASC CXX)
set(CMAKE_CXX_STANDARD 17)

set_source_files_properties(op_host/tail_centric_main.txt PROPERTIES LANGUAGE CXX)
add_library(tail_centric_kernel OBJECT op_kernel/tail_centric_kernel.txt)
set_source_files_properties(op_kernel/tail_centric_kernel.txt PROPERTIES LANGUAGE ASC)
add_executable(tail_centric op_host/tail_centric_main.txt $<TARGET_OBJECTS:tail_centric_kernel>)
target_include_directories(tail_centric PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/op_kernel
    ${CMAKE_CURRENT_SOURCE_DIR}/op_host)
target_link_libraries(tail_centric PRIVATE
    tiling_api register platform unified_dlog dl m graph_base)
target_compile_options(tail_centric PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=dav-2201>)
target_compile_options(tail_centric_kernel PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=dav-2201>)

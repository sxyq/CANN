cmake_minimum_required(VERSION 3.16)

find_package(ASC REQUIRED)
project(epi_x_fresh LANGUAGES ASC CXX)

set(CMAKE_CXX_STANDARD 17)

add_executable(epi_x_fresh op_host/epi_x_fresh.asc)
target_include_directories(epi_x_fresh PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/op_kernel
    ${CMAKE_CURRENT_SOURCE_DIR}/op_host
)
target_link_libraries(epi_x_fresh PRIVATE
    tiling_api
    register
    platform
    unified_dlog
    dl
    m
    graph_base
)
target_compile_options(epi_x_fresh PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=dav-2201>
)

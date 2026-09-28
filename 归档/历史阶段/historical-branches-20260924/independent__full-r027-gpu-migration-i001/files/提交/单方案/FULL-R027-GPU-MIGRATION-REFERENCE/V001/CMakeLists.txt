cmake_minimum_required(VERSION 3.16)
find_package(ASC REQUIRED)
project(full_r027_compile LANGUAGES ASC CXX)
set(CMAKE_CXX_STANDARD 14)
set(SOC_ARCH "dav-3510")
add_executable(full_r027_compile compile_main.asc)
target_link_libraries(full_r027_compile PRIVATE tiling_api register platform unified_dlog dl m graph_base)
target_link_directories(full_r027_compile PRIVATE /usr/lib/gcc/aarch64-linux-gnu/11)
target_include_directories(full_r027_compile PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(full_r027_compile PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=${SOC_ARCH}>
    $<$<COMPILE_LANGUAGE:ASC>:-isystem/usr/include/c++/11>
    $<$<COMPILE_LANGUAGE:ASC>:-isystem/usr/include/aarch64-linux-gnu/c++/11>
)

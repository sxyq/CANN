cmake_minimum_required(VERSION 3.16)

set(CMAKE_ASC_ARCHITECTURES "dav-2201" CACHE STRING "NPU architecture: dav-2201")
set(SOC_VERSION "Ascend910B3" CACHE STRING "SoC version")
set(ATX_KERNEL_REL "V001/kernel.asc" CACHE STRING "Kernel relative path from workspace root")
set(ATX_TYPES ${CMAKE_CURRENT_SOURCE_DIR}/support/judge_types.hpp)

find_package(ASC REQUIRED)

project(atx_local LANGUAGES ASC CXX)

set(ATX_KERNEL ${CMAKE_CURRENT_SOURCE_DIR}/${ATX_KERNEL_REL})
if(NOT EXISTS "${ATX_KERNEL}")
    message(FATAL_ERROR "missing kernel: ${ATX_KERNEL}")
endif()

set(ATX_HOST_INCLUDES
    /usr/include/c++/11
    /usr/include/aarch64-linux-gnu/c++/11
    /usr/include
    /usr/include/aarch64-linux-gnu
    $ENV{ASCEND_HOME_PATH}/include
    $ENV{ASCEND_HOME_PATH}/compiler/tikcpp/tikcfw
)
set(ATX_LINK_DIRS
    /usr/lib/gcc/aarch64-linux-gnu/11
    /usr/lib/aarch64-linux-gnu
)

add_library(atx_device OBJECT ${ATX_KERNEL})
target_include_directories(atx_device PRIVATE ${ATX_HOST_INCLUDES})
target_compile_options(atx_device PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=${CMAKE_ASC_ARCHITECTURES}>
    $<$<COMPILE_LANGUAGE:ASC>:-include>
    $<$<COMPILE_LANGUAGE:ASC>:${ATX_TYPES}>
)

add_library(atx_submission OBJECT ${ATX_KERNEL})
target_include_directories(atx_submission PRIVATE ${ATX_HOST_INCLUDES})
target_compile_options(atx_submission PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=${CMAKE_ASC_ARCHITECTURES}>
    $<$<COMPILE_LANGUAGE:ASC>:-include>
    $<$<COMPILE_LANGUAGE:ASC>:${ATX_TYPES}>
)

# Full link: include kernel path relative to workspace root via -I workspace.
# Driver source includes ATX_KERNEL_REL as a quoted path string from a generated header.
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/atx_kernel_path.h
     "#pragma once\n#include \"${ATX_KERNEL_REL}\"\n")
add_executable(atx_full ${CMAKE_CURRENT_SOURCE_DIR}/support/local_full_link_main.asc)
target_include_directories(atx_full PRIVATE
    ${ATX_HOST_INCLUDES}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/support
)
target_link_directories(atx_full PRIVATE ${ATX_LINK_DIRS})
target_link_libraries(atx_full PRIVATE stdc++ m dl)
set_target_properties(atx_full PROPERTIES LINKER_LANGUAGE ASC)
target_compile_options(atx_full PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=${CMAKE_ASC_ARCHITECTURES}>
)

cmake_minimum_required(VERSION 3.16)

set(CMAKE_ASC_ARCHITECTURES "dav-2201" CACHE STRING "NPU architecture: dav-2201")
set(SOC_VERSION "Ascend910B3" CACHE STRING "SoC version")
set(I001_REV "V002" CACHE STRING "Revision folder name")

find_package(ASC REQUIRED)

project(i001_v002 LANGUAGES ASC CXX)

set(I001_SUBMISSION ${CMAKE_CURRENT_SOURCE_DIR}/${I001_REV}/kernel.asc)
set(I001_TYPES ${CMAKE_CURRENT_SOURCE_DIR}/V001/judge_types.hpp)

set(I001_HOST_INCLUDES
    /usr/include/c++/11
    /usr/include/aarch64-linux-gnu/c++/11
    /usr/include
    /usr/include/aarch64-linux-gnu
    $ENV{ASCEND_HOME_PATH}/include
    $ENV{ASCEND_HOME_PATH}/compiler/tikcpp/tikcfw
)
set(I001_LINK_DIRS
    /usr/lib/gcc/aarch64-linux-gnu/11
    /usr/lib/aarch64-linux-gnu
)

# 1) device compile: object from submission
add_library(v002_device OBJECT ${I001_SUBMISSION})
target_include_directories(v002_device PRIVATE ${I001_HOST_INCLUDES})
target_compile_options(v002_device PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=${CMAKE_ASC_ARCHITECTURES}>
    $<$<COMPILE_LANGUAGE:ASC>:-include>
    $<$<COMPILE_LANGUAGE:ASC>:${I001_TYPES}>
)

# 2) submission compile: same TU, local types only
add_library(v002_submission OBJECT ${I001_SUBMISSION})
target_include_directories(v002_submission PRIVATE ${I001_HOST_INCLUDES})
target_compile_options(v002_submission PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=${CMAKE_ASC_ARCHITECTURES}>
    $<$<COMPILE_LANGUAGE:ASC>:-include>
    $<$<COMPILE_LANGUAGE:ASC>:${I001_TYPES}>
)

# 3) full link: local driver includes types + submission
add_executable(v002_full ${CMAKE_CURRENT_SOURCE_DIR}/local_full.asc)
target_include_directories(v002_full PRIVATE ${I001_HOST_INCLUDES})
target_link_directories(v002_full PRIVATE ${I001_LINK_DIRS})
target_link_libraries(v002_full PRIVATE stdc++ m dl)
set_target_properties(v002_full PROPERTIES LINKER_LANGUAGE ASC)
target_compile_options(v002_full PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=${CMAKE_ASC_ARCHITECTURES}>
)
target_include_directories(v002_full PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/${I001_REV}
)

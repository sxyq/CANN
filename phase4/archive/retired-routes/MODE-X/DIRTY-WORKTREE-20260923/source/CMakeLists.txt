cmake_minimum_required(VERSION 3.16)

set(CMAKE_ASC_ARCHITECTURES "dav-2201" CACHE STRING "NPU architecture: dav-2201")
set(SOC_VERSION "Ascend910B3" CACHE STRING "SoC version")

find_package(ASC REQUIRED)

project(mode_x_addrmsnormbias LANGUAGES ASC CXX)

set(CMAKE_CXX_STANDARD 17)

set(MODE_X_TYPES ${CMAKE_CURRENT_SOURCE_DIR}/local_types.h)
set(MODE_X_HOST_INCLUDES
    /usr/include/c++/11
    /usr/include/aarch64-linux-gnu/c++/11
    /usr/include
    /usr/include/aarch64-linux-gnu
    $ENV{ASCEND_HOME_PATH}/include
    $ENV{ASCEND_HOME_PATH}/compiler/tikcpp/tikcfw
)
set(MODE_X_LINK_DIRS
    /usr/lib/gcc/aarch64-linux-gnu/11
    /usr/lib/aarch64-linux-gnu
)

# 1) Device compile: __global__ __vector__ entry (host run_kernel compiled out).
add_library(mode_x_device OBJECT ${CMAKE_CURRENT_SOURCE_DIR}/device_shim.asc)
target_include_directories(mode_x_device PRIVATE ${MODE_X_HOST_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(mode_x_device PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=${CMAKE_ASC_ARCHITECTURES}>
)

# 2) Submission compile: judge-shaped source with local TensorInfo types supplied first.
add_library(mode_x_submission OBJECT ${CMAKE_CURRENT_SOURCE_DIR}/submission_shim.asc)
target_include_directories(mode_x_submission PRIVATE ${MODE_X_HOST_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(mode_x_submission PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=${CMAKE_ASC_ARCHITECTURES}>
)

# 3) Full link: host main + run_kernel + device kernel into one executable.
add_executable(mode_x_full_link ${CMAKE_CURRENT_SOURCE_DIR}/local_full.asc)
target_include_directories(mode_x_full_link PRIVATE ${MODE_X_HOST_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR})
target_link_directories(mode_x_full_link PRIVATE ${MODE_X_LINK_DIRS})
target_link_libraries(mode_x_full_link PRIVATE stdc++ m dl)
set_target_properties(mode_x_full_link PROPERTIES LINKER_LANGUAGE ASC)
target_compile_options(mode_x_full_link PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=${CMAKE_ASC_ARCHITECTURES}>
)

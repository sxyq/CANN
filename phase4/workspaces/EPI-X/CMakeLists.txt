cmake_minimum_required(VERSION 3.16)

set(CMAKE_ASC_ARCHITECTURES "dav-2201" CACHE STRING "NPU architecture: dav-2201")
set(SOC_VERSION "Ascend910B3" CACHE STRING "SoC version")
set(EPIX_REV "V001" CACHE STRING "Revision folder name")

find_package(ASC REQUIRED)

project(epi_x_v001 LANGUAGES ASC CXX)

set(EPIX_SUBMISSION ${CMAKE_CURRENT_SOURCE_DIR}/${EPIX_REV}/kernel.asc)
set(EPIX_TYPES ${CMAKE_CURRENT_SOURCE_DIR}/local_types.h)

set(EPIX_HOST_INCLUDES
    /usr/include/c++/11
    /usr/include/aarch64-linux-gnu/c++/11
    /usr/include
    /usr/include/aarch64-linux-gnu
    $ENV{ASCEND_HOME_PATH}/include
    $ENV{ASCEND_HOME_PATH}/runtime/include
    $ENV{ASCEND_HOME_PATH}/compiler/tikcpp/tikcfw
)
set(EPIX_LINK_DIRS
    /usr/lib/gcc/aarch64-linux-gnu/11
    /usr/lib/aarch64-linux-gnu
    $ENV{ASCEND_HOME_PATH}/aarch64-linux/lib64
)

# 1) Device compile: submission source as ASC object (__global__ __vector__).
add_library(epix_device OBJECT ${EPIX_SUBMISSION})
target_include_directories(epix_device PRIVATE ${EPIX_HOST_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(epix_device PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=${CMAKE_ASC_ARCHITECTURES}>
    $<$<COMPILE_LANGUAGE:ASC>:-include>
    $<$<COMPILE_LANGUAGE:ASC>:${EPIX_TYPES}>
)

# 2) Submission compile: judge-shaped source with locally provided types.
add_library(epix_submission OBJECT ${CMAKE_CURRENT_SOURCE_DIR}/submission_shim.asc)
target_include_directories(epix_submission PRIVATE ${EPIX_HOST_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(epix_submission PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=${CMAKE_ASC_ARCHITECTURES}>
    $<$<COMPILE_LANGUAGE:ASC>:-include>
    $<$<COMPILE_LANGUAGE:ASC>:${EPIX_TYPES}>
)

# 3) Full link: host main + run_kernel + device kernel.
add_executable(epix_full ${CMAKE_CURRENT_SOURCE_DIR}/local_full.asc)
target_include_directories(epix_full PRIVATE ${EPIX_HOST_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR})
target_link_directories(epix_full PRIVATE ${EPIX_LINK_DIRS})
target_link_libraries(epix_full PRIVATE ascendcl stdc++ m dl)
set_target_properties(epix_full PROPERTIES LINKER_LANGUAGE ASC)
target_compile_options(epix_full PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=${CMAKE_ASC_ARCHITECTURES}>
    $<$<COMPILE_LANGUAGE:ASC>:-include>
    $<$<COMPILE_LANGUAGE:ASC>:${EPIX_TYPES}>
)

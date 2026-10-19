cmake_minimum_required(VERSION 3.16)

set(CMAKE_ASC_ARCHITECTURES "dav-2201" CACHE STRING "NPU architecture: dav-2201")
set(SOC_VERSION "Ascend910B3" CACHE STRING "SoC version")

find_package(ASC REQUIRED)

project(i001_v001 LANGUAGES ASC CXX)

set(I001_SUBMISSION ${CMAKE_CURRENT_SOURCE_DIR}/V001/add_rms_norm_bias_v001.asc)
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

function(i001_setup target)
    target_include_directories(${target} PRIVATE ${I001_HOST_INCLUDES})
    target_link_directories(${target} PRIVATE ${I001_LINK_DIRS})
    target_link_libraries(${target} PRIVATE stdc++ m dl)
    set_target_properties(${target} PROPERTIES LINKER_LANGUAGE ASC)
    target_compile_options(${target} PRIVATE
        $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=${CMAKE_ASC_ARCHITECTURES}>
    )
endfunction()

# 1) device compile: submission source (kernels + run_kernel) as object
add_library(v001_device OBJECT ${I001_SUBMISSION})
target_include_directories(v001_device PRIVATE ${I001_HOST_INCLUDES})
target_compile_options(v001_device PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=${CMAKE_ASC_ARCHITECTURES}>
    $<$<COMPILE_LANGUAGE:ASC>:-include>
    $<$<COMPILE_LANGUAGE:ASC>:${I001_TYPES}>
)

# 2) submission compile: same source, local helper provides TensorInfo/TensorGroupInfo
add_library(v001_submission OBJECT ${I001_SUBMISSION})
target_include_directories(v001_submission PRIVATE ${I001_HOST_INCLUDES})
target_compile_options(v001_submission PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=${CMAKE_ASC_ARCHITECTURES}>
    $<$<COMPILE_LANGUAGE:ASC>:-include>
    $<$<COMPILE_LANGUAGE:ASC>:${I001_TYPES}>
)

# 3) full link: local driver includes types + submission and references run_kernel
add_executable(v001_full ${CMAKE_CURRENT_SOURCE_DIR}/local_full.asc)
i001_setup(v001_full)

cmake_minimum_required(VERSION 3.16)

find_package(ASC REQUIRED)

project(wide_x_addrmsnormbias LANGUAGES ASC CXX)

set(CMAKE_CXX_STANDARD 17)

if(DEFINED NPU_ARCH)
    set(SOC_ARCH ${NPU_ARCH})
else()
    set(SOC_ARCH "dav-2201")
endif()

set(SOC_VERSION Ascend910B3)

set(WIDE_X_HCC_ROOT "$ENV{ASCEND_HOME_PATH}/toolkit/toolchain/hcc")
set(WIDE_X_CXX_INC
    "-I${WIDE_X_HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0"
    "-I${WIDE_X_HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0/aarch64-target-linux-gnu"
    "-I${WIDE_X_HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0/backward"
    "-I${WIDE_X_HCC_ROOT}/aarch64-target-linux-gnu/include")
string(JOIN " " WIDE_X_CXX_INC_STR ${WIDE_X_CXX_INC})
set(CMAKE_ASC_COMPILE_OBJECT
    "<CMAKE_ASC_COMPILER> ${CMAKE_COMPILE_AS_ASC_FLAG} <DEFINES> <INCLUDES> ${WIDE_X_CXX_INC_STR} --cce-aicore-arch=dav-c220 -Xaicore-start -O3 -std=c++17 -Xaicore-end -Xhost-start ${WIDE_X_CXX_INC_STR} -fPIC -O2 -std=c++17 -Xhost-end <FLAGS> -o <OBJECT> -c -x asc <SOURCE>")

# 1) Device compile: __global__ __vector__ entry only (host run_kernel compiled out).
add_library(wide_x_device OBJECT
    device_shim.asc)
target_include_directories(wide_x_device PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(wide_x_device PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=${SOC_ARCH}>)

# 2) Submission compile: full judge-shaped source with local TensorInfo types supplied first.
add_library(wide_x_submission OBJECT
    submission_shim.asc)
target_include_directories(wide_x_submission PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(wide_x_submission PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=${SOC_ARCH}>)

# 3) Full link: host main + run_kernel + device kernel into one executable.
add_executable(wide_x_full_link main.asc)
target_include_directories(wide_x_full_link PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(wide_x_full_link PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=${SOC_ARCH}>)
target_link_directories(wide_x_full_link PRIVATE
    ${WIDE_X_HCC_ROOT}/aarch64-target-linux-gnu/lib64
    ${WIDE_X_HCC_ROOT}/lib64)
target_link_libraries(wide_x_full_link PRIVATE
    tiling_api
    register
    platform
    unified_dlog
    stdc++
    dl
    m
    graph_base
)

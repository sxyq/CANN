cmake_minimum_required(VERSION 3.16)

if(DEFINED ENV{ASCEND_HOME_PATH})
    set(_ASC_CMAKE "$ENV{ASCEND_HOME_PATH}/aarch64-linux/tikcpp/ascendc_kernel_cmake")
    list(APPEND CMAKE_MODULE_PATH "${_ASC_CMAKE}/ASC_CMake" "${_ASC_CMAKE}")
    list(APPEND CMAKE_PREFIX_PATH "${_ASC_CMAKE}")
endif()

find_package(ASC REQUIRED)

project(reduce_x_addrmsnormbias LANGUAGES ASC CXX)

set(CMAKE_CXX_STANDARD 17)

if(DEFINED NPU_ARCH)
    set(SOC_ARCH ${NPU_ARCH})
else()
    set(SOC_ARCH "dav-2201")
endif()

set(SOC_VERSION Ascend910B3)

set(REDUCE_X_HCC_ROOT "$ENV{ASCEND_HOME_PATH}/toolkit/toolchain/hcc")
set(REDUCE_X_ASC_KERNEL_OPTIONS
    "--cce-aicore-arch=dav-c220"
    "-I${REDUCE_X_HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0"
    "-I${REDUCE_X_HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0/aarch64-target-linux-gnu"
    "-I${REDUCE_X_HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0/backward")
string(JOIN " " REDUCE_X_ASC_KERNEL_OPTIONS_STR ${REDUCE_X_ASC_KERNEL_OPTIONS})
set(REDUCE_X_ASC_HOST_OPTIONS
    "-I${REDUCE_X_HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0"
    "-I${REDUCE_X_HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0/aarch64-target-linux-gnu"
    "-I${REDUCE_X_HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0/backward")
string(JOIN " " REDUCE_X_ASC_HOST_OPTIONS_STR ${REDUCE_X_ASC_HOST_OPTIONS})
set(CMAKE_ASC_COMPILE_OBJECT
    "<CMAKE_ASC_COMPILER> ${CMAKE_COMPILE_AS_ASC_FLAG} <DEFINES> <INCLUDES> ${REDUCE_X_ASC_KERNEL_OPTIONS_STR} -Xaicore-start -O3 -std=c++17 -Xaicore-end -Xhost-start ${REDUCE_X_ASC_HOST_OPTIONS_STR} -fPIC -O2 -Xhost-end <FLAGS> -o <OBJECT> -c -x asc <SOURCE>")

# 1) Device compile: submission via local type shim (contains __global__ __vector__).
add_library(reduce_x_device OBJECT
    submission_shim.asc)
target_include_directories(reduce_x_device PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(reduce_x_device PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=${SOC_ARCH}>)

# 2) Submission compile: judge-shaped source with locally provided TensorInfo types.
add_library(reduce_x_submission OBJECT
    submission_shim.asc)
target_include_directories(reduce_x_submission PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(reduce_x_submission PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=${SOC_ARCH}>)

# 3) Full link: host main + run_kernel + device kernel into one executable.
if(DEFINED ENV{ASCEND_HOME_PATH})
    set(_HCC_ROOT "$ENV{ASCEND_HOME_PATH}/toolkit/toolchain/hcc")
    set(_HCC_LIB "${_HCC_ROOT}/aarch64-target-linux-gnu/lib64")
    set(_HCC_SYSROOT_LIB "${_HCC_ROOT}/sysroot/lib64")
    set(_HCC_SYSROOT_USR "${_HCC_ROOT}/sysroot/usr/lib64")
    link_directories("${_HCC_LIB}" "${_HCC_SYSROOT_LIB}" "${_HCC_SYSROOT_USR}" "/lib/aarch64-linux-gnu" "/usr/lib/aarch64-linux-gnu")
endif()
add_executable(reduce_x_full_link main.asc)
target_include_directories(reduce_x_full_link PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(reduce_x_full_link PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=${SOC_ARCH}>)
if(DEFINED ENV{ASCEND_HOME_PATH})
    target_link_options(reduce_x_full_link PRIVATE
        "--sysroot=$ENV{ASCEND_HOME_PATH}/toolkit/toolchain/hcc/sysroot"
        "-B$ENV{ASCEND_HOME_PATH}/toolkit/toolchain/hcc/sysroot/usr/lib64"
        "-B/usr/lib/gcc/aarch64-linux-gnu/11"
        "-L$ENV{ASCEND_HOME_PATH}/toolkit/toolchain/hcc/sysroot/usr/lib64"
        "-L$ENV{ASCEND_HOME_PATH}/toolkit/toolchain/hcc/sysroot/lib64"
        "-L$ENV{ASCEND_HOME_PATH}/toolkit/toolchain/hcc/aarch64-target-linux-gnu/lib64"
        "-L/usr/lib/gcc/aarch64-linux-gnu/11")
endif()
target_link_libraries(reduce_x_full_link PRIVATE
    tiling_api
    register
    platform
    unified_dlog
    dl
    m
    graph_base
)

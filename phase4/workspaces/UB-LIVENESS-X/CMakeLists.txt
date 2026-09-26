cmake_minimum_required(VERSION 3.16)

if(DEFINED ENV{ASCEND_HOME_PATH})
    set(_ASC_CMAKE "$ENV{ASCEND_HOME_PATH}/aarch64-linux/tikcpp/ascendc_kernel_cmake")
    list(APPEND CMAKE_MODULE_PATH "${_ASC_CMAKE}/ASC_CMake" "${_ASC_CMAKE}")
    list(APPEND CMAKE_PREFIX_PATH "${_ASC_CMAKE}")
endif()

find_package(ASC REQUIRED)
project(ub_liveness_x_addrmsnormbias LANGUAGES ASC CXX)
set(CMAKE_CXX_STANDARD 17)

if(DEFINED NPU_ARCH)
    set(SOC_ARCH ${NPU_ARCH})
else()
    set(SOC_ARCH "dav-2201")
endif()
set(SOC_VERSION Ascend910B3)

set(ULX_HCC_ROOT "$ENV{ASCEND_HOME_PATH}/toolkit/toolchain/hcc")
set(ULX_ASC_KERNEL_OPTIONS
    "--cce-aicore-arch=dav-c220"
    "-I${ULX_HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0"
    "-I${ULX_HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0/aarch64-target-linux-gnu"
    "-I${ULX_HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0/backward")
string(JOIN " " ULX_ASC_KERNEL_OPTIONS_STR ${ULX_ASC_KERNEL_OPTIONS})
set(ULX_ASC_HOST_OPTIONS
    "-I${ULX_HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0"
    "-I${ULX_HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0/aarch64-target-linux-gnu"
    "-I${ULX_HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0/backward")
string(JOIN " " ULX_ASC_HOST_OPTIONS_STR ${ULX_ASC_HOST_OPTIONS})
set(CMAKE_ASC_COMPILE_OBJECT
    "<CMAKE_ASC_COMPILER> ${CMAKE_COMPILE_AS_ASC_FLAG} <DEFINES> <INCLUDES> ${ULX_ASC_KERNEL_OPTIONS_STR} -Xaicore-start -O3 -std=c++17 -Xaicore-end -Xhost-start ${ULX_ASC_HOST_OPTIONS_STR} -fPIC -O2 -Xhost-end <FLAGS> -o <OBJECT> -c -x asc <SOURCE>")

add_library(ub_liveness_x_device OBJECT submission_shim.asc)
target_include_directories(ub_liveness_x_device PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(ub_liveness_x_device PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=${SOC_ARCH}>)
if(DEFINED ULX_EXTRA_DEFS)
    target_compile_definitions(ub_liveness_x_device PRIVATE ${ULX_EXTRA_DEFS})
endif()

add_library(ub_liveness_x_submission OBJECT submission_shim.asc)
target_include_directories(ub_liveness_x_submission PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(ub_liveness_x_submission PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=${SOC_ARCH}>)

if(DEFINED ENV{ASCEND_HOME_PATH})
    set(ULX_HCC "$ENV{ASCEND_HOME_PATH}/toolkit/toolchain/hcc")
    set(ULX_HCC_LIB "${ULX_HCC}/aarch64-target-linux-gnu/lib64")
    set(ULX_SYSROOT_LIB "${ULX_HCC}/sysroot/lib64")
    set(ULX_SYSROOT_USR_LIB "${ULX_HCC}/sysroot/usr/lib64")
    link_directories("${ULX_HCC_LIB}" "${ULX_SYSROOT_LIB}" "${ULX_SYSROOT_USR_LIB}"
                     "/lib/aarch64-linux-gnu" "/usr/lib/aarch64-linux-gnu")
endif()
add_executable(ub_liveness_x_full_link main.asc)
target_include_directories(ub_liveness_x_full_link PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(ub_liveness_x_full_link PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=${SOC_ARCH}>)
if(DEFINED ENV{ASCEND_HOME_PATH})
    target_link_options(ub_liveness_x_full_link PRIVATE
        "--sysroot=$ENV{ASCEND_HOME_PATH}/toolkit/toolchain/hcc/sysroot"
        "-B$ENV{ASCEND_HOME_PATH}/toolkit/toolchain/hcc/sysroot/usr/lib64"
        "-B/usr/lib/gcc/aarch64-linux-gnu/11"
        "-L$ENV{ASCEND_HOME_PATH}/toolkit/toolchain/hcc/sysroot/usr/lib64"
        "-L$ENV{ASCEND_HOME_PATH}/toolkit/toolchain/hcc/sysroot/lib64"
        "-L$ENV{ASCEND_HOME_PATH}/toolkit/toolchain/hcc/aarch64-target-linux-gnu/lib64"
        "-L/usr/lib/gcc/aarch64-linux-gnu/11")
endif()
target_link_libraries(ub_liveness_x_full_link PRIVATE
    tiling_api register platform unified_dlog dl m graph_base)

cmake_minimum_required(VERSION 3.16)
find_package(ASC REQUIRED)
project(add_rms_norm_bias LANGUAGES ASC)

set(CMAKE_CXX_STANDARD 17)
set(SOC_VERSION Ascend910B3)

# The server image keeps the host compiler headers in the CANN HCC sysroot.
# Put them before the ASC kernel phase so the compiler does not accidentally
# select the system ARM NEON headers.
set(A001_HCC_ROOT "$ENV{ASCEND_HOME_PATH}/toolkit/toolchain/hcc")
set(A001_ASC_KERNEL_OPTIONS
    "--cce-aicore-arch=dav-c220"
    "-I${A001_HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0"
    "-I${A001_HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0/aarch64-target-linux-gnu"
    "-I${A001_HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0/backward")
string(JOIN " " A001_ASC_KERNEL_OPTIONS_STR ${A001_ASC_KERNEL_OPTIONS})
set(A001_ASC_HOST_OPTIONS
    "-I${A001_HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0"
    "-I${A001_HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0/aarch64-target-linux-gnu"
    "-I${A001_HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0/backward")
string(JOIN " " A001_ASC_HOST_OPTIONS_STR ${A001_ASC_HOST_OPTIONS})
set(CMAKE_ASC_COMPILE_OBJECT
    "<CMAKE_ASC_COMPILER> ${CMAKE_COMPILE_AS_ASC_FLAG} <DEFINES> <INCLUDES> ${A001_ASC_KERNEL_OPTIONS_STR} -Xaicore-start -O3 -std=c++17 -Xaicore-end -Xhost-start ${A001_ASC_HOST_OPTIONS_STR} -fPIC -O2 -Xhost-end <FLAGS> -o <OBJECT> -c -x asc <SOURCE>")

add_library(add_rms_norm_bias_kernel OBJECT
    op_kernel/add_rms_norm_bias_kernel.asc)

# Compile the direct-invocation adapter as the exact source shape sent to the
# judge. The locked base implementation above remains a separate target.
add_library(a001_submission OBJECT
    submission.asc)

add_library(a001_submission_v002 OBJECT
    submission_v002.asc)

add_library(a001_submission_v003 OBJECT
    submission_v003.asc)

add_library(a001_submission_v004 OBJECT
    submission_v004.asc)

add_library(a001_submission_v005 OBJECT
    submission_v005.asc)

add_library(a001_submission_v006 OBJECT
    submission_v006.asc)

add_library(a001_submission_v007 OBJECT
    submission_v007.asc)

add_library(a001_submission_v008 OBJECT
    submission_v008.asc)

add_library(a001_submission_v009 OBJECT
    submission_v009.asc)

add_library(a001_submission_v010 OBJECT
    submission_v010.asc)

target_include_directories(add_rms_norm_bias_kernel PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/op_kernel)

target_compile_options(add_rms_norm_bias_kernel PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=dav-2201>)

target_include_directories(a001_submission PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_options(a001_submission PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=dav-2201>)

target_include_directories(a001_submission_v002 PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_options(a001_submission_v002 PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=dav-2201>)

target_include_directories(a001_submission_v003 PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_options(a001_submission_v003 PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=dav-2201>)

target_include_directories(a001_submission_v004 PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_options(a001_submission_v004 PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=dav-2201>)

target_include_directories(a001_submission_v005 PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_options(a001_submission_v005 PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=dav-2201>)

target_include_directories(a001_submission_v006 PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_options(a001_submission_v006 PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=dav-2201>)

target_include_directories(a001_submission_v007 PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_options(a001_submission_v007 PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=dav-2201>)

target_include_directories(a001_submission_v008 PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_options(a001_submission_v008 PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=dav-2201>)

target_include_directories(a001_submission_v009 PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_options(a001_submission_v009 PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=dav-2201>)

target_include_directories(a001_submission_v010 PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_options(a001_submission_v010 PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=dav-2201>)

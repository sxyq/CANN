cmake_minimum_required(VERSION 3.16)

if(NOT DEFINED ENV{ASCEND_HOME_PATH})
    message(FATAL_ERROR "ASCEND_HOME_PATH must point to the selected CANN toolkit")
endif()

set(ASCEND_CANN_PACKAGE_PATH "$ENV{ASCEND_HOME_PATH}")
set(_ASC_CMAKE "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/tikcpp/ascendc_kernel_cmake")
list(APPEND CMAKE_MODULE_PATH "${_ASC_CMAKE}/ASC_CMake" "${_ASC_CMAKE}")
list(APPEND CMAKE_PREFIX_PATH "${_ASC_CMAKE}")
find_package(ASC REQUIRED)

project(smallmid_v001_correctness LANGUAGES ASC)

set(CMAKE_CXX_STANDARD 17)
set(SOC_VERSION Ascend910B3)
set(SOC_ARCH dav-2201)

set(HCC_ROOT "${ASCEND_CANN_PACKAGE_PATH}/toolkit/toolchain/hcc")
set(HCC_CXX_INCLUDES
    "-I${HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0"
    "-I${HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0/aarch64-target-linux-gnu"
    "-I${HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0/backward"
    "-I${HCC_ROOT}/aarch64-target-linux-gnu/include")
string(JOIN " " HCC_CXX_INCLUDES_STR ${HCC_CXX_INCLUDES})

set(CMAKE_ASC_COMPILE_OBJECT
    "<CMAKE_ASC_COMPILER> ${CMAKE_COMPILE_AS_ASC_FLAG} <DEFINES> <INCLUDES> --cce-aicore-arch=dav-c220 ${HCC_CXX_INCLUDES_STR} -Xaicore-start -O3 -std=c++17 -Xaicore-end -Xhost-start ${HCC_CXX_INCLUDES_STR} -fPIC -O2 -std=c++17 -Xhost-end <FLAGS> -o <OBJECT> -c -x asc <SOURCE>")

add_executable(smd_v001_correctness candidate_correctness.asc)
target_include_directories(smd_v001_correctness PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/..")
target_compile_options(smd_v001_correctness PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=${SOC_ARCH}>)
target_link_directories(smd_v001_correctness PRIVATE
    "${HCC_ROOT}/aarch64-target-linux-gnu/lib64"
    "${HCC_ROOT}/lib64"
    "${ASCEND_CANN_PACKAGE_PATH}/lib64")
target_link_options(smd_v001_correctness PRIVATE
    "--sysroot=${HCC_ROOT}/sysroot"
    "-B${HCC_ROOT}/sysroot/usr/lib64"
    "-B/usr/lib/gcc/aarch64-linux-gnu/11"
    "-L${HCC_ROOT}/sysroot/usr/lib64"
    "-L${HCC_ROOT}/sysroot/lib64"
    "-L${HCC_ROOT}/aarch64-target-linux-gnu/lib64"
    "-L/usr/lib/gcc/aarch64-linux-gnu/11"
    "-L/usr/lib/aarch64-linux-gnu"
    "-Wl,--rpath=${ASCEND_CANN_PACKAGE_PATH}/lib64")
target_link_libraries(smd_v001_correctness PRIVATE
    tiling_api register platform unified_dlog graph_base ascendcl stdc++ dl m)

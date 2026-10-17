cmake_minimum_required(VERSION 3.16)

if(DEFINED ENV{ASCEND_HOME_PATH})
    set(_ASC_CMAKE "$ENV{ASCEND_HOME_PATH}/aarch64-linux/tikcpp/ascendc_kernel_cmake")
    list(APPEND CMAKE_MODULE_PATH "${_ASC_CMAKE}/ASC_CMake" "${_ASC_CMAKE}")
    list(APPEND CMAKE_PREFIX_PATH "${_ASC_CMAKE}")
endif()

find_package(ASC REQUIRED)
project(reduce_finalize_v001 LANGUAGES ASC CXX)

set(CMAKE_CXX_STANDARD 17)
set(SOC_VERSION Ascend910B3)
if(DEFINED NPU_ARCH)
    set(SOC_ARCH ${NPU_ARCH})
else()
    set(SOC_ARCH "dav-2201")
endif()

set(HCC_ROOT "$ENV{ASCEND_HOME_PATH}/toolkit/toolchain/hcc")
set(HCC_CXX_INCLUDES
    "-I${HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0"
    "-I${HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0/aarch64-target-linux-gnu"
    "-I${HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0/backward"
    "-I${HCC_ROOT}/aarch64-target-linux-gnu/include")
string(JOIN " " HCC_CXX_INCLUDES_STR ${HCC_CXX_INCLUDES})

set(CMAKE_ASC_COMPILE_OBJECT
    "<CMAKE_ASC_COMPILER> ${CMAKE_COMPILE_AS_ASC_FLAG} <DEFINES> <INCLUDES> ${HCC_CXX_INCLUDES_STR} --cce-aicore-arch=dav-c220 -Xaicore-start -O3 -std=c++17 -Xaicore-end -Xhost-start ${HCC_CXX_INCLUDES_STR} -fPIC -O2 -std=c++17 -Xhost-end <FLAGS> -o <OBJECT> -c -x asc <SOURCE>")

add_executable(rfh_v001_build_probe build_probe.asc)
target_include_directories(rfh_v001_build_probe PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/../correctness"
    "${CMAKE_CURRENT_SOURCE_DIR}/../..")
target_compile_options(rfh_v001_build_probe PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=${SOC_ARCH}>)

link_directories(
    "${HCC_ROOT}/aarch64-target-linux-gnu/lib64"
    "${HCC_ROOT}/lib64"
    "$ENV{ASCEND_HOME_PATH}/lib64"
    "$ENV{ASCEND_HOME_PATH}/latest/lib64")
if(DEFINED ENV{ASCEND_HOME_PATH})
    target_link_options(rfh_v001_build_probe PRIVATE
        "--sysroot=$ENV{ASCEND_HOME_PATH}/toolkit/toolchain/hcc/sysroot"
        "-B$ENV{ASCEND_HOME_PATH}/toolkit/toolchain/hcc/sysroot/usr/lib64"
        "-B/usr/lib/gcc/aarch64-linux-gnu/11"
        "-L$ENV{ASCEND_HOME_PATH}/toolkit/toolchain/hcc/sysroot/usr/lib64"
        "-L$ENV{ASCEND_HOME_PATH}/toolkit/toolchain/hcc/sysroot/lib64"
        "-L$ENV{ASCEND_HOME_PATH}/toolkit/toolchain/hcc/aarch64-target-linux-gnu/lib64"
        "-L/usr/lib/gcc/aarch64-linux-gnu/11")
endif()
target_link_libraries(rfh_v001_build_probe PRIVATE
    tiling_api register platform unified_dlog stdc++ dl m graph_base ascendcl)

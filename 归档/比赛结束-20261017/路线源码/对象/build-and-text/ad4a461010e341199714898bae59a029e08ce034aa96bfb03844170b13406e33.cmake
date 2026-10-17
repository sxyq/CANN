cmake_minimum_required(VERSION 3.16)

if(DEFINED ENV{ASCEND_HOME_PATH})
    set(_ASC_CMAKE "$ENV{ASCEND_HOME_PATH}/aarch64-linux/tikcpp/ascendc_kernel_cmake")
    list(APPEND CMAKE_MODULE_PATH "${_ASC_CMAKE}/ASC_CMake" "${_ASC_CMAKE}")
    list(APPEND CMAKE_PREFIX_PATH "${_ASC_CMAKE}")
endif()

find_package(ASC REQUIRED)

project(reduce_invscale_x LANGUAGES ASC CXX)

set(CMAKE_CXX_STANDARD 17)

if(DEFINED NPU_ARCH)
    set(SOC_ARCH ${NPU_ARCH})
else()
    set(SOC_ARCH "dav-2201")
endif()

set(SOC_VERSION Ascend910B3)

set(HCC_ROOT "$ENV{ASCEND_HOME_PATH}/toolkit/toolchain/hcc")
set(HCC_CXX_INCLUDES
    "-I${HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0"
    "-I${HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0/aarch64-target-linux-gnu"
    "-I${HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0/backward"
    "-I${HCC_ROOT}/aarch64-target-linux-gnu/include")
string(JOIN " " HCC_CXX_INCLUDES_STR ${HCC_CXX_INCLUDES})

set(CMAKE_ASC_COMPILE_OBJECT
    "<CMAKE_ASC_COMPILER> ${CMAKE_COMPILE_AS_ASC_FLAG} <DEFINES> <INCLUDES> ${HCC_CXX_INCLUDES_STR} --cce-aicore-arch=dav-c220 -Xaicore-start -O3 -std=c++17 -Xaicore-end -Xhost-start ${HCC_CXX_INCLUDES_STR} -fPIC -O2 -std=c++17 -Xhost-end <FLAGS> -o <OBJECT> -c -x asc <SOURCE>")

# ---- compile evidence targets (device / submission / full link) ----
add_library(reduce_invscale_device OBJECT submission_shim.asc)
target_include_directories(reduce_invscale_device PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(reduce_invscale_device PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=${SOC_ARCH}>)

add_library(reduce_invscale_submission OBJECT submission_shim.asc)
target_include_directories(reduce_invscale_submission PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(reduce_invscale_submission PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=${SOC_ARCH}>)

link_directories(
    "${HCC_ROOT}/aarch64-target-linux-gnu/lib64"
    "${HCC_ROOT}/lib64"
    "/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/lib64"
    "/usr/local/Ascend/ascend-toolkit/latest/lib64")

add_executable(reduce_invscale_full_link main.asc)
target_include_directories(reduce_invscale_full_link PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(reduce_invscale_full_link PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=${SOC_ARCH}>)
if(DEFINED ENV{ASCEND_HOME_PATH})
    target_link_options(reduce_invscale_full_link PRIVATE
        "--sysroot=$ENV{ASCEND_HOME_PATH}/toolkit/toolchain/hcc/sysroot"
        "-B$ENV{ASCEND_HOME_PATH}/toolkit/toolchain/hcc/sysroot/usr/lib64"
        "-B/usr/lib/gcc/aarch64-linux-gnu/11"
        "-L$ENV{ASCEND_HOME_PATH}/toolkit/toolchain/hcc/sysroot/usr/lib64"
        "-L$ENV{ASCEND_HOME_PATH}/toolkit/toolchain/hcc/sysroot/lib64"
        "-L$ENV{ASCEND_HOME_PATH}/toolkit/toolchain/hcc/aarch64-target-linux-gnu/lib64"
        "-L/usr/lib/gcc/aarch64-linux-gnu/11")
endif()
target_link_libraries(reduce_invscale_full_link PRIVATE
    tiling_api register platform unified_dlog dl m graph_base)

# ---- NPU probe runners: parent vs V001 ----
foreach(variant IN ITEMS parent candidate v001ref)
    add_executable(reduce_invscale_${variant}_probe runner_${variant}.asc)
    target_include_directories(reduce_invscale_${variant}_probe PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR})
    target_compile_options(reduce_invscale_${variant}_probe PRIVATE
        $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=${SOC_ARCH}>)
    target_link_directories(reduce_invscale_${variant}_probe PRIVATE
        "${HCC_ROOT}/aarch64-target-linux-gnu/lib64"
        "${HCC_ROOT}/lib64"
        "/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/lib64"
        "/usr/local/Ascend/ascend-toolkit/latest/lib64")
    target_link_options(reduce_invscale_${variant}_probe PRIVATE
        "-L/usr/lib/gcc/aarch64-linux-gnu/11"
        "-L/usr/lib/aarch64-linux-gnu")
    target_link_libraries(reduce_invscale_${variant}_probe PRIVATE
        tiling_api register platform unified_dlog stdc++ dl m graph_base
        ascendcl
        -Wl,--rpath=/usr/local/Ascend/ascend-toolkit/latest/lib64
        -Wl,--rpath=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/lib64)
endforeach()

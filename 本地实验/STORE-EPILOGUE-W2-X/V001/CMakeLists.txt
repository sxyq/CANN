cmake_minimum_required(VERSION 3.16)

if(DEFINED ENV{ASCEND_HOME_PATH})
    set(_ASC_CMAKE "$ENV{ASCEND_HOME_PATH}/aarch64-linux/tikcpp/ascendc_kernel_cmake")
    list(APPEND CMAKE_MODULE_PATH "${_ASC_CMAKE}/ASC_CMake" "${_ASC_CMAKE}")
    list(APPEND CMAKE_PREFIX_PATH "${_ASC_CMAKE}")
endif()

find_package(ASC REQUIRED)
project(store_epilogue_w2_v001 LANGUAGES ASC CXX)

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

add_library(store_v001_submission OBJECT submission_shim.asc)
target_include_directories(store_v001_submission PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(store_v001_submission PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=${SOC_ARCH}>)

foreach(variant IN ITEMS parent candidate)
    add_executable(store_v001_${variant}_correctness correctness_${variant}.asc)
    target_include_directories(store_v001_${variant}_correctness PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR})
    target_compile_options(store_v001_${variant}_correctness PRIVATE
        $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=${SOC_ARCH}>)
    target_link_directories(store_v001_${variant}_correctness PRIVATE
        "${HCC_ROOT}/aarch64-target-linux-gnu/lib64"
        "${HCC_ROOT}/lib64"
        "/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/lib64"
        "/usr/local/Ascend/ascend-toolkit/latest/lib64")
    target_link_options(store_v001_${variant}_correctness PRIVATE
        "-L/usr/lib/gcc/aarch64-linux-gnu/11"
        "-L/usr/lib/aarch64-linux-gnu")
    target_link_libraries(store_v001_${variant}_correctness PRIVATE
        tiling_api register platform unified_dlog stdc++ dl m graph_base
        ascendcl
        -Wl,--rpath=/usr/local/Ascend/ascend-toolkit/latest/lib64
        -Wl,--rpath=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/lib64)
endforeach()

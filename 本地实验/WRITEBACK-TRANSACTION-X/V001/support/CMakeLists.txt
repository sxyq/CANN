cmake_minimum_required(VERSION 3.16)

if(DEFINED ENV{ASCEND_HOME_PATH})
  set(_ASC_CMAKE "$ENV{ASCEND_HOME_PATH}/aarch64-linux/tikcpp/ascendc_kernel_cmake")
  list(APPEND CMAKE_MODULE_PATH "${_ASC_CMAKE}/ASC_CMake" "${_ASC_CMAKE}")
  list(APPEND CMAKE_PREFIX_PATH "${_ASC_CMAKE}")
endif()

find_package(ASC REQUIRED)
project(writeback_transaction_v001 LANGUAGES ASC CXX)
set(CMAKE_CXX_STANDARD 17)
set(SOC_VERSION Ascend910B3)
set(SOC_ARCH dav-2201)

set(HCC_ROOT "$ENV{ASCEND_HOME_PATH}/toolkit/toolchain/hcc")
set(HCC_CXX_INCLUDES
  "-I${HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0"
  "-I${HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0/aarch64-target-linux-gnu"
  "-I${HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0/backward"
  "-I${HCC_ROOT}/aarch64-target-linux-gnu/include")
string(JOIN " " HCC_CXX_INCLUDES_STR ${HCC_CXX_INCLUDES})
set(CMAKE_ASC_COMPILE_OBJECT
  "<CMAKE_ASC_COMPILER> <DEFINES> <INCLUDES> ${HCC_CXX_INCLUDES_STR} --cce-aicore-arch=dav-c220 -Xaicore-start -O3 -std=c++17 -Xaicore-end -Xhost-start ${HCC_CXX_INCLUDES_STR} -fPIC -O2 -std=c++17 -Xhost-end <FLAGS> -o <OBJECT> -c -x asc <SOURCE>")

add_library(writeback_device OBJECT src/device_include.asc)
target_include_directories(writeback_device PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_compile_options(writeback_device PRIVATE $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=${SOC_ARCH}>)

add_library(writeback_submission OBJECT src/device_include.asc)
target_include_directories(writeback_submission PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_compile_options(writeback_submission PRIVATE $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=${SOC_ARCH}>)

add_executable(writeback_full_link src/adapter.asc)
target_include_directories(writeback_full_link PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_compile_options(writeback_full_link PRIVATE $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=${SOC_ARCH}>)
target_link_directories(writeback_full_link PRIVATE
  "${HCC_ROOT}/aarch64-target-linux-gnu/lib64"
  "${HCC_ROOT}/lib64"
  "$ENV{ASCEND_HOME_PATH}/lib64")
target_link_options(writeback_full_link PRIVATE
  "--sysroot=${HCC_ROOT}/sysroot"
  "-B${HCC_ROOT}/sysroot/usr/lib64"
  "-B/usr/lib/gcc/aarch64-linux-gnu/11"
  "-L${HCC_ROOT}/sysroot/usr/lib64"
  "-L${HCC_ROOT}/sysroot/lib64"
  "-L${HCC_ROOT}/aarch64-target-linux-gnu/lib64"
  "-L/usr/lib/gcc/aarch64-linux-gnu/11")
target_link_libraries(writeback_full_link PRIVATE tiling_api register platform unified_dlog dl m graph_base ascendcl)

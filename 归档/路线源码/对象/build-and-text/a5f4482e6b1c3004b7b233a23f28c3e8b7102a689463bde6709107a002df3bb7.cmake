cmake_minimum_required(VERSION 3.16)
project(c001_cooperative_d_slice LANGUAGES CXX)

if(NOT DEFINED ASCEND_CANN_PACKAGE_PATH)
    set(ASCEND_CANN_PACKAGE_PATH "/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002")
endif()
if(NOT DEFINED SOC_VERSION)
    set(SOC_VERSION "Ascend910B3")
endif()
if(NOT SOC_VERSION STREQUAL "Ascend910B3")
    message(FATAL_ERROR "C001 requires SOC_VERSION=Ascend910B3")
endif()

set(ASCENDC_KERNEL_CMAKE_DIR
    "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/tikcpp/ascendc_kernel_cmake")
include("${ASCENDC_KERNEL_CMAKE_DIR}/bisheng_config.cmake")
set(HCC_TOOLCHAIN "${ASCEND_CANN_PACKAGE_PATH}/toolkit/toolchain/hcc")

add_library(c001_kernel OBJECT c001_kernel.cpp)
target_compile_options(c001_kernel PRIVATE
    -O3
    -std=c++17
    --cce-aicore-lang
    --cce-aicore-arch=dav-c220-vec
    --cce-aicore-only
    --cce-auto-sync
    -D__DAV_C220__
    -DTILING_KEY_VAR=0
)
target_include_directories(c001_kernel PRIVATE
    "${ASCENDC_KERNEL_CMAKE_DIR}/../tikcfw"
    "${ASCENDC_KERNEL_CMAKE_DIR}/../tikcfw/interface"
    "${ASCENDC_KERNEL_CMAKE_DIR}/../tikcfw/impl"
    "${HCC_TOOLCHAIN}/aarch64-target-linux-gnu/include"
    "${HCC_TOOLCHAIN}/aarch64-target-linux-gnu/include/c++/7.3.0"
    "${HCC_TOOLCHAIN}/aarch64-target-linux-gnu/include/c++/7.3.0/aarch64-target-linux-gnu"
    "${HCC_TOOLCHAIN}/aarch64-target-linux-gnu/include/c++/7.3.0/backward"
)

set_target_properties(c001_kernel PROPERTIES
    CXX_STANDARD 17
    CXX_EXTENSIONS OFF
)

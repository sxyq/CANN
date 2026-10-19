cmake_minimum_required(VERSION 3.16)

set(SOC_VERSION "Ascend910B3" CACHE STRING "Ascend SoC")
set(CCE_AICORE_ARCH "dav-2201" CACHE STRING "Ascend C target")
set(ASCEND_CANN_PACKAGE_PATH "/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002" CACHE PATH "CANN root")

set(ASCENDC_INCLUDE_DIR
    "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/include")
set(ASCENDC_INTERNAL_INCLUDE_DIR
    "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/ascendc/include")
set(ASCENDC_IMPL_INCLUDE_DIR
    "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/ascendc/include/basic_api/impl")
set(ASCENDC_PUBLIC_BASIC_INCLUDE_DIR
    "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/asc/include/basic_api")
set(ASCENDC_PUBLIC_INCLUDE_DIR
    "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/asc/include")
set(ASCENDC_ROOT_INCLUDE_DIR
    "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux")
set(ASCENDC_ASC_ROOT_DIR
    "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/asc")
set(ASCENDC_PUBLIC_INTERFACE_INCLUDE_DIR
    "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/asc/include/interface")
set(BISHENG
    "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/ccec_compiler/bin/bisheng")

set(CMAKE_CXX_COMPILER "${BISHENG}" CACHE FILEPATH "Ascend C compiler" FORCE)
project(d001_add_rms_norm_bias LANGUAGES CXX)

add_library(d001_kernel OBJECT src/d001_add_rms_norm_bias.cpp)
set_source_files_properties(src/d001_add_rms_norm_bias.cpp PROPERTIES
    LANGUAGE CXX)
target_include_directories(d001_kernel PRIVATE
    "${ASCENDC_INCLUDE_DIR}"
    "${ASCENDC_INTERNAL_INCLUDE_DIR}"
    "${ASCENDC_IMPL_INCLUDE_DIR}"
    "${ASCENDC_ROOT_INCLUDE_DIR}"
    "${ASCENDC_ASC_ROOT_DIR}"
    "${ASCENDC_PUBLIC_INCLUDE_DIR}"
    "${ASCENDC_PUBLIC_BASIC_INCLUDE_DIR}"
    "${ASCENDC_PUBLIC_INTERFACE_INCLUDE_DIR}"
    "/usr/include/c++/11"
    "/usr/include/aarch64-linux-gnu/c++/11")
target_compile_options(d001_kernel PRIVATE
    "--cce-aicore-lang"
    "--cce-aicore-arch=dav-c220-vec"
    "--cce-aicore-only"
    "--cce-auto-sync"
    "--npu-arch=${CCE_AICORE_ARCH}"
    "--npu-soc=${SOC_VERSION}"
    "-std=c++17")
set_target_properties(d001_kernel PROPERTIES
    POSITION_INDEPENDENT_CODE OFF)

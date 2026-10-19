cmake_minimum_required(VERSION 3.16)

project(d001_add_rms_norm_bias LANGUAGES CXX)

set(SOC_VERSION "Ascend910B3" CACHE STRING "Ascend SoC")
set(CCE_AICORE_ARCH "dav-2201" CACHE STRING "Ascend C target")
set(ASCEND_CANN_PACKAGE_PATH "/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002" CACHE PATH "CANN root")

set(ASCENDC_INCLUDE_DIR
    "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/include")
set(ASCENDC_INTERNAL_INCLUDE_DIR
    "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/ascendc/include")
set(BISHENG
    "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/ccec_compiler/bin/bisheng")

add_library(d001_kernel OBJECT src/d001_add_rms_norm_bias.cpp)
set_source_files_properties(src/d001_add_rms_norm_bias.cpp PROPERTIES
    LANGUAGE CXX)
target_include_directories(d001_kernel PRIVATE
    "${ASCENDC_INCLUDE_DIR}"
    "${ASCENDC_INTERNAL_INCLUDE_DIR}")
target_compile_options(d001_kernel PRIVATE
    "--npu-arch=${CCE_AICORE_ARCH}"
    "--npu-soc=${SOC_VERSION}"
    "-std=c++17")
set_target_properties(d001_kernel PROPERTIES
    CXX_COMPILER "${BISHENG}")

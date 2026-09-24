cmake_minimum_required(VERSION 3.16)

set(SOC_VERSION "Ascend910B3" CACHE STRING "Ascend SoC")
set(CCE_AICORE_ARCH "dav-2201" CACHE STRING "Ascend C target")
set(ASCEND_CANN_PACKAGE_PATH
    "/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002"
    CACHE PATH "CANN root")

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
set(HCC_TOOLCHAIN
    "${ASCEND_CANN_PACKAGE_PATH}/toolkit/toolchain/hcc")
set(BISHENG
    "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/ccec_compiler/bin/bisheng")
set(ASCENDC_LLD "${ASCEND_CANN_PACKAGE_PATH}/tools/ccec_compiler/bin/ld.lld")
if(NOT EXISTS "${ASCENDC_LLD}")
    set(ASCENDC_LLD "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/ccec_compiler/bin/ld.lld")
endif()

set(CMAKE_CXX_COMPILER "${BISHENG}" CACHE FILEPATH "Ascend C compiler" FORCE)
project(mix_a_add_rms_norm_bias LANGUAGES CXX)

add_library(mix_a_submission OBJECT compile_adapter.cpp)
add_library(mix_a_device OBJECT MIX-A-V007-device.cpp)
target_include_directories(mix_a_submission PRIVATE
    "${ASCENDC_INCLUDE_DIR}"
    "${ASCENDC_INTERNAL_INCLUDE_DIR}"
    "${ASCENDC_IMPL_INCLUDE_DIR}"
    "${ASCENDC_ROOT_INCLUDE_DIR}"
    "${ASCENDC_ASC_ROOT_DIR}"
    "${ASCENDC_PUBLIC_INCLUDE_DIR}"
    "${ASCENDC_PUBLIC_BASIC_INCLUDE_DIR}"
    "${ASCENDC_PUBLIC_INTERFACE_INCLUDE_DIR}"
    "${HCC_TOOLCHAIN}/aarch64-target-linux-gnu/include"
    "${HCC_TOOLCHAIN}/aarch64-target-linux-gnu/include/c++/7.3.0"
    "${HCC_TOOLCHAIN}/aarch64-target-linux-gnu/include/c++/7.3.0/aarch64-target-linux-gnu"
    "${HCC_TOOLCHAIN}/aarch64-target-linux-gnu/include/c++/7.3.0/backward")
target_compile_options(mix_a_submission PRIVATE
    "--cce-aicore-lang"
    "--cce-aicore-arch=dav-c220-vec"
    "--cce-aicore-only"
    "--cce-auto-sync"
    "--npu-arch=${CCE_AICORE_ARCH}"
    "--npu-soc=${SOC_VERSION}"
    "-std=c++17")
target_include_directories(mix_a_device PRIVATE
    "${ASCENDC_INCLUDE_DIR}"
    "${ASCENDC_INTERNAL_INCLUDE_DIR}"
    "${ASCENDC_IMPL_INCLUDE_DIR}"
    "${ASCENDC_ROOT_INCLUDE_DIR}"
    "${ASCENDC_ASC_ROOT_DIR}"
    "${ASCENDC_PUBLIC_INCLUDE_DIR}"
    "${ASCENDC_PUBLIC_BASIC_INCLUDE_DIR}"
    "${ASCENDC_PUBLIC_INTERFACE_INCLUDE_DIR}"
    "${HCC_TOOLCHAIN}/aarch64-target-linux-gnu/include"
    "${HCC_TOOLCHAIN}/aarch64-target-linux-gnu/include/c++/7.3.0"
    "${HCC_TOOLCHAIN}/aarch64-target-linux-gnu/include/c++/7.3.0/aarch64-target-linux-gnu"
    "${HCC_TOOLCHAIN}/aarch64-target-linux-gnu/include/c++/7.3.0/backward")
target_compile_options(mix_a_device PRIVATE
    "--cce-aicore-lang"
    "--cce-aicore-arch=dav-c220-vec"
    "--cce-aicore-only"
    "--cce-auto-sync"
    "--npu-arch=${CCE_AICORE_ARCH}"
    "--npu-soc=${SOC_VERSION}"
    "-std=c++17")
add_custom_target(mix_a_full_link
    COMMAND "${ASCENDC_LLD}" -m aicorelinux -Ttext=0 -static
            -o "${CMAKE_CURRENT_BINARY_DIR}/MIX-A-V007-device.alink"
            $<TARGET_OBJECTS:mix_a_device>
    COMMAND "${ASCENDC_LLD}" -m aicorelinux -Ttext=0 -static
            -o "${CMAKE_CURRENT_BINARY_DIR}/MIX-A-V007-submission.alink"
            $<TARGET_OBJECTS:mix_a_submission>
    DEPENDS mix_a_device mix_a_submission
    VERBATIM)
set_target_properties(mix_a_submission PROPERTIES POSITION_INDEPENDENT_CODE OFF)
set_target_properties(mix_a_device PROPERTIES POSITION_INDEPENDENT_CODE OFF)

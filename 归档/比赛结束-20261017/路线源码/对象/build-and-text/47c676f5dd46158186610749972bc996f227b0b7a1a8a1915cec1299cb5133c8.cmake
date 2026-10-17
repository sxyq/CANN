cmake_minimum_required(VERSION 3.16)

set(SOC_VERSION "Ascend910B3" CACHE STRING "Ascend SoC")
set(CCE_AICORE_ARCH "dav-2201" CACHE STRING "Ascend C target")
set(ASCEND_CANN_PACKAGE_PATH
    "/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002"
    CACHE PATH "CANN root")

set(ASCENDC_INCLUDE_DIR "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/include")
set(ASCENDC_INTERNAL_INCLUDE_DIR "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/ascendc/include")
set(ASCENDC_IMPL_INCLUDE_DIR "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/ascendc/include/basic_api/impl")
set(ASCENDC_PUBLIC_BASIC_INCLUDE_DIR "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/asc/include/basic_api")
set(ASCENDC_PUBLIC_INCLUDE_DIR "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/asc/include")
set(ASCENDC_ROOT_INCLUDE_DIR "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux")
set(ASCENDC_ASC_ROOT_DIR "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/asc")
set(ASCENDC_PUBLIC_INTERFACE_INCLUDE_DIR "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/asc/include/interface")
set(HCC_TOOLCHAIN "${ASCEND_CANN_PACKAGE_PATH}/toolkit/toolchain/hcc")
set(BISHENG "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/ccec_compiler/bin/bisheng")
set(ACL_RUNTIME_INCLUDE "${ASCEND_CANN_PACKAGE_PATH}/runtime/include")

set(CMAKE_CXX_COMPILER "${BISHENG}" CACHE FILEPATH "Ascend C compiler" FORCE)
project(async_triple_x_add_rms_norm_bias LANGUAGES CXX)

set(ASYNC_TRIPLE_X_COMMON_INCLUDES
    "${ASCENDC_INCLUDE_DIR}"
    "${ASCENDC_INTERNAL_INCLUDE_DIR}"
    "${ASCENDC_IMPL_INCLUDE_DIR}"
    "${ASCENDC_ROOT_INCLUDE_DIR}"
    "${ASCENDC_ASC_ROOT_DIR}"
    "${ASCENDC_PUBLIC_INCLUDE_DIR}"
    "${ASCENDC_PUBLIC_BASIC_INCLUDE_DIR}"
    "${ASCENDC_PUBLIC_INTERFACE_INCLUDE_DIR}"
    "${ACL_RUNTIME_INCLUDE}"
    "${HCC_TOOLCHAIN}/aarch64-target-linux-gnu/include"
    "${HCC_TOOLCHAIN}/aarch64-target-linux-gnu/include/c++/7.3.0"
    "${HCC_TOOLCHAIN}/aarch64-target-linux-gnu/include/c++/7.3.0/aarch64-target-linux-gnu"
    "${HCC_TOOLCHAIN}/aarch64-target-linux-gnu/include/c++/7.3.0/backward")

set(ASYNC_TRIPLE_X_COMMON_OPTIONS
    "--cce-aicore-lang"
    "--cce-aicore-arch=dav-c220-vec"
    "--cce-aicore-only"
    "--cce-auto-sync"
    "--npu-arch=${CCE_AICORE_ARCH}"
    "--npu-soc=${SOC_VERSION}"
    "-std=c++17")

foreach(pair "v001;ASYNC-TRIPLE-X-V001-submission.cpp;async_triple_x_v001_submission"
              "v001;ASYNC-TRIPLE-X-V001-device.cpp;async_triple_x_v001_device"
              "seed;ASYNC-TRIPLE-X-SEED-submission.cpp;async_triple_x_seed_submission"
              "seed;ASYNC-TRIPLE-X-SEED-device.cpp;async_triple_x_seed_device")
    list(GET pair 0 rev_tag)
    list(GET pair 1 src)
    list(GET pair 2 tgt)
    add_library(${tgt} OBJECT ${src})
    target_include_directories(${tgt} PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}"
        ${ASYNC_TRIPLE_X_COMMON_INCLUDES})
    target_compile_options(${tgt} PRIVATE ${ASYNC_TRIPLE_X_COMMON_OPTIONS})
    set_target_properties(${tgt} PROPERTIES POSITION_INDEPENDENT_CODE OFF)
endforeach()

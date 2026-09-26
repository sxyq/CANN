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

set(CMAKE_CXX_COMPILER "${BISHENG}" CACHE FILEPATH "Ascend C compiler" FORCE)
project(sched_rowgroup_x LANGUAGES CXX)

function(srx_add_flags tgt)
    target_include_directories(${tgt} PRIVATE
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
        "${HCC_TOOLCHAIN}/aarch64-target-linux-gnu/include/c++/7.3.0/backward"
        "${CMAKE_CURRENT_SOURCE_DIR}")
    target_compile_options(${tgt} PRIVATE
        "--cce-aicore-lang"
        "--cce-aicore-arch=dav-c220-vec"
        "--cce-aicore-only"
        "--cce-auto-sync"
        "--npu-arch=${CCE_AICORE_ARCH}"
        "--npu-soc=${SOC_VERSION}"
        "-std=c++17")
    set_target_properties(${tgt} PROPERTIES POSITION_INDEPENDENT_CODE OFF)
endfunction()

add_library(srx_device OBJECT device_include.asc)
srx_add_flags(srx_device)

add_library(srx_submission OBJECT compile_adapter.cpp)
srx_add_flags(srx_submission)

add_executable(srx_full_link compile_adapter.cpp)
srx_add_flags(srx_full_link)
target_link_options(srx_full_link PRIVATE -L/usr/lib/gcc/aarch64-linux-gnu/11 -L/usr/lib/aarch64-linux-gnu)
target_link_libraries(srx_full_link PRIVATE stdc++ m dl)
set_target_properties(srx_full_link PROPERTIES LINKER_LANGUAGE CXX)

# Aliases expected by harness-style invocation
add_custom_target(device DEPENDS srx_device)
add_custom_target(submission DEPENDS srx_submission)
add_custom_target(full_link DEPENDS srx_full_link)

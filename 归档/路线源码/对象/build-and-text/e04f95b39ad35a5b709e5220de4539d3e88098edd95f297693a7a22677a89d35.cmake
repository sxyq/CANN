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
set(LLD "${ASCEND_CANN_PACKAGE_PATH}/tools/ccec_compiler/bin/ld.lld")
if(NOT EXISTS "${LLD}")
    set(LLD "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/ccec_compiler/bin/ld.lld")
endif()
list(PREPEND CMAKE_PREFIX_PATH
    "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/tikcpp/ascendc_kernel_cmake")

set(CMAKE_CXX_COMPILER "${BISHENG}" CACHE FILEPATH "Ascend C compiler" FORCE)
find_package(ASC REQUIRED)
project(ub_lifetime_safe_v001 LANGUAGES ASC CXX)

set(CMAKE_CXX_STANDARD 17)
link_directories(
    /usr/lib/gcc/aarch64-linux-gnu/11
    /usr/lib/aarch64-linux-gnu)

set(V001_HCC_ROOT "${HCC_TOOLCHAIN}")
set(V001_INCLUDE_DIRS
    "${ASCENDC_INCLUDE_DIR}"
    "${ASCENDC_INTERNAL_INCLUDE_DIR}"
    "${ASCENDC_IMPL_INCLUDE_DIR}"
    "${ASCENDC_ROOT_INCLUDE_DIR}"
    "${ASCENDC_ASC_ROOT_DIR}"
    "${ASCENDC_PUBLIC_INCLUDE_DIR}"
    "${ASCENDC_PUBLIC_BASIC_INCLUDE_DIR}"
    "${ASCENDC_PUBLIC_INTERFACE_INCLUDE_DIR}"
    "${V001_HCC_ROOT}/aarch64-target-linux-gnu/include"
    "${V001_HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0"
    "${V001_HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0/aarch64-target-linux-gnu"
    "${V001_HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0/backward")
set(V001_COMPILE_OPTIONS
    "--cce-aicore-lang"
    "--cce-aicore-arch=dav-c220-vec"
    "--cce-aicore-only"
    "--cce-auto-sync"
    "--npu-arch=${CCE_AICORE_ARCH}"
    "--npu-soc=${SOC_VERSION}"
    "-std=c++17")

add_library(ub_v001_submission OBJECT compile_adapter.cpp)
add_library(ub_v001_device OBJECT device.cpp)
foreach(target ub_v001_submission ub_v001_device)
    target_include_directories(${target} PRIVATE ${V001_INCLUDE_DIRS})
    target_compile_options(${target} PRIVATE ${V001_COMPILE_OPTIONS})
    set_target_properties(${target} PROPERTIES POSITION_INDEPENDENT_CODE OFF)
endforeach()

add_custom_target(full_link
    COMMAND "${LLD}" -m aicorelinux -Ttext=0
            "$<TARGET_OBJECTS:ub_v001_device>" -static
            -o "${CMAKE_CURRENT_BINARY_DIR}/ub-lifetime-safe-v001-device.alink"
    COMMAND "${LLD}" -m aicorelinux -Ttext=0
            "$<TARGET_OBJECTS:ub_v001_submission>" -static
            -o "${CMAKE_CURRENT_BINARY_DIR}/ub-lifetime-safe-v001-submission.alink"
    DEPENDS ub_v001_device ub_v001_submission
    VERBATIM)

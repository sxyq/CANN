cmake_minimum_required(VERSION 3.16)
set(SOC_VERSION Ascend910B3 CACHE STRING "soc")
set(CCE_AICORE_ARCH dav-2201 CACHE STRING "arch")
set(ASCEND_CANN_PACKAGE_PATH "/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002" CACHE PATH "cann")
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
set(CMAKE_CXX_COMPILER "${BISHENG}" CACHE FILEPATH "bisheng" FORCE)
project(srx LANGUAGES CXX)
function(srx_add_flags tgt)
  target_include_directories(${tgt} PRIVATE
    "${ASCENDC_INCLUDE_DIR}" "${ASCENDC_INTERNAL_INCLUDE_DIR}" "${ASCENDC_IMPL_INCLUDE_DIR}"
    "${ASCENDC_ROOT_INCLUDE_DIR}" "${ASCENDC_ASC_ROOT_DIR}" "${ASCENDC_PUBLIC_INCLUDE_DIR}"
    "${ASCENDC_PUBLIC_BASIC_INCLUDE_DIR}" "${ASCENDC_PUBLIC_INTERFACE_INCLUDE_DIR}"
    "${HCC_TOOLCHAIN}/aarch64-target-linux-gnu/include"
    "${HCC_TOOLCHAIN}/aarch64-target-linux-gnu/include/c++/7.3.0"
    "${HCC_TOOLCHAIN}/aarch64-target-linux-gnu/include/c++/7.3.0/aarch64-target-linux-gnu"
    "${HCC_TOOLCHAIN}/aarch64-target-linux-gnu/include/c++/7.3.0/backward"
    "${CMAKE_CURRENT_SOURCE_DIR}")
  target_compile_options(${tgt} PRIVATE
    "--cce-aicore-lang" "--cce-aicore-arch=dav-c220-vec"
    "--cce-aicore-only" "--cce-auto-sync"
    "--npu-arch=${CCE_AICORE_ARCH}" "--npu-soc=${SOC_VERSION}" "-std=c++17")
  set_target_properties(${tgt} PROPERTIES POSITION_INDEPENDENT_CODE OFF)
endfunction()
add_library(device OBJECT device_include.cpp)
srx_add_flags(device)
add_library(submission OBJECT compile_adapter.cpp)
srx_add_flags(submission)
add_custom_target(full_link
  COMMAND "${LLD}" -m aicorelinux -Ttext=0 -static
          -o "${CMAKE_CURRENT_BINARY_DIR}/device.alink"
          "$<TARGET_OBJECTS:device>"
  COMMAND "${LLD}" -m aicorelinux -Ttext=0 -static
          -o "${CMAKE_CURRENT_BINARY_DIR}/submission.alink"
          "$<TARGET_OBJECTS:submission>"
  DEPENDS device submission
  VERBATIM)

cmake_minimum_required(VERSION 3.16)

set(ASCEND_CANN_PACKAGE_PATH "/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002" CACHE PATH "CANN root")
set(ENV{ASCEND_HOME_PATH} "${ASCEND_CANN_PACKAGE_PATH}")
list(PREPEND CMAKE_PREFIX_PATH
    "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/tikcpp/ascendc_kernel_cmake")

find_package(ASC REQUIRED)
project(adaptive_core_ownership_v010 LANGUAGES ASC CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(SOC_VERSION Ascend910B3)
set(ASCEND_ARCH dav-2201)
set(ASCEND_INCLUDE_DIR "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/include")
set(ASCENDC_INCLUDE_DIR "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/ascendc/include")
set(ASCENDC_IMPL_INCLUDE_DIR "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/ascendc/include/basic_api/impl")
set(ASC_INCLUDE_DIR "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/asc/include")
set(HCC_TOOLCHAIN "${ASCEND_CANN_PACKAGE_PATH}/toolkit/toolchain/hcc")
set(ASCEND_LIB_DIR "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/lib64")
set(ASCEND_DRIVER_LIB_DIR "/usr/local/Ascend/driver/lib64/driver")
set(ASCEND_DRIVER_COMMON_LIB_DIR "/usr/local/Ascend/driver/lib64/common")

ascendc_library(adaptive_parent SHARED "${CMAKE_CURRENT_SOURCE_DIR}/support/parent_entry.asc")
ascendc_library(adaptive_candidate SHARED "${CMAKE_CURRENT_SOURCE_DIR}/support/candidate_entry.asc")

foreach(kernel_target IN ITEMS adaptive_parent adaptive_candidate)
    target_include_directories(${kernel_target} PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/support"
        "${ASCEND_INCLUDE_DIR}"
        "${ASCENDC_INCLUDE_DIR}"
        "${ASCENDC_IMPL_INCLUDE_DIR}"
        "${ASC_INCLUDE_DIR}"
        "${ASC_INCLUDE_DIR}/basic_api"
        "${ASC_INCLUDE_DIR}/interface"
        "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux")
    target_compile_options(${kernel_target} PRIVATE
        $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=${ASCEND_ARCH}>)
    target_link_directories(${kernel_target} PRIVATE
        "${ASCEND_LIB_DIR}"
        "${ASCEND_CANN_PACKAGE_PATH}/lib64"
        "${HCC_TOOLCHAIN}/aarch64-target-linux-gnu/lib64"
        /usr/lib/aarch64-linux-gnu)
    target_link_libraries(${kernel_target} PRIVATE
        tiling_api register platform unified_dlog dl m graph_base)
    target_link_options(${kernel_target} PRIVATE "-Wl,-Bsymbolic")
    set_target_properties(${kernel_target} PROPERTIES
        BUILD_RPATH "${ASCEND_LIB_DIR};${ASCEND_CANN_PACKAGE_PATH}/lib64;${ASCEND_DRIVER_LIB_DIR};${ASCEND_DRIVER_COMMON_LIB_DIR};$ORIGIN")
endforeach()

add_executable(adaptive_probe support/probe.cpp)
target_include_directories(adaptive_probe PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/support"
    "${ASCEND_INCLUDE_DIR}")
target_link_directories(adaptive_probe PRIVATE
    "${ASCEND_LIB_DIR}"
    "${ASCEND_CANN_PACKAGE_PATH}/lib64"
    /usr/lib/aarch64-linux-gnu)
target_link_libraries(adaptive_probe PRIVATE
    adaptive_parent adaptive_candidate ascendcl dl m)
set_target_properties(adaptive_probe PROPERTIES
    BUILD_RPATH "${ASCEND_LIB_DIR};${ASCEND_CANN_PACKAGE_PATH}/lib64;${ASCEND_DRIVER_LIB_DIR};${ASCEND_DRIVER_COMMON_LIB_DIR};$ORIGIN")

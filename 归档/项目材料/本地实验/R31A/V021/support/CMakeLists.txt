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
project(r31a_v021 LANGUAGES ASC CXX)

set(R31A_INCLUDE_DIRS
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
set(R31A_COMPILE_OPTIONS
    "--cce-aicore-lang"
    "--cce-aicore-arch=dav-c220-vec"
    "--cce-aicore-only"
    "--cce-auto-sync"
    "--npu-arch=${CCE_AICORE_ARCH}"
    "--npu-soc=${SOC_VERSION}"
    "-std=c++17")

foreach(probe probe_v016 probe_v021)
    add_executable(${probe} ${probe}.asc)
    target_include_directories(${probe} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${R31A_INCLUDE_DIRS})
    target_link_libraries(${probe} PRIVATE
        tiling_api register platform unified_dlog dl m graph_base)
    target_link_options(${probe} PRIVATE
        "-L/usr/lib/gcc/aarch64-linux-gnu/11"
        "-L/usr/lib/aarch64-linux-gnu"
        "-L${HCC_TOOLCHAIN}/aarch64-target-linux-gnu/lib64")
    target_compile_options(${probe} PRIVATE
        $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=${CCE_AICORE_ARCH}>)
endforeach()

add_library(r31a_paired_v016 SHARED paired_probe_v016.asc)
target_include_directories(r31a_paired_v016 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${R31A_INCLUDE_DIRS})
target_compile_options(r31a_paired_v016 PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=${CCE_AICORE_ARCH}>)
target_link_libraries(r31a_paired_v016 PRIVATE
    tiling_api register platform unified_dlog dl m graph_base)
target_link_options(r31a_paired_v016 PRIVATE
    "-Wl,-Bsymbolic"
    "-L${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/lib64"
    "-L/usr/lib/gcc/aarch64-linux-gnu/11"
    "-L/usr/lib/aarch64-linux-gnu"
    "-L${HCC_TOOLCHAIN}/aarch64-target-linux-gnu/lib64")

add_library(r31a_paired_v021 SHARED paired_probe_v021.asc)
target_include_directories(r31a_paired_v021 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${R31A_INCLUDE_DIRS})
target_compile_options(r31a_paired_v021 PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=${CCE_AICORE_ARCH}>)
target_link_libraries(r31a_paired_v021 PRIVATE
    tiling_api register platform unified_dlog dl m graph_base)
target_link_options(r31a_paired_v021 PRIVATE
    "-Wl,-Bsymbolic"
    "-L${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/lib64"
    "-L/usr/lib/gcc/aarch64-linux-gnu/11"
    "-L/usr/lib/aarch64-linux-gnu"
    "-L${HCC_TOOLCHAIN}/aarch64-target-linux-gnu/lib64")

set(R31A_SERVER3_GCC11_CXX_INCLUDE_DIRS
    "/usr/include/c++/11"
    "/usr/include/aarch64-linux-gnu/c++/11"
    "/usr/include/c++/11/backward")
list(JOIN R31A_SERVER3_GCC11_CXX_INCLUDE_DIRS ":" R31A_SERVER3_GCC11_CXX_INCLUDE_PATH)
set(R31A_ASC_REGISTRATION_ENV
    "${CMAKE_COMMAND} -E env CPATH=${R31A_SERVER3_GCC11_CXX_INCLUDE_PATH} CPLUS_INCLUDE_PATH=${R31A_SERVER3_GCC11_CXX_INCLUDE_PATH}")
set_property(TARGET r31a_paired_v016 r31a_paired_v021
    PROPERTY RULE_LAUNCH_COMPILE "${R31A_ASC_REGISTRATION_ENV}")

add_executable(r31a_paired_probe paired_probe.cpp)
add_dependencies(r31a_paired_probe r31a_paired_v016 r31a_paired_v021)
target_include_directories(r31a_paired_probe PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${R31A_INCLUDE_DIRS})
target_link_libraries(r31a_paired_probe PRIVATE
    ascendcl tiling_api register platform unified_dlog dl m graph_base stdc++)
target_link_options(r31a_paired_probe PRIVATE
    "-L${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/lib64"
    "-L/usr/lib/gcc/aarch64-linux-gnu/11"
    "-L/usr/lib/aarch64-linux-gnu"
    "-L${HCC_TOOLCHAIN}/aarch64-target-linux-gnu/lib64")
target_compile_options(r31a_paired_probe PRIVATE
    $<$<COMPILE_LANGUAGE:CXX>:-std=c++17>)

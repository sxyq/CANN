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
project(r31a_v019 LANGUAGES ASC CXX)

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

add_library(r31a_submission OBJECT compile_adapter.cpp)
add_library(r31a_device OBJECT device.cpp)
foreach(target r31a_submission r31a_device)
    target_include_directories(${target} PRIVATE ${R31A_INCLUDE_DIRS})
    target_compile_options(${target} PRIVATE ${R31A_COMPILE_OPTIONS})
    set_target_properties(${target} PROPERTIES POSITION_INDEPENDENT_CODE OFF)
endforeach()

foreach(probe probe_v016 probe_v019)
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

add_custom_target(full_link
    COMMAND "${LLD}" -m aicorelinux -Ttext=0
            "$<TARGET_OBJECTS:r31a_device>" -static
            -o "${CMAKE_CURRENT_BINARY_DIR}/R31A-V019-device.alink"
    COMMAND "${LLD}" -m aicorelinux -Ttext=0
            "$<TARGET_OBJECTS:r31a_submission>" -static
            -o "${CMAKE_CURRENT_BINARY_DIR}/R31A-V019-submission.alink"
    DEPENDS r31a_device r31a_submission
    VERBATIM)

cmake_minimum_required(VERSION 3.16)

find_package(ASC REQUIRED)

project(add_rms_norm_bias_custom LANGUAGES ASC CXX)

set(CMAKE_CXX_STANDARD 14)

if(DEFINED NPU_ARCH)
    set(SOC_ARCH ${NPU_ARCH})
else()
    set(SOC_ARCH "dav-2201")
endif()

add_executable(add_rms_norm_bias_custom main.asc)

target_link_libraries(add_rms_norm_bias_custom PRIVATE
    tiling_api
    register
    platform
    unified_dlog
    dl
    m
    graph_base
)

target_include_directories(add_rms_norm_bias_custom PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_options(add_rms_norm_bias_custom PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=${SOC_ARCH}>
)

# On this host BiSheng discovers an incomplete GCC 12 installation before the
# usable GCC 11 headers. Prefer the C++ headers bundled with the matching CANN
# toolchain, and apply them only to Ascend C compilation.
if(DEFINED ENV{ASCEND_HOME_PATH})
    # CANN9 stores HCC under tools/hcc; retain the older toolkit/toolchain
    # fallback for installations that use the legacy layout.
    set(CANN_HCC_ROOT "$ENV{ASCEND_HOME_PATH}/tools/hcc/aarch64-target-linux-gnu")
    if(NOT EXISTS "${CANN_HCC_ROOT}/include/c++/7.3.0/cstdint")
        set(CANN_HCC_ROOT "$ENV{ASCEND_HOME_PATH}/toolkit/toolchain/hcc/aarch64-target-linux-gnu")
    endif()
    set(CANN_HCC_CXX_INCLUDE "${CANN_HCC_ROOT}/include/c++/7.3.0")
    set(CANN_HCC_CXX_TARGET_INCLUDE "${CANN_HCC_CXX_INCLUDE}/aarch64-target-linux-gnu")
    set(CANN_HCC_CXX_SYSTEM_INCLUDE "${CANN_HCC_ROOT}/include")
    set(CANN_HCC_CXX_LIBRARY "${CANN_HCC_ROOT}/lib64")
    if(EXISTS "${CANN_HCC_CXX_INCLUDE}/cstdint" AND EXISTS "${CANN_HCC_CXX_TARGET_INCLUDE}/bits/c++config.h" AND EXISTS "${CANN_HCC_CXX_SYSTEM_INCLUDE}")
        target_compile_options(add_rms_norm_bias_custom PRIVATE
            $<$<COMPILE_LANGUAGE:ASC>:-isystem${CANN_HCC_CXX_INCLUDE}>
            $<$<COMPILE_LANGUAGE:ASC>:-isystem${CANN_HCC_CXX_TARGET_INCLUDE}>
            $<$<COMPILE_LANGUAGE:ASC>:-isystem${CANN_HCC_CXX_SYSTEM_INCLUDE}>
        )
        # The compiler also launches a separate host-side translation unit to
        # register the device binary.  That subprocess does not inherit
        # target include flags, so pass the bundled C++ search path through
        # the environment of the ASC compile command as well.
        set(CANN_HCC_CXX_INCLUDE_PATH
            "${CANN_HCC_CXX_INCLUDE}:${CANN_HCC_CXX_TARGET_INCLUDE}:${CANN_HCC_CXX_SYSTEM_INCLUDE}")
        set(_ASC_COMPILE_COMMAND "${CMAKE_ASC_COMPILE_OBJECT}")
        set(CMAKE_ASC_COMPILE_OBJECT
            "CPLUS_INCLUDE_PATH=${CANN_HCC_CXX_INCLUDE_PATH} ${_ASC_COMPILE_COMMAND}")
    endif()
    if(EXISTS "${CANN_HCC_CXX_LIBRARY}/libstdc++.so")
        target_link_directories(add_rms_norm_bias_custom PRIVATE "${CANN_HCC_CXX_LIBRARY}")
    endif()
endif()

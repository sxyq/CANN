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
    set(CANN_HCC_CXX_INCLUDE "$ENV{ASCEND_HOME_PATH}/tools/hcc/aarch64-target-linux-gnu/include/c++/7.3.0")
    set(CANN_HCC_CXX_TARGET_INCLUDE "${CANN_HCC_CXX_INCLUDE}/aarch64-target-linux-gnu")
    set(CANN_HCC_CXX_LIBRARY "$ENV{ASCEND_HOME_PATH}/tools/hcc/aarch64-target-linux-gnu/lib64")
    if(EXISTS "${CANN_HCC_CXX_INCLUDE}/cstdint" AND EXISTS "${CANN_HCC_CXX_TARGET_INCLUDE}/bits/c++config.h")
        target_compile_options(add_rms_norm_bias_custom PRIVATE
            $<$<COMPILE_LANGUAGE:ASC>:-isystem${CANN_HCC_CXX_INCLUDE}>
            $<$<COMPILE_LANGUAGE:ASC>:-isystem${CANN_HCC_CXX_TARGET_INCLUDE}>
        )
    endif()
    if(EXISTS "${CANN_HCC_CXX_LIBRARY}/libstdc++.so")
        target_link_directories(add_rms_norm_bias_custom PRIVATE "${CANN_HCC_CXX_LIBRARY}")
    endif()
endif()

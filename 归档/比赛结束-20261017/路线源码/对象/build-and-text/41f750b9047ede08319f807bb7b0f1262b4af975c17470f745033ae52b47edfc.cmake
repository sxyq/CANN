cmake_minimum_required(VERSION 3.16)

find_package(ASC REQUIRED)

project(full_r008_tile_cross_core_i001 LANGUAGES ASC CXX)

set(CMAKE_CXX_STANDARD 14)

if(DEFINED NPU_ARCH)
    set(SOC_ARCH ${NPU_ARCH})
else()
    set(SOC_ARCH "dav-2201")
endif()

set(HCC_TARGET_ROOT "$ENV{ASCEND_HOME_PATH}/tools/hcc/aarch64-target-linux-gnu")
set(HCC_LIB_DIR "${HCC_TARGET_ROOT}/lib64")
file(GLOB HCC_CXX_INCLUDE_DIRS "${HCC_TARGET_ROOT}/include/c++/*")
list(SORT HCC_CXX_INCLUDE_DIRS)
list(GET HCC_CXX_INCLUDE_DIRS 0 HCC_CXX_INCLUDE_DIR)

add_executable(full_r008_tile_cross_core_i001 main.asc)

target_link_libraries(full_r008_tile_cross_core_i001 PRIVATE
    tiling_api
    register
    platform
    unified_dlog
    dl
    m
    graph_base
)

target_include_directories(full_r008_tile_cross_core_i001 PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_directories(full_r008_tile_cross_core_i001 PRIVATE
    ${HCC_LIB_DIR}
)

target_compile_options(full_r008_tile_cross_core_i001 PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=${SOC_ARCH}>
    $<$<COMPILE_LANGUAGE:ASC>:-isystem${HCC_CXX_INCLUDE_DIR}>
    $<$<COMPILE_LANGUAGE:ASC>:-isystem${HCC_CXX_INCLUDE_DIR}/aarch64-target-linux-gnu>
    $<$<COMPILE_LANGUAGE:ASC>:-isystem${HCC_CXX_INCLUDE_DIR}/backward>
    $<$<COMPILE_LANGUAGE:ASC>:-isystem${HCC_TARGET_ROOT}/include>
)

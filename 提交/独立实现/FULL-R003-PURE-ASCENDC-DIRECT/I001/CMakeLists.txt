cmake_minimum_required(VERSION 3.16)

find_package(ASC REQUIRED)

project(full_r003_pure_ascendc_direct_i001 LANGUAGES ASC CXX)

set(CMAKE_CXX_STANDARD 14)

if(DEFINED NPU_ARCH)
    set(SOC_ARCH ${NPU_ARCH})
else()
    set(SOC_ARCH "dav-2201")
endif()

add_executable(full_r003_pure_ascendc_direct_i001 main.asc)

target_link_libraries(full_r003_pure_ascendc_direct_i001 PRIVATE
    tiling_api
    register
    platform
    unified_dlog
    dl
    m
    graph_base
)

target_include_directories(full_r003_pure_ascendc_direct_i001 PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_options(full_r003_pure_ascendc_direct_i001 PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=${SOC_ARCH}>
)

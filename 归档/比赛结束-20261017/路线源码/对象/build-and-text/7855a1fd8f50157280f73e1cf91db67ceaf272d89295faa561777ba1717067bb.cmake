cmake_minimum_required(VERSION 3.16)

set(TOOLKIT_ROOT "$ENV{ASCEND_HOME_PATH}")
if(NOT TOOLKIT_ROOT)
    set(TOOLKIT_ROOT "/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002")
endif()
set(ASCEND_ROOT "${TOOLKIT_ROOT}/aarch64-linux")
set(HCC_ROOT "${TOOLKIT_ROOT}/toolkit/toolchain/hcc/aarch64-target-linux-gnu")
set(ASC_CMAKE_ROOT "${ASCEND_ROOT}/tikcpp/ascendc_kernel_cmake")
set(CMAKE_PREFIX_PATH "${ASC_CMAKE_ROOT}" ${CMAKE_PREFIX_PATH})
set(CMAKE_CXX_COMPILER "${ASCEND_ROOT}/ccec_compiler/bin/bisheng" CACHE FILEPATH "CANN CXX compiler" FORCE)
set(CMAKE_ASC_COMPILER "${ASCEND_ROOT}/ccec_compiler/bin/bisheng" CACHE FILEPATH "CANN ASC compiler" FORCE)
set(CMAKE_ASC_LLD_LINKER "${ASCEND_ROOT}/ccec_compiler/bin/ld.lld" CACHE FILEPATH "CANN ASC linker" FORCE)
set(SOC_VERSION "Ascend910B3" CACHE STRING "Target SoC" FORCE)
set(CCE_AICORE_ARCH "dav-2201" CACHE STRING "Target NPU arch" FORCE)

find_package(ASC REQUIRED)
project(async_triple_x_compiler_managed_probe LANGUAGES ASC CXX)

set(PROBE_INCLUDES
    "${ASCEND_ROOT}/ascendc/include"
    "${ASCEND_ROOT}/ascendc/include/basic_api/impl"
    "${ASCEND_ROOT}/asc/include"
    "${ASCEND_ROOT}/asc/include/basic_api"
    "${ASCEND_ROOT}/asc/include/interface"
    "${HCC_ROOT}/include"
    "${HCC_ROOT}/include/c++/7.3.0"
    "${HCC_ROOT}/include/c++/7.3.0/aarch64-target-linux-gnu"
    "${HCC_ROOT}/include/c++/7.3.0/backward"
    "${ASCEND_ROOT}/include"
    "${ASCEND_ROOT}/asc"
    "${TOOLKIT_ROOT}/include"
    "${TOOLKIT_ROOT}/compiler/tikcpp/tikcfw"
)

function(add_route_correctness_probe target source)
    add_executable(${target} "${source}")
    set_source_files_properties("${source}" PROPERTIES LANGUAGE ASC)
    set_target_properties(${target} PROPERTIES LINKER_LANGUAGE ASC)
    target_include_directories(${target} PRIVATE ${PROBE_INCLUDES})
    target_link_directories(${target} PRIVATE "${ASCEND_ROOT}/lib64")
    target_link_libraries(${target} PRIVATE
        tiling_api register platform unified_dlog dl m graph_base
        ascendc_runtime ascendcl runtime error_manager profapi ge_common_base
        ascendalog mmpa ascend_dump c_sec
    )
    target_compile_options(${target} PRIVATE
        --cce-aicore-arch=dav-c220-vec
        --npu-arch=dav-2201
        --npu-soc=Ascend910B3
    )
    set_target_properties(${target} PROPERTIES
        BUILD_RPATH "${ASCEND_ROOT}/lib64"
    )
endfunction()

add_route_correctness_probe(async_triple_candidate_probe candidate_probe.asc)
add_route_correctness_probe(async_triple_parent_probe parent_probe.asc)

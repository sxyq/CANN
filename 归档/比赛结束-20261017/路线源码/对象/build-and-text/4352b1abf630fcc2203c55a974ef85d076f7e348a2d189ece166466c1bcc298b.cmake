cmake_minimum_required(VERSION 3.16)

find_package(ASC REQUIRED)
project(e001_add_rms_norm_bias LANGUAGES ASC CXX)

set(CMAKE_CXX_STANDARD 17)
set(ASCEND_HOME_PATH "/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002" CACHE PATH "CANN toolkit")
set(ENV{ASCEND_HOME_PATH} "${ASCEND_HOME_PATH}")
set(ACL_INCLUDE_DIR "${ASCEND_HOME_PATH}/aarch64-linux/include")
set(ASC_INCLUDE_DIR "${ASCEND_HOME_PATH}/aarch64-linux/asc/include")
set(HCC_CXX_INCLUDE "${ASCEND_HOME_PATH}/toolkit/toolchain/hcc/aarch64-target-linux-gnu/include/c++/7.3.0")
set(HCC_TARGET_INCLUDE "${HCC_CXX_INCLUDE}/aarch64-target-linux-gnu")
set(ENV{CPLUS_INCLUDE_PATH} "${HCC_CXX_INCLUDE}:${HCC_TARGET_INCLUDE}")
set(ACL_LIB_DIR "${ASCEND_HOME_PATH}/aarch64-linux/lib64")

add_executable(e001_tiling_probe op_host/e001_tiling_probe.cpp)
target_include_directories(e001_tiling_probe PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/op_kernel"
    "${CMAKE_CURRENT_SOURCE_DIR}/op_host"
    "${ACL_INCLUDE_DIR}"
    "${ASC_INCLUDE_DIR}"
    "${HCC_CXX_INCLUDE}"
    "${HCC_TARGET_INCLUDE}"
)
target_link_directories(e001_tiling_probe PRIVATE "${ACL_LIB_DIR}")
target_link_libraries(e001_tiling_probe PRIVATE
    tiling_api register platform unified_dlog dl m graph_base
)
target_compile_options(e001_tiling_probe PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=dav-2201>
)

add_library(e001_kernel OBJECT op_kernel/e001_kernel.asc)
target_include_directories(e001_kernel PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/op_kernel"
    "${ACL_INCLUDE_DIR}"
    "${ASC_INCLUDE_DIR}"
    "${HCC_CXX_INCLUDE}"
    "${HCC_TARGET_INCLUDE}"
)
target_compile_options(e001_kernel PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=dav-2201>
)
ascendc_compile_options(e001_kernel PRIVATE
    -forward-options-to-host-compiler
    -I${HCC_CXX_INCLUDE}
    -I${HCC_TARGET_INCLUDE}
)

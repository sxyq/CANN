cmake_minimum_required(VERSION 3.16)
set(ASCEND_CANN_PACKAGE_PATH "/usr/local/Ascend/ascend-toolkit/latest" CACHE PATH "CANN root")
set(ENV{ASCEND_HOME_PATH} "${ASCEND_CANN_PACKAGE_PATH}")
list(APPEND CMAKE_PREFIX_PATH
    "${ASCEND_CANN_PACKAGE_PATH}/compiler/tikcpp/ascendc_kernel_cmake")
find_package(ASC REQUIRED)
project(r31b_v011 LANGUAGES ASC CXX)

set(CMAKE_CXX_STANDARD 17)
set(R31B_HCC_ROOT "${ASCEND_CANN_PACKAGE_PATH}/toolkit/toolchain/hcc")

add_executable(r31b_submission_v011 compile_adapter_v011.asc)
target_include_directories(r31b_submission_v011 PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/include"
    "${R31B_HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0"
    "${R31B_HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0/aarch64-target-linux-gnu"
    "${R31B_HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0/backward")
target_compile_options(r31b_submission_v011 PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=dav-2201>)
target_link_directories(r31b_submission_v011 PRIVATE
    "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/lib64"
    /usr/lib/gcc/aarch64-linux-gnu/11
    /usr/lib/aarch64-linux-gnu)
target_link_libraries(r31b_submission_v011 PRIVATE
    tiling_api register platform unified_dlog dl m graph_base ascendcl ascendc_runtime)

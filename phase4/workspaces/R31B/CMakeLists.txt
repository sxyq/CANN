cmake_minimum_required(VERSION 3.16)

set(ASCEND_CANN_PACKAGE_PATH
    "/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002"
    CACHE PATH "CANN root")
set(ENV{ASCEND_HOME_PATH} "${ASCEND_CANN_PACKAGE_PATH}")
list(APPEND CMAKE_PREFIX_PATH
    "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/tikcpp/ascendc_kernel_cmake")

find_package(ASC REQUIRED)
project(n09_rms_reduction_topology LANGUAGES ASC CXX)

set(CMAKE_CXX_STANDARD 17)

set(ASCEND_INCLUDE_DIRS
    "${ASCEND_CANN_PACKAGE_PATH}/include"
    "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/include"
    "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/asc/include"
    "${ASCEND_CANN_PACKAGE_PATH}/compiler/tikcpp/tikcfw"
    "${ASCEND_CANN_PACKAGE_PATH}/toolkit/toolchain/hcc/aarch64-target-linux-gnu/include/c++/7.3.0"
    "${ASCEND_CANN_PACKAGE_PATH}/toolkit/toolchain/hcc/aarch64-target-linux-gnu/include/c++/7.3.0/aarch64-target-linux-gnu"
    "${ASCEND_CANN_PACKAGE_PATH}/toolkit/toolchain/hcc/aarch64-target-linux-gnu/include/c++/7.3.0/backward")

add_executable(r31b_submission_v011 compile_adapter_v011.asc)
target_include_directories(r31b_submission_v011 PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${ASCEND_INCLUDE_DIRS})
target_link_directories(r31b_submission_v011 PRIVATE
    "${ASCEND_CANN_PACKAGE_PATH}/lib64"
    "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/lib64"
    /usr/lib/gcc/aarch64-linux-gnu/11
    /usr/lib/aarch64-linux-gnu)
target_link_libraries(r31b_submission_v011 PRIVATE
    tiling_api
    register
    platform
    unified_dlog
    graph_base
    dl
    m)
target_compile_options(r31b_submission_v011 PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=dav-2201>)

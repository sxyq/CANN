cmake_minimum_required(VERSION 3.16)
set(ASCEND_CANN_PACKAGE_PATH "/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002"
    CACHE PATH "CANN root")
set(ENV{ASCEND_HOME_PATH} "${ASCEND_CANN_PACKAGE_PATH}")
set(CMAKE_PREFIX_PATH
    "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/tikcpp/ascendc_kernel_cmake"
    ${CMAKE_PREFIX_PATH})
find_package(ASC REQUIRED)
project(dtype_special_x_v001 LANGUAGES ASC CXX)

set(CMAKE_CXX_STANDARD 17)
set(SOC_VERSION Ascend910B3)
link_directories(/usr/lib/gcc/aarch64-linux-gnu/11 /usr/lib/aarch64-linux-gnu)
set(HCC_ROOT "${ASCEND_CANN_PACKAGE_PATH}/toolkit/toolchain/hcc")
set(ASC_KERNEL_OPTIONS
    "--cce-aicore-arch=dav-c220"
    "-I${HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0"
    "-I${HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0/aarch64-target-linux-gnu"
    "-I${HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0/backward")
string(JOIN " " ASC_KERNEL_OPTIONS_STR ${ASC_KERNEL_OPTIONS})
set(ASC_HOST_OPTIONS
    "-I${HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0"
    "-I${HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0/aarch64-target-linux-gnu"
    "-I${HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0/backward")
string(JOIN " " ASC_HOST_OPTIONS_STR ${ASC_HOST_OPTIONS})
set(CMAKE_ASC_COMPILE_OBJECT
    "<CMAKE_ASC_COMPILER> ${CMAKE_COMPILE_AS_ASC_FLAG} <DEFINES> <INCLUDES> ${ASC_KERNEL_OPTIONS_STR} -Xaicore-start -O3 -std=c++17 -Xaicore-end -Xhost-start ${ASC_HOST_OPTIONS_STR} -fPIC -O2 -Xhost-end <FLAGS> -o <OBJECT> -c -x asc <SOURCE>")

add_executable(dtype_special_x_v001 compile_adapter.asc)
target_compile_options(dtype_special_x_v001 PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=dav-2201>)
target_link_directories(dtype_special_x_v001 PRIVATE
    "${ASCEND_CANN_PACKAGE_PATH}/lib64"
    "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/lib64"
    /usr/lib/gcc/aarch64-linux-gnu/11
    /usr/lib/aarch64-linux-gnu)
target_link_libraries(dtype_special_x_v001 PRIVATE
    ascendcl tiling_api register platform unified_dlog dl m graph_base)

add_executable(dtype_special_x_v001_npu_correctness npu_correctness.cpp)
target_include_directories(dtype_special_x_v001_npu_correctness PRIVATE
    "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/include"
    "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/ascendc/include"
    "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/ascendc/include/basic_api/impl"
    "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/asc/include/basic_api"
    "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/asc/include"
    "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux"
    "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/asc"
    "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/asc/include/interface"
    /usr/include/c++/11
    /usr/include/aarch64-linux-gnu/c++/11
    /usr/include/c++/11/backward
    /usr/include/aarch64-linux-gnu)
set_source_files_properties(npu_correctness.cpp PROPERTIES LANGUAGE ASC)
target_compile_options(dtype_special_x_v001_npu_correctness PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=dav-2201>)
target_link_directories(dtype_special_x_v001_npu_correctness PRIVATE
    "${ASCEND_CANN_PACKAGE_PATH}/lib64"
    "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/lib64"
    /usr/lib/gcc/aarch64-linux-gnu/11
    /usr/lib/aarch64-linux-gnu)
target_link_libraries(dtype_special_x_v001_npu_correctness PRIVATE
    ascendcl tiling_api register platform unified_dlog dl m graph_base)

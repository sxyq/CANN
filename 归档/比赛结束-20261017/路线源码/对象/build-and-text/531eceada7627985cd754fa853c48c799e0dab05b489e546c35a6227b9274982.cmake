cmake_minimum_required(VERSION 3.16)
find_package(ASC REQUIRED)
project(scx_v001 LANGUAGES ASC)

set(CMAKE_CXX_STANDARD 17)
set(SOC_VERSION Ascend910B3)

set(SCX_HCC_ROOT "$ENV{ASCEND_HOME_PATH}/toolkit/toolchain/hcc")
set(SCX_ASC_KERNEL_OPTIONS
    "--cce-aicore-arch=dav-c220"
    "-I${SCX_HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0"
    "-I${SCX_HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0/aarch64-target-linux-gnu"
    "-I${SCX_HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0/backward")
string(JOIN " " SCX_ASC_KERNEL_OPTIONS_STR ${SCX_ASC_KERNEL_OPTIONS})
set(SCX_ASC_HOST_OPTIONS
    "-I${SCX_HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0"
    "-I${SCX_HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0/aarch64-target-linux-gnu"
    "-I${SCX_HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0/backward")
string(JOIN " " SCX_ASC_HOST_OPTIONS_STR ${SCX_ASC_HOST_OPTIONS})
set(CMAKE_ASC_COMPILE_OBJECT
    "<CMAKE_ASC_COMPILER> ${CMAKE_COMPILE_AS_ASC_FLAG} <DEFINES> <INCLUDES> ${SCX_ASC_KERNEL_OPTIONS_STR} -Xaicore-start -O3 -std=c++17 -Xaicore-end -Xhost-start ${SCX_ASC_HOST_OPTIONS_STR} -fPIC -O2 -Xhost-end <FLAGS> -o <OBJECT> -c -x asc <SOURCE>")

add_executable(scx_v001_probe npu_correctness.asc)
target_include_directories(scx_v001_probe PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
    ${ASCEND_HOME_PATH}/aarch64-linux/include
    /usr/include/c++/11
    /usr/include/aarch64-linux-gnu/c++/11)
target_link_directories(scx_v001_probe PRIVATE
    ${ASCEND_HOME_PATH}/lib64)
target_link_options(scx_v001_probe PRIVATE
    -L/usr/lib/gcc/aarch64-linux-gnu/11
    -L/usr/lib/aarch64-linux-gnu)
target_link_libraries(scx_v001_probe PRIVATE
    ascendcl tiling_api register platform unified_dlog dl m graph_base)
target_compile_options(scx_v001_probe PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=dav-2201>
    -std=c++17)

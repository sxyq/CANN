cmake_minimum_required(VERSION 3.16)
set(ASCEND_CANN_PACKAGE_PATH "/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002" CACHE PATH "CANN root")
set(ENV{ASCEND_HOME_PATH} "${ASCEND_CANN_PACKAGE_PATH}")
set(CMAKE_PREFIX_PATH
    "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/tikcpp/ascendc_kernel_cmake"
    ${CMAKE_PREFIX_PATH})
find_package(ASC REQUIRED)
project(hotloop_index_arith_v001 LANGUAGES ASC)

set(CMAKE_CXX_STANDARD 17)
set(SOC_VERSION Ascend910B3)
link_directories(
    /usr/lib/gcc/aarch64-linux-gnu/11
    /usr/lib/aarch64-linux-gnu)

set(HOTLOOP_HCC_ROOT "${ASCEND_CANN_PACKAGE_PATH}/toolkit/toolchain/hcc")
set(HOTLOOP_ASC_KERNEL_OPTIONS
    "--cce-aicore-arch=dav-c220"
    "-I${HOTLOOP_HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0"
    "-I${HOTLOOP_HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0/aarch64-target-linux-gnu"
    "-I${HOTLOOP_HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0/backward")
string(JOIN " " HOTLOOP_ASC_KERNEL_OPTIONS_STR ${HOTLOOP_ASC_KERNEL_OPTIONS})
set(HOTLOOP_ASC_HOST_OPTIONS
    "-I${HOTLOOP_HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0"
    "-I${HOTLOOP_HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0/aarch64-target-linux-gnu"
    "-I${HOTLOOP_HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0/backward")
string(JOIN " " HOTLOOP_ASC_HOST_OPTIONS_STR ${HOTLOOP_ASC_HOST_OPTIONS})
set(CMAKE_ASC_COMPILE_OBJECT
    "<CMAKE_ASC_COMPILER> ${CMAKE_COMPILE_AS_ASC_FLAG} <DEFINES> <INCLUDES> ${HOTLOOP_ASC_KERNEL_OPTIONS_STR} -Xaicore-start -O3 -std=c++17 -Xaicore-end -Xhost-start ${HOTLOOP_ASC_HOST_OPTIONS_STR} -fPIC -O2 -Xhost-end <FLAGS> -o <OBJECT> -c -x asc <SOURCE>")

add_executable(hotloop_index_arith_v001 compile_adapter_v001.asc)
target_include_directories(hotloop_index_arith_v001 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(hotloop_index_arith_v001 PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=dav-2201>)
target_link_libraries(hotloop_index_arith_v001 PRIVATE stdc++ m dl)

cmake_minimum_required(VERSION 3.16)

set(ASCEND_CANN_PACKAGE_PATH "/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002" CACHE PATH "CANN root")
set(ENV{ASCEND_HOME_PATH} "${ASCEND_CANN_PACKAGE_PATH}")
set(CMAKE_PREFIX_PATH
    "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/tikcpp/ascendc_kernel_cmake"
    ${CMAKE_PREFIX_PATH})
find_package(ASC REQUIRED)
project(shape_tiling_seg LANGUAGES ASC CXX)
set(CMAKE_CXX_STANDARD 17)
set(SOC_VERSION Ascend910B3)

set(HCC_TOOLCHAIN "${ASCEND_CANN_PACKAGE_PATH}/toolkit/toolchain/hcc")
set(STX_SEG_INCLUDES
    "-I/usr/include/c++/11"
    "-I/usr/include/aarch64-linux-gnu/c++/11"
    "-I${HCC_TOOLCHAIN}/aarch64-target-linux-gnu/include/c++/7.3.0"
    "-I${HCC_TOOLCHAIN}/aarch64-target-linux-gnu/include/c++/7.3.0/aarch64-target-linux-gnu"
    "-I${HCC_TOOLCHAIN}/aarch64-target-linux-gnu/include/c++/7.3.0/backward")
string(JOIN " " STX_SEG_INCLUDES_STR ${STX_SEG_INCLUDES})
set(CMAKE_ASC_COMPILE_OBJECT
    "<CMAKE_ASC_COMPILER> ${CMAKE_COMPILE_AS_ASC_FLAG} <DEFINES> <INCLUDES> ${STX_SEG_INCLUDES_STR} --cce-aicore-arch=dav-c220 -Xaicore-start -O3 -std=c++17 -Xaicore-end -Xhost-start ${STX_SEG_INCLUDES_STR} -fPIC -O2 -std=c++17 -Xhost-end <FLAGS> -o <OBJECT> -c -x asc <SOURCE>")

add_executable(seg_runner seg_pair.asc)
target_include_directories(seg_runner PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/..
    ${CMAKE_CURRENT_LIST_DIR}/../..
    ${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/include
    ${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/ascendc/include
    ${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/ascendc/include/basic_api/impl
    ${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/asc/include/basic_api
    ${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/asc/include
    ${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/asc
    ${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/asc/include/interface
    /usr/include/aarch64-linux-gnu)
target_compile_options(seg_runner PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=dav-2201>)
target_link_directories(seg_runner PRIVATE
    /usr/lib/aarch64-linux-gnu
    ${ASCEND_CANN_PACKAGE_PATH}/lib64
    ${HCC_TOOLCHAIN}/aarch64-target-linux-gnu/lib64
    ${HCC_TOOLCHAIN}/lib64
    /usr/lib/gcc/aarch64-linux-gnu/11
    /usr/lib/aarch64-linux-gnu)
target_link_libraries(seg_runner PRIVATE
    tiling_api register platform unified_dlog
    /usr/lib/aarch64-linux-gnu/libstdc++.so.6 dl m graph_base)

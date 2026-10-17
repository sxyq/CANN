cmake_minimum_required(VERSION 3.16)

set(ASCEND_CANN_PACKAGE_PATH "/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002" CACHE PATH "CANN root")
set(ENV{ASCEND_HOME_PATH} "${ASCEND_CANN_PACKAGE_PATH}")
set(CMAKE_PREFIX_PATH
    "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/tikcpp/ascendc_kernel_cmake"
    ${CMAKE_PREFIX_PATH})
find_package(ASC REQUIRED)
project(w4_r15_v001_paired_runner LANGUAGES ASC CXX)
set(CMAKE_CXX_STANDARD 17)
set(SOC_VERSION Ascend910B3)

set(HCC_TOOLCHAIN "${ASCEND_CANN_PACKAGE_PATH}/toolkit/toolchain/hcc")
set(R15_COMPILE_INCLUDES
    "-I/usr/include/c++/11"
    "-I/usr/include/aarch64-linux-gnu/c++/11"
    "-I${HCC_TOOLCHAIN}/aarch64-target-linux-gnu/include/c++/7.3.0"
    "-I${HCC_TOOLCHAIN}/aarch64-target-linux-gnu/include/c++/7.3.0/aarch64-target-linux-gnu"
    "-I${HCC_TOOLCHAIN}/aarch64-target-linux-gnu/include/c++/7.3.0/backward")
string(JOIN " " R15_COMPILE_INCLUDES_STR ${R15_COMPILE_INCLUDES})
set(CMAKE_ASC_COMPILE_OBJECT
    "<CMAKE_ASC_COMPILER> ${CMAKE_COMPILE_AS_ASC_FLAG} <DEFINES> <INCLUDES> ${R15_COMPILE_INCLUDES_STR} --cce-aicore-arch=dav-c220 -Xaicore-start -O3 -std=c++17 -Xaicore-end -Xhost-start ${R15_COMPILE_INCLUDES_STR} -fPIC -O2 -std=c++17 -Xhost-end <FLAGS> -o <OBJECT> -c -x asc <SOURCE>")

add_executable(r15_v001_paired_runner paired_runner.cpp paired_runner_r15.asc)
set_target_properties(r15_v001_paired_runner PROPERTIES LINKER_LANGUAGE ASC)
target_include_directories(r15_v001_paired_runner PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    /usr/include/c++/11
    /usr/include/aarch64-linux-gnu/c++/11
    ${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/include
    ${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/ascendc/include
    ${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/ascendc/include/basic_api/impl
    ${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/asc/include/basic_api
    ${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/asc/include
    ${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/asc
    ${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/asc/include/interface
    /usr/include/aarch64-linux-gnu)
target_compile_options(r15_v001_paired_runner PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=dav-2201>)
target_link_directories(r15_v001_paired_runner PRIVATE
    /usr/lib/aarch64-linux-gnu
    ${ASCEND_CANN_PACKAGE_PATH}/lib64
    )
target_link_libraries(r15_v001_paired_runner PRIVATE
    tiling_api register platform unified_dlog
    /usr/lib/aarch64-linux-gnu/libstdc++.so.6 dl m graph_base)

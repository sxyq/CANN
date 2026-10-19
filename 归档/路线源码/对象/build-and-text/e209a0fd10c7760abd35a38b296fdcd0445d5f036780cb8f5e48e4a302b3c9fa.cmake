cmake_minimum_required(VERSION 3.16)

set(ASCEND_CANN_PACKAGE_PATH "/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002" CACHE PATH "CANN root")
set(ENV{ASCEND_HOME_PATH} "${ASCEND_CANN_PACKAGE_PATH}")
set(CMAKE_PREFIX_PATH
    "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/tikcpp/ascendc_kernel_cmake"
    ${CMAKE_PREFIX_PATH})
find_package(ASC REQUIRED)
project(addr_v001_correctness LANGUAGES ASC CXX)
set(CMAKE_CXX_STANDARD 17)
set(SOC_VERSION Ascend910B3)

set(ADDR_HCC_ROOT "${ASCEND_CANN_PACKAGE_PATH}/toolkit/toolchain/hcc")
set(ADDR_CORRECTNESS_INCLUDE_FLAGS
    "-I/usr/include/c++/11"
    "-I/usr/include/aarch64-linux-gnu/c++/11"
    "-I${ADDR_HCC_ROOT}/aarch64-target-linux-gnu/include"
    "-I${ADDR_HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0"
    "-I${ADDR_HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0/aarch64-target-linux-gnu"
    "-I${ADDR_HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0/backward")
string(JOIN " " ADDR_CORRECTNESS_INCLUDE_FLAGS_STR ${ADDR_CORRECTNESS_INCLUDE_FLAGS})
set(CMAKE_ASC_COMPILE_OBJECT
    "<CMAKE_ASC_COMPILER> ${CMAKE_COMPILE_AS_ASC_FLAG} <DEFINES> <INCLUDES> ${ADDR_CORRECTNESS_INCLUDE_FLAGS_STR} --cce-aicore-arch=dav-c220 -Xaicore-start -O3 -std=c++17 -Xaicore-end -Xhost-start ${ADDR_CORRECTNESS_INCLUDE_FLAGS_STR} -fPIC -O2 -std=c++17 -Xhost-end <FLAGS> -o <OBJECT> -c -x asc <SOURCE>")

set(ADDR_CORRECTNESS_INCLUDE_ENV
    "/usr/include/c++/11:/usr/include/aarch64-linux-gnu/c++/11:${ADDR_HCC_ROOT}/aarch64-target-linux-gnu/include:${ADDR_HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0:${ADDR_HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0/aarch64-target-linux-gnu:${ADDR_HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0/backward")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/../submission.asc"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../线上结果/R31B/V011/submission.asc")

add_executable(addr_v001_correctness correctness_runner.asc)
target_include_directories(addr_v001_correctness PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../线上结果/R31B/V011
    ${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/include
    ${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/ascendc/include
    ${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/ascendc/include/basic_api/impl
    ${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/asc/include/basic_api
    ${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/asc/include
    ${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/asc
    ${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/asc/include/interface
    /usr/include/aarch64-linux-gnu)
target_compile_options(addr_v001_correctness PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=dav-2201>)
set_property(TARGET addr_v001_correctness PROPERTY RULE_LAUNCH_COMPILE
    "${CMAKE_COMMAND} -E env CPLUS_INCLUDE_PATH=${ADDR_CORRECTNESS_INCLUDE_ENV} CPLUSPLUS_INCLUDE_PATH=${ADDR_CORRECTNESS_INCLUDE_ENV}")
target_link_directories(addr_v001_correctness PRIVATE
    /usr/lib/aarch64-linux-gnu
    ${ASCEND_CANN_PACKAGE_PATH}/lib64
    ${ADDR_HCC_ROOT}/aarch64-target-linux-gnu/lib64
    ${ADDR_HCC_ROOT}/lib64
    /usr/lib/gcc/aarch64-linux-gnu/11
    /usr/lib/aarch64-linux-gnu)
target_link_libraries(addr_v001_correctness PRIVATE
    tiling_api register platform unified_dlog
    /usr/lib/aarch64-linux-gnu/libstdc++.so.6 dl m graph_base)

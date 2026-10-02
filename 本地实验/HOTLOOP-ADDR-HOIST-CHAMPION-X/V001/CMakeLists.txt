cmake_minimum_required(VERSION 3.16)
set(ASCEND_CANN_PACKAGE_PATH "/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002" CACHE PATH "CANN root")
set(ENV{ASCEND_HOME_PATH} "${ASCEND_CANN_PACKAGE_PATH}")
set(CMAKE_PREFIX_PATH
    "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/tikcpp/ascendc_kernel_cmake"
    ${CMAKE_PREFIX_PATH})
set(CMAKE_ASC_COMPILER
    "${ASCEND_CANN_PACKAGE_PATH}/compiler/ccec_compiler/bin/bisheng"
    CACHE FILEPATH "CANN ASC compiler" FORCE)
find_package(ASC REQUIRED)
project(addr_v001_h3 LANGUAGES ASC)

set(CMAKE_CXX_STANDARD 17)
set(SOC_VERSION Ascend910B3)
link_directories(
    /usr/lib/gcc/aarch64-linux-gnu/11
    /usr/lib/aarch64-linux-gnu)

set(ADDR_HCC_ROOT "${ASCEND_CANN_PACKAGE_PATH}/toolkit/toolchain/hcc")
set(ADDR_HCC_INCLUDE_FLAGS
    "-I${ADDR_HCC_ROOT}/aarch64-target-linux-gnu/include"
    "-I${ADDR_HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0"
    "-I${ADDR_HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0/aarch64-target-linux-gnu"
    "-I${ADDR_HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0/backward")
string(JOIN " " ADDR_HCC_INCLUDE_FLAGS_STR ${ADDR_HCC_INCLUDE_FLAGS})
set(CMAKE_ASC_COMPILE_OBJECT
    "<CMAKE_ASC_COMPILER> ${CMAKE_COMPILE_AS_ASC_FLAG} <DEFINES> <INCLUDES> ${ADDR_HCC_INCLUDE_FLAGS_STR} --cce-aicore-arch=dav-c220 -Xaicore-start -O3 -std=c++17 -Xaicore-end -Xhost-start ${ADDR_HCC_INCLUDE_FLAGS_STR} -fPIC -O2 -std=c++17 -Xhost-end <FLAGS> -o <OBJECT> -c -x asc <SOURCE>")

add_executable(addr_v001_h3 compile_adapter.asc)
target_include_directories(addr_v001_h3 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(addr_v001_h3 PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=dav-2201>)
target_link_libraries(addr_v001_h3 PRIVATE stdc++ m dl)

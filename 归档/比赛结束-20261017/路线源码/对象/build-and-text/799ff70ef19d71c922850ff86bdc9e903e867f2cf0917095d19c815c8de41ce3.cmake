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
project(addr_v002_h1 LANGUAGES ASC)

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

add_executable(addr_v002_h1 compile_adapter.asc)
target_include_directories(addr_v002_h1 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(addr_v002_h1 PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=dav-2201>)
# The ASC compiler emits a host registration/ASCPLUGIN translation unit and
# invokes a nested host compiler for it.  Pass the HCC C++ search path through
# the environment as well as the explicit outer command-line flags above.
set(ADDR_HCC_INCLUDE_ENV_VALUE
    "${ADDR_HCC_ROOT}/aarch64-target-linux-gnu/include:${ADDR_HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0:${ADDR_HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0/aarch64-target-linux-gnu:${ADDR_HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0/backward")
set(ADDR_CPLUS_INCLUDE_PATH
    "${ADDR_HCC_INCLUDE_ENV_VALUE}:$ENV{CPLUS_INCLUDE_PATH}")
set(ADDR_CPLUSPLUS_INCLUDE_PATH
    "${ADDR_HCC_INCLUDE_ENV_VALUE}:$ENV{CPLUSPLUS_INCLUDE_PATH}")
set_property(TARGET addr_v002_h1 PROPERTY RULE_LAUNCH_COMPILE
    "${CMAKE_COMMAND} -E env CPLUS_INCLUDE_PATH=${ADDR_CPLUS_INCLUDE_PATH} CPLUSPLUS_INCLUDE_PATH=${ADDR_CPLUSPLUS_INCLUDE_PATH}")
target_link_libraries(addr_v002_h1 PRIVATE stdc++ m dl)

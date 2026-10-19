cmake_minimum_required(VERSION 3.16)
find_package(ASC REQUIRED)
project(batch_timing LANGUAGES ASC)
set(CMAKE_CXX_STANDARD 17)
set(SOC_VERSION Ascend910B3)
set(HCC_ROOT "$ENV{ASCEND_HOME_PATH}/toolkit/toolchain/hcc")
set(HCC_INCLUDES
    "-I${HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0"
    "-I${HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0/aarch64-target-linux-gnu"
    "-I${HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0/backward")
string(JOIN " " HCC_INCLUDES_STR ${HCC_INCLUDES})
set(CMAKE_ASC_COMPILE_OBJECT
    "<CMAKE_ASC_COMPILER> ${CMAKE_COMPILE_AS_ASC_FLAG} <DEFINES> <INCLUDES> ${HCC_INCLUDES_STR} -Xaicore-start -O3 -std=c++17 -Xaicore-end -Xhost-start ${HCC_INCLUDES_STR} -fPIC -O2 -Xhost-end <FLAGS> -o <OBJECT> -c -x asc <SOURCE>")
add_executable(timing_probe timing_probe.asc)
target_include_directories(timing_probe PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${ASCEND_HOME_PATH}/aarch64-linux/include"
    /usr/include/c++/11
    /usr/include/aarch64-linux-gnu/c++/11)
target_compile_options(timing_probe PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=dav-2201>
    -std=c++17)
target_link_directories(timing_probe PRIVATE
    "${ASCEND_HOME_PATH}/lib64")
target_link_options(timing_probe PRIVATE
    -L/usr/lib/gcc/aarch64-linux-gnu/11
    -L/usr/lib/aarch64-linux-gnu)
target_link_libraries(timing_probe PRIVATE
    ascendcl tiling_api register platform unified_dlog dl m graph_base)

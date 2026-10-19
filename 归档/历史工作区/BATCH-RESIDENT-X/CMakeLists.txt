cmake_minimum_required(VERSION 3.16)
find_package(ASC REQUIRED)
project(batch_resident_x LANGUAGES ASC)

set(CMAKE_CXX_STANDARD 17)
set(SOC_VERSION Ascend910B3)

set(BRX_HCC_ROOT "$ENV{ASCEND_HOME_PATH}/toolkit/toolchain/hcc")
set(BRX_ASC_KERNEL_OPTIONS
    "--cce-aicore-arch=dav-c220"
    "-I${BRX_HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0"
    "-I${BRX_HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0/aarch64-target-linux-gnu"
    "-I${BRX_HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0/backward")
string(JOIN " " BRX_ASC_KERNEL_OPTIONS_STR ${BRX_ASC_KERNEL_OPTIONS})
set(BRX_ASC_HOST_OPTIONS
    "-I${BRX_HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0"
    "-I${BRX_HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0/aarch64-target-linux-gnu"
    "-I${BRX_HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0/backward")
string(JOIN " " BRX_ASC_HOST_OPTIONS_STR ${BRX_ASC_HOST_OPTIONS})
set(CMAKE_ASC_COMPILE_OBJECT
    "<CMAKE_ASC_COMPILER> ${CMAKE_COMPILE_AS_ASC_FLAG} <DEFINES> <INCLUDES> ${BRX_ASC_KERNEL_OPTIONS_STR} -Xaicore-start -O3 -std=c++17 -Xaicore-end -Xhost-start ${BRX_ASC_HOST_OPTIONS_STR} -fPIC -O2 -Xhost-end <FLAGS> -o <OBJECT> -c -x asc <SOURCE>")

# Device/submission compile target: exact source shape sent to the judge.
add_library(batch_resident_x_v001 OBJECT
    submission_v001.asc)

target_include_directories(batch_resident_x_v001 PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_options(batch_resident_x_v001 PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=dav-2201>)

# Submission compile/link adapter (run_kernel + legal dtypes entry present).
add_library(batch_resident_x_adapter OBJECT
    compile_adapter.cpp)

target_include_directories(batch_resident_x_adapter PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_options(batch_resident_x_adapter PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=dav-2201>)

# Linked NPU correctness probe (run_kernel + golden compare).
add_executable(batch_resident_x_probe npu_correctness.asc)
target_include_directories(batch_resident_x_probe PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${ASCEND_HOME_PATH}/aarch64-linux/include
    /usr/include/c++/11
    /usr/include/aarch64-linux-gnu/c++/11)
target_link_directories(batch_resident_x_probe PRIVATE
    ${ASCEND_HOME_PATH}/lib64)
target_link_options(batch_resident_x_probe PRIVATE
    -L/usr/lib/gcc/aarch64-linux-gnu/11
    -L/usr/lib/aarch64-linux-gnu)
target_link_libraries(batch_resident_x_probe PRIVATE
    ascendcl tiling_api register platform unified_dlog dl m graph_base)
target_compile_options(batch_resident_x_probe PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=dav-2201>
    -std=c++17)

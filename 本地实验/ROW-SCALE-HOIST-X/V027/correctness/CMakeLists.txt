cmake_minimum_required(VERSION 3.16)

find_package(ASC REQUIRED)
project(row_scale_hoist_v027_correctness LANGUAGES ASC CXX)

set(CMAKE_CXX_STANDARD 17)
set(ACL_INCLUDE_DIR "$ENV{ASCEND_HOME_PATH}/aarch64-linux/include")
set(ACL_LIB_DIR "$ENV{ASCEND_HOME_PATH}/lib64")
set(HCC_TOOLCHAIN "$ENV{ASCEND_HOME_PATH}/toolkit/toolchain/hcc")

add_executable(v027_correctness correctness_runner.asc)
target_include_directories(v027_correctness PRIVATE
    "${ACL_INCLUDE_DIR}"
    "${HCC_TOOLCHAIN}/aarch64-target-linux-gnu/include"
    "${HCC_TOOLCHAIN}/aarch64-target-linux-gnu/include/c++/7.3.0"
    "${HCC_TOOLCHAIN}/aarch64-target-linux-gnu/include/c++/7.3.0/aarch64-target-linux-gnu"
    "${HCC_TOOLCHAIN}/aarch64-target-linux-gnu/include/c++/7.3.0/backward"
)
target_link_options(v027_correctness PRIVATE
    -L/usr/lib/gcc/aarch64-linux-gnu/11
    -L/usr/lib/aarch64-linux-gnu
)
target_link_directories(v027_correctness PRIVATE "${ACL_LIB_DIR}")
target_link_libraries(v027_correctness PRIVATE
    ascendcl
    ascendc_runtime
    tiling_api
    register
    platform
    unified_dlog
    dl
    m
    graph_base
)
target_compile_options(v027_correctness PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=dav-2201>
)
ascendc_compile_options(v027_correctness PRIVATE
    -forward-options-to-host-compiler
    "-isystem${HCC_TOOLCHAIN}/aarch64-target-linux-gnu/include"
    "-isystem${HCC_TOOLCHAIN}/aarch64-target-linux-gnu/include/c++/7.3.0"
    "-isystem${HCC_TOOLCHAIN}/aarch64-target-linux-gnu/include/c++/7.3.0/aarch64-target-linux-gnu"
    "-isystem${HCC_TOOLCHAIN}/aarch64-target-linux-gnu/include/c++/7.3.0/backward"
)

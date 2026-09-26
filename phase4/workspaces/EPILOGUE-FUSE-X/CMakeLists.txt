cmake_minimum_required(VERSION 3.16)
find_package(ASC REQUIRED)
project(epilogue_fuse_x LANGUAGES ASC)

set(CMAKE_CXX_STANDARD 17)
set(SOC_VERSION Ascend910B3)

set(EFX_HCC_ROOT "$ENV{ASCEND_HOME_PATH}/toolkit/toolchain/hcc")
set(EFX_ASC_KERNEL_OPTIONS
    "--cce-aicore-arch=dav-c220"
    "-I${EFX_HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0"
    "-I${EFX_HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0/aarch64-target-linux-gnu"
    "-I${EFX_HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0/backward")
string(JOIN " " EFX_ASC_KERNEL_OPTIONS_STR ${EFX_ASC_KERNEL_OPTIONS})
set(EFX_ASC_HOST_OPTIONS
    "-I${EFX_HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0"
    "-I${EFX_HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0/aarch64-target-linux-gnu"
    "-I${EFX_HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0/backward")
string(JOIN " " EFX_ASC_HOST_OPTIONS_STR ${EFX_ASC_HOST_OPTIONS})
set(CMAKE_ASC_COMPILE_OBJECT
    "<CMAKE_ASC_COMPILER> ${CMAKE_COMPILE_AS_ASC_FLAG} <DEFINES> <INCLUDES> ${EFX_ASC_KERNEL_OPTIONS_STR} -Xaicore-start -O3 -std=c++17 -Xaicore-end -Xhost-start ${EFX_ASC_HOST_OPTIONS_STR} -fPIC -O2 -Xhost-end <FLAGS> -o <OBJECT> -c -x asc <SOURCE>")

add_executable(epilogue_fuse_x_link compile_adapter.asc)
target_include_directories(epilogue_fuse_x_link PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR})
target_link_directories(epilogue_fuse_x_link PRIVATE
    /usr/lib/gcc/aarch64-linux-gnu/11
    /usr/lib/aarch64-linux-gnu)
target_compile_options(epilogue_fuse_x_link PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=dav-2201>)
target_link_libraries(epilogue_fuse_x_link PRIVATE stdc++ m dl)

add_executable(epilogue_fuse_x_probe npu_correctness.asc)
target_include_directories(epilogue_fuse_x_probe PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${ASCEND_HOME_PATH}/aarch64-linux/include
    /usr/include/c++/11
    /usr/include/aarch64-linux-gnu/c++/11)
target_link_directories(epilogue_fuse_x_probe PRIVATE
    ${ASCEND_HOME_PATH}/lib64)
target_link_options(epilogue_fuse_x_probe PRIVATE
    -L/usr/lib/gcc/aarch64-linux-gnu/11
    -L/usr/lib/aarch64-linux-gnu)
target_link_libraries(epilogue_fuse_x_probe PRIVATE
    ascendcl tiling_api register platform unified_dlog dl m graph_base)
target_compile_options(epilogue_fuse_x_probe PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=dav-2201>
    -std=c++17)

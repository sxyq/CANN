cmake_minimum_required(VERSION 3.16)

find_package(ASC REQUIRED)
project(norm_affine_arith_x_v001_correctness LANGUAGES ASC CXX)

set(CMAKE_CXX_STANDARD 17)
set(SOC_VERSION Ascend910B3)
set(HCC_ROOT "$ENV{ASCEND_HOME_PATH}/toolkit/toolchain/hcc")
set(HCC_INCLUDES
    "-I${HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0"
    "-I${HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0/aarch64-target-linux-gnu"
    "-I${HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0/backward"
    "-I${HCC_ROOT}/aarch64-target-linux-gnu/include")
string(JOIN " " HCC_INCLUDES_STR ${HCC_INCLUDES})
set(CMAKE_ASC_COMPILE_OBJECT
    "<CMAKE_ASC_COMPILER> ${CMAKE_COMPILE_AS_ASC_FLAG} <DEFINES> <INCLUDES> ${HCC_INCLUDES_STR} --cce-aicore-arch=dav-c220 -Xaicore-start -O3 -std=c++17 -Xaicore-end -Xhost-start ${HCC_INCLUDES_STR} -fPIC -O2 -std=c++17 -Xhost-end <FLAGS> -o <OBJECT> -c -x asc <SOURCE>")

foreach(variant IN ITEMS parent v001)
    add_executable(norm_affine_${variant}_correctness runner_${variant}.asc)
    target_include_directories(norm_affine_${variant}_correctness PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}")
    target_compile_options(norm_affine_${variant}_correctness PRIVATE
        $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=dav-2201>)
    target_link_directories(norm_affine_${variant}_correctness PRIVATE
        "${HCC_ROOT}/aarch64-target-linux-gnu/lib64"
        "${HCC_ROOT}/lib64")
    target_link_libraries(norm_affine_${variant}_correctness PRIVATE
        tiling_api register platform unified_dlog stdc++ dl m graph_base)
endforeach()

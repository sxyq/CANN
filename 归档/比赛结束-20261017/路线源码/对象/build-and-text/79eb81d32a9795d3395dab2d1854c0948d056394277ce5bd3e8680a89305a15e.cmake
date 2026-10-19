cmake_minimum_required(VERSION 3.16)

find_package(ASC REQUIRED)
project(mix_a_v007_probe LANGUAGES ASC CXX)

set(CMAKE_CXX_STANDARD 17)
set(SOC_VERSION Ascend910B3)
set(SOC_ARCH dav-2201)
set(HCC_ROOT "$ENV{ASCEND_HOME_PATH}/toolkit/toolchain/hcc")
set(HCC_CXX_INCLUDE_DIRS
    "${HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0"
    "${HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0/aarch64-target-linux-gnu"
    "${HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0/backward"
    "${HCC_ROOT}/aarch64-target-linux-gnu/include")
set(HCC_CXX_INCLUDES)
foreach(include_dir IN LISTS HCC_CXX_INCLUDE_DIRS)
    list(APPEND HCC_CXX_INCLUDES "-I${include_dir}")
endforeach()
string(JOIN " " HCC_CXX_INCLUDES_STR ${HCC_CXX_INCLUDES})
string(JOIN ":" HCC_CXX_INCLUDE_PATHS ${HCC_CXX_INCLUDE_DIRS})
set(CMAKE_ASC_COMPILE_OBJECT
    "${CMAKE_COMMAND} -E env CPATH=${HCC_CXX_INCLUDE_PATHS} CPLUS_INCLUDE_PATH=${HCC_CXX_INCLUDE_PATHS} <CMAKE_ASC_COMPILER> ${CMAKE_COMPILE_AS_ASC_FLAG} <DEFINES> <INCLUDES> ${HCC_CXX_INCLUDES_STR} --cce-aicore-arch=dav-c220 -Xaicore-start -O3 -std=c++17 -Xaicore-end -Xhost-start ${HCC_CXX_INCLUDES_STR} -fPIC -O2 -std=c++17 -Xhost-end <FLAGS> -o <OBJECT> -c -x asc <SOURCE>")

set(ROUTE_WORKSPACE "${CMAKE_CURRENT_LIST_DIR}/../../../../workspaces/MIX-A")
if(NOT EXISTS "${ROUTE_WORKSPACE}/MIX-A-V003-submission.asc")
    set(ROUTE_WORKSPACE "${CMAKE_CURRENT_LIST_DIR}/..")
endif()
foreach(variant IN ITEMS v003 v007)
    add_executable(mix_a_${variant}_probe runner_${variant}.asc)
    target_include_directories(mix_a_${variant}_probe PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}"
        "${ROUTE_WORKSPACE}")
    target_compile_options(mix_a_${variant}_probe PRIVATE
        $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=${SOC_ARCH}>)
    target_link_directories(mix_a_${variant}_probe PRIVATE
        "${HCC_ROOT}/aarch64-target-linux-gnu/lib64"
        "${HCC_ROOT}/lib64")
    target_link_libraries(mix_a_${variant}_probe PRIVATE
        tiling_api register platform unified_dlog stdc++ dl m graph_base)
endforeach()

add_executable(mix_a_v007_unified_probe runner_unified.asc)
target_include_directories(mix_a_v007_unified_probe PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${ROUTE_WORKSPACE}")
target_compile_options(mix_a_v007_unified_probe PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=${SOC_ARCH}>)
target_link_directories(mix_a_v007_unified_probe PRIVATE
    "${HCC_ROOT}/aarch64-target-linux-gnu/lib64"
    "${HCC_ROOT}/lib64")
target_link_libraries(mix_a_v007_unified_probe PRIVATE
    tiling_api register platform unified_dlog stdc++ dl m graph_base)

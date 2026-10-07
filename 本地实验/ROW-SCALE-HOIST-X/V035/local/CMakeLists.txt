cmake_minimum_required(VERSION 3.16)

find_package(ASC REQUIRED)
project(row_scale_hoist_v035_local LANGUAGES ASC CXX)

set(CMAKE_CXX_STANDARD 17)
set(ACL_INCLUDE_DIR "$ENV{ASCEND_HOME_PATH}/aarch64-linux/include")
set(ACL_LIB_DIR "$ENV{ASCEND_HOME_PATH}/lib64")
set(HCC_TOOLCHAIN "$ENV{ASCEND_HOME_PATH}/toolkit/toolchain/hcc")
set(HCC_STANDARD_INCLUDE_PATH
    "${HCC_TOOLCHAIN}/aarch64-target-linux-gnu/include:${HCC_TOOLCHAIN}/aarch64-target-linux-gnu/include/c++/7.3.0:${HCC_TOOLCHAIN}/aarch64-target-linux-gnu/include/c++/7.3.0/aarch64-target-linux-gnu:${HCC_TOOLCHAIN}/aarch64-target-linux-gnu/include/c++/7.3.0/backward")
set(CMAKE_ASC_COMPILE_OBJECT
    "cmake -E env CPLUS_INCLUDE_PATH=${HCC_STANDARD_INCLUDE_PATH} C_INCLUDE_PATH=${HCC_STANDARD_INCLUDE_PATH} ${CMAKE_ASC_COMPILE_OBJECT}")

foreach(target_name IN ITEMS v035_local_parent v035_local_candidate)
    if(target_name STREQUAL "v035_local_parent")
        set(source_file parent_runner.asc)
    else()
        set(source_file candidate_runner.asc)
    endif()
    add_executable(${target_name} ${source_file})
    target_include_directories(${target_name} PRIVATE
        "${ACL_INCLUDE_DIR}"
        "${HCC_TOOLCHAIN}/aarch64-target-linux-gnu/include"
        "${HCC_TOOLCHAIN}/aarch64-target-linux-gnu/include/c++/7.3.0"
        "${HCC_TOOLCHAIN}/aarch64-target-linux-gnu/include/c++/7.3.0/aarch64-target-linux-gnu"
        "${HCC_TOOLCHAIN}/aarch64-target-linux-gnu/include/c++/7.3.0/backward"
    )
    target_link_options(${target_name} PRIVATE
        -L/usr/lib/gcc/aarch64-linux-gnu/11
        -L/usr/lib/aarch64-linux-gnu
    )
    target_link_directories(${target_name} PRIVATE "${ACL_LIB_DIR}")
    target_link_libraries(${target_name} PRIVATE
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
    target_compile_options(${target_name} PRIVATE
        $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=dav-2201>
    )
    ascendc_compile_options(${target_name} PRIVATE
        -forward-options-to-host-compiler
        "-isystem${HCC_TOOLCHAIN}/aarch64-target-linux-gnu/include"
        "-isystem${HCC_TOOLCHAIN}/aarch64-target-linux-gnu/include/c++/7.3.0"
        "-isystem${HCC_TOOLCHAIN}/aarch64-target-linux-gnu/include/c++/7.3.0/aarch64-target-linux-gnu"
        "-isystem${HCC_TOOLCHAIN}/aarch64-target-linux-gnu/include/c++/7.3.0/backward"
    )
endforeach()

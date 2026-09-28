cmake_minimum_required(VERSION 3.16)

set(CMAKE_ASC_ARCHITECTURES "dav-2201" CACHE STRING "Ascend C architecture")
set(SOC_VERSION "Ascend910B3" CACHE STRING "Ascend SoC")
find_package(ASC REQUIRED)

project(ext_ascend_x_v001 LANGUAGES ASC CXX)

set(EXT_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}
    $ENV{ASCEND_HOME_PATH}/aarch64-linux/include
    $ENV{ASCEND_HOME_PATH}/aarch64-linux/asc/include
    /usr/include/c++/11
    /usr/include/aarch64-linux-gnu/c++/11
    /usr/include
    /usr/include/aarch64-linux-gnu)

add_library(ext_device OBJECT device_wrap.asc)
target_include_directories(ext_device PRIVATE ${EXT_INCLUDE_DIRS})
set_property(TARGET ext_device PROPERTY RULE_LAUNCH_COMPILE
    "${CMAKE_COMMAND} -E env CPLUS_INCLUDE_PATH=/usr/include/c++/11:/usr/include/aarch64-linux-gnu/c++/11:/usr/include")
target_compile_options(ext_device PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=dav-2201>
    $<$<COMPILE_LANGUAGE:ASC>:--npu-soc=Ascend910B3>)

add_library(ext_submission OBJECT submission_wrap.asc)
target_include_directories(ext_submission PRIVATE ${EXT_INCLUDE_DIRS})
set_property(TARGET ext_submission PROPERTY RULE_LAUNCH_COMPILE
    "${CMAKE_COMMAND} -E env CPLUS_INCLUDE_PATH=/usr/include/c++/11:/usr/include/aarch64-linux-gnu/c++/11:/usr/include")
target_compile_options(ext_submission PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=dav-2201>
    $<$<COMPILE_LANGUAGE:ASC>:--npu-soc=Ascend910B3>)

add_executable(ext_correctness local_full.asc)
target_include_directories(ext_correctness PRIVATE ${EXT_INCLUDE_DIRS})
set_property(TARGET ext_correctness PROPERTY RULE_LAUNCH_COMPILE
    "${CMAKE_COMMAND} -E env CPLUS_INCLUDE_PATH=/usr/include/c++/11:/usr/include/aarch64-linux-gnu/c++/11:/usr/include")
target_compile_options(ext_correctness PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=dav-2201>
    $<$<COMPILE_LANGUAGE:ASC>:--npu-soc=Ascend910B3>)
target_link_directories(ext_correctness PRIVATE
    $ENV{ASCEND_HOME_PATH}/aarch64-linux/lib64
    $ENV{ASCEND_HOME_PATH}/lib64
    /usr/lib/gcc/aarch64-linux-gnu/11
    /usr/lib/aarch64-linux-gnu)
target_link_libraries(ext_correctness PRIVATE
    ascendcl tiling_api register platform unified_dlog graph_base dl m)
set_target_properties(ext_correctness PROPERTIES LINKER_LANGUAGE ASC)

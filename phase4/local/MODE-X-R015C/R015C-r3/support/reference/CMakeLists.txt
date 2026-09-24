cmake_minimum_required(VERSION 3.16)
set(ASCEND_CMAKE_DIR "$ENV{ASCEND_HOME_PATH}/aarch64-linux/tikcpp/ascendc_kernel_cmake")
list(APPEND CMAKE_MODULE_PATH "${ASCEND_CMAKE_DIR}/ASC_CMake")
project(mode_x_r015c_reference LANGUAGES ASC CXX)

find_package(ASC REQUIRED)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(ASCEND_ROOT "$ENV{ASCEND_HOME_PATH}")
if(NOT ASCEND_ROOT)
    message(FATAL_ERROR "Source the CANN set_env.sh before configuring")
endif()

add_executable(row_copy_reference op_host/row_copy_host.asc)
target_include_directories(row_copy_reference PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/op_kernel"
    "${ASCEND_ROOT}/aarch64-linux/include"
    /usr/include/c++/11
    /usr/include/aarch64-linux-gnu/c++/11
    /usr/include/aarch64-linux-gnu)
target_link_directories(row_copy_reference PRIVATE "${ASCEND_ROOT}/lib64")
target_link_options(row_copy_reference PRIVATE
    -L/usr/lib/gcc/aarch64-linux-gnu/11
    -L/usr/lib/aarch64-linux-gnu)
target_link_libraries(row_copy_reference PRIVATE ascendcl tiling_api register platform unified_dlog dl m graph_base)
target_compile_options(row_copy_reference PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=dav-2201>)

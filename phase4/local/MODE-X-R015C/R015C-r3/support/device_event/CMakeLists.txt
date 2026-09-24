cmake_minimum_required(VERSION 3.16)
set(ASCEND_CMAKE_DIR "$ENV{ASCEND_HOME_PATH}/aarch64-linux/tikcpp/ascendc_kernel_cmake")
list(APPEND CMAKE_MODULE_PATH "${ASCEND_CMAKE_DIR}/ASC_CMake")
project(mode_x_r015c_event_probe LANGUAGES ASC CXX)

find_package(ASC REQUIRED)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(ASCEND_ROOT "$ENV{ASCEND_HOME_PATH}")
if(NOT ASCEND_ROOT)
    message(FATAL_ERROR "Source the CANN set_env.sh before configuring")
endif()
set(ROW_COPY_KERNEL_DIR "" CACHE PATH "Declared Route kernel source directory")
if(NOT ROW_COPY_KERNEL_DIR OR NOT EXISTS "${ROW_COPY_KERNEL_DIR}/row_copy_kernel.asc")
    message(FATAL_ERROR "Set ROW_COPY_KERNEL_DIR to the declared revision kernel directory")
endif()

add_executable(row_copy_event_probe op_host/row_copy_event_host.asc)
target_include_directories(row_copy_event_probe PRIVATE
    "${ROW_COPY_KERNEL_DIR}"
    "${ASCEND_ROOT}/aarch64-linux/include"
    /usr/include/c++/11
    /usr/include/aarch64-linux-gnu/c++/11
    /usr/include/aarch64-linux-gnu)
target_link_directories(row_copy_event_probe PRIVATE "${ASCEND_ROOT}/lib64")
target_link_options(row_copy_event_probe PRIVATE
    -L/usr/lib/gcc/aarch64-linux-gnu/11
    -L/usr/lib/aarch64-linux-gnu)
target_link_libraries(row_copy_event_probe PRIVATE ascendcl tiling_api register platform unified_dlog dl m graph_base)
target_compile_options(row_copy_event_probe PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=dav-2201>)

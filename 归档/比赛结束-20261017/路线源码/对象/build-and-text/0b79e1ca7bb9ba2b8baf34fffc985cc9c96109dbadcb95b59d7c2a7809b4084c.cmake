cmake_minimum_required(VERSION 3.16)
project(full_r005_large_tile_i001 LANGUAGES NONE)

if(NOT DEFINED ENV{ASCEND_HOME_PATH})
    message(FATAL_ERROR "ASCEND_HOME_PATH is required")
endif()

set(ASCEND_HOME_PATH "$ENV{ASCEND_HOME_PATH}")
set(BISHENG "$ENV{ASCEND_HOME_PATH}/bin/bisheng")
set(KERNEL_ARCH "dav-2201")
set(KERNEL_OBJECT "${CMAKE_CURRENT_BINARY_DIR}/kernel.o")

add_custom_command(
    OUTPUT "${KERNEL_OBJECT}"
    COMMAND "${BISHENG}"
            -x cce
            --cce-aicore-only
            --npu-arch=${KERNEL_ARCH}
            -std=c++17
            -I${ASCEND_HOME_PATH}/include
            -I${ASCEND_HOME_PATH}/aarch64-linux/asc/include
            -DINPUT_T=half
            -c "${CMAKE_CURRENT_SOURCE_DIR}/kernel.txt"
            -o "${KERNEL_OBJECT}"
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/kernel.txt"
    VERBATIM
)

add_custom_target(compile_kernel ALL DEPENDS "${KERNEL_OBJECT}")

cmake_minimum_required(VERSION 3.16)
project(full_r020_i001 NONE)

set(CANN_ROOT "$ENV{CANN_ROOT}" CACHE PATH "Ascend CANN installation root")
if(NOT CANN_ROOT)
    set(CANN_ROOT "/usr/local/Ascend/ascend-toolkit/latest")
endif()

set(BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}/device")
add_custom_target(full_r020_i001_compile ALL
    COMMAND "${CMAKE_COMMAND}" -E env
            CANN_ROOT=${CANN_ROOT}
            BUILD_DIR=${BUILD_DIR}
            "${CMAKE_CURRENT_SOURCE_DIR}/compile.sh"
    BYPRODUCTS "${BUILD_DIR}/kernel.o"
    VERBATIM)

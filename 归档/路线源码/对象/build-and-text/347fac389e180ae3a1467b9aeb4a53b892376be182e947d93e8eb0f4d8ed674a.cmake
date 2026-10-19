cmake_minimum_required(VERSION 3.16)
project(h001_add_rms_norm_bias LANGUAGES NONE)

set(SOC_VERSION "Ascend910B3" CACHE STRING "Ascend SoC")
set(CANN_ROOT "/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002" CACHE PATH "CANN toolkit")
set(BISHENG "${CANN_ROOT}/aarch64-linux/ccec_compiler/bin/bisheng" CACHE FILEPATH "bisheng compiler")
set(ASCENDC_ROOT "${CANN_ROOT}/aarch64-linux/ascendc")
set(ASCENDC_INCLUDE "${ASCENDC_ROOT}/include/basic_api")
set(ASCEND_INCLUDE "${CANN_ROOT}/aarch64-linux/include")
set(ASCEND_UTIL_INCLUDE "${CANN_ROOT}/aarch64-linux/asc")
set(CXX_INCLUDE "/usr/include/c++/11")
set(CXX_TARGET_INCLUDE "/usr/include/aarch64-linux-gnu/c++/11")

set(H001_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/src/add_rms_norm_bias_kernel.cpp")
set(H001_OBJECT "${CMAKE_CURRENT_BINARY_DIR}/add_rms_norm_bias_kernel.o")

add_custom_command(
    OUTPUT "${H001_OBJECT}"
    COMMAND "${BISHENG}" -c -x cce
            --cce-aicore-only
            --npu-soc=${SOC_VERSION}
            --npu-arch=dav-2201
            -O2 -std=c++17
            -isystem ${CXX_INCLUDE}
            -isystem ${CXX_TARGET_INCLUDE}
            -I${ASCEND_UTIL_INCLUDE}
            -I${ASCENDC_INCLUDE}
            -I${ASCENDC_ROOT}/include/basic_api/impl
            -I${ASCENDC_ROOT}/include/basic_api/interface
            -I${ASCEND_INCLUDE}
            "${H001_SOURCE}" -o "${H001_OBJECT}"
    DEPENDS "${H001_SOURCE}"
    VERBATIM)

add_custom_target(h001_kernel ALL DEPENDS "${H001_OBJECT}")

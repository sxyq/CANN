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
set(ASCEND_LIB64 "${CANN_ROOT}/aarch64-linux/lib64")

set(H001_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/src/add_rms_norm_bias_kernel.cpp")
set(H001_ADAPTER "${CMAKE_CURRENT_SOURCE_DIR}/src/compile_adapter.hpp")
set(H001_DEVICE_OBJ "${CMAKE_CURRENT_BINARY_DIR}/add_rms_norm_bias_device.o")
set(H001_SUBMISSION_OBJ "${CMAKE_CURRENT_BINARY_DIR}/add_rms_norm_bias_submission.o")
set(H001_LINK_SO "${CMAKE_CURRENT_BINARY_DIR}/libh001_add_rms_norm_bias.so")

set(H001_COMMON_FLAGS
    -x cce
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
)

# 1) Device compile: Vector kernel only, all dtypes instantiated.
add_custom_command(
    OUTPUT "${H001_DEVICE_OBJ}"
    COMMAND "${BISHENG}" -c ${H001_COMMON_FLAGS}
            --cce-aicore-only
            -DH001_DEVICE_ONLY
            "${H001_SOURCE}" -o "${H001_DEVICE_OBJ}"
    DEPENDS "${H001_SOURCE}"
    VERBATIM)

# 2) Submission compile: kernel + run_kernel with judge ABI stand-ins.
add_custom_command(
    OUTPUT "${H001_SUBMISSION_OBJ}"
    COMMAND "${BISHENG}" -c ${H001_COMMON_FLAGS}
            -fPIC
            -include "${H001_ADAPTER}"
            "${H001_SOURCE}" -o "${H001_SUBMISSION_OBJ}"
    DEPENDS "${H001_SOURCE}" "${H001_ADAPTER}"
    VERBATIM)

# 3) Full link: submission object + ASC default runtime set (shared).
# Mirrors ASC FindASC.cmake default libs so run_kernel launch symbols resolve.
add_custom_command(
    OUTPUT "${H001_LINK_SO}"
    COMMAND "${BISHENG}" -shared
            "${H001_SUBMISSION_OBJ}"
            -o "${H001_LINK_SO}"
            "${ASCEND_LIB64}/libascendc_runtime.a"
            "${ASCEND_LIB64}/libascendcl.so"
            "${ASCEND_LIB64}/libruntime.so"
            "${ASCEND_LIB64}/libregister.so"
            "${ASCEND_LIB64}/liberror_manager.so"
            "${ASCEND_LIB64}/libprofapi.so"
            "${ASCEND_LIB64}/libge_common_base.so"
            "${ASCEND_LIB64}/libascendalog.so"
            "${ASCEND_LIB64}/libmmpa.so"
            "${ASCEND_LIB64}/libascend_dump.so"
            "${ASCEND_LIB64}/libc_sec.so"
            -ldl
    DEPENDS "${H001_SUBMISSION_OBJ}"
    VERBATIM)

add_custom_target(h001_device DEPENDS "${H001_DEVICE_OBJ}")
add_custom_target(h001_submission DEPENDS "${H001_SUBMISSION_OBJ}")
add_custom_target(h001_link DEPENDS "${H001_LINK_SO}")
add_custom_target(h001_kernel ALL DEPENDS h001_device h001_submission h001_link)

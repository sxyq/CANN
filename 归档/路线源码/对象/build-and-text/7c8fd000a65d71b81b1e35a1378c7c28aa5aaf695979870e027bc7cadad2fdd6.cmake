cmake_minimum_required(VERSION 3.16)

find_package(ASC REQUIRED)

project(f001_add_rms_norm_bias LANGUAGES ASC CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(ACL_INCLUDE_DIR "$ENV{ASCEND_HOME_PATH}/aarch64-linux/include")
set(ACL_LIB_DIR "$ENV{ASCEND_HOME_PATH}/aarch64-linux/lib64")
set(HCC_CXX_INCLUDE "$ENV{ASCEND_HOME_PATH}/toolkit/toolchain/hcc/aarch64-target-linux-gnu/include/c++/7.3.0")
set(HCC_TARGET_INCLUDE "$ENV{ASCEND_HOME_PATH}/toolkit/toolchain/hcc/aarch64-target-linux-gnu/include/c++/7.3.0/aarch64-target-linux-gnu")

add_library(f001_add_rms_norm_bias_kernel OBJECT
    op_kernel/f001_add_rms_norm_bias_kernel.asc
)
target_compile_options(f001_add_rms_norm_bias_kernel PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=dav-2201>
)
target_include_directories(f001_add_rms_norm_bias_kernel PRIVATE
    ${HCC_CXX_INCLUDE}
    ${HCC_TARGET_INCLUDE}
)
ascendc_compile_options(f001_add_rms_norm_bias_kernel PRIVATE
    -forward-options-to-host-compiler
    -isystem ${HCC_CXX_INCLUDE}
    -isystem ${HCC_TARGET_INCLUDE}
)

# Compile the exact self-contained source that is sent to CANNJudge as well.
add_library(f001_submission_source OBJECT kernel.asc)
target_include_directories(f001_submission_source PRIVATE
    ${ACL_INCLUDE_DIR}
    ${HCC_CXX_INCLUDE}
    ${HCC_TARGET_INCLUDE}
)
target_compile_options(f001_submission_source PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=dav-2201>
)
ascendc_compile_options(f001_submission_source PRIVATE
    -forward-options-to-host-compiler
    -isystem ${HCC_CXX_INCLUDE}
    -isystem ${HCC_TARGET_INCLUDE}
)

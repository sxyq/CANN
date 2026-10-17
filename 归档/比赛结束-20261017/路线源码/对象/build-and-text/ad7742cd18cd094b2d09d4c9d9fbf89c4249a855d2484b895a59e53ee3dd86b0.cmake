cmake_minimum_required(VERSION 3.16)
set(ASCEND_CANN_PACKAGE_PATH
    "/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002"
    CACHE PATH "CANN package root")
list(PREPEND CMAKE_PREFIX_PATH
    "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/tikcpp/ascendc_kernel_cmake")
find_package(ASC REQUIRED)

set(SOC_VERSION Ascend910B3)
set(SOC_ARCH dav-2201)
set(HCC_ROOT "${ASCEND_CANN_PACKAGE_PATH}/toolkit/toolchain/hcc")
project(tiny_v001_proxy_correctness LANGUAGES ASC CXX)
set(CMAKE_CXX_STANDARD 17)

set(ASCENDC_INCLUDES
    "${ASCEND_CANN_PACKAGE_PATH}/include"
    "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/include"
    "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/ascendc/include"
    "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/ascendc/include/basic_api/impl"
    "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/asc/include/basic_api"
    "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/asc/include"
    "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux"
    "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/asc"
    "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/asc/include/interface"
    "${CMAKE_CURRENT_SOURCE_DIR}/.."
    "${HCC_ROOT}/aarch64-target-linux-gnu/include"
    "${HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0"
    "${HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0/aarch64-target-linux-gnu"
    "${HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0/backward"
    "${CMAKE_CURRENT_SOURCE_DIR}")

add_executable(tiny_v001_proxy_correctness runner_tiny_proxy.asc)
target_include_directories(tiny_v001_proxy_correctness PRIVATE ${ASCENDC_INCLUDES})
target_compile_options(tiny_v001_proxy_correctness PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=${SOC_ARCH}>)
ascendc_compile_options(tiny_v001_proxy_correctness PRIVATE
    -forward-options-to-host-compiler
    -std=c++17
    "-I${HCC_ROOT}/aarch64-target-linux-gnu/include"
    "-I${HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0"
    "-I${HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0/aarch64-target-linux-gnu"
    "-I${HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0/backward")
set_target_properties(tiny_v001_proxy_correctness PROPERTIES POSITION_INDEPENDENT_CODE OFF)

target_link_directories(tiny_v001_proxy_correctness BEFORE PRIVATE
    "/usr/lib/aarch64-linux-gnu")
target_link_directories(tiny_v001_proxy_correctness PRIVATE
    "${ASCEND_CANN_PACKAGE_PATH}/runtime/lib64"
    "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/lib64"
    "${HCC_ROOT}/aarch64-target-linux-gnu/lib64"
    "${HCC_ROOT}/lib64")
target_link_options(tiny_v001_proxy_correctness PRIVATE
    "-Wl,-rpath,/usr/lib/aarch64-linux-gnu"
    "-Wl,-rpath,${ASCEND_CANN_PACKAGE_PATH}/runtime/lib64"
    "-Wl,-rpath,${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/lib64"
    "-Wl,-rpath,${HCC_ROOT}/aarch64-target-linux-gnu/lib64"
    "-Wl,-rpath,${HCC_ROOT}/lib64")
target_link_libraries(tiny_v001_proxy_correctness PRIVATE
    ascendcl tiling_api register platform unified_dlog
    /usr/lib/aarch64-linux-gnu/libstdc++.so.6
    dl m graph_base)

set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/../submission.asc"
    "${CMAKE_CURRENT_SOURCE_DIR}/parent_submission.asc")
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/parent_submission.asc" TINY_PARENT_SOURCE_TEXT)
string(REPLACE "AddRmsNormBiasKernel" "TinyParentAddRmsNormBiasKernel"
    TINY_PARENT_SOURCE_TEXT "${TINY_PARENT_SOURCE_TEXT}")
string(REPLACE "add_rms_norm_bias_custom" "tiny_parent_add_rms_norm_bias_custom"
    TINY_PARENT_SOURCE_TEXT "${TINY_PARENT_SOURCE_TEXT}")
string(REPLACE "run_kernel" "run_parent_kernel"
    TINY_PARENT_SOURCE_TEXT "${TINY_PARENT_SOURCE_TEXT}")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/parent_timing_submission.asc"
    "${TINY_PARENT_SOURCE_TEXT}")

file(READ "${CMAKE_CURRENT_SOURCE_DIR}/../submission.asc" TINY_CANDIDATE_SOURCE_TEXT)
string(REPLACE "AddRmsNormBiasKernel" "TinyCandidateAddRmsNormBiasKernel"
    TINY_CANDIDATE_SOURCE_TEXT "${TINY_CANDIDATE_SOURCE_TEXT}")
string(REPLACE "add_rms_norm_bias_custom" "tiny_candidate_add_rms_norm_bias_custom"
    TINY_CANDIDATE_SOURCE_TEXT "${TINY_CANDIDATE_SOURCE_TEXT}")
string(REPLACE "run_kernel" "run_candidate_kernel"
    TINY_CANDIDATE_SOURCE_TEXT "${TINY_CANDIDATE_SOURCE_TEXT}")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/candidate_timing_submission.asc"
    "${TINY_CANDIDATE_SOURCE_TEXT}")

add_executable(tiny_v001_timing_harness runner_tiny_v001_timing.asc)
target_include_directories(tiny_v001_timing_harness PRIVATE
    ${ASCENDC_INCLUDES} "${CMAKE_CURRENT_BINARY_DIR}")
target_compile_options(tiny_v001_timing_harness PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=${SOC_ARCH}>)
ascendc_compile_options(tiny_v001_timing_harness PRIVATE
    -forward-options-to-host-compiler
    -std=c++17
    "-I${HCC_ROOT}/aarch64-target-linux-gnu/include"
    "-I${HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0"
    "-I${HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0/aarch64-target-linux-gnu"
    "-I${HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0/backward")
set_target_properties(tiny_v001_timing_harness PROPERTIES POSITION_INDEPENDENT_CODE OFF)
target_link_directories(tiny_v001_timing_harness BEFORE PRIVATE
    "/usr/lib/aarch64-linux-gnu")
target_link_directories(tiny_v001_timing_harness PRIVATE
    "${ASCEND_CANN_PACKAGE_PATH}/runtime/lib64"
    "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/lib64"
    "${HCC_ROOT}/aarch64-target-linux-gnu/lib64"
    "${HCC_ROOT}/lib64")
target_link_options(tiny_v001_timing_harness PRIVATE
    "-Wl,-rpath,/usr/lib/aarch64-linux-gnu"
    "-Wl,-rpath,${ASCEND_CANN_PACKAGE_PATH}/runtime/lib64"
    "-Wl,-rpath,${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/lib64"
    "-Wl,-rpath,${HCC_ROOT}/aarch64-target-linux-gnu/lib64"
    "-Wl,-rpath,${HCC_ROOT}/lib64")
target_link_libraries(tiny_v001_timing_harness PRIVATE
    ascendcl tiling_api register platform unified_dlog
    /usr/lib/aarch64-linux-gnu/libstdc++.so.6
    dl m graph_base)

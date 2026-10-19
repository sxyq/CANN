cmake_minimum_required(VERSION 3.16)
set(SOC_VERSION Ascend910B3)
set(SOC_ARCH dav-2201)
set(ASCEND_CANN_PACKAGE_PATH "/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002")
set(HCC_ROOT "${ASCEND_CANN_PACKAGE_PATH}/toolkit/toolchain/hcc")
set(BISHENG "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/ccec_compiler/bin/bisheng")
set(CMAKE_CXX_COMPILER "${BISHENG}" CACHE FILEPATH "Bisheng" FORCE)
project(tiny_v001_proxy_correctness LANGUAGES CXX)
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
    "${HCC_ROOT}/aarch64-target-linux-gnu/include"
    "${HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0"
    "${HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0/aarch64-target-linux-gnu"
    "${HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0/backward"
    "${CMAKE_CURRENT_SOURCE_DIR}")

add_executable(tiny_v001_proxy_correctness runner_tiny_proxy.cpp)
target_include_directories(tiny_v001_proxy_correctness PRIVATE ${ASCENDC_INCLUDES})
target_compile_options(tiny_v001_proxy_correctness PRIVATE
    "--cce-aicore-lang"
    "--cce-aicore-arch=dav-c220-vec"
    "--cce-aicore-only"
    "--cce-auto-sync"
    "--npu-arch=${SOC_ARCH}"
    "--npu-soc=${SOC_VERSION}"
    "-std=c++17")
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

cmake_minimum_required(VERSION 3.16)

project(dtype_lowp_fastpath_v001 LANGUAGES ASC CXX)
set(CMAKE_CXX_STANDARD 17)

set(ASCEND_HOME_PATH "/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002")
set(HCC_ROOT "${ASCEND_HOME_PATH}/toolkit/toolchain/hcc")
set(HCC_CXX "${HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0")
set(ASCEND_INCLUDE "${ASCEND_HOME_PATH}/aarch64-linux/include")
set(ASCENDC_INCLUDE "${ASCEND_HOME_PATH}/aarch64-linux/ascendc/include")
set(ASCENDC_IMPL "${ASCENDC_INCLUDE}/basic_api/impl")
set(ASC_INCLUDE "${ASCEND_HOME_PATH}/aarch64-linux/asc/include")
set(ASC_BASIC_INCLUDE "${ASC_INCLUDE}/basic_api")
set(ASC_INTERFACE_INCLUDE "${ASC_INCLUDE}/interface")

set(CMAKE_ASC_COMPILE_OBJECT "<CMAKE_ASC_COMPILER> <DEFINES> <INCLUDES> -I${ASCEND_INCLUDE} -I${ASCENDC_INCLUDE} -I${ASCENDC_IMPL} -I${ASC_BASIC_INCLUDE} -I${ASC_INCLUDE} -I${ASCEND_HOME_PATH}/aarch64-linux -I${ASCEND_HOME_PATH}/aarch64-linux/asc -I${ASC_INTERFACE_INCLUDE} -I${HCC_CXX} -I${HCC_CXX}/aarch64-target-linux-gnu -I${HCC_CXX}/backward --cce-aicore-arch=dav-c220 -Xaicore-start -O3 -std=c++17 -Xaicore-end -Xhost-start -I${HCC_CXX} -I${HCC_CXX}/aarch64-target-linux-gnu -I${HCC_CXX}/backward -fPIC -O2 -std=c++17 -Xhost-end <FLAGS> -o <OBJECT> -c -x asc <SOURCE>")

add_library(dtype_lowp_fastpath_v001_device OBJECT submission_shim.asc)
target_include_directories(dtype_lowp_fastpath_v001_device PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
target_compile_options(dtype_lowp_fastpath_v001_device PRIVATE --npu-arch=dav-2201)

add_library(dtype_lowp_fastpath_v001_submission OBJECT submission_shim.asc)
target_include_directories(dtype_lowp_fastpath_v001_submission PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
target_compile_options(dtype_lowp_fastpath_v001_submission PRIVATE --npu-arch=dav-2201)

add_executable(dtype_lowp_fastpath_v001_full_link main.asc)
target_include_directories(dtype_lowp_fastpath_v001_full_link PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
target_compile_options(dtype_lowp_fastpath_v001_full_link PRIVATE --npu-arch=dav-2201)
target_link_directories(dtype_lowp_fastpath_v001_full_link PRIVATE
    "${HCC_ROOT}/aarch64-target-linux-gnu/lib64"
    "${ASCEND_HOME_PATH}/lib64"
    "${ASCEND_HOME_PATH}/aarch64-linux/lib64")
target_link_options(dtype_lowp_fastpath_v001_full_link PRIVATE
    -L/usr/lib/gcc/aarch64-linux-gnu/11 -L/usr/lib/aarch64-linux-gnu)
target_link_libraries(dtype_lowp_fastpath_v001_full_link PRIVATE
    tiling_api register platform unified_dlog ascendcl dl m graph_base)

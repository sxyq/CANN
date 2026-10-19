cmake_minimum_required(VERSION 3.16)
project(async_triple_x_v001 LANGUAGES CXX)

set(TOOLKIT_ROOT "$ENV{ASCEND_HOME_PATH}")
set(ASCEND_ROOT "${TOOLKIT_ROOT}/aarch64-linux")
set(HCC_ROOT "${TOOLKIT_ROOT}/toolkit/toolchain/hcc/aarch64-target-linux-gnu")
set(CCEC_LINKER "${ASCEND_ROOT}/ccec_compiler/bin/ld.lld")
set(SOC_ARCH "dav-2201")
set(SOC_VERSION "Ascend910B3")
set(AICORE_ARCH "dav-c220-vec")

set(HCC_INCLUDE_FLAGS
    -isystem${HCC_ROOT}/include
    -isystem${HCC_ROOT}/include/c++/7.3.0
    -isystem${HCC_ROOT}/include/c++/7.3.0/aarch64-target-linux-gnu
    -isystem${HCC_ROOT}/include/c++/7.3.0/backward
)
set(ASC_FLAGS
    --cce-aicore-lang
    --cce-aicore-arch=${AICORE_ARCH}
    --cce-aicore-only
    --cce-auto-sync
    --npu-arch=${SOC_ARCH}
    --npu-soc=${SOC_VERSION}
    -std=c++17
    ${HCC_INCLUDE_FLAGS}
    -I${ASCEND_ROOT}/include
    -I${ASCEND_ROOT}/asc/include/basic_api
    -I${ASCEND_ROOT}/asc/include
    -I${ASCEND_ROOT}/asc/impl/basic_api
    -I${ASCEND_ROOT}/asc/impl/basic_api/dav_c220
    -I${ASCEND_ROOT}/asc/impl
    -I${TOOLKIT_ROOT}/include
    -I${TOOLKIT_ROOT}/compiler/tikcpp/tikcfw
)
set(DEVICE_OBJ ${CMAKE_CURRENT_BINARY_DIR}/device_wrapper.cpp.o)
set(DEVICE_ALINK ${CMAKE_CURRENT_BINARY_DIR}/device_wrapper.alink)
set(SUBMISSION_OBJ ${CMAKE_CURRENT_BINARY_DIR}/submission_wrapper.cpp.o)
set(SUBMISSION_ALINK ${CMAKE_CURRENT_BINARY_DIR}/submission_wrapper.alink)

add_custom_command(
    OUTPUT ${DEVICE_OBJ}
    COMMAND ${CMAKE_CXX_COMPILER} ${ASC_FLAGS} -c ${CMAKE_CURRENT_SOURCE_DIR}/device_wrapper.cpp -o ${DEVICE_OBJ}
    DEPENDS device_wrapper.cpp device_main.asc kernel.asc
    VERBATIM
)
add_custom_command(
    OUTPUT ${DEVICE_ALINK}
    COMMAND ${CCEC_LINKER} -m aicorelinux -Ttext=0 -static ${DEVICE_OBJ} -o ${DEVICE_ALINK}
    DEPENDS ${DEVICE_OBJ}
    VERBATIM
)
add_custom_target(async_triple_device DEPENDS ${DEVICE_ALINK})

add_custom_command(
    OUTPUT ${SUBMISSION_OBJ}
    COMMAND ${CMAKE_CXX_COMPILER} ${ASC_FLAGS} -c ${CMAKE_CURRENT_SOURCE_DIR}/submission_wrapper.cpp -o ${SUBMISSION_OBJ}
    DEPENDS submission_wrapper.cpp main.asc kernel.asc
    VERBATIM
)
add_custom_command(
    OUTPUT ${SUBMISSION_ALINK}
    COMMAND ${CCEC_LINKER} -m aicorelinux -Ttext=0 -static ${SUBMISSION_OBJ} -o ${SUBMISSION_ALINK}
    DEPENDS ${SUBMISSION_OBJ}
    VERBATIM
)
add_custom_target(async_triple_submission DEPENDS ${SUBMISSION_ALINK})

add_custom_target(async_triple_full_link
    DEPENDS async_triple_device async_triple_submission
)

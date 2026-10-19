cmake_minimum_required(VERSION 3.16)
set(SOC_VERSION Ascend910B3 CACHE STRING "soc")
set(CCE_AICORE_ARCH dav-2201 CACHE STRING "arch")
set(ASCEND_CANN_PACKAGE_PATH "/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002" CACHE PATH "cann")
set(ASCENDC_INCLUDE_DIR "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/include")
set(ASCENDC_INTERNAL_INCLUDE_DIR "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/ascendc/include")
set(ASCENDC_IMPL_INCLUDE_DIR "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/ascendc/include/basic_api/impl")
set(ASCENDC_PUBLIC_BASIC_INCLUDE_DIR "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/asc/include/basic_api")
set(ASCENDC_PUBLIC_INCLUDE_DIR "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/asc/include")
set(ASCENDC_ROOT_INCLUDE_DIR "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux")
set(ASCENDC_ASC_ROOT_DIR "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/asc")
set(ASCENDC_PUBLIC_INTERFACE_INCLUDE_DIR "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/asc/include/interface")
set(HCC_TOOLCHAIN "${ASCEND_CANN_PACKAGE_PATH}/toolkit/toolchain/hcc")
set(BISHENG "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/ccec_compiler/bin/bisheng")
set(CMAKE_CXX_COMPILER "${BISHENG}" CACHE FILEPATH "bisheng" FORCE)
project(repro LANGUAGES CXX)
add_compile_definitions(FRESH4_LOCAL_BUILD)

set(REPRO_INCLUDES
  "${ASCENDC_INCLUDE_DIR}" "${ASCENDC_INTERNAL_INCLUDE_DIR}" "${ASCENDC_IMPL_INCLUDE_DIR}"
  "${ASCENDC_ROOT_INCLUDE_DIR}" "${ASCENDC_ASC_ROOT_DIR}" "${ASCENDC_PUBLIC_INCLUDE_DIR}"
  "${ASCENDC_PUBLIC_BASIC_INCLUDE_DIR}" "${ASCENDC_PUBLIC_INTERFACE_INCLUDE_DIR}"
  "${HCC_TOOLCHAIN}/aarch64-target-linux-gnu/include"
  "${HCC_TOOLCHAIN}/aarch64-target-linux-gnu/include/c++/7.3.0"
  "${HCC_TOOLCHAIN}/aarch64-target-linux-gnu/include/c++/7.3.0/aarch64-target-linux-gnu"
  "${HCC_TOOLCHAIN}/aarch64-target-linux-gnu/include/c++/7.3.0/backward"
  "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(REPRO_FLAGS --cce-aicore-lang --cce-aicore-arch=dav-c220-vec --cce-aicore-only --cce-auto-sync --npu-arch=dav-2201 --npu-soc=Ascend910B3 -std=c++17)

# device: real compile of submission content
set(DEVICE_OBJ ${CMAKE_BINARY_DIR}/device.o)
add_custom_command(
  OUTPUT ${DEVICE_OBJ}
  COMMAND ${BISHENG} ${REPRO_FLAGS}
    -I${ASCENDC_INCLUDE_DIR} -I${ASCENDC_INTERNAL_INCLUDE_DIR} -I${ASCENDC_IMPL_INCLUDE_DIR}
    -I${ASCENDC_ROOT_INCLUDE_DIR} -I${ASCENDC_ASC_ROOT_DIR} -I${ASCENDC_PUBLIC_INCLUDE_DIR}
    -I${ASCENDC_PUBLIC_BASIC_INCLUDE_DIR} -I${ASCENDC_PUBLIC_INTERFACE_INCLUDE_DIR}
    -I${HCC_TOOLCHAIN}/aarch64-target-linux-gnu/include
    -I${HCC_TOOLCHAIN}/aarch64-target-linux-gnu/include/c++/7.3.0
    -I${HCC_TOOLCHAIN}/aarch64-target-linux-gnu/include/c++/7.3.0/aarch64-target-linux-gnu
    -I${HCC_TOOLCHAIN}/aarch64-target-linux-gnu/include/c++/7.3.0/backward
    -I${CMAKE_CURRENT_SOURCE_DIR}/src
    -c -x asc ${CMAKE_CURRENT_SOURCE_DIR}/src/submission.asc -o ${DEVICE_OBJ}
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/submission.asc
  COMMENT "device compile submission.asc"
  VERBATIM
)
add_custom_target(device DEPENDS ${DEVICE_OBJ})

function(repro_flags tgt)
  target_include_directories(${tgt} PRIVATE ${REPRO_INCLUDES})
  target_compile_options(${tgt} PRIVATE ${REPRO_FLAGS})
  set_target_properties(${tgt} PROPERTIES POSITION_INDEPENDENT_CODE OFF)
endfunction()

add_library(submission OBJECT src/adapter.cpp)
repro_flags(submission)

set(LLD "/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/tools/ccec_compiler/bin/ld.lld")
if(NOT EXISTS "${LLD}")
  set(LLD "/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/ccec_compiler/bin/ld.lld")
endif()

# Full link: separate aicorelinux links (historical R31A pattern), not merged (avoids g_tilingKey duplicate)
add_custom_target(full_link
  COMMAND bash ${CMAKE_CURRENT_SOURCE_DIR}/link_one.sh
          ${CMAKE_BINARY_DIR}/device.alink
          $<TARGET_OBJECTS:device>
  COMMAND bash ${CMAKE_CURRENT_SOURCE_DIR}/link_one.sh
          ${CMAKE_BINARY_DIR}/submission.alink
          $<TARGET_OBJECTS:submission>
  DEPENDS device submission
  VERBATIM
)

cmake_minimum_required(VERSION 3.16)
find_package(ASC REQUIRED)
project(wide_x_fresh4_unified_runner LANGUAGES ASC CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(ASCEND_HOME_PATH "$ENV{ASCEND_HOME_PATH}" CACHE PATH "CANN toolkit root")
if(NOT ASCEND_HOME_PATH)
    message(FATAL_ERROR "ASCEND_HOME_PATH must point to CANN 8.5.0.alpha002")
endif()

set(ACL_INCLUDE_DIR "${ASCEND_HOME_PATH}/aarch64-linux/include")
set(ACL_LIB_DIR "${ASCEND_HOME_PATH}/aarch64-linux/lib64")
set(ASCEND_DRIVER_LIB_DIR "/usr/local/Ascend/driver/lib64/driver")
set(ASCEND_DRIVER_COMMON_LIB_DIR "/usr/local/Ascend/driver/lib64/common")
set(ROUTE_ROOT "${CMAKE_CURRENT_LIST_DIR}/../..")
set(ABI_INCLUDE_DIR "${ROUTE_ROOT}/BUILD-FIX-001/parent-control")
set(PARENT_SOURCE "${ROUTE_ROOT}/BUILD-FIX-001/submission.asc")
set(CANDIDATE_SOURCE "${CMAKE_CURRENT_LIST_DIR}/../submission.asc")
set(PARENT_SOURCE_SHA256 "5d0ee01165e46a281cbb7d1605feba3605ad59883845400a063e97704f27f2be")
set(CANDIDATE_SOURCE_SHA256 "f7628795e6699288669dbff8963181e376e10741ba4d46c768af51cf09bf6895")

ascendc_library(wide_x_fresh4_parent SHARED "${PARENT_SOURCE}")
ascendc_library(wide_x_fresh4_candidate SHARED "${CANDIDATE_SOURCE}")

foreach(kernel_target IN ITEMS wide_x_fresh4_parent wide_x_fresh4_candidate)
    target_include_directories(${kernel_target} PRIVATE "${ABI_INCLUDE_DIR}" "${ACL_INCLUDE_DIR}")
    target_compile_definitions(${kernel_target} PRIVATE FRESH4_LOCAL_BUILD)
    target_compile_options(${kernel_target} PRIVATE $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=dav-2201>)
    target_link_directories(${kernel_target} PRIVATE "${ACL_LIB_DIR}" "${ASCEND_HOME_PATH}/lib64")
    target_link_options(${kernel_target} PRIVATE "-Wl,-Bsymbolic")
    set_target_properties(${kernel_target} PROPERTIES BUILD_RPATH "${ACL_LIB_DIR};${ASCEND_HOME_PATH}/lib64;${ASCEND_DRIVER_LIB_DIR};${ASCEND_DRIVER_COMMON_LIB_DIR};$ORIGIN")
endforeach()

# Keep the ASC kernel declaration and launch token identical for ASCPLUGIN metadata generation.
ascendc_compile_definitions(wide_x_fresh4_parent PRIVATE run_kernel=run_kernel_parent)
ascendc_compile_definitions(wide_x_fresh4_candidate PRIVATE run_kernel=run_kernel_candidate)

add_executable(wide_x_fresh4_unified_runner runner_main.cpp)
target_include_directories(wide_x_fresh4_unified_runner PRIVATE "${ABI_INCLUDE_DIR}" "${ACL_INCLUDE_DIR}")
target_compile_definitions(wide_x_fresh4_unified_runner PRIVATE
    WIDE_PARENT_SOURCE_SHA256="${PARENT_SOURCE_SHA256}"
    WIDE_CANDIDATE_SOURCE_SHA256="${CANDIDATE_SOURCE_SHA256}")
add_dependencies(wide_x_fresh4_unified_runner wide_x_fresh4_parent wide_x_fresh4_candidate)
target_link_libraries(wide_x_fresh4_unified_runner PRIVATE
    "$<TARGET_FILE:wide_x_fresh4_parent>"
    "$<TARGET_FILE:wide_x_fresh4_candidate>"
    "${ACL_LIB_DIR}/libascendcl.so"
    dl m)
set_target_properties(wide_x_fresh4_unified_runner PROPERTIES BUILD_RPATH "${ACL_LIB_DIR};${ASCEND_HOME_PATH}/lib64;${ASCEND_DRIVER_LIB_DIR};${ASCEND_DRIVER_COMMON_LIB_DIR};$ORIGIN")

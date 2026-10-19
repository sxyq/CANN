cmake_minimum_required(VERSION 3.16)
find_package(ASC REQUIRED)
project(multirow_panel_rms_runner LANGUAGES ASC CXX)

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
set(RUN_ROOT "${CMAKE_CURRENT_LIST_DIR}/..")
set(PARENT_SOURCE "${RUN_ROOT}/parent/submission.asc")
set(CANDIDATE_SOURCE "${RUN_ROOT}/candidate/submission.asc")
set(PARENT_WRAPPER "${CMAKE_CURRENT_LIST_DIR}/parent_kernel.asc")
set(CANDIDATE_WRAPPER "${CMAKE_CURRENT_LIST_DIR}/candidate_kernel.asc")

ascendc_library(multirow_panel_rms_parent SHARED "${PARENT_WRAPPER}")
ascendc_library(multirow_panel_rms_candidate SHARED "${CANDIDATE_WRAPPER}")

foreach(kernel_target IN ITEMS multirow_panel_rms_parent multirow_panel_rms_candidate)
    target_include_directories(${kernel_target} PRIVATE "${CMAKE_CURRENT_LIST_DIR}" "${ACL_INCLUDE_DIR}")
    target_compile_definitions(${kernel_target} PRIVATE R4_LOCAL_BUILD)
    target_compile_options(${kernel_target} PRIVATE $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=dav-2201>)
    target_link_directories(${kernel_target} PRIVATE "${ACL_LIB_DIR}" "${ASCEND_HOME_PATH}/lib64")
    target_link_options(${kernel_target} PRIVATE "-Wl,-Bsymbolic")
    set_target_properties(${kernel_target} PROPERTIES BUILD_RPATH "${ACL_LIB_DIR};${ASCEND_HOME_PATH}/lib64;${ASCEND_DRIVER_LIB_DIR};${ASCEND_DRIVER_COMMON_LIB_DIR};$ORIGIN")
endforeach()

ascendc_compile_definitions(multirow_panel_rms_parent PRIVATE run_kernel=run_kernel_parent)
ascendc_compile_definitions(multirow_panel_rms_candidate PRIVATE run_kernel=run_kernel_candidate)

add_executable(multirow_panel_rms_runner runner_main.cpp)
target_include_directories(multirow_panel_rms_runner PRIVATE "${CMAKE_CURRENT_LIST_DIR}" "${ACL_INCLUDE_DIR}")
target_compile_definitions(multirow_panel_rms_runner PRIVATE
    R4_PARENT_SOURCE_SHA256="$ENV{R4_PARENT_SOURCE_SHA256}"
    R4_CANDIDATE_SOURCE_SHA256="$ENV{R4_CANDIDATE_SOURCE_SHA256}")
add_dependencies(multirow_panel_rms_runner multirow_panel_rms_parent multirow_panel_rms_candidate)
target_link_libraries(multirow_panel_rms_runner PRIVATE
    "$<TARGET_FILE:multirow_panel_rms_parent>"
    "$<TARGET_FILE:multirow_panel_rms_candidate>"
    "${ACL_LIB_DIR}/libascendcl.so"
    dl m)
set_target_properties(multirow_panel_rms_runner PROPERTIES BUILD_RPATH "${ACL_LIB_DIR};${ASCEND_HOME_PATH}/lib64;${ASCEND_DRIVER_LIB_DIR};${ASCEND_DRIVER_COMMON_LIB_DIR};$ORIGIN")

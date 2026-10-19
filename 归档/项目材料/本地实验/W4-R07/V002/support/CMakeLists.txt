cmake_minimum_required(VERSION 3.16)
find_package(ASC REQUIRED)
project(w4r07_runner LANGUAGES ASC CXX)

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
set(PARENT_SOURCE "${RUN_ROOT}/Parent.asc")
set(CANDIDATE_SOURCE "${RUN_ROOT}/Candidate.asc")
set(PARENT_WRAPPER "${CMAKE_CURRENT_LIST_DIR}/parent_kernel.asc")
set(CANDIDATE_WRAPPER "${CMAKE_CURRENT_LIST_DIR}/candidate_kernel.asc")

ascendc_library(w4r07_parent SHARED "${PARENT_WRAPPER}")
ascendc_library(w4r07_candidate SHARED "${CANDIDATE_WRAPPER}")

foreach(kernel_target IN ITEMS w4r07_parent w4r07_candidate)
    target_include_directories(${kernel_target} PRIVATE "${CMAKE_CURRENT_LIST_DIR}" "${ACL_INCLUDE_DIR}")
    target_compile_options(${kernel_target} PRIVATE $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=dav-2201>)
    target_link_directories(${kernel_target} PRIVATE "${ACL_LIB_DIR}" "${ASCEND_HOME_PATH}/lib64")
    target_link_options(${kernel_target} PRIVATE "-Wl,-Bsymbolic")
    set_target_properties(${kernel_target} PROPERTIES BUILD_RPATH "${ACL_LIB_DIR};${ASCEND_HOME_PATH}/lib64;${ASCEND_DRIVER_LIB_DIR};${ASCEND_DRIVER_COMMON_LIB_DIR};$ORIGIN")
endforeach()

ascendc_compile_definitions(w4r07_parent PRIVATE run_kernel=run_kernel_parent)
ascendc_compile_definitions(w4r07_candidate PRIVATE run_kernel=run_kernel_candidate)

add_executable(w4r07_runner runner_main.cpp)
target_include_directories(w4r07_runner PRIVATE "${CMAKE_CURRENT_LIST_DIR}" "${ACL_INCLUDE_DIR}")
add_dependencies(w4r07_runner w4r07_parent w4r07_candidate)
target_link_libraries(w4r07_runner PRIVATE
    "$<TARGET_FILE:w4r07_parent>"
    "$<TARGET_FILE:w4r07_candidate>"
    "${ACL_LIB_DIR}/libascendcl.so"
    dl m)
set_target_properties(w4r07_runner PROPERTIES BUILD_RPATH "${ACL_LIB_DIR};${ASCEND_HOME_PATH}/lib64;${ASCEND_DRIVER_LIB_DIR};${ASCEND_DRIVER_COMMON_LIB_DIR};$ORIGIN")

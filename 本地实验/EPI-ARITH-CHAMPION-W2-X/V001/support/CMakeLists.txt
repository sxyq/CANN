cmake_minimum_required(VERSION 3.16)

if(DEFINED ENV{ASCEND_HOME_PATH})
    set(_ASC_CMAKE "$ENV{ASCEND_HOME_PATH}/aarch64-linux/tikcpp/ascendc_kernel_cmake")
    list(APPEND CMAKE_MODULE_PATH "${_ASC_CMAKE}/ASC_CMake" "${_ASC_CMAKE}")
    list(APPEND CMAKE_PREFIX_PATH "${_ASC_CMAKE}")
endif()

find_package(ASC REQUIRED)
project(epi_arith_v001_correctness LANGUAGES ASC CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT DEFINED ENV{ASCEND_HOME_PATH})
    message(FATAL_ERROR "ASCEND_HOME_PATH must name the installed CANN toolkit")
endif()

set(ASCEND_HOME_PATH "$ENV{ASCEND_HOME_PATH}" CACHE PATH "CANN toolkit root")
set(ACL_INCLUDE_DIR "${ASCEND_HOME_PATH}/aarch64-linux/include")
set(ACL_LIB_DIR "${ASCEND_HOME_PATH}/aarch64-linux/lib64")
set(ASCEND_DRIVER_LIB_DIR "/usr/local/Ascend/driver/lib64/driver")
set(ASCEND_DRIVER_COMMON_LIB_DIR "/usr/local/Ascend/driver/lib64/common")
set(SUPPORT_DIR "${CMAKE_CURRENT_LIST_DIR}")
set(ROUTE_V001_DIR "${SUPPORT_DIR}/..")
set(ABI_INCLUDE_DIR "${SUPPORT_DIR}")
set(PARENT_SOURCE "${SUPPORT_DIR}/runner_parent.asc")
set(CANDIDATE_SOURCE "${SUPPORT_DIR}/runner_candidate.asc")
set(PARENT_SHA256 "a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3")
set(CANDIDATE_SHA256 "9a28f5cd8703dc4ff5c46fba09f59a537132594e5a879a50a17349086bfe4d59")

ascendc_library(epi_arith_v001_parent SHARED "${PARENT_SOURCE}")
ascendc_library(epi_arith_v001_candidate SHARED "${CANDIDATE_SOURCE}")

foreach(kernel_target IN ITEMS epi_arith_v001_parent epi_arith_v001_candidate)
    target_include_directories(${kernel_target} PRIVATE "${ABI_INCLUDE_DIR}" "${ACL_INCLUDE_DIR}")
    target_compile_options(${kernel_target} PRIVATE
        $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=dav-2201>)
    target_link_directories(${kernel_target} PRIVATE "${ACL_LIB_DIR}" "${ASCEND_HOME_PATH}/lib64")
    target_link_options(${kernel_target} PRIVATE "-Wl,-Bsymbolic")
    set_target_properties(${kernel_target} PROPERTIES
        BUILD_RPATH "${ACL_LIB_DIR};${ASCEND_HOME_PATH}/lib64;${ASCEND_DRIVER_LIB_DIR};${ASCEND_DRIVER_COMMON_LIB_DIR};$ORIGIN")
endforeach()

ascendc_compile_definitions(epi_arith_v001_parent PRIVATE run_kernel=epi_run_kernel_parent)
ascendc_compile_definitions(epi_arith_v001_candidate PRIVATE run_kernel=epi_run_kernel_candidate)

add_executable(epi_arith_v001_correctness runner_correctness.cpp)
target_include_directories(epi_arith_v001_correctness PRIVATE "${ABI_INCLUDE_DIR}" "${ACL_INCLUDE_DIR}")
target_compile_definitions(epi_arith_v001_correctness PRIVATE
    EPI_PARENT_SOURCE_SHA256=\"${PARENT_SHA256}\"
    EPI_CANDIDATE_SOURCE_SHA256=\"${CANDIDATE_SHA256}\")
target_compile_options(epi_arith_v001_correctness PRIVATE -ffp-contract=off)
add_dependencies(epi_arith_v001_correctness epi_arith_v001_parent epi_arith_v001_candidate)
target_link_libraries(epi_arith_v001_correctness PRIVATE
    "$<TARGET_FILE:epi_arith_v001_parent>"
    "$<TARGET_FILE:epi_arith_v001_candidate>"
    "${ACL_LIB_DIR}/libascendcl.so"
    dl m)
set_target_properties(epi_arith_v001_correctness PROPERTIES
    BUILD_RPATH "${ACL_LIB_DIR};${ASCEND_HOME_PATH}/lib64;${ASCEND_DRIVER_LIB_DIR};${ASCEND_DRIVER_COMMON_LIB_DIR};$ORIGIN")

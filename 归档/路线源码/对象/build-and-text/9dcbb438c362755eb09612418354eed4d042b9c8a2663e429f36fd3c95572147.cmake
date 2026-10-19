cmake_minimum_required(VERSION 3.16)
set(ASCEND_CMAKE_DIR "$ENV{ASCEND_HOME_PATH}/aarch64-linux/tikcpp/ascendc_kernel_cmake")
list(APPEND CMAKE_MODULE_PATH "${ASCEND_CMAKE_DIR}/ASC_CMake")
project(mode_x_r015c_paired_runner LANGUAGES ASC CXX)

find_package(ASC REQUIRED)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(ASCEND_ROOT "$ENV{ASCEND_HOME_PATH}")
if(NOT ASCEND_ROOT)
    message(FATAL_ERROR "Source the CANN environment before configuring")
endif()

set(MODE_PARENT_DIR "" CACHE PATH "Route R015C-r3 source directory")
set(MODE_CANDIDATE_DIR "" CACHE PATH "Route R015C-r4 source directory")
foreach(source_dir IN ITEMS "${MODE_PARENT_DIR}" "${MODE_CANDIDATE_DIR}")
    if(NOT EXISTS "${source_dir}/submission.asc" OR NOT EXISTS "${source_dir}/row_copy_tiling.h")
        message(FATAL_ERROR "Each source directory must contain submission.asc and row_copy_tiling.h")
    endif()
endforeach()

set(MODE_PARENT_SOURCE "${MODE_PARENT_DIR}/submission.asc")
set(MODE_PARENT_TILING "${MODE_PARENT_DIR}/row_copy_tiling.h")
set(MODE_CANDIDATE_SOURCE "${MODE_CANDIDATE_DIR}/submission.asc")
set(MODE_CANDIDATE_TILING "${MODE_CANDIDATE_DIR}/row_copy_tiling.h")
set(MODE_EXPECTED_PARENT_SHA "e1786bec2673519f41887fdffa0e11751f515a81037d854c88ae7edc0e4af903")
set(MODE_EXPECTED_PARENT_TILING_SHA "5e4ad750f5c63da357324c0a829674aff81b7c0b655e8ff08072b1c289c66c41")
set(MODE_EXPECTED_CANDIDATE_SHA "9367db4ebb4edf6b7bf6cde97f4846f987e6ab230c2aeee44d942d5e881a1c74")
set(MODE_EXPECTED_CANDIDATE_TILING_SHA "0939ba8498426fcd77645d826fcc45a8eb65c78996ad0ce1da1a00d1ac01a250")

file(SHA256 "${MODE_PARENT_SOURCE}" MODE_PARENT_SOURCE_SHA)
file(SHA256 "${MODE_PARENT_TILING}" MODE_PARENT_TILING_SHA)
file(SHA256 "${MODE_CANDIDATE_SOURCE}" MODE_CANDIDATE_SOURCE_SHA)
file(SHA256 "${MODE_CANDIDATE_TILING}" MODE_CANDIDATE_TILING_SHA)
if(NOT MODE_PARENT_SOURCE_SHA STREQUAL MODE_EXPECTED_PARENT_SHA OR
   NOT MODE_PARENT_TILING_SHA STREQUAL MODE_EXPECTED_PARENT_TILING_SHA OR
   NOT MODE_CANDIDATE_SOURCE_SHA STREQUAL MODE_EXPECTED_CANDIDATE_SHA OR
   NOT MODE_CANDIDATE_TILING_SHA STREQUAL MODE_EXPECTED_CANDIDATE_TILING_SHA)
    message(FATAL_ERROR "Route source identity does not match R015C-r3 / R015C-r4")
endif()

file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/generated")
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/generated/parent")
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/generated/candidate")
configure_file(op_kernel/runner_source_paths.h.in
    "${CMAKE_CURRENT_BINARY_DIR}/generated/runner_source_paths.h" @ONLY)
configure_file("${MODE_PARENT_SOURCE}"
    "${CMAKE_CURRENT_BINARY_DIR}/generated/parent/submission.asc" COPYONLY)
configure_file("${MODE_PARENT_TILING}"
    "${CMAKE_CURRENT_BINARY_DIR}/generated/parent/row_copy_tiling.h" COPYONLY)
configure_file(op_kernel/parent_kernel.asc.in
    "${CMAKE_CURRENT_BINARY_DIR}/generated/parent/parent_kernel.asc" COPYONLY)
configure_file("${MODE_CANDIDATE_SOURCE}"
    "${CMAKE_CURRENT_BINARY_DIR}/generated/candidate/submission.asc" COPYONLY)
configure_file("${MODE_CANDIDATE_TILING}"
    "${CMAKE_CURRENT_BINARY_DIR}/generated/candidate/row_copy_tiling.h" COPYONLY)
configure_file(op_kernel/candidate_kernel.asc.in
    "${CMAKE_CURRENT_BINARY_DIR}/generated/candidate/candidate_kernel.asc" COPYONLY)

ascendc_library(r015c_parent_kernel SHARED
    "${CMAKE_CURRENT_BINARY_DIR}/generated/parent/parent_kernel.asc")
ascendc_library(r015c_candidate_kernel SHARED
    "${CMAKE_CURRENT_BINARY_DIR}/generated/candidate/candidate_kernel.asc")
target_include_directories(r015c_parent_kernel PRIVATE
    "${ASCEND_ROOT}/aarch64-linux/include")
target_include_directories(r015c_candidate_kernel PRIVATE
    "${ASCEND_ROOT}/aarch64-linux/include")
target_link_directories(r015c_parent_kernel PRIVATE
    /usr/lib/gcc/aarch64-linux-gnu/11
    /usr/lib/aarch64-linux-gnu)
target_link_directories(r015c_candidate_kernel PRIVATE
    /usr/lib/gcc/aarch64-linux-gnu/11
    /usr/lib/aarch64-linux-gnu)
target_compile_options(r015c_parent_kernel PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=dav-2201>)
target_compile_options(r015c_candidate_kernel PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=dav-2201>)

add_executable(r015c_pair_runner op_host/paired_runner.asc)
target_include_directories(r015c_pair_runner PRIVATE
    "${CMAKE_CURRENT_BINARY_DIR}/generated"
    "${ASCEND_ROOT}/aarch64-linux/include"
    /usr/include/c++/11
    /usr/include/aarch64-linux-gnu/c++/11
    /usr/include/aarch64-linux-gnu)
target_link_directories(r015c_pair_runner PRIVATE "${ASCEND_ROOT}/lib64")
target_link_options(r015c_pair_runner PRIVATE
    -L/usr/lib/gcc/aarch64-linux-gnu/11
    -L/usr/lib/aarch64-linux-gnu)
target_link_libraries(r015c_pair_runner PRIVATE
    r015c_parent_kernel r015c_candidate_kernel
    ascendcl tiling_api register platform unified_dlog dl m graph_base)
target_compile_options(r015c_pair_runner PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=dav-2201>)

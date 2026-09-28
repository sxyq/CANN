cmake_minimum_required(VERSION 3.16)
find_package(ASC REQUIRED)
project(shape_tiling_v001 LANGUAGES ASC CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(ASCEND_HOME_PATH "$ENV{ASCEND_HOME_PATH}" CACHE PATH "CANN toolkit root")
if(NOT ASCEND_HOME_PATH)
    set(ASCEND_HOME_PATH "/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002" CACHE PATH "CANN toolkit root")
endif()

set(ACL_INCLUDE_DIR "${ASCEND_HOME_PATH}/aarch64-linux/include")
set(ACL_LIB_DIR "${ASCEND_HOME_PATH}/aarch64-linux/lib64")
set(ASCEND_DRIVER_LIB_DIR "/usr/local/Ascend/driver/lib64/driver")
set(ASCEND_DRIVER_COMMON_LIB_DIR "/usr/local/Ascend/driver/lib64/common")
set(CANDIDATE_SOURCE "${CMAKE_CURRENT_LIST_DIR}/kernel_wrap.asc")
set(PARENT_SOURCE "${CMAKE_CURRENT_LIST_DIR}/kernel_wrap_parent.asc")

ascendc_library(shape_tiling_v001 SHARED "${CANDIDATE_SOURCE}")
target_include_directories(shape_tiling_v001 PRIVATE
    "${ACL_INCLUDE_DIR}" "${CMAKE_CURRENT_LIST_DIR}" "${CMAKE_CURRENT_LIST_DIR}/..")
target_compile_options(shape_tiling_v001 PRIVATE $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=dav-2201>)
target_link_directories(shape_tiling_v001 PRIVATE "${ACL_LIB_DIR}" "${ASCEND_HOME_PATH}/lib64")
target_link_options(shape_tiling_v001 PRIVATE "-Wl,-Bsymbolic")
set_target_properties(shape_tiling_v001 PROPERTIES
    BUILD_RPATH "${ACL_LIB_DIR};${ASCEND_HOME_PATH}/lib64;${ASCEND_DRIVER_LIB_DIR};${ASCEND_DRIVER_COMMON_LIB_DIR};$ORIGIN")

ascendc_library(shape_tiling_v001_parent SHARED "${PARENT_SOURCE}")
target_include_directories(shape_tiling_v001_parent PRIVATE
    "${ACL_INCLUDE_DIR}" "${CMAKE_CURRENT_LIST_DIR}" "${CMAKE_CURRENT_LIST_DIR}/..")
target_compile_options(shape_tiling_v001_parent PRIVATE $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=dav-2201>)
target_link_directories(shape_tiling_v001_parent PRIVATE "${ACL_LIB_DIR}" "${ASCEND_HOME_PATH}/lib64")
target_link_options(shape_tiling_v001_parent PRIVATE "-Wl,-Bsymbolic")
set_target_properties(shape_tiling_v001_parent PROPERTIES
    BUILD_RPATH "${ACL_LIB_DIR};${ASCEND_HOME_PATH}/lib64;${ASCEND_DRIVER_LIB_DIR};${ASCEND_DRIVER_COMMON_LIB_DIR};$ORIGIN")

add_executable(correctness_probe correctness_probe.cpp)
target_include_directories(correctness_probe PRIVATE "${ACL_INCLUDE_DIR}" "${CMAKE_CURRENT_LIST_DIR}")
target_compile_options(correctness_probe PRIVATE -std=c++17)
add_dependencies(correctness_probe shape_tiling_v001)
target_link_libraries(correctness_probe PRIVATE
    "$<TARGET_FILE:shape_tiling_v001>"
    "${ACL_LIB_DIR}/libascendcl.so"
    dl m)
set_target_properties(correctness_probe PROPERTIES
    BUILD_RPATH "${ACL_LIB_DIR};${ASCEND_HOME_PATH}/lib64;${ASCEND_DRIVER_LIB_DIR};${ASCEND_DRIVER_COMMON_LIB_DIR};$ORIGIN")

add_executable(correctness_probe_parent correctness_probe.cpp)
target_include_directories(correctness_probe_parent PRIVATE "${ACL_INCLUDE_DIR}" "${CMAKE_CURRENT_LIST_DIR}")
target_compile_options(correctness_probe_parent PRIVATE -std=c++17)
add_dependencies(correctness_probe_parent shape_tiling_v001_parent)
target_link_libraries(correctness_probe_parent PRIVATE
    "$<TARGET_FILE:shape_tiling_v001_parent>"
    "${ACL_LIB_DIR}/libascendcl.so"
    dl m)
set_target_properties(correctness_probe_parent PROPERTIES
    BUILD_RPATH "${ACL_LIB_DIR};${ASCEND_HOME_PATH}/lib64;${ASCEND_DRIVER_LIB_DIR};${ASCEND_DRIVER_COMMON_LIB_DIR};$ORIGIN")

cmake_minimum_required(VERSION 3.16)

set(ASCEND_HOME_PATH "$ENV{ASCEND_HOME_PATH}" CACHE PATH "CANN toolkit root")
if(NOT ASCEND_HOME_PATH)
    message(FATAL_ERROR "ASCEND_HOME_PATH must point to CANN 8.5.0.alpha002")
endif()
set(ENV{ASCEND_HOME_PATH} "${ASCEND_HOME_PATH}")
set(CMAKE_PREFIX_PATH
    "${ASCEND_HOME_PATH}/aarch64-linux/tikcpp/ascendc_kernel_cmake"
    ${CMAKE_PREFIX_PATH})

find_package(ASC REQUIRED)
project(w4r09_v002_paired_runner LANGUAGES ASC CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(HCC_ROOT "${ASCEND_HOME_PATH}/toolkit/toolchain/hcc")
set(HCC_CXX_INCLUDES
    "-I${HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0"
    "-I${HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0/aarch64-target-linux-gnu"
    "-I${HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0/backward")
string(JOIN " " HCC_CXX_INCLUDES_STR ${HCC_CXX_INCLUDES})
set(CMAKE_ASC_COMPILE_OBJECT
    "<CMAKE_ASC_COMPILER> ${CMAKE_COMPILE_AS_ASC_FLAG} <DEFINES> <INCLUDES> ${HCC_CXX_INCLUDES_STR} --cce-aicore-arch=dav-c220 -Xaicore-start -O3 -std=c++17 -Xaicore-end -Xhost-start ${HCC_CXX_INCLUDES_STR} -fPIC -O2 -Xhost-end <FLAGS> -o <OBJECT> -c -x asc <SOURCE>")

set(ACL_INCLUDE_DIR "${ASCEND_HOME_PATH}/aarch64-linux/include")
set(ACL_LIB_DIR "${ASCEND_HOME_PATH}/aarch64-linux/lib64")
set(ASCEND_DRIVER_LIB_DIR "/usr/local/Ascend/driver/lib64/driver")
set(ASCEND_DRIVER_COMMON_LIB_DIR "/usr/local/Ascend/driver/lib64/common")
set(HOST_CXX_LIB_DIR "/usr/lib/gcc/aarch64-linux-gnu/11")
set(HOST_SYSTEM_LIB_DIR "/usr/lib/aarch64-linux-gnu")

ascendc_library(w4r09_v002_parent SHARED "${CMAKE_CURRENT_LIST_DIR}/parent_adapter.asc")
ascendc_library(w4r09_v002_candidate SHARED "${CMAKE_CURRENT_LIST_DIR}/candidate_adapter.asc")

foreach(kernel_target IN ITEMS w4r09_v002_parent w4r09_v002_candidate)
    target_include_directories(${kernel_target} PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}"
        "${ACL_INCLUDE_DIR}")
    target_compile_options(${kernel_target} PRIVATE
        $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=dav-2201>)
    target_link_directories(${kernel_target} PRIVATE
        "${ACL_LIB_DIR}"
        "${ASCEND_HOME_PATH}/lib64"
        "${HOST_CXX_LIB_DIR}"
        "${HOST_SYSTEM_LIB_DIR}")
    target_link_options(${kernel_target} PRIVATE "-Wl,-Bsymbolic")
    set_target_properties(${kernel_target} PROPERTIES
        BUILD_RPATH "${ACL_LIB_DIR};${ASCEND_HOME_PATH}/lib64;${ASCEND_DRIVER_LIB_DIR};${ASCEND_DRIVER_COMMON_LIB_DIR};$ORIGIN")
endforeach()

ascendc_compile_definitions(w4r09_v002_parent PRIVATE run_kernel=run_kernel_parent)
ascendc_compile_definitions(w4r09_v002_candidate PRIVATE run_kernel=run_kernel_candidate)

add_executable(w4r09_v002_paired_runner runner_main.cpp)
target_include_directories(w4r09_v002_paired_runner PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}"
    "${ACL_INCLUDE_DIR}")
add_dependencies(w4r09_v002_paired_runner
    w4r09_v002_parent
    w4r09_v002_candidate)
target_link_libraries(w4r09_v002_paired_runner PRIVATE
    "$<TARGET_FILE:w4r09_v002_parent>"
    "$<TARGET_FILE:w4r09_v002_candidate>"
    "${ACL_LIB_DIR}/libascendcl.so"
    stdc++
    dl
    m)
set_target_properties(w4r09_v002_paired_runner PROPERTIES
    BUILD_RPATH "${ACL_LIB_DIR};${ASCEND_HOME_PATH}/lib64;${ASCEND_DRIVER_LIB_DIR};${ASCEND_DRIVER_COMMON_LIB_DIR};$ORIGIN")

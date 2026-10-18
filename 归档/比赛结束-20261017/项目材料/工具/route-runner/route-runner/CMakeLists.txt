cmake_minimum_required(VERSION 3.16)
set(ASCEND_HOME_PATH "$ENV{ASCEND_HOME_PATH}" CACHE PATH "CANN toolkit root")
if(NOT ASCEND_HOME_PATH)
    message(FATAL_ERROR "ASCEND_HOME_PATH must be set")
endif()
foreach(required_var IN ITEMS RUNNER_PARENT_SOURCE RUNNER_CANDIDATE_SOURCE
        RUNNER_PARENT_SHA256 RUNNER_CANDIDATE_SHA256 RUNNER_ASCENDC_ARCH)
    if(NOT DEFINED ENV{${required_var}} OR "$ENV{${required_var}}" STREQUAL "")
        message(FATAL_ERROR "${required_var} must be set by build_server3.sh")
    endif()
endforeach()
set(ENV{ASCEND_HOME_PATH} "${ASCEND_HOME_PATH}")
list(PREPEND CMAKE_PREFIX_PATH "${ASCEND_HOME_PATH}/aarch64-linux/tikcpp/ascendc_kernel_cmake")
find_package(ASC REQUIRED)
project(exact_source_pair_runner LANGUAGES ASC CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(ACL_INCLUDE_DIR "${ASCEND_HOME_PATH}/aarch64-linux/include")
set(ACL_LIB_DIR "${ASCEND_HOME_PATH}/aarch64-linux/lib64")
set(ASCEND_DRIVER_LIB_DIR "/usr/local/Ascend/driver/lib64/driver")
set(ASCEND_DRIVER_COMMON_LIB_DIR "/usr/local/Ascend/driver/lib64/common")
set(HCC_ROOT "${ASCEND_HOME_PATH}/toolkit/toolchain/hcc")
set(HCC_CXX_INCLUDES
    "-I${HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0"
    "-I${HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0/aarch64-target-linux-gnu"
    "-I${HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0/backward")
set(HCC_CXX_INCLUDE_DIRS
    "${HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0"
    "${HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0/aarch64-target-linux-gnu"
    "${HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0/backward")
string(JOIN " " HCC_CXX_INCLUDES_STR ${HCC_CXX_INCLUDES})
set(CMAKE_ASC_COMPILE_OBJECT
    "<CMAKE_ASC_COMPILER> ${CMAKE_COMPILE_AS_ASC_FLAG} <DEFINES> <INCLUDES> ${HCC_CXX_INCLUDES_STR} --cce-aicore-arch=$ENV{RUNNER_ASCENDC_ARCH} -Xaicore-start -O3 -std=c++17 -Xaicore-end -Xhost-start ${HCC_CXX_INCLUDES_STR} -fPIC -O2 -Xhost-end <FLAGS> -o <OBJECT> -c -x asc <SOURCE>")
set(PARENT_WRAPPER "${CMAKE_CURRENT_LIST_DIR}/parent_adapter.asc")
set(CANDIDATE_WRAPPER "${CMAKE_CURRENT_LIST_DIR}/candidate_adapter.asc")
ascendc_library(route_runner_parent SHARED "${PARENT_WRAPPER}")
ascendc_library(route_runner_candidate SHARED "${CANDIDATE_WRAPPER}")
foreach(target IN ITEMS route_runner_parent route_runner_candidate)
    target_include_directories(${target} PRIVATE "${CMAKE_CURRENT_LIST_DIR}" "${ACL_INCLUDE_DIR}" ${HCC_CXX_INCLUDE_DIRS})
    target_compile_options(${target} PRIVATE $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=$ENV{RUNNER_ASCENDC_ARCH}>)
    target_link_directories(${target} PRIVATE "${ACL_LIB_DIR}" "${ASCEND_HOME_PATH}/lib64")
    target_link_directories(${target} PRIVATE "/usr/lib/gcc/aarch64-linux-gnu/11" "/usr/lib/aarch64-linux-gnu")
    target_link_options(${target} PRIVATE "-Wl,-Bsymbolic")
    set_target_properties(${target} PROPERTIES BUILD_RPATH "${ACL_LIB_DIR};${ASCEND_HOME_PATH}/lib64;${ASCEND_DRIVER_LIB_DIR};${ASCEND_DRIVER_COMMON_LIB_DIR};$ORIGIN")
endforeach()
target_compile_definitions(route_runner_parent PRIVATE run_kernel=run_kernel_parent ROUTE_PARENT_SOURCE="$ENV{RUNNER_PARENT_SOURCE}")
target_compile_definitions(route_runner_candidate PRIVATE run_kernel=run_kernel_candidate ROUTE_CANDIDATE_SOURCE="$ENV{RUNNER_CANDIDATE_SOURCE}")
add_executable(route_runner runner_main.cpp)
target_include_directories(route_runner PRIVATE "${CMAKE_CURRENT_LIST_DIR}" "${ACL_INCLUDE_DIR}")
target_compile_definitions(route_runner PRIVATE
    RUNNER_PARENT_SOURCE_PATH="$ENV{RUNNER_PARENT_SOURCE}"
    RUNNER_CANDIDATE_SOURCE_PATH="$ENV{RUNNER_CANDIDATE_SOURCE}"
    RUNNER_PARENT_SOURCE_SHA256="$ENV{RUNNER_PARENT_SHA256}"
    RUNNER_CANDIDATE_SOURCE_SHA256="$ENV{RUNNER_CANDIDATE_SHA256}")
add_dependencies(route_runner route_runner_parent route_runner_candidate)
target_link_libraries(route_runner PRIVATE "$<TARGET_FILE:route_runner_parent>" "$<TARGET_FILE:route_runner_candidate>" "${ACL_LIB_DIR}/libascendcl.so" stdc++ dl m)
set_target_properties(route_runner PROPERTIES BUILD_RPATH "${ACL_LIB_DIR};${ASCEND_HOME_PATH}/lib64;${ASCEND_DRIVER_LIB_DIR};${ASCEND_DRIVER_COMMON_LIB_DIR};$ORIGIN")

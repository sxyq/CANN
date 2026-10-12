cmake_minimum_required(VERSION 3.16)

set(ASCEND_CANN_PACKAGE_PATH "/usr/local/Ascend/ascend-toolkit/latest" CACHE PATH "CANN root")
set(ENV{ASCEND_HOME_PATH} "${ASCEND_CANN_PACKAGE_PATH}")
set(CMAKE_PREFIX_PATH
    "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/tikcpp/ascendc_kernel_cmake"
    ${CMAKE_PREFIX_PATH})

find_package(ASC REQUIRED)
set(SOC_VERSION Ascend910B3)
project(r31b_v011 LANGUAGES ASC)

set(CMAKE_CXX_STANDARD 17)
set(R31B_HCC_CXX_INCLUDE
    "${ASCEND_CANN_PACKAGE_PATH}/toolkit/toolchain/hcc/aarch64-target-linux-gnu/include/c++/7.3.0:${ASCEND_CANN_PACKAGE_PATH}/toolkit/toolchain/hcc/aarch64-target-linux-gnu/include/c++/7.3.0/aarch64-target-linux-gnu:${ASCEND_CANN_PACKAGE_PATH}/toolkit/toolchain/hcc/aarch64-target-linux-gnu/include/c++/7.3.0/backward")
set(CMAKE_ASC_COMPILE_OBJECT
    "cmake -E env CPLUS_INCLUDE_PATH=${R31B_HCC_CXX_INCLUDE} ${CMAKE_ASC_COMPILE_OBJECT}")
link_directories(/usr/lib/gcc/aarch64-linux-gnu/11 /usr/lib/aarch64-linux-gnu)

add_executable(r31b_submission_v011 compile_adapter_v011.asc)
target_include_directories(r31b_submission_v011 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(r31b_submission_v011 PRIVATE stdc++ m dl)

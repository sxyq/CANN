cmake_minimum_required(VERSION 3.16)
set(ASCEND_CANN_PACKAGE_PATH "/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002" CACHE PATH "CANN root")
set(ENV{ASCEND_HOME_PATH} "${ASCEND_CANN_PACKAGE_PATH}")
set(CMAKE_PREFIX_PATH
    "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/tikcpp/ascendc_kernel_cmake"
    ${CMAKE_PREFIX_PATH})
find_package(ASC REQUIRED)
project(sync_barrier_elision_v079 LANGUAGES ASC)

set(CMAKE_CXX_STANDARD 17)
set(SOC_VERSION Ascend910B3)
link_directories(/usr/lib/gcc/aarch64-linux-gnu/11 /usr/lib/aarch64-linux-gnu)

set(HCC_ROOT "${ASCEND_CANN_PACKAGE_PATH}/toolkit/toolchain/hcc")
set(ASC_KERNEL_OPTIONS
    "--cce-aicore-arch=dav-c220"
    "-I${HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0"
    "-I${HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0/aarch64-target-linux-gnu"
    "-I${HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0/backward"
    "-I/usr/include/c++/11"
    "-I/usr/include/aarch64-linux-gnu/c++/11"
    "-I/usr/include/c++/11/backward")
string(JOIN " " ASC_KERNEL_OPTIONS_STR ${ASC_KERNEL_OPTIONS})
set(ASC_HOST_OPTIONS
    "-I${HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0"
    "-I${HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0/aarch64-target-linux-gnu"
    "-I${HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0/backward"
    "-I/usr/include/c++/11"
    "-I/usr/include/aarch64-linux-gnu/c++/11"
    "-I/usr/include/c++/11/backward")
string(JOIN " " ASC_HOST_OPTIONS_STR ${ASC_HOST_OPTIONS})
set(CMAKE_ASC_COMPILE_OBJECT
    "<CMAKE_ASC_COMPILER> ${CMAKE_COMPILE_AS_ASC_FLAG} <DEFINES> <INCLUDES> ${ASC_KERNEL_OPTIONS_STR} -Xaicore-start -O3 -std=c++17 -Xaicore-end -Xhost-start ${ASC_HOST_OPTIONS_STR} -fPIC -O2 -Xhost-end <FLAGS> -o <OBJECT> -c -x asc <SOURCE>")

add_executable(sync_barrier_elision_v079 src/compile_adapter.asc)
target_include_directories(sync_barrier_elision_v079 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_compile_options(sync_barrier_elision_v079 PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=dav-2201>)
target_link_libraries(sync_barrier_elision_v079 PRIVATE stdc++ m dl)
set(ASC_REGISTRATION_INCLUDE_PATH
    "${HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0:${HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0/aarch64-target-linux-gnu:${HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0/backward:${HCC_ROOT}/aarch64-target-linux-gnu/include:/usr/include/c++/11:/usr/include/aarch64-linux-gnu/c++/11:/usr/include")
set_property(TARGET sync_barrier_elision_v079 PROPERTY RULE_LAUNCH_COMPILE
    "${CMAKE_COMMAND} -E env CPATH=${ASC_REGISTRATION_INCLUDE_PATH} CPLUS_INCLUDE_PATH=${ASC_REGISTRATION_INCLUDE_PATH}")

set(CORRECTNESS_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/support
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/include
    ${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/asc/include
    /usr/include/c++/11
    /usr/include/aarch64-linux-gnu/c++/11)

add_executable(sync_barrier_elision_correctness
    support/correctness_runner.asc)
set_property(TARGET sync_barrier_elision_correctness PROPERTY RULE_LAUNCH_COMPILE
    "${CMAKE_COMMAND} -E env CPATH=${ASC_REGISTRATION_INCLUDE_PATH} CPLUS_INCLUDE_PATH=${ASC_REGISTRATION_INCLUDE_PATH}")
target_include_directories(sync_barrier_elision_correctness PRIVATE
    ${CORRECTNESS_INCLUDE_DIRS})
target_compile_options(sync_barrier_elision_correctness PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=dav-2201>
    -std=c++17)
target_link_directories(sync_barrier_elision_correctness PRIVATE
    ${ASCEND_CANN_PACKAGE_PATH}/lib64
    /usr/lib/gcc/aarch64-linux-gnu/11
    /usr/lib/aarch64-linux-gnu)
target_link_libraries(sync_barrier_elision_correctness PRIVATE
    ascendcl tiling_api register platform unified_dlog graph_base dl m)

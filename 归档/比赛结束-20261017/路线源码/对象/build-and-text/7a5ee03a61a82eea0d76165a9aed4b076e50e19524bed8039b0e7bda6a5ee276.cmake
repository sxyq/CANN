cmake_minimum_required(VERSION 3.16)
set(ASCEND_CANN_PACKAGE_PATH "/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002" CACHE PATH "CANN root")
set(ENV{ASCEND_HOME_PATH} "${ASCEND_CANN_PACKAGE_PATH}")
set(CMAKE_PREFIX_PATH
    "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/tikcpp/ascendc_kernel_cmake"
    ${CMAKE_PREFIX_PATH})
find_package(ASC REQUIRED)
set(HCC_TOOLCHAIN "${ASCEND_CANN_PACKAGE_PATH}/toolkit/toolchain/hcc")
set(BISHENG "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/ccec_compiler/bin/bisheng")
set(CMAKE_CXX_COMPILER "${BISHENG}" CACHE FILEPATH "bisheng" FORCE)
project(r31b_v016 LANGUAGES CXX)

function(r31b_v016_flags tgt)
    target_include_directories(${tgt} PRIVATE
        "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/include"
        "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/ascendc/include"
        "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/ascendc/include/basic_api/impl"
        "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/asc/include/basic_api"
        "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/asc/include"
        "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux"
        "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/asc"
        "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/asc/include/interface"
        "${HCC_TOOLCHAIN}/aarch64-target-linux-gnu/include"
        "${HCC_TOOLCHAIN}/aarch64-target-linux-gnu/include/c++/7.3.0"
        "${HCC_TOOLCHAIN}/aarch64-target-linux-gnu/include/c++/7.3.0/aarch64-target-linux-gnu"
        "${HCC_TOOLCHAIN}/aarch64-target-linux-gnu/include/c++/7.3.0/backward"
        ${CMAKE_CURRENT_SOURCE_DIR})
    target_compile_options(${tgt} PRIVATE
        "--cce-aicore-lang" "--cce-aicore-arch=dav-c220-vec"
        "--cce-aicore-only" "--cce-auto-sync"
        "--npu-arch=dav-2201" "--npu-soc=Ascend910B3" "-std=c++17")
    set_target_properties(${tgt} PROPERTIES POSITION_INDEPENDENT_CODE OFF)
endfunction()

set(CMAKE_CXX_STANDARD 17)
set(SOC_VERSION Ascend910B3)
link_directories(
    /usr/lib/gcc/aarch64-linux-gnu/11
    /usr/lib/aarch64-linux-gnu)

set(R31B_HCC_ROOT "${ASCEND_CANN_PACKAGE_PATH}/toolkit/toolchain/hcc")
set(R31B_ASC_KERNEL_OPTIONS
    "--cce-aicore-arch=dav-c220"
    "-I${R31B_HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0"
    "-I${R31B_HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0/aarch64-target-linux-gnu"
    "-I${R31B_HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0/backward")
string(JOIN " " R31B_ASC_KERNEL_OPTIONS_STR ${R31B_ASC_KERNEL_OPTIONS})
set(R31B_ASC_HOST_OPTIONS
    "-I${R31B_HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0"
    "-I${R31B_HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0/aarch64-target-linux-gnu"
    "-I${R31B_HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0/backward")
string(JOIN " " R31B_ASC_HOST_OPTIONS_STR ${R31B_ASC_HOST_OPTIONS})
set(CMAKE_ASC_COMPILE_OBJECT
    "<CMAKE_ASC_COMPILER> ${CMAKE_COMPILE_AS_ASC_FLAG} <DEFINES> <INCLUDES> ${R31B_ASC_KERNEL_OPTIONS_STR} -Xaicore-start -O3 -std=c++17 -Xaicore-end -Xhost-start ${R31B_ASC_HOST_OPTIONS_STR} -fPIC -O2 -Xhost-end <FLAGS> -o <OBJECT> -c -x asc <SOURCE>")

add_library(device OBJECT device.cpp)
r31b_v016_flags(device)
add_library(submission OBJECT adapter.cpp)
r31b_v016_flags(submission)

add_custom_target(full_link
    COMMAND bash ${CMAKE_CURRENT_SOURCE_DIR}/link_one.sh
            ${CMAKE_BINARY_DIR}/device.alink
            $<TARGET_OBJECTS:device>
    COMMAND bash ${CMAKE_CURRENT_SOURCE_DIR}/link_one.sh
            ${CMAKE_BINARY_DIR}/submission.alink
            $<TARGET_OBJECTS:submission>
    DEPENDS device submission
    VERBATIM)

function(r31b_v016_runtime_flags tgt)
    target_include_directories(${tgt} PRIVATE
        "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/include"
        "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/ascendc/include"
        "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/ascendc/include/basic_api/impl"
        "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/asc/include/basic_api"
        "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/asc/include"
        "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux"
        "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/asc"
        "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/asc/include/interface"
        "${HCC_TOOLCHAIN}/aarch64-target-linux-gnu/include"
        "${HCC_TOOLCHAIN}/aarch64-target-linux-gnu/include/c++/7.3.0"
        "${HCC_TOOLCHAIN}/aarch64-target-linux-gnu/include/c++/7.3.0/aarch64-target-linux-gnu"
        "${HCC_TOOLCHAIN}/aarch64-target-linux-gnu/include/c++/7.3.0/backward"
        ${CMAKE_CURRENT_SOURCE_DIR})
    target_compile_options(${tgt} PRIVATE
        "--cce-aicore-lang" "--cce-aicore-arch=dav-c220-vec"
        "--cce-auto-sync" "--npu-arch=dav-2201" "--npu-soc=Ascend910B3"
        "-std=c++17")
    target_link_directories(${tgt} PRIVATE "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/lib64")
    target_link_libraries(${tgt} PRIVATE ascendc_runtime ascendcl runtime register
        error_manager profapi ge_common_base ascendalog mmpa dl ascend_dump c_sec
        stdc++ m)
    set_target_properties(${tgt} PROPERTIES POSITION_INDEPENDENT_CODE OFF)
endfunction()

set(R31B_PARENT_SOURCE_DIR "/home/data4t2/lelinfeng/phase4-workspaces/R31B" CACHE PATH "V011 parent source")
add_executable(probe_v016 runtime_probe.cpp)
r31b_v016_runtime_flags(probe_v016)
add_executable(probe_v011 runtime_probe.cpp)
target_compile_definitions(probe_v011 PRIVATE R31B_PROBE_PARENT=1)
target_include_directories(probe_v011 PRIVATE "${R31B_PARENT_SOURCE_DIR}")
r31b_v016_runtime_flags(probe_v011)

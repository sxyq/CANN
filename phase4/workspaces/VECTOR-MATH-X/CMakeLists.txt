cmake_minimum_required(VERSION 3.16)
find_package(ASC REQUIRED)
project(vector_math_x LANGUAGES ASC)
set(CMAKE_CXX_STANDARD 17)
set(SOC_VERSION Ascend910B3)
set(VMX_HCC_ROOT "$ENV{ASCEND_HOME_PATH}/toolkit/toolchain/hcc")
set(VMX_ASC_KERNEL_OPTIONS
    "--cce-aicore-arch=dav-c220"
    "-I${VMX_HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0"
    "-I${VMX_HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0/aarch64-target-linux-gnu"
    "-I${VMX_HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0/backward")
string(JOIN " " VMX_ASC_KERNEL_OPTIONS_STR ${VMX_ASC_KERNEL_OPTIONS})
set(VMX_ASC_HOST_OPTIONS
    "-I${VMX_HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0"
    "-I${VMX_HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0/aarch64-target-linux-gnu"
    "-I${VMX_HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0/backward")
string(JOIN " " VMX_ASC_HOST_OPTIONS_STR ${VMX_ASC_HOST_OPTIONS})
set(CMAKE_ASC_COMPILE_OBJECT
    "<CMAKE_ASC_COMPILER> ${CMAKE_COMPILE_AS_ASC_FLAG} <DEFINES> <INCLUDES> ${VMX_ASC_KERNEL_OPTIONS_STR} -Xaicore-start -O3 -std=c++17 -Xaicore-end -Xhost-start ${VMX_ASC_HOST_OPTIONS_STR} -fPIC -O2 -Xhost-end <FLAGS> -o <OBJECT> -c -x asc <SOURCE>")

add_executable(vector_math_x_link compile_adapter.asc)
target_include_directories(vector_math_x_link PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_directories(vector_math_x_link PRIVATE /usr/lib/gcc/aarch64-linux-gnu/11 /usr/lib/aarch64-linux-gnu)
target_compile_options(vector_math_x_link PRIVATE $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=dav-2201>)
target_link_libraries(vector_math_x_link PRIVATE stdc++ m dl)

add_executable(vector_math_x_probe npu_correctness.asc)
target_include_directories(vector_math_x_probe PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${ASCEND_HOME_PATH}/aarch64-linux/include
    /usr/include/c++/11
    /usr/include/aarch64-linux-gnu/c++/11)
target_link_directories(vector_math_x_probe PRIVATE ${ASCEND_HOME_PATH}/lib64)
target_link_options(vector_math_x_probe PRIVATE -L/usr/lib/gcc/aarch64-linux-gnu/11 -L/usr/lib/aarch64-linux-gnu)
target_link_libraries(vector_math_x_probe PRIVATE ascendcl tiling_api register platform unified_dlog dl m graph_base)
target_compile_options(vector_math_x_probe PRIVATE $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=dav-2201> -std=c++17)

function(vmx_add_ref_probe target source)
    add_executable(${target} ${source})
    target_include_directories(${target} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/support
        ${ASCEND_HOME_PATH}/aarch64-linux/include /usr/include/c++/11 /usr/include/aarch64-linux-gnu/c++/11)
    target_link_directories(${target} PRIVATE ${ASCEND_HOME_PATH}/lib64)
    target_link_options(${target} PRIVATE -L/usr/lib/gcc/aarch64-linux-gnu/11 -L/usr/lib/aarch64-linux-gnu)
    target_link_libraries(${target} PRIVATE ascendcl tiling_api register platform unified_dlog dl m graph_base)
    target_compile_options(${target} PRIVATE $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=dav-2201> -std=c++17)
endfunction()
vmx_add_ref_probe(vmx_ref_parent support/runner_ref_parent.asc)
vmx_add_ref_probe(vmx_ref_v001 support/runner_ref_v001.asc)

vmx_add_ref_probe(vmx_ref_v002 support/runner_ref_v002.asc)

vmx_add_ref_probe(vmx_div_probe support/div_probe.asc)

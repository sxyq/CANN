cmake_minimum_required(VERSION 3.16)
set(ASCEND_CANN_PACKAGE_PATH "/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002" CACHE PATH "CANN root")
set(ENV{ASCEND_HOME_PATH} "${ASCEND_CANN_PACKAGE_PATH}")
set(CMAKE_PREFIX_PATH
    "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/tikcpp/ascendc_kernel_cmake"
    ${CMAKE_PREFIX_PATH})
find_package(ASC REQUIRED)
project(r31b_wide_panel_resident LANGUAGES ASC)

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

add_executable(r31b_submission_v002 compile_adapter_v002.asc)
target_include_directories(r31b_submission_v002 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(r31b_submission_v002 PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=dav-2201>)
target_link_libraries(r31b_submission_v002 PRIVATE stdc++ m dl)

add_executable(r31b_submission_v003 compile_adapter_v003.asc)
target_include_directories(r31b_submission_v003 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(r31b_submission_v003 PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=dav-2201>)
target_link_libraries(r31b_submission_v003 PRIVATE stdc++ m dl)

add_executable(r31b_submission_v004 compile_adapter_v004.asc)
target_include_directories(r31b_submission_v004 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(r31b_submission_v004 PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=dav-2201>)
target_link_libraries(r31b_submission_v004 PRIVATE stdc++ m dl)

add_executable(r31b_submission_v005 compile_adapter_v005.asc)
target_include_directories(r31b_submission_v005 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(r31b_submission_v005 PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=dav-2201>)
target_link_libraries(r31b_submission_v005 PRIVATE stdc++ m dl)

add_executable(r31b_submission_v006 compile_adapter_v006.asc)
target_include_directories(r31b_submission_v006 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(r31b_submission_v006 PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=dav-2201>)
target_link_libraries(r31b_submission_v006 PRIVATE stdc++ m dl)

add_executable(r31b_submission_v007 compile_adapter_v007.asc)
target_include_directories(r31b_submission_v007 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(r31b_submission_v007 PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=dav-2201>)
target_link_libraries(r31b_submission_v007 PRIVATE stdc++ m dl)

add_executable(r31b_submission_v008 compile_adapter_v008.asc)
target_include_directories(r31b_submission_v008 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(r31b_submission_v008 PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=dav-2201>)
target_link_libraries(r31b_submission_v008 PRIVATE stdc++ m dl)

add_executable(r31b_submission_v009 compile_adapter_v009.asc)
target_include_directories(r31b_submission_v009 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(r31b_submission_v009 PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=dav-2201>)
target_link_libraries(r31b_submission_v009 PRIVATE stdc++ m dl)

add_executable(r31b_submission_v010 compile_adapter_v010.asc)
target_include_directories(r31b_submission_v010 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(r31b_submission_v010 PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=dav-2201>)
target_link_libraries(r31b_submission_v010 PRIVATE stdc++ m dl)

add_executable(r31b_submission_v015 compile_adapter_v015.asc)
target_include_directories(r31b_submission_v015 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(r31b_submission_v015 PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=dav-2201>)
target_link_libraries(r31b_submission_v015 PRIVATE stdc++ m dl)

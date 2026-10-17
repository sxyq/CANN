cmake_minimum_required(VERSION 3.16)
set(ASCEND_CANN_PACKAGE_PATH "/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002" CACHE PATH "CANN root")
set(ENV{ASCEND_HOME_PATH} "${ASCEND_CANN_PACKAGE_PATH}")
list(PREPEND CMAKE_PREFIX_PATH "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/tikcpp/ascendc_kernel_cmake")
find_package(ASC REQUIRED)
project(r05_gamma_view LANGUAGES ASC CXX)
set(CMAKE_CXX_STANDARD 17)
set(SOC_VERSION Ascend910B3)
set(R05_PARENT_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../线上结果/R31B/V011" CACHE PATH "Unmodified Parent directory")
set(HCC_TOOLCHAIN "${ASCEND_CANN_PACKAGE_PATH}/toolkit/toolchain/hcc")
set(R05_INCLUDES "-I/usr/include/c++/11 -I/usr/include/aarch64-linux-gnu/c++/11 -I${HCC_TOOLCHAIN}/aarch64-target-linux-gnu/include/c++/7.3.0 -I${HCC_TOOLCHAIN}/aarch64-target-linux-gnu/include/c++/7.3.0/aarch64-target-linux-gnu -I${HCC_TOOLCHAIN}/aarch64-target-linux-gnu/include/c++/7.3.0/backward")
# Compiler arguments follow the successful R12 Parent build.
set(CMAKE_ASC_COMPILE_OBJECT "<CMAKE_ASC_COMPILER> ${CMAKE_COMPILE_AS_ASC_FLAG} <DEFINES> <INCLUDES> ${R05_INCLUDES} --cce-aicore-arch=dav-c220 -Xaicore-start -O3 -std=c++17 -Xaicore-end -Xhost-start ${R05_INCLUDES} -fPIC -O2 -std=c++17 -Xhost-end <FLAGS> -o <OBJECT> -c -x asc <SOURCE>")
set(R05_TARGETS r05_parent r05_address_probe)
ascendc_library(r05_parent SHARED parent_entry.asc)
target_include_directories(r05_parent PRIVATE "${R05_PARENT_DIR}")
add_executable(r05_address_probe address_probe.asc)
target_include_directories(r05_address_probe PRIVATE "${R05_PARENT_DIR}")
if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/candidate.asc")
    ascendc_library(r05_candidate SHARED candidate_entry.asc)
    list(APPEND R05_TARGETS r05_candidate)
endif()
foreach(target IN LISTS R05_TARGETS)
    target_include_directories(${target} PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}" "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/include"
        "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/ascendc/include"
        "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/ascendc/include/basic_api/impl"
        "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/asc/include"
        "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/asc/include/basic_api"
        "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/asc/include/interface")
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${target}-temps")
    target_compile_options(${target} PRIVATE $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=dav-2201> $<$<COMPILE_LANGUAGE:ASC>:-save-temps=${CMAKE_CURRENT_BINARY_DIR}/${target}-temps>)
    target_link_directories(${target} PRIVATE "${ASCEND_CANN_PACKAGE_PATH}/lib64"
        "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/lib64" /usr/lib/aarch64-linux-gnu
        "${HCC_TOOLCHAIN}/aarch64-target-linux-gnu/lib64" "${HCC_TOOLCHAIN}/lib64")
    target_link_libraries(${target} PRIVATE tiling_api register platform unified_dlog
        /usr/lib/aarch64-linux-gnu/libstdc++.so.6 dl m graph_base)
    target_link_options(${target} PRIVATE "-Wl,-Bsymbolic")
endforeach()
add_executable(r05_probe probe.cpp)
target_include_directories(r05_probe PRIVATE "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/include")
target_link_directories(r05_probe PRIVATE "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/lib64")
target_link_libraries(r05_probe PRIVATE ascendcl dl m)

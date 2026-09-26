cmake_minimum_required(VERSION 3.16)
set(SOC_VERSION Ascend910B3)
set(SOC_ARCH dav-2201)
set(ASCEND_CANN_PACKAGE_PATH "/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002")
set(ASC_DIR "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/tikcpp/ascendc_kernel_cmake" CACHE PATH "ASC" FORCE)
list(PREPEND CMAKE_PREFIX_PATH "${ASC_DIR}")
find_package(ASC REQUIRED)
set(CMAKE_ASC_COMPILER "${ASCEND_CANN_PACKAGE_PATH}/compiler/ccec_compiler/bin/bisheng" CACHE FILEPATH "ASC" FORCE)
set(CMAKE_CXX_COMPILER "${ASCEND_CANN_PACKAGE_PATH}/compiler/ccec_compiler/bin/bisheng" CACHE FILEPATH "CXX" FORCE)
project(atx_ref_probe LANGUAGES ASC CXX)
set(CMAKE_CXX_STANDARD 17)
set(HCC_ROOT "${ASCEND_CANN_PACKAGE_PATH}/toolkit/toolchain/hcc")
set(HCC_CXX_INCLUDES
    "-I${HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0"
    "-I${HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0/aarch64-target-linux-gnu"
    "-I${HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0/backward"
    "-I${HCC_ROOT}/aarch64-target-linux-gnu/include"
    "-I/usr/include/c++/11"
    "-I/usr/include/aarch64-linux-gnu/c++/11"
    "-I/usr/include/aarch64-linux-gnu"
    "-I/usr/include")
string(JOIN " " HCC_CXX_INCLUDES_STR ${HCC_CXX_INCLUDES})
set(CMAKE_ASC_COMPILE_OBJECT
    "<CMAKE_ASC_COMPILER> ${CMAKE_COMPILE_AS_ASC_FLAG} <DEFINES> <INCLUDES> ${HCC_CXX_INCLUDES_STR} --cce-aicore-arch=dav-c220 -Xaicore-start -O3 -std=c++17 -Xaicore-end -Xhost-start ${HCC_CXX_INCLUDES_STR} -fPIC -O2 -std=c++17 -Xhost-end <FLAGS> -o <OBJECT> -c -x asc <SOURCE>")

set(ROUTE_WORKSPACE "${CMAKE_CURRENT_SOURCE_DIR}/..")
foreach(variant IN ITEMS ref_parent ref_v001)
    add_executable(atx_${variant}_probe runner_${variant}.asc)
    target_include_directories(atx_${variant}_probe PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}"
        "${ROUTE_WORKSPACE}")
    target_compile_options(atx_${variant}_probe PRIVATE
        $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=${SOC_ARCH}>)
    # System libstdc++ dir FIRST so GLIBCXX_3.4.29 resolves at runtime
    target_link_directories(atx_${variant}_probe BEFORE PRIVATE
        "/usr/lib/aarch64-linux-gnu")
    target_link_directories(atx_${variant}_probe PRIVATE
        "${HCC_ROOT}/aarch64-target-linux-gnu/lib64"
        "${HCC_ROOT}/lib64")
    target_link_options(atx_${variant}_probe PRIVATE
        "-Wl,-rpath,/usr/lib/aarch64-linux-gnu"
        "-Wl,-rpath,${HCC_ROOT}/aarch64-target-linux-gnu/lib64"
        "-Wl,-rpath,${HCC_ROOT}/lib64")
    target_link_libraries(atx_${variant}_probe PRIVATE
        tiling_api register platform unified_dlog
        /usr/lib/aarch64-linux-gnu/libstdc++.so.6
        dl m graph_base)
endforeach()

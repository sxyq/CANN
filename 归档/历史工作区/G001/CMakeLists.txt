cmake_minimum_required(VERSION 3.16)

set(SOC_VERSION "Ascend910B3" CACHE STRING "Ascend SoC")
set(ASCEND_CANN_PACKAGE_PATH "/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002" CACHE PATH "CANN toolkit")
set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type")
set(CMAKE_CXX_COMPILER
    "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/ccec_compiler/bin/bisheng"
    CACHE FILEPATH "Ascend C compiler")

set(ENV{ASCEND_HOME_PATH} "${ASCEND_CANN_PACKAGE_PATH}")
set(CMAKE_PREFIX_PATH "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/tikcpp/ascendc_kernel_cmake" ${CMAKE_PREFIX_PATH})
find_package(ASC REQUIRED)
project(g001_add_rms_norm_bias LANGUAGES ASC)

set(G001_TIKCPP_DIR "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/tikcpp/tikcfw")
set(G001_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/src/add_rms_norm_bias.cpp")
set(G001_OBJECT "${CMAKE_CURRENT_BINARY_DIR}/g001_add_rms_norm_bias.o")

add_custom_command(
    OUTPUT "${G001_OBJECT}"
    COMMAND "${CMAKE_CXX_COMPILER}" -x cce -c "${G001_SOURCE}" -o "${G001_OBJECT}" -DG001_DEVICE_ONLY
            -I${G001_TIKCPP_DIR}
            -I${G001_TIKCPP_DIR}/interface
            -I${G001_TIKCPP_DIR}/impl
            -isystem /usr/include/c++/11
            -isystem /usr/include/aarch64-linux-gnu/c++/11
            -isystem /usr/include/aarch64-linux-gnu
            -isystem /usr/include
            -O3 -std=c++17 --cce-aicore-lang --cce-aicore-arch=dav-c220-vec
            --cce-aicore-only --cce-auto-sync --npu-arch=dav-2201
    DEPENDS "${G001_SOURCE}"
    COMMENT "Compile G001 AddRmsNormBias kernel for ${SOC_VERSION} / dav-2201"
    VERBATIM
)

add_custom_target(g001_compile ALL DEPENDS "${G001_OBJECT}")

set(G001_HCC_ROOT "${ASCEND_CANN_PACKAGE_PATH}/toolkit/toolchain/hcc")
set(G001_CPP_INCLUDES
    "-I${G001_HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0"
    "-I${G001_HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0/aarch64-target-linux-gnu"
    "-I${G001_HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0/backward")
string(JOIN " " G001_CPP_FLAGS ${G001_CPP_INCLUDES})
set(CMAKE_ASC_COMPILE_OBJECT
    "<CMAKE_ASC_COMPILER> ${CMAKE_COMPILE_AS_ASC_FLAG} <DEFINES> <INCLUDES> --cce-aicore-arch=dav-c220 ${G001_CPP_FLAGS} -Xaicore-start -O3 -std=c++17 -Xaicore-end -Xhost-start ${G001_CPP_FLAGS} -fPIC -O2 -Xhost-end <FLAGS> -o <OBJECT> -c -x asc <SOURCE>")
link_directories(/usr/lib/gcc/aarch64-linux-gnu/11 /usr/lib/aarch64-linux-gnu)
add_executable(g001_submission_compile compile_adapter.asc)
target_compile_options(g001_submission_compile PRIVATE --npu-arch=dav-2201)
target_link_libraries(g001_submission_compile PRIVATE stdc++ m dl)

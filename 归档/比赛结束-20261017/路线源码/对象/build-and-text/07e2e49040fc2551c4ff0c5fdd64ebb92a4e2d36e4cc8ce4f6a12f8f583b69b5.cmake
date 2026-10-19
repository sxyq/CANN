cmake_minimum_required(VERSION 3.16)

set(SOC_VERSION "Ascend910B3" CACHE STRING "Ascend SoC")
set(ASCEND_CANN_PACKAGE_PATH "/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002" CACHE PATH "CANN toolkit")
set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type")
set(CMAKE_CXX_COMPILER
    "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/ccec_compiler/bin/bisheng"
    CACHE FILEPATH "Ascend C compiler")

project(g001_add_rms_norm_bias LANGUAGES CXX)

set(G001_TIKCPP_DIR "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/tikcpp/tikcfw")
set(G001_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/src/add_rms_norm_bias.cpp")
set(G001_OBJECT "${CMAKE_CURRENT_BINARY_DIR}/g001_add_rms_norm_bias.o")

add_custom_command(
    OUTPUT "${G001_OBJECT}"
    COMMAND "${CMAKE_CXX_COMPILER}" -x cce -c "${G001_SOURCE}" -o "${G001_OBJECT}"
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

#!/bin/bash
CANN=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002
source "$CANN/bin/setenv.bash" >/dev/null 2>&1
export ASCEND_HOME_PATH="$CANN"
cd /home/data4t2/lelinfeng/phase4-workspaces/ASYNC-OVERLAP-CHAMPION-X/V001/support
cat > CMakeLists.txt << 'EOFCM'
cmake_minimum_required(VERSION 3.16)
set(SOC_VERSION Ascend910B3)
set(SOC_ARCH dav-2201)
set(ASCEND_CANN_PACKAGE_PATH "/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002")
set(ASC_DIR "${ASCEND_CANN_PACKAGE_PATH}/aarch64-linux/tikcpp/ascendc_kernel_cmake" CACHE PATH "ASC" FORCE)
list(PREPEND CMAKE_PREFIX_PATH "${ASC_DIR}")
find_package(ASC REQUIRED)
set(CMAKE_ASC_COMPILER "${ASCEND_CANN_PACKAGE_PATH}/compiler/ccec_compiler/bin/bisheng" CACHE FILEPATH "ASC" FORCE)
set(CMAKE_CXX_COMPILER "${ASCEND_CANN_PACKAGE_PATH}/compiler/ccec_compiler/bin/bisheng" CACHE FILEPATH "CXX" FORCE)
project(aoc_corr LANGUAGES ASC CXX)
set(CMAKE_CXX_STANDARD 17)
set(HCC_ROOT "${ASCEND_CANN_PACKAGE_PATH}/toolkit/toolchain/hcc")
set(HCC_CXX_INCLUDES "-I${HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0 -I${HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0/aarch64-target-linux-gnu -I${HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0/backward -I${HCC_ROOT}/aarch64-target-linux-gnu/include -I/usr/include/c++/11 -I/usr/include/aarch64-linux-gnu/c++/11 -I/usr/include/aarch64-linux-gnu -I/usr/include")
string(JOIN " " HCC_CXX_INCLUDES_STR ${HCC_CXX_INCLUDES})
set(CMAKE_ASC_COMPILE_OBJECT "<CMAKE_ASC_COMPILER> ${CMAKE_COMPILE_AS_ASC_FLAG} <DEFINES> <INCLUDES> ${HCC_CXX_INCLUDES_STR} --cce-aicore-arch=dav-c220 -Xaicore-start -O3 -std=c++17 -Xaicore-end -Xhost-start ${HCC_CXX_INCLUDES_STR} -fPIC -O2 -std=c++17 -Xhost-end <FLAGS> -o <OBJECT> -c -x asc <SOURCE>")
add_executable(aoc_v001_correctness runner_correctness.asc)
target_include_directories(aoc_v001_correctness PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/../src")
target_compile_options(aoc_v001_correctness PRIVATE $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=${SOC_ARCH}>)
target_link_directories(aoc_v001_correctness BEFORE PRIVATE "/usr/lib/aarch64-linux-gnu")
target_link_directories(aoc_v001_correctness PRIVATE "${HCC_ROOT}/aarch64-target-linux-gnu/lib64" "${HCC_ROOT}/lib64")
target_link_options(aoc_v001_correctness PRIVATE "-Wl,-rpath,/usr/lib/aarch64-linux-gnu" "-Wl,-rpath,${HCC_ROOT}/aarch64-target-linux-gnu/lib64" "-Wl,-rpath,${HCC_ROOT}/lib64")
target_link_libraries(aoc_v001_correctness PRIVATE tiling_api register platform unified_dlog /usr/lib/aarch64-linux-gnu/libstdc++.so.6 dl m graph_base)
EOFCM
rm -rf build && mkdir -p build
cmake -S . -B build > build/cfg.log 2>&1
echo "CFG_RC=$?"
cmake --build build -j2 > build/build.log 2>&1
echo "BUILD_RC=$?"
ls -la build/aoc_v001_correctness 2>/dev/null
tail -10 build/build.log

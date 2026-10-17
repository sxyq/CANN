#!/usr/bin/env bash
set -eo pipefail

R07_ROOT="${1:?revision directory is required}"
R07_CANN="/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002"
source "${R07_CANN}/aarch64-linux/script/set_env.sh"
set -u
export ASCEND_HOME_PATH="${R07_CANN}"
export ASCEND_CANN_PACKAGE_PATH="${R07_CANN}"
export CMAKE_PREFIX_PATH="${R07_CANN}/aarch64-linux/tikcpp/ascendc_kernel_cmake"
export LD_LIBRARY_PATH="${R07_CANN}/aarch64-linux/lib64:${R07_CANN}/lib64:/usr/local/Ascend/driver/lib64/driver:/usr/local/Ascend/driver/lib64/common${LD_LIBRARY_PATH:+:${LD_LIBRARY_PATH}}"
R07_GCC_VERSION="$(g++ -dumpversion)"
R07_GCC_TARGET="$(gcc -dumpmachine)"
R07_GCC_MULTIARCH="$(gcc -print-multiarch)"
export CPLUS_INCLUDE_PATH="/usr/include/c++/${R07_GCC_VERSION}:/usr/include/${R07_GCC_MULTIARCH}/c++/${R07_GCC_VERSION}:/usr/include/c++/${R07_GCC_VERSION}/backward${CPLUS_INCLUDE_PATH:+:${CPLUS_INCLUDE_PATH}}"
export C_INCLUDE_PATH="/usr/include/${R07_GCC_MULTIARCH}${C_INCLUDE_PATH:+:${C_INCLUDE_PATH}}"
export LIBRARY_PATH="/usr/lib/gcc/${R07_GCC_TARGET}/${R07_GCC_VERSION}:/usr/lib/${R07_GCC_MULTIARCH}:/lib/${R07_GCC_MULTIARCH}${LIBRARY_PATH:+:${LIBRARY_PATH}}"

date -Is
cmake -S "${R07_ROOT}/support" -B "${R07_ROOT}/build" \
    -DCMAKE_BUILD_TYPE=Release \
    -DASCEND_HOME_PATH="${R07_CANN}" \
    -DASCEND_CANN_PACKAGE_PATH="${R07_CANN}" \
    -DCMAKE_PREFIX_PATH="${CMAKE_PREFIX_PATH}"
cmake --build "${R07_ROOT}/build" --parallel 2
ldd "${R07_ROOT}/build/w4r07_runner"
date -Is

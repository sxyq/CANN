#!/usr/bin/env bash
set -euo pipefail
R02_SUPPORT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
R02_CANN_ROOT="/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002"
export ASCEND_HOME_PATH="${R02_CANN_ROOT}"
export PATH="${R02_CANN_ROOT}/compiler/ccec_compiler/bin:${R02_CANN_ROOT}/tools/ccec_compiler/bin:${R02_CANN_ROOT}/aarch64-linux/bin:${PATH}"
export ASCEND_HOME_PATH="${R02_CANN_ROOT}"
export ASCEND_CANN_PACKAGE_PATH="${R02_CANN_ROOT}"
export LD_LIBRARY_PATH="${R02_CANN_ROOT}/aarch64-linux/lib64:${R02_CANN_ROOT}/lib64:/usr/local/Ascend/driver/lib64/driver:/usr/local/Ascend/driver/lib64/common${LD_LIBRARY_PATH:+:${LD_LIBRARY_PATH}}"
R02_GCC_VERSION="$(g++ -dumpversion)"
R02_GCC_TARGET="$(gcc -dumpmachine)"
R02_GCC_MULTIARCH="$(gcc -print-multiarch)"
export CPLUS_INCLUDE_PATH="/usr/include/c++/${R02_GCC_VERSION}:/usr/include/${R02_GCC_MULTIARCH}/c++/${R02_GCC_VERSION}:/usr/include/c++/${R02_GCC_VERSION}/backward${CPLUS_INCLUDE_PATH:+:${CPLUS_INCLUDE_PATH}}"
export C_INCLUDE_PATH="/usr/include/${R02_GCC_MULTIARCH}${C_INCLUDE_PATH:+:${C_INCLUDE_PATH}}"
export LIBRARY_PATH="/usr/lib/gcc/${R02_GCC_TARGET}/${R02_GCC_VERSION}:/usr/lib/${R02_GCC_MULTIARCH}:/lib/${R02_GCC_MULTIARCH}${LIBRARY_PATH:+:${LIBRARY_PATH}}"
cmake -S "${R02_SUPPORT_DIR}" -B "${R02_SUPPORT_DIR}/../build" \
    -DCMAKE_BUILD_TYPE=Release \
    -DASCEND_HOME_PATH="${R02_CANN_ROOT}" \
    -DASCEND_CANN_PACKAGE_PATH="${R02_CANN_ROOT}" \
    -DCMAKE_PREFIX_PATH="${R02_CANN_ROOT}/aarch64-linux/tikcpp/ascendc_kernel_cmake"
cmake --build "${R02_SUPPORT_DIR}/../build" --parallel 4
ldd "${R02_SUPPORT_DIR}/../build/r02_runner"

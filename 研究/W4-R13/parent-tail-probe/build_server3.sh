#!/usr/bin/env bash
set -eo pipefail
W4_PROBE_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
W4_CANN_ROOT=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002
export PATH="${W4_CANN_ROOT}/compiler/ccec_compiler/bin:${W4_CANN_ROOT}/tools/ccec_compiler/bin:${W4_CANN_ROOT}/aarch64-linux/bin:${PATH}"
export LD_LIBRARY_PATH="${W4_CANN_ROOT}/aarch64-linux/lib64:${W4_CANN_ROOT}/lib64:/usr/local/Ascend/driver/lib64/driver:/usr/local/Ascend/driver/lib64/common${LD_LIBRARY_PATH:+:${LD_LIBRARY_PATH}}"
export ASCEND_HOME_PATH="${W4_CANN_ROOT}"
export ASCEND_CANN_PACKAGE_PATH="${W4_CANN_ROOT}"
export CMAKE_PREFIX_PATH="${W4_CANN_ROOT}/aarch64-linux/tikcpp/ascendc_kernel_cmake"
W4_GCC_VERSION="$(g++ -dumpversion)"
W4_GCC_TARGET="$(gcc -dumpmachine)"
W4_GCC_MULTIARCH="$(gcc -print-multiarch)"
export CPLUS_INCLUDE_PATH="/usr/include/c++/${W4_GCC_VERSION}:/usr/include/${W4_GCC_MULTIARCH}/c++/${W4_GCC_VERSION}:/usr/include/c++/${W4_GCC_VERSION}/backward"
export C_INCLUDE_PATH="/usr/include/${W4_GCC_MULTIARCH}"
export LIBRARY_PATH="/usr/lib/gcc/${W4_GCC_TARGET}/${W4_GCC_VERSION}:/usr/lib/${W4_GCC_MULTIARCH}:/lib/${W4_GCC_MULTIARCH}"
cmake -S "${W4_PROBE_ROOT}" -B "${W4_PROBE_ROOT}/build" \
    -DCMAKE_BUILD_TYPE=Release -DASCEND_HOME_PATH="${W4_CANN_ROOT}" \
    -DASCEND_CANN_PACKAGE_PATH="${W4_CANN_ROOT}"
cmake --build "${W4_PROBE_ROOT}/build" --parallel 2 "$@"

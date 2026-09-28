#!/usr/bin/env bash
set -euo pipefail

: "${ASCEND_HOME_PATH:=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002}"
export ASCEND_HOME_PATH
export PATH="${ASCEND_HOME_PATH}/aarch64-linux/ccec_compiler/bin:${PATH}"
export CPATH="${ASCEND_HOME_PATH}/toolkit/toolchain/hcc/aarch64-target-linux-gnu/include/c++/7.3.0:${ASCEND_HOME_PATH}/toolkit/toolchain/hcc/aarch64-target-linux-gnu/include/c++/7.3.0/aarch64-target-linux-gnu${CPATH:+:${CPATH}}"

cmake -S . -B build \
    -DSOC_VERSION=Ascend910B3 \
    -DCMAKE_PREFIX_PATH="${ASCEND_HOME_PATH}/aarch64-linux/tikcpp/ascendc_kernel_cmake"
cmake --build build -j2

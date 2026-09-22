#!/bin/bash
# WIDE-X compile: device object + submission object + full link on cann-server3
set -eo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"
export LD_LIBRARY_PATH="${LD_LIBRARY_PATH:-}"
export PATH="${PATH:-/usr/bin:/bin}"
export PYTHONPATH="${PYTHONPATH:-}"
source /usr/local/Ascend/ascend-toolkit/set_env.sh
export ASCEND_HOME_PATH="${ASCEND_HOME_PATH:-/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002}"
export ASC_DIR="${ASCEND_HOME_PATH}/aarch64-linux/tikcpp/ascendc_kernel_cmake"
export CMAKE_PREFIX_PATH="${ASC_DIR}${CMAKE_PREFIX_PATH:+:${CMAKE_PREFIX_PATH}}"
HCC_ROOT="${ASCEND_HOME_PATH}/toolkit/toolchain/hcc"
export CPLUS_INCLUDE_PATH="${HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0:${HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0/aarch64-target-linux-gnu:${HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0/backward:${CPLUS_INCLUDE_PATH:-}"
export C_INCLUDE_PATH="${HCC_ROOT}/aarch64-target-linux-gnu/include:${C_INCLUDE_PATH:-}"
set -u

mkdir -p logs build
rm -rf build/*
mkdir -p build
cd build

echo "=== configure ===" | tee ../logs/configure_v001.log
cmake .. 2>&1 | tee -a ../logs/configure_v001.log

echo "=== device compile (wide_x_device) ===" | tee ../logs/compile_device_v001.log
make wide_x_device -j4 2>&1 | tee -a ../logs/compile_device_v001.log

echo "=== submission compile (wide_x_submission) ===" | tee ../logs/compile_submission_v001.log
make wide_x_submission -j4 2>&1 | tee -a ../logs/compile_submission_v001.log

echo "=== full link (wide_x_full_link) ===" | tee ../logs/compile_fulllink_v001.log
make wide_x_full_link -j4 2>&1 | tee -a ../logs/compile_fulllink_v001.log

echo "=== ALL COMPILE STAGES DONE ==="
ls -la wide_x_device* wide_x_submission* wide_x_full_link 2>/dev/null || find . -name 'wide_x_*' | head

#!/bin/bash
# MODE-X compile: device object + submission object + full link on cann-server3
set -eo pipefail
ROOT="$(cd "$(dirname "$0")" && pwd)"
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

REV="${REV:-V001}"
STAMP="$(date +%Y%m%d_%H%M%S)"
mkdir -p logs
rm -rf build/*
mkdir -p build
cd build

echo "=== configure ${REV} ${STAMP} ===" | tee "../logs/configure_${REV}_${STAMP}.log"
cmake .. -DCMAKE_ASC_ARCHITECTURES=dav-2201 -DSOC_VERSION=Ascend910B3 2>&1 | tee -a "../logs/configure_${REV}_${STAMP}.log"

echo "=== device compile (mode_x_device) ===" | tee "../logs/compile_device_${REV}_${STAMP}.log"
make mode_x_device -j8 2>&1 | tee -a "../logs/compile_device_${REV}_${STAMP}.log"

echo "=== submission compile (mode_x_submission) ===" | tee "../logs/compile_submission_${REV}_${STAMP}.log"
make mode_x_submission -j8 2>&1 | tee -a "../logs/compile_submission_${REV}_${STAMP}.log"

echo "=== full link (mode_x_full_link) ===" | tee "../logs/compile_fulllink_${REV}_${STAMP}.log"
make mode_x_full_link -j8 2>&1 | tee -a "../logs/compile_fulllink_${REV}_${STAMP}.log"

{
  echo "MODE-X ${REV} build summary ${STAMP}"
  echo "SOURCE=${ROOT}/submission.asc"
  echo "has_cmath_first=$(head -1 ${ROOT}/submission.asc | grep -c '#include <cmath>' || true)"
  echo "has_vector_entry=$(grep -c '__global__ __vector__' ${ROOT}/submission.asc || true)"
  echo "has_run_kernel=$(grep -c 'extern \"C\" void run_kernel' ${ROOT}/submission.asc || true)"
  echo "has_rightPadding=$(grep -c 'rightPadding' ${ROOT}/submission.asc || true)"
  echo "has_reduce_tmp_8k=$(grep -c 'kReduceTmpBytes = 8' ${ROOT}/submission.asc || true)"
  echo "has_launch_null_smdesc=$(grep -c '<<<.*, nullptr, stream>>>' ${ROOT}/submission.asc || true)"
  echo "has_tensorinfo_redefine=$(grep -c 'struct TensorInfo' ${ROOT}/submission.asc || true)"
  echo "mode_select=$(grep -c 'MODE_SPLIT_D\|MODE_SINGLE_N\|MODE_MERGE_N\|MODE_MULTI_N\|MODE_NORMAL' ${ROOT}/submission.asc || true)"
} | tee "../logs/build_summary_${REV}_${STAMP}.log"

echo "=== ALL COMPILE STAGES DONE ==="
ls -la mode_x_device* mode_x_submission* mode_x_full_link 2>/dev/null || find . -name 'mode_x_*' | head

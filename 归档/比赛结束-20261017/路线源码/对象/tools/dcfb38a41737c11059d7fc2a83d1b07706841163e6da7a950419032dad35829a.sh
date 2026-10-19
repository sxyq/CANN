#!/bin/bash
set -euo pipefail

export ASCEND_HOME_PATH="${ASCEND_HOME_PATH:-/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002}"
export PATH="${ASCEND_HOME_PATH}/bin:${ASCEND_HOME_PATH}/aarch64-linux/ccec_compiler/bin:${PATH}"
set +u
source "${ASCEND_HOME_PATH}/bin/setenv.bash" 2>/dev/null || true
set -u

ROOT="$(cd "$(dirname "$0")" && pwd)"
BUILD="${ROOT}/build"
KIT="${ASCEND_HOME_PATH}/aarch64-linux/tikcpp/ascendc_kernel_cmake"
HCC="${ASCEND_HOME_PATH}/toolkit/toolchain/hcc/aarch64-target-linux-gnu/include/c++/7.3.0"
export CPLUS_INCLUDE_PATH="${HCC}:${HCC}/aarch64-target-linux-gnu:${HCC}/backward:${CPLUS_INCLUDE_PATH:-}"
export C_INCLUDE_PATH="${HCC}:${HCC}/aarch64-target-linux-gnu:${C_INCLUDE_PATH:-}"
export CMAKE_PREFIX_PATH="${KIT}:${CMAKE_PREFIX_PATH:-}"

{
  echo "=== NORM-AFFINE-ARITH-X V001 compile $(date -u +%Y-%m-%dT%H:%M:%SZ) ==="
  echo "ASCEND_HOME_PATH=${ASCEND_HOME_PATH}"
  echo "PARENT_SHA=$(sha256sum "${ROOT}/../../../../线上结果/R31B/V011/submission.asc" | awk '{print $1}')"
  echo "V001_SHA=$(sha256sum "${ROOT}/../submission.asc" | awk '{print $1}')"
  cmake -S "${ROOT}" -B "${BUILD}" \
    -DCMAKE_MODULE_PATH="${KIT}/ASC_CMake;${KIT}" \
    -DASC_DIR="${KIT}" -DCMAKE_PREFIX_PATH="${KIT}" \
    -DSOC_VERSION=Ascend910B3 -DNPU_ARCH=dav-2201
  cmake --build "${BUILD}" --target norm_affine_arith_x_v001 -j2
  sha256sum "${BUILD}/norm_affine_arith_x_v001"
  echo "=== COMPILE_OK ==="
} 2>&1 | tee "${ROOT}/compile-evidence-rerun.txt"

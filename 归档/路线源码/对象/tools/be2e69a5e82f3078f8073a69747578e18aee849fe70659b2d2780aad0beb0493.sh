#!/bin/bash
# UB-LIVENESS-X V001 compile on cann-server3
set -euo pipefail
export ASCEND_HOME_PATH=${ASCEND_HOME_PATH:-/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002}
export PATH="${ASCEND_HOME_PATH}/bin:${ASCEND_HOME_PATH}/aarch64-linux/ccec_compiler/bin:${PATH}"
source "${ASCEND_HOME_PATH}/bin/setenv.sh" 2>/dev/null || true

KIT="${ASCEND_HOME_PATH}/aarch64-linux/tikcpp/ascendc_kernel_cmake"
HCC="${ASCEND_HOME_PATH}/toolkit/toolchain/hcc/aarch64-target-linux-gnu/include/c++/7.3.0"
export CPLUS_INCLUDE_PATH="${HCC}:${HCC}/aarch64-target-linux-gnu:${HCC}/backward:${CPLUS_INCLUDE_PATH:-}"
export CMAKE_PREFIX_PATH="${KIT}:${CMAKE_PREFIX_PATH:-}"
export ASC_DIR="${KIT}"

ROOT="$(cd "$(dirname "$0")" && pwd)"
BUILD="${ROOT}/build"
mkdir -p "${BUILD}"
cd "${BUILD}"

LOG="${ROOT}/compile.log"
{
  echo "=== UB-LIVENESS-X compile $(date -u +%Y-%m-%dT%H:%M:%SZ) ==="
  echo "ASCEND_HOME_PATH=${ASCEND_HOME_PATH}"
  echo "SOC: Ascend910B3 arch dav-2201"
  echo "--- cmake ---"
  cmake .. \
    -DCMAKE_MODULE_PATH="${KIT}/ASC_CMake;${KIT}" \
    -DASC_DIR="${KIT}" \
    -DCMAKE_PREFIX_PATH="${KIT}" \
    -DCMAKE_CXX_FLAGS="-I${HCC} -I${HCC}/aarch64-target-linux-gnu -I${HCC}/backward" \
    -DCMAKE_C_FLAGS="-I${HCC} -I${HCC}/aarch64-target-linux-gnu" \
    -DSOC_VERSION=Ascend910B3 \
    -DNPU_ARCH=dav-2201
  echo "--- device compile ---"
  cmake --build . --target ub_liveness_x_device -j"$(nproc)"
  echo "--- submission compile ---"
  cmake --build . --target ub_liveness_x_submission -j"$(nproc)"
  echo "--- full link ---"
  cmake --build . --target ub_liveness_x_full_link -j"$(nproc)"
  echo "=== COMPILE_OK ==="
} 2>&1 | tee "${LOG}"

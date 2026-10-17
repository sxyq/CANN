#!/bin/bash
# REDUCE-INVSCALE-X V001: device compile, submission compile, full link, probes.
set -euo pipefail
export ASCEND_HOME_PATH=${ASCEND_HOME_PATH:-/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002}
export PATH="${ASCEND_HOME_PATH}/bin:${ASCEND_HOME_PATH}/aarch64-linux/ccec_compiler/bin:${PATH}"
set +u; source "${ASCEND_HOME_PATH}/bin/setenv.bash" 2>/dev/null || true; set -u

ROOT="$(cd "$(dirname "$0")" && pwd)"
BUILD="${ROOT}/build"
LOG="${ROOT}/compile.log"
mkdir -p "${BUILD}"

HCC="${ASCEND_HOME_PATH}/toolkit/toolchain/hcc/aarch64-target-linux-gnu/include/c++/7.3.0"
KIT="${ASCEND_HOME_PATH}/aarch64-linux/tikcpp/ascendc_kernel_cmake"
export CPLUS_INCLUDE_PATH="${HCC}:${HCC}/aarch64-target-linux-gnu:${HCC}/backward:${CPLUS_INCLUDE_PATH:-}"
export C_INCLUDE_PATH="${HCC}:${HCC}/aarch64-target-linux-gnu:${C_INCLUDE_PATH:-}"
export CMAKE_PREFIX_PATH="${KIT}:${CMAKE_PREFIX_PATH:-}"
export ASC_DIR="${KIT}"

{
  echo "=== REDUCE-INVSCALE-X compile $(date -u +%Y-%m-%dT%H:%M:%SZ) ==="
  echo "ASCEND_HOME_PATH=${ASCEND_HOME_PATH}"
  echo "SOC: Ascend910B3 arch dav-2201"
  echo "PARENT_SHA=$(sha256sum ${ROOT}/parent.asc | awk '{print $1}')"
  echo "V001_SHA=$(sha256sum ${ROOT}/submission.asc | awk '{print $1}')"
  echo "--- cmake configure ---"
  cmake -S "${ROOT}" -B "${BUILD}" \
    -DCMAKE_MODULE_PATH="${KIT}/ASC_CMake;${KIT}" \
    -DASC_DIR="${KIT}" \
    -DCMAKE_PREFIX_PATH="${KIT}" \
    -DCMAKE_CXX_FLAGS="-I${HCC} -I${HCC}/aarch64-target-linux-gnu -I${HCC}/backward" \
    -DCMAKE_C_FLAGS="-I${HCC} -I${HCC}/aarch64-target-linux-gnu" \
    -DSOC_VERSION=Ascend910B3 -DNPU_ARCH=dav-2201
  echo "--- device compile (submission.asc via shim) ---"
  cmake --build "${BUILD}" --target reduce_invscale_device -j"$(nproc)"
  echo "DEVICE_COMPILE=PASS"
  echo "--- submission compile ---"
  cmake --build "${BUILD}" --target reduce_invscale_submission -j"$(nproc)"
  echo "SUBMISSION_COMPILE=PASS"
  echo "--- full link ---"
  cmake --build "${BUILD}" --target reduce_invscale_full_link -j"$(nproc)"
  echo "FULL_LINK=PASS"
  echo "--- probe runners: parent + candidate + v001ref ---"
  cmake --build "${BUILD}" --target reduce_invscale_parent_probe reduce_invscale_candidate_probe reduce_invscale_v001ref_probe -j"$(nproc)"
  echo "PROBE_BUILD=PASS"
  echo "=== COMPILE_OK ==="
} 2>&1 | tee "${LOG}"

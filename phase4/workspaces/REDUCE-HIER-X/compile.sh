#!/bin/bash
# REDUCE-HIER-X V001: compile + link on server3. Numeric RCs only.
set -uo pipefail
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
  echo "=== REDUCE-HIER-X V001 compile $(date -u +%Y-%m-%dT%H:%M:%SZ) ==="
  echo "ASCEND_HOME_PATH=${ASCEND_HOME_PATH}"
  echo "SOC: Ascend910B3 arch dav-2201"
  echo "SOURCE_SHA=$(sha256sum "${ROOT}/submission.asc" | awk '{print $1}')"
  echo "--- cmake configure ---"
  cmake -S "${ROOT}" -B "${BUILD}" \
    -DCMAKE_MODULE_PATH="${KIT}/ASC_CMake;${KIT}" \
    -DASC_DIR="${KIT}" \
    -DCMAKE_PREFIX_PATH="${KIT}" \
    -DCMAKE_CXX_FLAGS="-I${HCC} -I${HCC}/aarch64-target-linux-gnu -I${HCC}/backward" \
    -DCMAKE_C_FLAGS="-I${HCC} -I${HCC}/aarch64-target-linux-gnu" \
    -DSOC_VERSION=Ascend910B3 -DNPU_ARCH=dav-2201
  CMAKE_RC=$?
  echo "CMAKE_RC=${CMAKE_RC}"
  echo "--- compile+link target rhx_v001_submission ---"
  cmake --build "${BUILD}" --target rhx_v001_submission -j"$(nproc)"
  BUILD_RC=$?
  echo "BUILD_RC=${BUILD_RC}"
  if [ -f "${BUILD}/rhx_v001_submission" ]; then
    echo "EXECUTABLE_SHA=$(sha256sum "${BUILD}/rhx_v001_submission" | awk '{print $1}')"
    echo "EXECUTABLE_BYTES=$(stat -c%s "${BUILD}/rhx_v001_submission")"
  else
    echo "EXECUTABLE_SHA=MISSING"
    echo "EXECUTABLE_BYTES=0"
  fi
  echo "=== COMPILE_DONE CMAKE_RC=${CMAKE_RC} BUILD_RC=${BUILD_RC} ==="
} 2>&1 | tee "${LOG}"
exit ${BUILD_RC:-1}

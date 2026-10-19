#!/bin/bash
# STORE-EPILOGUE-X V001: compile + link on server3. Numeric RCs only.
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
  echo "=== STORE-EPILOGUE-X V001 compile $(date -u +%Y-%m-%dT%H:%M:%SZ) ==="
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
  echo "--- compile submission object ---"
  cmake --build "${BUILD}" --target se_v001_submission -j"$(nproc)"
  OBJ_RC=$?
  echo "OBJ_RC=${OBJ_RC}"
  echo "--- link target se_full_link ---"
  cmake --build "${BUILD}" --target se_full_link -j"$(nproc)"
  LINK_RC=$?
  echo "LINK_RC=${LINK_RC}"
  if [ -f "${BUILD}/se_full_link" ]; then
    echo "EXECUTABLE_SHA=$(sha256sum "${BUILD}/se_full_link" | awk '{print $1}')"
    echo "EXECUTABLE_BYTES=$(stat -c%s "${BUILD}/se_full_link")"
  else
    echo "EXECUTABLE_SHA=MISSING"
    echo "EXECUTABLE_BYTES=0"
  fi
  echo "--- build correctness probes ---"
  cmake --build "${BUILD}" --target se_parent_probe se_candidate_probe -j"$(nproc)"
  PROBE_RC=$?
  echo "PROBE_RC=${PROBE_RC}"
  for t in se_parent_probe se_candidate_probe; do
    if [ -f "${BUILD}/${t}" ]; then
      echo "PROBE_SHA_${t}=$(sha256sum "${BUILD}/${t}" | awk '{print $1}')"
    fi
  done
  BUILD_RC=$(( OBJ_RC != 0 ? OBJ_RC : (LINK_RC != 0 ? LINK_RC : PROBE_RC) ))
  echo "=== COMPILE_DONE CMAKE_RC=${CMAKE_RC} BUILD_RC=${BUILD_RC} ==="
} 2>&1 | tee "${LOG}"
exit ${BUILD_RC:-1}

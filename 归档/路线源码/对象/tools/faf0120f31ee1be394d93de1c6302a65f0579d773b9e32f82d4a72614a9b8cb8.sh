#!/usr/bin/env bash
# W4-R10 V001: server3 compile. Build only; no measurement, no device lease.
set -uo pipefail
export ASCEND_HOME_PATH=${ASCEND_HOME_PATH:-/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002}
export LD_LIBRARY_PATH=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/lib64:/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/lib64:/usr/lib/aarch64-linux-gnu
export PATH="${ASCEND_HOME_PATH}/bin:${ASCEND_HOME_PATH}/aarch64-linux/ccec_compiler/bin:${PATH}"
set +u; source "${ASCEND_HOME_PATH}/bin/setenv.bash" >/dev/null 2>&1 || true; set -u

ROOT="$(cd "$(dirname "$0")" && pwd)"
BUILD="${ROOT}/build"
LOG="${ROOT}/compile.log"
mkdir -p "${BUILD}"

HCC="${ASCEND_HOME_PATH}/toolkit/toolchain/hcc/aarch64-target-linux-gnu/include/c++/7.3.0"
KIT="${ASCEND_HOME_PATH}/aarch64-linux/tikcpp/ascendc_kernel_cmake"
export CPLUS_INCLUDE_PATH="${HCC}:${HCC}/aarch64-target-linux-gnu:${HCC}/backward:/usr/include/c++/11:/usr/include/aarch64-linux-gnu/c++/11:/usr/include/aarch64-linux-gnu/c++/11/backward:${CPLUS_INCLUDE_PATH:-}"
export C_INCLUDE_PATH="${HCC}:${HCC}/aarch64-target-linux-gnu:/usr/include:/usr/include/aarch64-linux-gnu:${C_INCLUDE_PATH:-}"
export CMAKE_PREFIX_PATH="${KIT}:${CMAKE_PREFIX_PATH:-}"
export ASC_DIR="${KIT}"

{
  echo "=== W4-R10 V001 compile $(date -u +%Y-%m-%dT%H:%M:%SZ) ==="
  echo "HOST=$(hostname) USER=$(whoami)"
  echo "ASCEND_HOME_PATH=${ASCEND_HOME_PATH}"
  echo "SOC: Ascend910B3 arch dav-2201"
  echo "PARENT_SOURCE_SHA=$(sha256sum "${ROOT}/parent.asc" | awk "{print \$1}")"
  echo "CANDIDATE_SOURCE_SHA=$(sha256sum "${ROOT}/submission.asc" | awk "{print \$1}")"
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
  echo "--- build parent probe ---"
  cmake --build "${BUILD}" --target w4r10_ref_parent_probe -j2
  P_RC=$?
  echo "PARENT_BUILD_RC=${P_RC}"
  echo "--- build candidate probe ---"
  cmake --build "${BUILD}" --target w4r10_ref_candidate_probe -j2
  C_RC=$?
  echo "CANDIDATE_BUILD_RC=${C_RC}"
  for f in w4r10_ref_parent_probe w4r10_ref_candidate_probe; do
    if [ -f "${BUILD}/${f}" ]; then
      echo "EXE_${f}=$(sha256sum "${BUILD}/${f}" | awk "{print \$1}")"
    else
      echo "EXE_${f}=MISSING"
    fi
  done
  echo "=== COMPILE_DONE CMAKE_RC=${CMAKE_RC} PARENT_BUILD_RC=${P_RC} CANDIDATE_BUILD_RC=${C_RC} ==="
} 2>&1 | tee "${LOG}"
exit ${C_RC:-1}

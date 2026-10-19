#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")" && pwd)"
WS="$(cd "${ROOT}/.." && pwd)"
LOGDIR="${WS}/logs"
BUILD="${ROOT}/build"
TOOLKIT="${TOOLKIT:-/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002}"

export ASCEND_HOME_PATH="${TOOLKIT}"
set +u
source "${TOOLKIT}/bin/setenv.bash" >/dev/null 2>&1 || true
source /usr/local/Ascend/ascend-toolkit/set_env.sh >/dev/null 2>&1 || true
set -u
export ASCEND_HOME_PATH="${TOOLKIT}"

HCC="${ASCEND_HOME_PATH}/toolkit/toolchain/hcc"
export CPLUS_INCLUDE_PATH="${HCC}/aarch64-target-linux-gnu/include/c++/7.3.0:${HCC}/aarch64-target-linux-gnu/include/c++/7.3.0/aarch64-target-linux-gnu:${HCC}/aarch64-target-linux-gnu/include/c++/7.3.0/backward:${HCC}/aarch64-target-linux-gnu/include${CPLUS_INCLUDE_PATH:+:$CPLUS_INCLUDE_PATH}"
export C_INCLUDE_PATH="${HCC}/aarch64-target-linux-gnu/include${C_INCLUDE_PATH:+:$C_INCLUDE_PATH}"
export CMAKE_PREFIX_PATH="${TOOLKIT}/aarch64-linux/tikcpp/ascendc_kernel_cmake:${CMAKE_PREFIX_PATH:-}"

# Force the versioned bisheng, not latest symlink.
BISHENG="${TOOLKIT}/compiler/ccec_compiler/bin/bisheng"
test -x "${BISHENG}" || BISHENG="${TOOLKIT}/aarch64-linux/ccec_compiler/bin/bisheng"
test -d "${HCC}" || { echo "missing HCC at ${HCC}" >&2; exit 2; }

echo "ASCEND_HOME_PATH=${ASCEND_HOME_PATH}"
echo "BISHENG=${BISHENG}"
echo "CPLUS_INCLUDE_PATH=${CPLUS_INCLUDE_PATH}"

STAMP="$(date +%Y%m%d_%H%M%S)"
rm -rf "${BUILD}"
mkdir -p "${BUILD}" "${LOGDIR}"
cd "${BUILD}"
cmake "${ROOT}" \
  -DCMAKE_PREFIX_PATH="${CMAKE_PREFIX_PATH}" \
  -DCMAKE_ASC_COMPILER="${BISHENG}" \
  >"${LOGDIR}/probe_cmake_${STAMP}.log" 2>&1
cmake --build . -j"$(nproc)" \
  >"${LOGDIR}/probe_build_${STAMP}.log" 2>&1 || {
    echo "PROBE_BUILD_FAIL ${STAMP}"
    grep -n "fatal error\|error:" "${LOGDIR}/probe_build_${STAMP}.log" | head -40
    tail -n 40 "${LOGDIR}/probe_build_${STAMP}.log"
    exit 1
  }
echo "PROBE_BUILD_OK ${STAMP}"
ls -la "${BUILD}"

#!/usr/bin/env bash
# ALIGN-TAIL-X unified reference harness build (port of SCHED runner_ref pattern).
# Build/link only — no measurement, no device lease.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")" && pwd)"
BUILD="${ROOT}/build"
LOGDIR="${ROOT}/logs"
TOOLKIT="${TOOLKIT:-/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002}"

export ASCEND_HOME_PATH="${TOOLKIT}"
set +u
source "${TOOLKIT}/bin/setenv.bash" >/dev/null 2>&1 || true
set -u
export ASCEND_HOME_PATH="${TOOLKIT}"
# cann-server3 is aarch64: the x86_64 dir from the build instruction does not exist
# here, so the aarch64 equivalent (used by all prior ALIGN builds) is appended.
export CPLUS_INCLUDE_PATH="/usr/include/c++/11:/usr/include/x86_64-linux-gnu/c++/11:/usr/include/c++/11/backward:/usr/include/aarch64-linux-gnu/c++/11:/usr/include/aarch64-linux-gnu"
export C_INCLUDE_PATH="/usr/include:/usr/include/aarch64-linux-gnu"

STAMP="$(date +%Y%m%d_%H%M%S)"
mkdir -p "${BUILD}" "${LOGDIR}"
cd "${BUILD}"
cmake "${ROOT}" >"${LOGDIR}/ref_cfg_${STAMP}.log" 2>&1 || {
  echo "REF_CFG_FAIL ${STAMP}"
  tail -n 40 "${LOGDIR}/ref_cfg_${STAMP}.log"
  exit 1
}
cmake --build . -j1 >"${LOGDIR}/ref_build_${STAMP}.log" 2>&1 || {
  echo "REF_BUILD_FAIL ${STAMP}"
  grep -n "fatal error\|error:" "${LOGDIR}/ref_build_${STAMP}.log" | head -40
  tail -n 40 "${LOGDIR}/ref_build_${STAMP}.log"
  exit 1
}
echo "REF_BUILD_OK ${STAMP}"
ls -la "${BUILD}"/atx_ref_parent_probe "${BUILD}"/atx_ref_v001_probe

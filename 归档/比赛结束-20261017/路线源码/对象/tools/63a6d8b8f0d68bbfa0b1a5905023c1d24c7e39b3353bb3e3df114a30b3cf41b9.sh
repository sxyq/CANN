#!/usr/bin/env bash
# ASYNC-TRIPLE-X unified reference harness build (port of SCHED/ALIGN runner_ref pattern).
# Build/link only — no measurement, no device lease. No kernel/Candidate source edit.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")" && pwd)"
BUILD="${ROOT}/build-ref"
LOGDIR="${ROOT}/logs-ref"
TOOLKIT="${TOOLKIT:-/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002}"

export ASCEND_HOME_PATH="${TOOLKIT}"
set +u
source "${TOOLKIT}/bin/setenv.bash" >/dev/null 2>&1 || true
set -u
export ASCEND_HOME_PATH="${TOOLKIT}"
export CPLUS_INCLUDE_PATH="/usr/include/c++/11:/usr/include/x86_64-linux-gnu/c++/11:/usr/include/c++/11/backward:/usr/include/aarch64-linux-gnu/c++/11:/usr/include/aarch64-linux-gnu"
export C_INCLUDE_PATH="/usr/include:/usr/include/aarch64-linux-gnu"

if [ ! -f "${ROOT}/CMakeLists.ref.txt" ]; then
  echo "MISSING ${ROOT}/CMakeLists.ref.txt"
  exit 2
fi

STAMP="$(date +%Y%m%d_%H%M%S)"
mkdir -p "${BUILD}" "${LOGDIR}"

# Isolated source view so the workspace's original CMakeLists.txt / CMakeLists.probe.txt
# are never touched (build only; no Candidate source edit).
SRCVIEW="${BUILD}/srcview"
mkdir -p "${SRCVIEW}"
cp "${ROOT}/CMakeLists.ref.txt" "${SRCVIEW}/CMakeLists.txt"
ln -sfn "${ROOT}/runner_ref.inc" "${SRCVIEW}/runner_ref.inc"
ln -sfn "${ROOT}/runner_ref_seed.asc" "${SRCVIEW}/runner_ref_seed.asc"
ln -sfn "${ROOT}/runner_ref_v001.asc" "${SRCVIEW}/runner_ref_v001.asc"
ln -sfn "${ROOT}/local_types.h" "${SRCVIEW}/local_types.h"
ln -sfn "${ROOT}/ASYNC-TRIPLE-X-SEED.asc" "${SRCVIEW}/ASYNC-TRIPLE-X-SEED.asc"
ln -sfn "${ROOT}/ASYNC-TRIPLE-X-V001-submission.asc" "${SRCVIEW}/ASYNC-TRIPLE-X-V001-submission.asc"

cd "${BUILD}"
cmake "${SRCVIEW}" >"${LOGDIR}/ref_cfg_${STAMP}.log" 2>&1 || {
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
ls -la "${BUILD}"/async_ref_seed_probe "${BUILD}"/async_ref_v001_probe

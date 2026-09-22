#!/usr/bin/env bash
# I001 V001 build on cann-server3 (Ascend910B3 / dav-2201).
set -eo pipefail

ROOT="$(cd "$(dirname "$0")" && pwd)"
LOGDIR="${ROOT}/logs"
BUILD="${ROOT}/build"
mkdir -p "${LOGDIR}"

TOOLKIT="${TOOLKIT:-/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002}"
set +u
# shellcheck disable=SC1091
source "${TOOLKIT}/bin/setenv.bash" >/dev/null 2>&1 || true
set -u

export ASCEND_HOME_PATH="${TOOLKIT}"
export CMAKE_PREFIX_PATH="${TOOLKIT}/aarch64-linux/tikcpp/ascendc_kernel_cmake:${CMAKE_PREFIX_PATH:-}"
export CPLUS_INCLUDE_PATH="/usr/include/c++/11:/usr/include/aarch64-linux-gnu/c++/11${CPLUS_INCLUDE_PATH:+:$CPLUS_INCLUDE_PATH}"
export C_INCLUDE_PATH="/usr/include:/usr/include/aarch64-linux-gnu${C_INCLUDE_PATH:+:$C_INCLUDE_PATH}"

STAMP="$(date +%Y%m%d_%H%M%S)"
DEV_LOG="${LOGDIR}/device_compile_${STAMP}.log"
SUB_LOG="${LOGDIR}/submission_compile_${STAMP}.log"
FULL_LOG="${LOGDIR}/full_link_${STAMP}.log"
SUMMARY="${LOGDIR}/build_summary_${STAMP}.log"

rm -rf "${BUILD}"
mkdir -p "${BUILD}"
cd "${BUILD}"

CMAKE_ARGS=(
  -DCMAKE_ASC_ARCHITECTURES=dav-2201
  -DSOC_VERSION=Ascend910B3
  -DCMAKE_PREFIX_PATH="${CMAKE_PREFIX_PATH}"
)

set +e
cmake "${ROOT}" "${CMAKE_ARGS[@]}" >"${LOGDIR}/cmake_configure_${STAMP}.log" 2>&1
CFG_RC=$?
cmake --build . --target v001_device -j"$(nproc)" >"${DEV_LOG}" 2>&1
DEV_RC=$?
cmake --build . --target v001_submission -j"$(nproc)" >"${SUB_LOG}" 2>&1
SUB_RC=$?
cmake --build . --target v001_full -j"$(nproc)" >"${FULL_LOG}" 2>&1
FULL_RC=$?
set -e

{
  echo "I001 V001 build summary ${STAMP}"
  echo "TOOLKIT=${TOOLKIT}"
  echo "SOC_VERSION=Ascend910B3"
  echo "ARCH=dav-2201"
  echo "cmake_configure_rc=${CFG_RC}"
  echo "device_compile_rc=${DEV_RC} log=${DEV_LOG}"
  echo "submission_compile_rc=${SUB_RC} log=${SUB_LOG}"
  echo "full_link_rc=${FULL_RC} log=${FULL_LOG}"
} | tee "${SUMMARY}"

if [[ ${DEV_RC} -eq 0 && ${SUB_RC} -eq 0 && ${FULL_RC} -eq 0 ]]; then
  find "${BUILD}" -type f \( -name '*.o' -o -name '*.d' -o -name '*.obj' \) -delete 2>/dev/null || true
  echo "CLEANUP=removed object intermediates under build/"
else
  echo "CLEANUP=kept build tree for diagnosis"
fi

exit $(( CFG_RC != 0 || DEV_RC != 0 || SUB_RC != 0 || FULL_RC != 0 ))

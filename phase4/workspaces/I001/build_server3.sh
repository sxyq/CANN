#!/usr/bin/env bash
# I001 build on cann-server3 (Ascend910B3 / dav-2201).
set -eo pipefail

ROOT="$(cd "$(dirname "$0")" && pwd)"
LOGDIR="${ROOT}/logs"
BUILD="${ROOT}/build"
mkdir -p "${LOGDIR}"

TOOLKIT="${TOOLKIT:-/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002}"
set +u
source "${TOOLKIT}/bin/setenv.bash" >/dev/null 2>&1 || true
set -u

export ASCEND_HOME_PATH="${TOOLKIT}"
export CMAKE_PREFIX_PATH="${TOOLKIT}/aarch64-linux/tikcpp/ascendc_kernel_cmake:${CMAKE_PREFIX_PATH:-}"
export CPLUS_INCLUDE_PATH="/usr/include/c++/11:/usr/include/aarch64-linux-gnu/c++/11${CPLUS_INCLUDE_PATH:+:$CPLUS_INCLUDE_PATH}"
export C_INCLUDE_PATH="/usr/include:/usr/include/aarch64-linux-gnu${C_INCLUDE_PATH:+:$C_INCLUDE_PATH}"

REV="${REV:-V005}"
SRC="${ROOT}/${REV}/kernel.asc"
STAMP="$(date +%Y%m%d_%H%M%S)"

rm -rf "${BUILD}"
mkdir -p "${BUILD}"
cd "${BUILD}"

set +e
cmake "${ROOT}" -DCMAKE_ASC_ARCHITECTURES=dav-2201 -DSOC_VERSION=Ascend910B3 \
  -DCMAKE_PREFIX_PATH="${CMAKE_PREFIX_PATH}" -DI001_REV="${REV}" \
  >"${LOGDIR}/cmake_configure_${REV}_${STAMP}.log" 2>&1
CFG_RC=$?
cmake --build . --target v003_device -j"$(nproc)" >"${LOGDIR}/device_compile_${REV}_${STAMP}.log" 2>&1
DEV_RC=$?
cmake --build . --target v003_submission -j"$(nproc)" >"${LOGDIR}/submission_compile_${REV}_${STAMP}.log" 2>&1
SUB_RC=$?
cmake --build . --target v003_full -j"$(nproc)" >"${LOGDIR}/full_link_${REV}_${STAMP}.log" 2>&1
FULL_RC=$?
set -e

{
  echo "I001 ${REV} build summary ${STAMP}"
  echo "SOURCE=${SRC}"
  echo "cmake_configure_rc=${CFG_RC}"
  echo "device_compile_rc=${DEV_RC}"
  echo "submission_compile_rc=${SUB_RC}"
  echo "full_link_rc=${FULL_RC}"
} | tee "${LOGDIR}/build_summary_${REV}_${STAMP}.log"

if [[ ${DEV_RC} -eq 0 && ${SUB_RC} -eq 0 && ${FULL_RC} -eq 0 ]]; then
  find "${BUILD}" -type f \( -name '*.o' -o -name '*.d' -o -name '*.obj' \) -delete 2>/dev/null || true
  echo "CLEANUP=ok"
else
  echo "CLEANUP=kept build tree"
fi

exit $(( CFG_RC != 0 || DEV_RC != 0 || SUB_RC != 0 || FULL_RC != 0 ))

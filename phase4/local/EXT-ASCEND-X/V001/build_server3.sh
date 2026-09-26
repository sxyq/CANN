#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TOOLKIT="${TOOLKIT:-/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002}"
BUILD="${ROOT}/build"
LOGS="${ROOT}/logs"
mkdir -p "${LOGS}"
source "${TOOLKIT}/bin/setenv.bash" >/dev/null 2>&1 || true
export ASCEND_HOME_PATH="${TOOLKIT}"
export CMAKE_PREFIX_PATH="${TOOLKIT}/aarch64-linux/tikcpp/ascendc_kernel_cmake${CMAKE_PREFIX_PATH:+:${CMAKE_PREFIX_PATH}}"
GCC_CXX_INCLUDE_PATHS="/usr/include/c++/11:/usr/include/aarch64-linux-gnu/c++/11:/usr/include"
export CPLUS_INCLUDE_PATH="${GCC_CXX_INCLUDE_PATHS}${CPLUS_INCLUDE_PATH:+:${CPLUS_INCLUDE_PATH}}"
export CPLUS_INCLUDE_PATH="/usr/include/c++/11:/usr/include/aarch64-linux-gnu/c++/11:/usr/include${CPLUS_INCLUDE_PATH:+:${CPLUS_INCLUDE_PATH}}"
STAMP="$(date +%Y%m%d_%H%M%S)"
CFG_LOG="${LOGS}/configure_${STAMP}.log"
DEV_LOG="${LOGS}/device_compile_${STAMP}.log"
SUB_LOG="${LOGS}/submission_compile_${STAMP}.log"
LINK_LOG="${LOGS}/full_link_${STAMP}.log"

{
  echo "ROUTE=EXT-ASCEND-X"
  echo "REVISION=V001"
  echo "SOURCE=${ROOT}/submission.asc"
  sha256sum "${ROOT}/submission.asc"
  echo "SERVER=$(hostname)"
  date -Is
} | tee "${LOGS}/build_context_${STAMP}.log"

cmake -S "${ROOT}" -B "${BUILD}" -DCMAKE_BUILD_TYPE=Release -DSOC_VERSION=Ascend910B3 \
  -DCMAKE_ASC_ARCHITECTURES=dav-2201 >"${CFG_LOG}" 2>&1
cmake --build "${BUILD}" --target ext_device -j2 >"${DEV_LOG}" 2>&1
cmake --build "${BUILD}" --target ext_submission -j2 >"${SUB_LOG}" 2>&1
cmake --build "${BUILD}" --target ext_correctness -j2 >"${LINK_LOG}" 2>&1
echo "DEVICE_COMPILE=PASS" | tee -a "${LOGS}/build_context_${STAMP}.log"
echo "SUBMISSION_COMPILE=PASS" | tee -a "${LOGS}/build_context_${STAMP}.log"
echo "FULL_LINK=PASS" | tee -a "${LOGS}/build_context_${STAMP}.log"

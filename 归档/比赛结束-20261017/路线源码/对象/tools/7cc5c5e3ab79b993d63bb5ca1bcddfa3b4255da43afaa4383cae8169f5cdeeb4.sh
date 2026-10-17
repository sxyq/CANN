#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
TOOLKIT="${ASCEND_CANN_PACKAGE_PATH:-/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002}"
BUILD_DIR="${ROOT}/build-server3-v008"
LOG_DIR="${ROOT}/logs"
STAMP="$(date -u +%Y%m%dT%H%M%SZ)"
LOG_FILE="${LOG_DIR}/server3-compile-${STAMP}.log"

mkdir -p "${LOG_DIR}"
source "${TOOLKIT}/set_env.sh" >/dev/null 2>&1 || true
export ASCEND_HOME_PATH="${TOOLKIT}"
export ASCEND_CANN_PACKAGE_PATH="${TOOLKIT}"
export CMAKE_PREFIX_PATH="${TOOLKIT}/aarch64-linux/tikcpp/ascendc_kernel_cmake${CMAKE_PREFIX_PATH:+:${CMAKE_PREFIX_PATH}}"
export CPLUS_INCLUDE_PATH="/usr/include/c++/11:/usr/include/aarch64-linux-gnu/c++/11${CPLUS_INCLUDE_PATH:+:${CPLUS_INCLUDE_PATH}}"

set +e
cmake -S "${ROOT}" -B "${BUILD_DIR}" \
    -DCMAKE_BUILD_TYPE=Release \
    -DASCEND_CANN_PACKAGE_PATH="${TOOLKIT}" \
    -DSOC_VERSION=Ascend910B3 \
    -DASCEND_ARCH=dav-2201 >"${LOG_FILE}" 2>&1
CONFIGURE_RC=$?
BUILD_RC=125
if [[ ${CONFIGURE_RC} -eq 0 ]]; then
    cmake --build "${BUILD_DIR}" \
        --target adaptive_parent adaptive_candidate adaptive_probe \
        --parallel 1 >>"${LOG_FILE}" 2>&1
    BUILD_RC=$?
fi
set -e

printf 'CONFIGURE_RC=%s\nBUILD_RC=%s\nBUILD_LOG=%s\n' \
    "${CONFIGURE_RC}" "${BUILD_RC}" "${LOG_FILE}"
tail -n 100 "${LOG_FILE}"
if [[ ${CONFIGURE_RC} -ne 0 || ${BUILD_RC} -ne 0 ]]; then
    exit 1
fi

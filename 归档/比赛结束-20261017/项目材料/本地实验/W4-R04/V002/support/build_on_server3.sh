#!/usr/bin/env bash
set -euo pipefail

R04_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
R04_TOOLKIT="${ASCEND_CANN_PACKAGE_PATH:-/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002}"
R04_BUILD_DIR="${R04_ROOT}/build-server3-v002"
R04_LOG_DIR="${R04_ROOT}/logs"
R04_STAMP="$(date -u +%Y%m%dT%H%M%SZ)-$"
R04_LOG_FILE="${R04_LOG_DIR}/server3-compile-${R04_STAMP}.log"

mkdir -p "${R04_LOG_DIR}"
source "${R04_TOOLKIT}/set_env.sh" >/dev/null 2>&1 || true
export ASCEND_HOME_PATH="${R04_TOOLKIT}"
export ASCEND_CANN_PACKAGE_PATH="${R04_TOOLKIT}"
export CMAKE_PREFIX_PATH="${R04_TOOLKIT}/aarch64-linux/tikcpp/ascendc_kernel_cmake${CMAKE_PREFIX_PATH:+:${CMAKE_PREFIX_PATH}}"
export CPLUS_INCLUDE_PATH="/usr/include/c++/11:/usr/include/aarch64-linux-gnu/c++/11${CPLUS_INCLUDE_PATH:+:${CPLUS_INCLUDE_PATH}}"

set +e
cmake -S "${R04_ROOT}" -B "${R04_BUILD_DIR}" \
    -DCMAKE_BUILD_TYPE=Release \
    -DASCEND_CANN_PACKAGE_PATH="${R04_TOOLKIT}" \
    -DSOC_VERSION=Ascend910B3 \
    -DASCEND_ARCH=dav-2201 >"${R04_LOG_FILE}" 2>&1
R04_CONFIGURE_RC=$?
R04_BUILD_RC=125
if [[ ${R04_CONFIGURE_RC} -eq 0 ]]; then
    cmake --build "${R04_BUILD_DIR}" \
        --target r04_parent r04_candidate r04_probe \
        --parallel 1 >>"${R04_LOG_FILE}" 2>&1
    R04_BUILD_RC=$?
fi
set -e

printf 'R04_CONFIGURE_RC=%s\nBUILD_RC=%s\nBUILD_LOG=%s\n' \
    "${R04_CONFIGURE_RC}" "${R04_BUILD_RC}" "${R04_LOG_FILE}"
tail -n 100 "${R04_LOG_FILE}"
if [[ ${R04_CONFIGURE_RC} -ne 0 || ${R04_BUILD_RC} -ne 0 ]]; then
    exit 1
fi

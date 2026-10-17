#!/usr/bin/env bash
set -eo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
if [[ -z "${ASCEND_HOME_PATH:-}" ]]; then
  ASCEND_HOME_PATH="/usr/local/Ascend/ascend-toolkit/latest"
fi
export ASCEND_HOME_PATH
export CPLUS_INCLUDE_PATH="${CPLUS_INCLUDE_PATH:-/usr/include/c++/11:/usr/include/aarch64-linux-gnu/c++/11:/usr/include/c++/11/backward:/usr/include/aarch64-linux-gnu}"
ENV_SCRIPT="${CANN_ENV_SCRIPT:-/usr/local/Ascend/ascend-toolkit/set_env.sh}"
[[ -f "${ENV_SCRIPT}" ]] || { echo "CANN environment script not found: ${ENV_SCRIPT}" >&2; exit 1; }
source "${ENV_SCRIPT}"
BUILD_DIR="${ROOT}/build"
mkdir -p "${BUILD_DIR}"
cmake -S "${ROOT}" -B "${BUILD_DIR}" -DCMAKE_BUILD_TYPE=Release
cmake --build "${BUILD_DIR}" --parallel 4
"${BUILD_DIR}/row_copy_probe" --host-probe

if [[ "${1:-}" == "--device-probes" ]]; then
  DEVICE_ID="${ASCEND_DEVICE_ID:-0}"
  "${BUILD_DIR}/row_copy_probe" --device-probe 2 256 "${DEVICE_ID}"
  "${BUILD_DIR}/row_copy_probe" --device-probe 5 4096 "${DEVICE_ID}"
fi

#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CANN_ROOT="${CANN_ROOT:-/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002}"
BUILD_DIR="${ROOT}/build-server3"

if [[ ! -f "${CANN_ROOT}/set_env.sh" ]]; then
    echo "missing CANN environment at ${CANN_ROOT}" >&2
    exit 2
fi
export ASCEND_HOME_PATH="${CANN_ROOT}"
source "${CANN_ROOT}/set_env.sh"
mkdir -p "${BUILD_DIR}"
cmake -S "${ROOT}" -B "${BUILD_DIR}" -DCMAKE_BUILD_TYPE=Release -DASCEND_HOME_PATH="${CANN_ROOT}"
cmake --build "${BUILD_DIR}" --parallel 4

#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CANN_ROOT="${CANN_ROOT:-/usr/local/Ascend/ascend-toolkit/latest}"
BUILD_DIR="${BUILD_DIR:-${SCRIPT_DIR}/build}"

mkdir -p "${BUILD_DIR}"

if [[ ! -x "${CANN_ROOT}/bin/bisheng" ]]; then
    echo "bisheng not found under CANN_ROOT=${CANN_ROOT}" >&2
    exit 2
fi

"${CANN_ROOT}/bin/bisheng" \
    --cce-aicore-only \
    --cce-soc-version=Ascend910B3 \
    --asc-aicore-lang \
    -x cce \
    -I"${CANN_ROOT}/include" \
    -I"${CANN_ROOT}/include/ascendc/basic_api" \
    -c "${SCRIPT_DIR}/kernel.txt" \
    -o "${BUILD_DIR}/kernel.o"

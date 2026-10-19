#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
OUT_DIR="${SCRIPT_DIR}/build"
ASCEND_HOME_PATH="${ASCEND_HOME_PATH:-/home/data4t2/lelinfeng/cann_game/.toolchains/cann-9.0/cann-9.0.0}"
COMPILER="${ASCEND_HOME_PATH}/aarch64-linux/bin/bisheng"

if [[ ! -x "${COMPILER}" ]]; then
    echo "bisheng not found: ${COMPILER}" >&2
    exit 2
fi

mkdir -p "${OUT_DIR}"
"${COMPILER}" -x cce -c "${SCRIPT_DIR}/kernel.txt" \
    -DFULL_R023_AICORE_COMPILE=1 \
    --cce-aicore-arch=dav-c220 -std=c++17 \
    -I"${ASCEND_HOME_PATH}/aarch64-linux/include" \
    -I"${ASCEND_HOME_PATH}/aarch64-linux/include/ascendc/basic_api" \
    -I"${ASCEND_HOME_PATH}/aarch64-linux/ascendc/include" \
    -I"${ASCEND_HOME_PATH}/aarch64-linux/asc/include" \
    -I"${ASCEND_HOME_PATH}/aarch64-linux/asc" \
    -I"${ASCEND_HOME_PATH}/aarch64-linux/asc/include/basic_api" \
    -I"${ASCEND_HOME_PATH}/aarch64-linux/asc/include/interface" \
    -isystem /usr/include/c++/11 \
    -isystem /usr/include/aarch64-linux-gnu/c++/11 \
    -isystem /usr/include/c++/11/backward \
    -I"${ASCEND_HOME_PATH}/aarch64-linux/asc/impl" \
    -I"${ASCEND_HOME_PATH}/aarch64-linux/asc/impl/basic_api" \
    -o "${OUT_DIR}/kernel.o"

echo "compiled ${OUT_DIR}/kernel.o"

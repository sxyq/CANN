#!/bin/bash
set -eo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "${SCRIPT_DIR}"

TOOLKIT_ROOT="${ASCEND_HOME_PATH:-/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002}"
COMPILER="${TOOLKIT_ROOT}/aarch64-linux/ccec_compiler/bin/bisheng"
set +u
source "${TOOLKIT_ROOT}/aarch64-linux/script/set_env.sh"
set -u

cmake -S . -B build-server3 \
    -DCMAKE_CXX_COMPILER="${COMPILER}" \
    -DCMAKE_BUILD_TYPE=Release
echo 'STAGE=device compile + .alink'
cmake --build build-server3 --target async_triple_device -j4
echo 'STAGE=submission compile + .alink'
cmake --build build-server3 --target async_triple_submission -j4
echo 'STAGE=full link validation'
cmake --build build-server3 --target async_triple_full_link -j4

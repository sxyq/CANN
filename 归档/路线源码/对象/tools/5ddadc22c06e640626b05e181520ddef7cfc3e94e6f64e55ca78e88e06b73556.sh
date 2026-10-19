#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
export ASCEND_HOME_PATH="/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002"
export LD_LIBRARY_PATH="${LD_LIBRARY_PATH:-}"
export PYTHONPATH="${PYTHONPATH:-}"
set +u
source "${ASCEND_HOME_PATH}/bin/setenv.bash" ""
set -u
cmake -S "${ROOT}" -B "${ROOT}/build" -DCMAKE_BUILD_TYPE=Release
cmake --build "${ROOT}/build" --parallel 4

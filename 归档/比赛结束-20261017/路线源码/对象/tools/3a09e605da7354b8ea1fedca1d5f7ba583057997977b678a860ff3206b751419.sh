#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TOOLKIT="/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002"
source "${TOOLKIT}/aarch64-linux/script/set_env.sh"
cmake -S "${ROOT}" -B "${ROOT}/build" -DCMAKE_BUILD_TYPE=Release
cmake --build "${ROOT}/build" --parallel 4

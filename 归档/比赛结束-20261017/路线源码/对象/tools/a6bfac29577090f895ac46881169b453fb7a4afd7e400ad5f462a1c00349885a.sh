#!/usr/bin/env bash
set -euo pipefail

route_root="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
cann_home="/usr/local/Ascend/ascend-toolkit/latest"
hcc_root="${cann_home}/toolkit/toolchain/hcc/aarch64-target-linux-gnu/include/c++/7.3.0"

if [[ "${1:-}" != "build" ]]; then
    printf 'usage: %s build\n' "$0" >&2
    exit 64
fi

set +u
source /usr/local/Ascend/ascend-toolkit/set_env.sh
set -u
export ASCEND_HOME_PATH="$cann_home"
export CPLUS_INCLUDE_PATH="${hcc_root}:${hcc_root}/aarch64-target-linux-gnu:${hcc_root}/backward"

mkdir -p "${route_root}/results"
build_log="${route_root}/results/build-$(date +%Y%m%dT%H%M%S%z).log"
printf 'build log: %s\n' "$build_log"

set +e
cmake -S "${route_root}/phase4/workspaces/R31B" -B "${route_root}/build" 2>&1 | tee "$build_log"
status=${PIPESTATUS[0]}
if (( status != 0 )); then
    exit "$status"
fi

cmake --build "${route_root}/build" --target r31b_submission_v011 -j2 2>&1 | tee -a "$build_log"
status=${PIPESTATUS[0]}
exit "$status"

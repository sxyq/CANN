#!/usr/bin/env bash
set -eo pipefail

if [[ "${1:-}" != "build" || "$#" -ne 1 ]]; then
    printf 'usage: %s build\n' "$0" >&2
    exit 2
fi

worktree_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cann_root="${ASCEND_CANN_PACKAGE_PATH:-/usr/local/Ascend/ascend-toolkit/latest}"
hcc_root="${cann_root}/toolkit/toolchain/hcc/aarch64-target-linux-gnu/include/c++/7.3.0"
log_path="${worktree_root}/results/build-current.log"

export LD_LIBRARY_PATH="${LD_LIBRARY_PATH:-}"
source /usr/local/Ascend/ascend-toolkit/set_env.sh
export CPLUS_INCLUDE_PATH="${hcc_root}:${hcc_root}/aarch64-target-linux-gnu:${hcc_root}/backward${CPLUS_INCLUDE_PATH:+:${CPLUS_INCLUDE_PATH}}"

mkdir -p "${worktree_root}/results"
cmake -S "${worktree_root}/phase4/workspaces/R31B" \
    -B "${worktree_root}/build" \
    -DCMAKE_BUILD_TYPE=Release \
    -DASCEND_CANN_PACKAGE_PATH="${cann_root}" 2>&1 | tee "${log_path}"
cmake --build "${worktree_root}/build" \
    --target r31b_submission_v011 --parallel 2 2>&1 | tee -a "${log_path}"
sha256sum "${worktree_root}/build/r31b_submission_v011" \
    | tee "${worktree_root}/results/build-artifact.sha256"

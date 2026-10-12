#!/usr/bin/env bash
set -eo pipefail

if [[ "${1:-}" != "build" || "$#" -ne 1 ]]; then
    printf 'usage: %s build\n' "$0" >&2
    exit 2
fi

worktree_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cann_root="${ASCEND_CANN_PACKAGE_PATH:-/usr/local/Ascend/ascend-toolkit/latest}"
hcc_root="${cann_root}/toolkit/toolchain/hcc/aarch64-target-linux-gnu/include/c++/7.3.0"
results_root="${worktree_root}/results"
run_number=1
while :; do
    run_id="$(printf '%03d' "${run_number}")"
    if [[ ! -e "${results_root}/run_${run_id}.json" &&
          ! -e "${results_root}/build_${run_id}.log" &&
          ! -e "${results_root}/build-artifact_${run_id}.sha256" ]]; then
        break
    fi
    run_number=$((run_number + 1))
done
log_path="${results_root}/build_${run_id}.log"
artifact_sha_path="${results_root}/build-artifact_${run_id}.sha256"

export LD_LIBRARY_PATH="${LD_LIBRARY_PATH:-}"
source /usr/local/Ascend/ascend-toolkit/set_env.sh
export CPLUS_INCLUDE_PATH="${hcc_root}:${hcc_root}/aarch64-target-linux-gnu:${hcc_root}/backward${CPLUS_INCLUDE_PATH:+:${CPLUS_INCLUDE_PATH}}"

mkdir -p "${results_root}"
cmake -S "${worktree_root}/phase4/workspaces/R31B" \
    -B "${worktree_root}/build" \
    -DCMAKE_BUILD_TYPE=Release \
    -DASCEND_CANN_PACKAGE_PATH="${cann_root}" 2>&1 | tee "${log_path}"
cmake --build "${worktree_root}/build" \
    --target r31b_submission_v011 --parallel 2 2>&1 | tee -a "${log_path}"
sha256sum "${worktree_root}/build/r31b_submission_v011" \
    | tee "${artifact_sha_path}"

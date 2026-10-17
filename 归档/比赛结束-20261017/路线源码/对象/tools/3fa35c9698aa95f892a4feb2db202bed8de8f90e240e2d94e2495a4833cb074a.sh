#!/usr/bin/env bash
set -o pipefail

if [[ "${1:-}" != "build" || "$#" -ne 1 ]]; then
    printf 'usage: %s build\n' "$0" >&2
    exit 2
fi

worktree="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
project_dir="$worktree/phase4/workspaces/R31B"
toolkit="${ASCEND_CANN_PACKAGE_PATH:-/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002}"
build_dir="${TMPDIR:-/tmp}/n10-row-core-balance-build"
results_dir="$worktree/results"
mkdir -p "$results_dir"
timestamp="$(date -u +%Y%m%dT%H%M%S%NZ)"
log_file="$results_dir/build-${timestamp}.log"
exec > >(tee "$log_file") 2>&1

build_rc=0
printf 'ROUTE=N10\n'
printf 'UTC=%s\n' "$timestamp"
printf 'WORKTREE=%s\n' "$worktree"
printf 'BRANCH=%s\n' "$(git -C "$worktree" branch --show-current)"
printf 'PARENT_OR_HEAD=%s\n' "$(git -C "$worktree" rev-parse HEAD)"
printf 'TOOLKIT=%s\n' "$toolkit"
printf 'COMMAND=cmake -S %q -B %q -DASCEND_CANN_PACKAGE_PATH=%q && cmake --build %q --target r31b_submission_v011 --parallel 2\n' \
    "$project_dir" "$build_dir" "$toolkit" "$build_dir"
printf 'SOURCE_SHA256\n'
sha256sum "$project_dir/R31B-V011-LP-ROW-PIPELINE_kernel.asc" \
    "$project_dir/compile_adapter_v011.asc" "$project_dir/CMakeLists.txt"

if [[ ! -x "$toolkit/compiler/bin/setenv.bash" ]]; then
    printf 'ERROR=missing CANN compiler environment script\n'
    build_rc=127
else
    source "$toolkit/compiler/bin/setenv.bash"
    export ASCEND_HOME_PATH="$toolkit"
    stdlib_root="$toolkit/toolkit/toolchain/hcc/aarch64-target-linux-gnu/include/c++/7.3.0"
    export CPLUS_INCLUDE_PATH="$stdlib_root:$stdlib_root/aarch64-target-linux-gnu:$stdlib_root/backward${CPLUS_INCLUDE_PATH:+:$CPLUS_INCLUDE_PATH}"
    cmake -S "$project_dir" -B "$build_dir" \
        -DASCEND_CANN_PACKAGE_PATH="$toolkit" || build_rc=$?
    if [[ "$build_rc" -eq 0 ]]; then
        cmake --build "$build_dir" --target r31b_submission_v011 --parallel 2 || build_rc=$?
    fi
fi

if [[ "$build_rc" -eq 0 ]]; then
    artifact="$build_dir/r31b_submission_v011"
    if [[ -f "$artifact" ]]; then
        printf 'ARTIFACT_SHA256\n'
        sha256sum "$artifact"
    else
        printf 'ERROR=successful build did not produce expected artifact: %s\n' "$artifact"
        build_rc=1
    fi
fi

printf 'BUILD_RC=%s\n' "$build_rc"
exit "$build_rc"

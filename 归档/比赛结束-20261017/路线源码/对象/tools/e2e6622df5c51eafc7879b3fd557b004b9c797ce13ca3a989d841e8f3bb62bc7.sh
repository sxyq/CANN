#!/usr/bin/env bash
set -uo pipefail

ROOT_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)
WORK_DIR="$ROOT_DIR/phase4/workspaces/R31B"
BUILD_DIR="$ROOT_DIR/build/n09-v011"
RESULTS_DIR="$ROOT_DIR/results"
LOG_FILE="$RESULTS_DIR/build.log"
STATE_FILE="$RESULTS_DIR/build_state.txt"
TARGET="r31b_submission_v011"
CANN_ROOT="${ASCEND_CANN_PACKAGE_PATH:-/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002}"
export ASCEND_HOME_PATH="$CANN_ROOT"

mkdir -p "$RESULTS_DIR"

record_state() {
    local status="$1"
    local state_path="$STATE_FILE"
    case "$status" in
        correctness_*) state_path="$RESULTS_DIR/correctness_state.txt" ;;
        local_*) state_path="$RESULTS_DIR/local_state.txt" ;;
    esac
    {
        printf 'status=%s\n' "$status"
        printf 'branch=%s\n' "$(git -C "$ROOT_DIR" branch --show-current)"
        printf 'parent_or_source_commit=%s\n' "$(git -C "$ROOT_DIR" rev-parse HEAD)"
        printf 'kernel_sha256='
        sha256sum "$WORK_DIR/R31B-V011-LP-ROW-PIPELINE_kernel.asc" | awk '{print $1}'
        printf 'adapter_sha256='
        sha256sum "$WORK_DIR/compile_adapter_v011.asc" | awk '{print $1}'
        printf 'cmake_sha256='
        sha256sum "$WORK_DIR/CMakeLists.txt" | awk '{print $1}'
        printf 'isolation_event_scan_path=/tmp/bisheng-plugin-output-*.json\n'
        printf 'isolation_event_observation=other_worktree_absolute_paths_in_shared_build_metadata\n'
        printf 'isolation_event_source_or_experiment_read=none\n'
        printf 'isolation_event_followup=stopped_shared_tmp_scan_and_continued_only_in_n09_worktree\n'
        printf 'artifact='
        if [[ -f "$BUILD_DIR/$TARGET" ]]; then
            printf '%s\n' "$BUILD_DIR/$TARGET"
            printf 'artifact_sha256='
            sha256sum "$BUILD_DIR/$TARGET" | awk '{print $1}'
        else
            printf 'missing\nartifact_sha256=missing\n'
        fi
    } > "$state_path"
}

case "${1:-}" in
    build)
        exec > >(tee "$LOG_FILE") 2>&1
        printf 'entry=%s build\n' "$ROOT_DIR/tools/run.sh"
        printf 'environment_script=%s\n' "$CANN_ROOT/aarch64-linux/script/set_env.sh"
        export LD_LIBRARY_PATH="${LD_LIBRARY_PATH:-}"
        export PYTHONPATH="${PYTHONPATH:-}"
        source "$CANN_ROOT/aarch64-linux/script/set_env.sh"
        environment_rc=$?
        printf 'environment_return_code=%s\n' "$environment_rc"
        if [[ "$environment_rc" -ne 0 ]]; then
            record_state "environment_failed"
            exit "$environment_rc"
        fi
        export ASCEND_HOME_PATH="$CANN_ROOT"
        HCC_CXX_INCLUDE="$CANN_ROOT/toolkit/toolchain/hcc/aarch64-target-linux-gnu/include/c++/7.3.0"
        HCC_INCLUDE_PATHS="$HCC_CXX_INCLUDE:$HCC_CXX_INCLUDE/aarch64-target-linux-gnu:$HCC_CXX_INCLUDE/backward"
        export CPLUS_INCLUDE_PATH="$HCC_INCLUDE_PATHS${CPLUS_INCLUDE_PATH:+:$CPLUS_INCLUDE_PATH}"
        export CPATH="$HCC_INCLUDE_PATHS${CPATH:+:$CPATH}"
        printf 'host_cxx_include_path=%s\n' "$HCC_INCLUDE_PATHS"
        printf 'configure_command=cmake -S %q -B %q -DCMAKE_BUILD_TYPE=Release\n' "$WORK_DIR" "$BUILD_DIR"
        cmake -S "$WORK_DIR" -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Release \
            -DASCEND_CANN_PACKAGE_PATH="$CANN_ROOT"
        configure_rc=$?
        printf 'configure_return_code=%s\n' "$configure_rc"
        if [[ "$configure_rc" -ne 0 ]]; then
            record_state "build_failed"
            exit "$configure_rc"
        fi

        printf 'build_command=cmake --build %q --target %q --parallel 2\n' "$BUILD_DIR" "$TARGET"
        cmake --build "$BUILD_DIR" --target "$TARGET" --parallel 2
        build_rc=$?
        printf 'build_return_code=%s\n' "$build_rc"
        if [[ "$build_rc" -eq 0 ]]; then
            record_state "build_passed"
        else
            record_state "build_failed"
        fi
        exit "$build_rc"
        ;;
    correctness|local)
        exec > >(tee "$RESULTS_DIR/${1}_not_run.log") 2>&1
        printf 'entry=%s %s\n' "$ROOT_DIR/tools/run.sh" "$1"
        printf 'status=NOT_EXECUTED\nreason=no route-local %s runner in sparse checkout; NPU devices are occupied\n' "$1"
        record_state "${1}_not_executed"
        printf 'return_code=125\n'
        exit 125
        ;;
    *)
        printf 'usage: %s {build|correctness|local}\n' "$0" >&2
        exit 2
        ;;
esac

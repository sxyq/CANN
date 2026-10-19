#!/usr/bin/env bash
set -eo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
WORKTREE_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"
SOURCE_DIR="${WORKTREE_ROOT}/phase4/workspaces/R31B"
BUILD_DIR="${SOURCE_DIR}/build-n07"

case "${1:-}" in
  build)
    source /usr/local/Ascend/ascend-toolkit/set_env.sh
    cmake -S "${SOURCE_DIR}" -B "${BUILD_DIR}"
    cmake --build "${BUILD_DIR}" --target r31b_submission_v011 -j2
    ;;
  correctness)
    printf '%s\n' "not_run: correctness harness is not present in this sparse worktree" >&2
    exit 3
    ;;
  local)
    printf '%s\n' "not_run: canonical Local runner and scorer are not present in this sparse worktree" >&2
    exit 3
    ;;
  *)
    printf '%s\n' "usage: $0 {build|correctness|local}" >&2
    exit 2
    ;;
esac

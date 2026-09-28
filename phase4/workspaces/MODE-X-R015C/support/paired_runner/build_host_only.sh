#!/usr/bin/env bash
set -eo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
INPUT_ROOT="${1:?usage: build_host_only.sh INPUT_ROOT NEW_BUILD_DIR}"
BUILD_DIR="${2:?usage: build_host_only.sh INPUT_ROOT NEW_BUILD_DIR}"
ENV_SCRIPT="${CANN_ENV_SCRIPT:-/usr/local/Ascend/ascend-toolkit/set_env.sh}"

[[ -f "$ENV_SCRIPT" ]] || { printf 'CANN environment file missing: %s\n' "$ENV_SCRIPT" >&2; exit 2; }
[[ -d "$INPUT_ROOT/parent" && -d "$INPUT_ROOT/candidate" ]] || { printf 'input root must contain parent/ and candidate/\n' >&2; exit 2; }
[[ ! -e "$BUILD_DIR" ]] || { printf 'build directory already exists: %s\n' "$BUILD_DIR" >&2; exit 2; }

source "$ENV_SCRIPT"
export CPLUS_INCLUDE_PATH="${CPLUS_INCLUDE_PATH:-/usr/include/c++/11:/usr/include/aarch64-linux-gnu/c++/11:/usr/include/c++/11/backward:/usr/include/aarch64-linux-gnu}"

cmake -S "$SCRIPT_DIR" -B "$BUILD_DIR" \
    -DCMAKE_BUILD_TYPE=Release \
    -DMODE_PARENT_DIR:PATH="$INPUT_ROOT/parent" \
    -DMODE_CANDIDATE_DIR:PATH="$INPUT_ROOT/candidate"
cmake --build "$BUILD_DIR" --target r015c_pair_runner --parallel 4
bash "$SCRIPT_DIR/test_host_only.sh" \
    "$BUILD_DIR/r015c_pair_runner" "$BUILD_DIR" \
    "$INPUT_ROOT/parent/submission.asc" "$INPUT_ROOT/parent/row_copy_tiling.h" \
    "$INPUT_ROOT/candidate/submission.asc" "$INPUT_ROOT/candidate/row_copy_tiling.h"

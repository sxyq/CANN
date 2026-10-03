#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")" && pwd)"
ASCEND_HOME_PATH="${ASCEND_HOME_PATH:-/usr/local/Ascend/ascend-toolkit/latest}"
EXPECTED_CANDIDATE_SHA="7098d7330fa1c29af8928be66c41eb48f0d2310c2d7746c7902c31e58cce25fb"
EXPECTED_PARENT_SHA="a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3"
SOURCE_COMMIT="fe4f61166c5fd356b216b32d1a8ac55b5d2f3275"

candidate_sha="$(sha256sum "$ROOT/../submission.asc" | awk '{print $1}')"
parent_sha="$(sha256sum "$ROOT/../parent.asc" | awk '{print $1}')"
if [[ "$candidate_sha" != "$EXPECTED_CANDIDATE_SHA" ||
      "$parent_sha" != "$EXPECTED_PARENT_SHA" ]]; then
  printf 'source identity mismatch: candidate=%s parent=%s\n' \
    "$candidate_sha" "$parent_sha" >&2
  exit 2
fi

export ASCEND_HOME_PATH
export PATH="$ASCEND_HOME_PATH/bin:$ASCEND_HOME_PATH/compiler/ccec_compiler/bin:$PATH"
set +u
source "$ASCEND_HOME_PATH/bin/setenv.bash" >/dev/null 2>&1 || true
set -u

HCC="$ASCEND_HOME_PATH/toolkit/toolchain/hcc/aarch64-target-linux-gnu/include/c++/7.3.0"
KIT="$ASCEND_HOME_PATH/aarch64-linux/tikcpp/ascendc_kernel_cmake"
export CPLUS_INCLUDE_PATH="$HCC:$HCC/aarch64-target-linux-gnu:$HCC/backward:${CPLUS_INCLUDE_PATH:-}"
export C_INCLUDE_PATH="$HCC:$HCC/aarch64-target-linux-gnu:${C_INCLUDE_PATH:-}"
export CMAKE_PREFIX_PATH="$KIT:${CMAKE_PREFIX_PATH:-}"
export ASC_DIR="$KIT"

BUILD_DIR="${BUILD_DIR:-$ROOT/build}"
LOG_DIR="${LOG_DIR:-$ROOT/logs}"
mkdir -p "$BUILD_DIR" "$LOG_DIR"

run_logged() {
  local log_path="$1"
  shift
  printf 'RUN:'
  printf ' %q' "$@"
  printf '\nLOG=%s\n' "$log_path"
  if "$@" >"$log_path" 2>&1; then
    printf 'RC=0\n'
  else
    local rc=$?
    printf 'RC=%s\n' "$rc"
    return "$rc"
  fi
}

run_logged "$LOG_DIR/cmake-configure.log" \
  cmake -S "$ROOT" -B "$BUILD_DIR" \
    -DCMAKE_MODULE_PATH="$KIT/ASC_CMake;$KIT" \
    -DASC_DIR="$KIT" -DCMAKE_PREFIX_PATH="$KIT" \
    -DSOC_VERSION=Ascend910B3 -DNPU_ARCH=dav-2201 || exit $?
run_logged "$LOG_DIR/build-submission-object.log" \
  cmake --build "$BUILD_DIR" --target selective_v001_submission -j2 || exit $?
run_logged "$LOG_DIR/build-parent-correctness.log" \
  cmake --build "$BUILD_DIR" --target selective_v001_parent_correctness -j2 || exit $?
run_logged "$LOG_DIR/build-candidate-correctness.log" \
  cmake --build "$BUILD_DIR" --target selective_v001_candidate_correctness -j2 || exit $?

printf 'SOURCE_COMMIT=%s\nCANDIDATE_SHA=%s\nPARENT_SHA=%s\n' \
  "$SOURCE_COMMIT" "$candidate_sha" "$parent_sha"
submission_object="$(find "$BUILD_DIR/CMakeFiles/selective_v001_submission.dir" \
  -maxdepth 1 -type f -name '*.o' -print -quit)"
if [[ -z "$submission_object" ]]; then
  printf 'submission object not found under %s\n' "$BUILD_DIR" >&2
  exit 3
fi
sha256sum "$submission_object" \
  "$BUILD_DIR/selective_v001_parent_correctness" \
  "$BUILD_DIR/selective_v001_candidate_correctness"

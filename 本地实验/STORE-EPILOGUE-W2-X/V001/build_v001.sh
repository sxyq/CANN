#!/usr/bin/env bash
set -uo pipefail

ROOT="$(cd "$(dirname "$0")" && pwd)"
BUILD_DIR="${BUILD_DIR:-$ROOT/build}"
LOG_DIR="${LOG_DIR:-$ROOT/logs}"
ASCEND_HOME_PATH="${ASCEND_HOME_PATH:-/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002}"
EXPECTED_CANDIDATE_SHA="48b9428dc2fc97c7c9d95f03ad8cec8e758c88e1197aa2328b1b1edebc018f88"
EXPECTED_PARENT_SHA="59fb8eada4da0b2b83cccffa4fb89fb97b7503ba3dcb08d5e0fe348e3caeb839"

candidate_sha="$(sha256sum "$ROOT/submission.asc" | awk '{print $1}')"
parent_sha="$(sha256sum "$ROOT/parent.asc" | awk '{print $1}')"
if [[ "$candidate_sha" != "$EXPECTED_CANDIDATE_SHA" ||
      "$parent_sha" != "$EXPECTED_PARENT_SHA" ]]; then
  printf 'source identity mismatch: candidate=%s parent=%s\n' \
    "$candidate_sha" "$parent_sha" >&2
  exit 2
fi

export ASCEND_HOME_PATH
export PATH="$ASCEND_HOME_PATH/bin:$ASCEND_HOME_PATH/aarch64-linux/ccec_compiler/bin:$PATH"
set +u
source "$ASCEND_HOME_PATH/bin/setenv.bash" >/dev/null 2>&1 || true
set -u

HCC="$ASCEND_HOME_PATH/toolkit/toolchain/hcc/aarch64-target-linux-gnu/include/c++/7.3.0"
KIT="$ASCEND_HOME_PATH/aarch64-linux/tikcpp/ascendc_kernel_cmake"
export CPLUS_INCLUDE_PATH="$HCC:$HCC/aarch64-target-linux-gnu:$HCC/backward:${CPLUS_INCLUDE_PATH:-}"
export C_INCLUDE_PATH="$HCC:$HCC/aarch64-target-linux-gnu:${C_INCLUDE_PATH:-}"
export CMAKE_PREFIX_PATH="$KIT:${CMAKE_PREFIX_PATH:-}"
export ASC_DIR="$KIT"

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
    -DASC_DIR="$KIT" \
    -DCMAKE_PREFIX_PATH="$KIT" \
    -DCMAKE_CXX_FLAGS="-I$HCC -I$HCC/aarch64-target-linux-gnu -I$HCC/backward" \
    -DCMAKE_C_FLAGS="-I$HCC -I$HCC/aarch64-target-linux-gnu" \
    -DSOC_VERSION=Ascend910B3 -DNPU_ARCH=dav-2201 || exit $?

run_logged "$LOG_DIR/build-submission-object.log" \
  cmake --build "$BUILD_DIR" --target store_v001_submission -j"$(nproc)" || exit $?

run_logged "$LOG_DIR/build-parent-correctness.log" \
  cmake --build "$BUILD_DIR" --target store_v001_parent_correctness -j"$(nproc)" || exit $?

run_logged "$LOG_DIR/build-candidate-correctness.log" \
  cmake --build "$BUILD_DIR" --target store_v001_candidate_correctness -j"$(nproc)" || exit $?

printf 'CANDIDATE_SHA=%s\nPARENT_SHA=%s\n' "$candidate_sha" "$parent_sha"
sha256sum "$BUILD_DIR/store_v001_parent_correctness" \
  "$BUILD_DIR/store_v001_candidate_correctness"

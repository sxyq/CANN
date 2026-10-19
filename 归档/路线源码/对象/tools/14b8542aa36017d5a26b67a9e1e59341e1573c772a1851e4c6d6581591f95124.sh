#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
SUPPORT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
V001_ROOT="$(cd "$SUPPORT_ROOT/.." && pwd)"
BUILD_DIR="${1:?pass a new build-repeatability-* directory}"
ASCEND_HOME_PATH="${ASCEND_HOME_PATH:-/usr/local/Ascend/ascend-toolkit/latest}"
EXPECTED_CANDIDATE_SHA="7098d7330fa1c29af8928be66c41eb48f0d2310c2d7746c7902c31e58cce25fb"
EXPECTED_PARENT_SHA="a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3"
BUILD_PARENT="${BUILD_DIR%/*}"
BUILD_NAME="${BUILD_DIR##*/}"

if [[ "$BUILD_PARENT" != "$SUPPORT_ROOT" ||
      "$BUILD_NAME" != build-repeatability-* ]]; then
  printf 'build path must be one new child of %s\n' "$SUPPORT_ROOT" >&2
  exit 2
fi
if [[ -e "$BUILD_DIR" || -L "$BUILD_DIR" ]]; then
  printf 'refusing existing build path: %s\n' "$BUILD_DIR" >&2
  exit 2
fi

candidate_sha="$(sha256sum "$V001_ROOT/submission.asc" | awk '{print $1}')"
parent_sha="$(sha256sum "$V001_ROOT/parent.asc" | awk '{print $1}')"
if [[ "$candidate_sha" != "$EXPECTED_CANDIDATE_SHA" ||
      "$parent_sha" != "$EXPECTED_PARENT_SHA" ]]; then
  printf 'source identity mismatch: candidate=%s parent=%s\n' \
    "$candidate_sha" "$parent_sha" >&2
  exit 2
fi

mkdir "$BUILD_DIR"
printf 'candidate_source_commit\tfe4f61166c5fd356b216b32d1a8ac55b5d2f3275\n' \
  > "$BUILD_DIR/source-identities.tsv"
printf 'candidate\t%s\nparent\t%s\n' "$candidate_sha" "$parent_sha" \
  >> "$BUILD_DIR/source-identities.tsv"

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

run_logged() {
  local log_path="$1"
  shift
  printf 'RUN:'
  printf ' %q' "$@"
  printf '\nLOG=%s\n' "$log_path"
  if "$@" > "$log_path" 2>&1; then
    cat "$log_path"
  else
    local rc=$?
    cat "$log_path"
    printf 'RC=%s\n' "$rc" >&2
    return "$rc"
  fi
}

run_logged "$BUILD_DIR/cmake-configure.log" \
  cmake -S "$SCRIPT_DIR" -B "$BUILD_DIR" \
    -DCMAKE_MODULE_PATH="$KIT/ASC_CMake;$KIT" \
    -DASC_DIR="$KIT" -DCMAKE_PREFIX_PATH="$KIT" \
    -DSOC_VERSION=Ascend910B3 -DNPU_ARCH=dav-2201
run_logged "$BUILD_DIR/build-parent.log" \
  cmake --build "$BUILD_DIR" --target selective_v001_repeatability_parent -j2
run_logged "$BUILD_DIR/build-candidate.log" \
  cmake --build "$BUILD_DIR" --target selective_v001_repeatability_candidate -j2

sha256sum \
  "$BUILD_DIR/selective_v001_repeatability_parent" \
  "$BUILD_DIR/selective_v001_repeatability_candidate" \
  "$BUILD_DIR/CMakeFiles/selective_v001_repeatability_parent.dir/repeatability_parent.asc.o" \
  "$BUILD_DIR/CMakeFiles/selective_v001_repeatability_candidate.dir/repeatability_candidate.asc.o" \
  > "$BUILD_DIR/executable-and-object-sha256.txt"
sha256sum \
  "$SCRIPT_DIR/CMakeLists.txt" \
  "$SCRIPT_DIR/build_repeatability.sh" \
  "$SCRIPT_DIR/run_repeatability.sh" \
  "$SCRIPT_DIR/repeatability_parent.asc" \
  "$SCRIPT_DIR/repeatability_candidate.asc" \
  "$SCRIPT_DIR/repeatability_main.inc" \
  "$SCRIPT_DIR/verify_cpu_golden.py" \
  > "$BUILD_DIR/support-source-sha256.txt"
cat "$BUILD_DIR/executable-and-object-sha256.txt"
cat "$BUILD_DIR/support-source-sha256.txt"

#!/usr/bin/env bash
set -uo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
SUPPORT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
BUILD_DIR="${1:?pass the newly created build-repeatability-* directory}"
RESULT_DIR="${2:?pass a new results/repeatability-* directory}"
DEVICE_ID="${3:?pass the Main-assigned device id}"
ASCEND_HOME_PATH="${ASCEND_HOME_PATH:-/usr/local/Ascend/ascend-toolkit/latest}"
BUILD_PARENT="${BUILD_DIR%/*}"
BUILD_NAME="${BUILD_DIR##*/}"
RESULT_PARENT="${RESULT_DIR%/*}"
RESULT_NAME="${RESULT_DIR##*/}"
RESULTS_ROOT="$SUPPORT_ROOT/results"
V001_ROOT="$(cd "$SUPPORT_ROOT/.." && pwd)"
EXPECTED_CANDIDATE_SHA="7098d7330fa1c29af8928be66c41eb48f0d2310c2d7746c7902c31e58cce25fb"
EXPECTED_PARENT_SHA="a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3"
EXPECTED_CANDIDATE_COMMIT="fe4f61166c5fd356b216b32d1a8ac55b5d2f3275"

if [[ "$BUILD_PARENT" != "$SUPPORT_ROOT" ||
      "$BUILD_NAME" != build-repeatability-* || -L "$BUILD_DIR" ||
      ! -x "$BUILD_DIR/selective_v001_repeatability_parent" ||
      ! -x "$BUILD_DIR/selective_v001_repeatability_candidate" ]]; then
  printf 'invalid or incomplete repeatability build directory: %s\n' "$BUILD_DIR" >&2
  exit 2
fi
if [[ ! -f "$BUILD_DIR/source-identities.tsv" ||
      ! -f "$BUILD_DIR/executable-and-object-sha256.txt" ||
      ! -f "$BUILD_DIR/support-source-sha256.txt" ]]; then
  printf 'missing exact-source build identity files under: %s\n' "$BUILD_DIR" >&2
  exit 2
fi
candidate_sha="$(sha256sum "$V001_ROOT/submission.asc" | awk '{print $1}')"
parent_sha="$(sha256sum "$V001_ROOT/parent.asc" | awk '{print $1}')"
if [[ "$candidate_sha" != "$EXPECTED_CANDIDATE_SHA" ||
      "$parent_sha" != "$EXPECTED_PARENT_SHA" ]] ||
   ! grep -Fqx $'candidate_source_commit\t'"$EXPECTED_CANDIDATE_COMMIT" "$BUILD_DIR/source-identities.tsv" ||
   ! grep -Fqx $'candidate\t'"$EXPECTED_CANDIDATE_SHA" "$BUILD_DIR/source-identities.tsv" ||
   ! grep -Fqx $'parent\t'"$EXPECTED_PARENT_SHA" "$BUILD_DIR/source-identities.tsv" ||
   ! sha256sum --check --status "$BUILD_DIR/executable-and-object-sha256.txt" ||
   ! sha256sum --check --status "$BUILD_DIR/support-source-sha256.txt"; then
  printf 'source identity mismatch: candidate=%s parent=%s\n' \
    "$candidate_sha" "$parent_sha" >&2
  exit 2
fi
if [[ "$RESULT_PARENT" != "$RESULTS_ROOT" ||
      "$RESULT_NAME" != repeatability-* || "$RESULT_NAME" == repeatability- ]]; then
  printf 'result path must be one new child of %s\n' "$RESULTS_ROOT" >&2
  exit 2
fi
if [[ ! "$DEVICE_ID" =~ ^[0-9]+$ ]]; then
  printf 'device id must be numeric: %s\n' "$DEVICE_ID" >&2
  exit 2
fi
if [[ -L "$RESULTS_ROOT" ]]; then
  printf 'refusing symlink result root: %s\n' "$RESULTS_ROOT" >&2
  exit 2
fi
mkdir -p "$RESULTS_ROOT"
if [[ "$(cd "$RESULTS_ROOT" && pwd -P)" != "$RESULTS_ROOT" ]]; then
  printf 'result root resolves outside its expected path: %s\n' "$RESULTS_ROOT" >&2
  exit 2
fi
if ! mkdir "$RESULT_DIR"; then
  printf 'refusing existing result path: %s\n' "$RESULT_DIR" >&2
  exit 2
fi

export LD_LIBRARY_PATH="$ASCEND_HOME_PATH/aarch64-linux/lib64${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
printf 'key\tvalue\n' > "$RESULT_DIR/identity.tsv"
printf 'device_id\t%s\nshape\t[2,16384]\ndtype\tFP32\n' "$DEVICE_ID" \
  >> "$RESULT_DIR/identity.tsv"
sha256sum \
  "$BUILD_DIR/selective_v001_repeatability_parent" \
  "$BUILD_DIR/selective_v001_repeatability_candidate" \
  > "$RESULT_DIR/executable-sha256.txt"
cp "$BUILD_DIR/source-identities.tsv" "$RESULT_DIR/source-identities.tsv"
cp "$BUILD_DIR/executable-and-object-sha256.txt" \
  "$RESULT_DIR/executable-and-object-sha256.txt"
cp "$BUILD_DIR/support-source-sha256.txt" "$RESULT_DIR/support-source-sha256.txt"

printf 'attempt\tvariant\trunner_rc\tcpu_golden_rc\tresult_dir\n' > "$RESULT_DIR/calls.tsv"
failed=0

run_one() {
  local attempt="$1"
  local variant="$2"
  local executable="$BUILD_DIR/selective_v001_repeatability_${variant}"
  local case_dir="$RESULT_DIR/${variant}_rep${attempt}"
  if ! mkdir "$case_dir"; then
    printf 'refusing existing invocation path: %s\n' "$case_dir" >&2
    failed=1
    return 0
  fi

  printf 'COMMAND=%s %s %s\n' "$executable" "$DEVICE_ID" "$case_dir" \
    > "$case_dir/command.txt"
  if "$executable" "$DEVICE_ID" "$case_dir" > "$case_dir/runner.log" 2>&1; then
    local rc=0
  else
    local rc=$?
  fi
  if python3 "$SCRIPT_DIR/verify_cpu_golden.py" "$case_dir" \
      > "$case_dir/cpu-golden.log" 2>&1; then
    local golden_rc=0
  else
    local golden_rc=$?
  fi

  shopt -s nullglob
  local artifacts=("$case_dir"/*.bin)
  if ((${#artifacts[@]})); then
    sha256sum "${artifacts[@]}" > "$case_dir/artifact-sha256.txt"
  else
    printf 'NO_BINARY_ARTIFACTS\n' > "$case_dir/artifact-sha256.txt"
  fi
  printf '%s\t%s\t%s\t%s\t%s\n' \
    "$attempt" "$variant" "$rc" "$golden_rc" "$case_dir" \
    >> "$RESULT_DIR/calls.tsv"
  printf 'CALL attempt=%s variant=%s rc=%s cpu_golden_rc=%s dir=%s\n' \
    "$attempt" "$variant" "$rc" "$golden_rc" "$case_dir"
  if [[ "$rc" -ne 0 || "$golden_rc" -ne 0 ]]; then
    failed=1
  fi
}

run_one 1 parent
run_one 1 candidate
run_one 2 parent
run_one 2 candidate

printf 'result_dir=%s\n' "$RESULT_DIR"
if [[ "$failed" -ne 0 ]]; then
  exit 1
fi
exit 0

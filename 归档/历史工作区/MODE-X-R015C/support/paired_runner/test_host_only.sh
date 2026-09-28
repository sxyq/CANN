#!/usr/bin/env bash
set -euo pipefail

if [[ $# -ne 6 ]]; then
    printf 'usage: test_host_only.sh RUNNER BUILD_DIR PARENT_SOURCE PARENT_TILING CANDIDATE_SOURCE CANDIDATE_TILING\n' >&2
    exit 2
fi

RUNNER="$1"
BUILD_DIR="$2"
PARENT_SOURCE="$3"
PARENT_TILING="$4"
CANDIDATE_SOURCE="$5"
CANDIDATE_TILING="$6"
EXPECTED_PARENT_SHA=e1786bec2673519f41887fdffa0e11751f515a81037d854c88ae7edc0e4af903
EXPECTED_PARENT_TILING_SHA=5e4ad750f5c63da357324c0a829674aff81b7c0b655e8ff08072b1c289c66c41
EXPECTED_CANDIDATE_SHA=9367db4ebb4edf6b7bf6cde97f4846f987e6ab230c2aeee44d942d5e881a1c74
EXPECTED_CANDIDATE_TILING_SHA=0939ba8498426fcd77645d826fcc45a8eb65c78996ad0ce1da1a00d1ac01a250

sha_of() { sha256sum "$1" | awk '{print $1}'; }
need_line() { grep -Fq "$2" <<< "$1" || { printf 'missing identity output: %s\n' "$2" >&2; exit 1; }; }
expect_usage_failure() {
    local label="$1"
    shift
    local output rc
    if output="$("$RUNNER" "$@" 2>&1)"; then
        printf 'unexpected success: %s\n%s\n' "$label" "$output" >&2
        exit 1
    else
        rc=$?
    fi
    [[ "$rc" -eq 64 ]] || { printf 'unexpected exit for %s: %s\n%s\n' "$label" "$rc" "$output" >&2; exit 1; }
    printf 'ARGUMENT_CASE PASS name=%s exit=%s\n' "$label" "$rc"
}

[[ -x "$RUNNER" ]] || { printf 'runner executable missing: %s\n' "$RUNNER" >&2; exit 1; }
[[ "$(sha_of "$PARENT_SOURCE")" == "$EXPECTED_PARENT_SHA" ]]
[[ "$(sha_of "$PARENT_TILING")" == "$EXPECTED_PARENT_TILING_SHA" ]]
[[ "$(sha_of "$CANDIDATE_SOURCE")" == "$EXPECTED_CANDIDATE_SHA" ]]
[[ "$(sha_of "$CANDIDATE_TILING")" == "$EXPECTED_CANDIDATE_TILING_SHA" ]]
[[ "$(sha_of "$BUILD_DIR/generated/parent/submission.asc")" == "$EXPECTED_PARENT_SHA" ]]
[[ "$(sha_of "$BUILD_DIR/generated/parent/row_copy_tiling.h")" == "$EXPECTED_PARENT_TILING_SHA" ]]
[[ "$(sha_of "$BUILD_DIR/generated/candidate/submission.asc")" == "$EXPECTED_CANDIDATE_SHA" ]]
[[ "$(sha_of "$BUILD_DIR/generated/candidate/row_copy_tiling.h")" == "$EXPECTED_CANDIDATE_TILING_SHA" ]]

IDENTITY="$("$RUNNER" --identity)"
need_line "$IDENTITY" "PARENT_SOURCE_SHA256=$EXPECTED_PARENT_SHA"
need_line "$IDENTITY" "PARENT_TILING_SHA256=$EXPECTED_PARENT_TILING_SHA"
need_line "$IDENTITY" "CANDIDATE_SOURCE_SHA256=$EXPECTED_CANDIDATE_SHA"
need_line "$IDENTITY" "CANDIDATE_TILING_SHA256=$EXPECTED_CANDIDATE_TILING_SHA"
need_line "$IDENTITY" "PROCESS_MODEL=SINGLE_PROCESS_SHARED_ACL_CONTEXT"
need_line "$IDENTITY" "NPU_ACCESS=NONE"
printf '%s\n' "$IDENTITY"

PARENT_OBJECT="$BUILD_DIR/CMakeFiles/r015c_parent_kernel.dir/generated/parent/parent_kernel.asc.o"
CANDIDATE_OBJECT="$BUILD_DIR/CMakeFiles/r015c_candidate_kernel.dir/generated/candidate/candidate_kernel.asc.o"
PARENT_LIBRARY="$BUILD_DIR/libr015c_parent_kernel.so"
CANDIDATE_LIBRARY="$BUILD_DIR/libr015c_candidate_kernel.so"
PARENT_DEP="$PARENT_OBJECT.d"
CANDIDATE_DEP="$CANDIDATE_OBJECT.d"
RUNNER_OBJECT="$BUILD_DIR/CMakeFiles/r015c_pair_runner.dir/op_host/paired_runner.asc.o"
for path in "$RUNNER_OBJECT" "$PARENT_OBJECT" "$CANDIDATE_OBJECT" \
            "$PARENT_LIBRARY" "$CANDIDATE_LIBRARY" "$PARENT_DEP" "$CANDIDATE_DEP"; do
    [[ -f "$path" ]] || { printf 'missing build identity file: %s\n' "$path" >&2; exit 1; }
done
grep -Fq "$BUILD_DIR/generated/parent/submission.asc" "$PARENT_DEP"
grep -Fq "$BUILD_DIR/generated/parent/row_copy_tiling.h" "$PARENT_DEP"
grep -Fq "$BUILD_DIR/generated/candidate/submission.asc" "$CANDIDATE_DEP"
grep -Fq "$BUILD_DIR/generated/candidate/row_copy_tiling.h" "$CANDIDATE_DEP"

for shape in '2 256' '5 4096' '3 8192'; do
    read -r rows cols <<< "$shape"
    PLAN="$("$RUNNER" --plan 4 "$rows" "$cols" 10 21 4)"
    need_line "$PLAN" "MODE=HOST_ONLY_PLAN device=4 rows=$rows D=$cols warmups_per_kernel=10 samples_per_block=21 pairs=4"
    need_line "$PLAN" 'QUALIFICATION=parent_same_binary blocks=2'
    need_line "$PLAN" 'PAIR_ORDER=PC,CP alternating_pairs=4'
    need_line "$PLAN" 'EXECUTION=NOT_RUN NPU_ACCESS=NONE TIMING=NOT_RUN'
    printf 'PLAN_CASE PASS rows=%s D=%s\n' "$rows" "$cols"
done

expect_usage_failure low_warmups --plan 4 2 256 9 21 4
expect_usage_failure low_samples --plan 4 2 256 10 20 4
expect_usage_failure low_pairs --plan 4 2 256 10 21 3
expect_usage_failure invalid_shape --plan 4 2 257 10 21 4
expect_usage_failure device_mode_guard --run HOST_ONLY 4 2 256 10 20 4

printf 'PARENT_OBJECT_SHA256=%s\n' "$(sha_of "$PARENT_OBJECT")"
printf 'CANDIDATE_OBJECT_SHA256=%s\n' "$(sha_of "$CANDIDATE_OBJECT")"
printf 'PARENT_LIBRARY_SHA256=%s\n' "$(sha_of "$PARENT_LIBRARY")"
printf 'CANDIDATE_LIBRARY_SHA256=%s\n' "$(sha_of "$CANDIDATE_LIBRARY")"
printf 'RUNNER_OBJECT_SHA256=%s\n' "$(sha_of "$RUNNER_OBJECT")"
printf 'PARENT_SOURCE_SHA256=%s\n' "$(sha_of "$PARENT_SOURCE")"
printf 'CANDIDATE_SOURCE_SHA256=%s\n' "$(sha_of "$CANDIDATE_SOURCE")"
printf 'RUNNER_EXECUTABLE_SHA256=%s\n' "$(sha_of "$RUNNER")"
printf 'IDENTITY_TEST=PASS ARGUMENT_TEST=PASS NPU_ACCESS=NOT_RUN TIMING=NOT_RUN\n'

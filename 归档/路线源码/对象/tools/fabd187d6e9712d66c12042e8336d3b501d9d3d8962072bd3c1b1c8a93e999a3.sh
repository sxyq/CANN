#!/usr/bin/env bash
set -uo pipefail

ROOT="$(cd "$(dirname "$0")" && pwd)"
DEVICE_ID="${DEVICE_ID:-5}"
RESULT_DIR="${RESULT_DIR:-$ROOT/logs/correctness-run-001}"
BUILD_DIR="${BUILD_DIR:-$ROOT/build}"
PARENT_BIN="$BUILD_DIR/store_v001_parent_correctness"
CANDIDATE_BIN="$BUILD_DIR/store_v001_candidate_correctness"
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

if [[ ! -x "$PARENT_BIN" || ! -x "$CANDIDATE_BIN" ]]; then
  echo "missing correctness binaries; build both CMake targets first" >&2
  exit 2
fi
if [[ -e "$RESULT_DIR" ]]; then
  echo "result directory already exists: $RESULT_DIR" >&2
  exit 2
fi

SUMMARY="$RESULT_DIR/correctness-summary.tsv"

SHAPES=(
  "1 1024 0" "1 4096 0" "2 4096 0" "1 8192 0" "2 8192 0"
  "1 16384 0" "1 32768 0" "3 6144 0"
  "1 256 1" "1 4096 1" "1 8192 1" "1 16384 1"
  "1 256 2" "1 4096 2" "1 8192 2" "1 32768 2"
  "1 100 0" "1 65 1" "33 100 0" "8 256 0" "2 256 0"
  "32 256 0" "17 257 1" "2 6144 0" "8 8192 0"
)

seen_tags=()
for shape in "${SHAPES[@]}"; do
  read -r rows width dtype <<< "$shape"
  tag="r${rows}_d${width}_t${dtype}"
  for seen_tag in "${seen_tags[@]}"; do
    if [[ "$seen_tag" == "$tag" ]]; then
      printf 'duplicate correctness case key: %s\n' "$tag" >&2
      exit 2
    fi
  done
  seen_tags+=("$tag")
done

mkdir -p "$RESULT_DIR"
printf 'rows\twidth\tdtype\tparent_rc\tcandidate_rc\tbyte_equal\tparent_bad\tcandidate_bad\tstatus\n' > "$SUMMARY"

failed=0
for shape in "${SHAPES[@]}"; do
  read -r rows width dtype <<< "$shape"
  tag="r${rows}_d${width}_t${dtype}"
  parent_prefix="$RESULT_DIR/parent_${tag}"
  candidate_prefix="$RESULT_DIR/candidate_${tag}"

  "$PARENT_BIN" "$DEVICE_ID" "$rows" "$width" "$dtype" \
    "${parent_prefix}.bin" "${parent_prefix}.tsv" \
    > "${parent_prefix}.log" 2>&1
  parent_rc=$?
  "$CANDIDATE_BIN" "$DEVICE_ID" "$rows" "$width" "$dtype" \
    "${candidate_prefix}.bin" "${candidate_prefix}.tsv" \
    > "${candidate_prefix}.log" 2>&1
  candidate_rc=$?

  parent_bad="NA"
  candidate_bad="NA"
  [[ -f "${parent_prefix}.tsv" ]] && parent_bad="$(awk -F '\t' 'NR == 2 {print $7}' "${parent_prefix}.tsv")"
  [[ -f "${candidate_prefix}.tsv" ]] && candidate_bad="$(awk -F '\t' 'NR == 2 {print $7}' "${candidate_prefix}.tsv")"
  byte_equal=no
  if [[ -f "${parent_prefix}.bin" && -f "${candidate_prefix}.bin" ]] && \
     cmp -s "${parent_prefix}.bin" "${candidate_prefix}.bin"; then
    byte_equal=yes
  fi

  status=FAIL
  if [[ "$parent_rc" == 0 && "$candidate_rc" == 0 && "$byte_equal" == yes &&
        "$parent_bad" == 0 && "$candidate_bad" == 0 ]]; then
    status=PASS
  elif [[ "$rows" == 1 && "$dtype" == 0 &&
          ( "$width" == 16384 || "$width" == 32768 ) &&
          "$parent_rc" == 3 && "$candidate_rc" == 3 &&
          "$byte_equal" == yes && "$parent_bad" != 0 && "$candidate_bad" != 0 ]]; then
    status=PARENT_GOLDEN_MISMATCH_OUTPUT_EQUAL
  fi

  printf '%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\n' \
    "$rows" "$width" "$dtype" "$parent_rc" "$candidate_rc" "$byte_equal" \
    "$parent_bad" "$candidate_bad" "$status" >> "$SUMMARY"
  if [[ "$status" == PASS || "$status" == PARENT_GOLDEN_MISMATCH_OUTPUT_EQUAL ]]; then
    if ! rm -f -- "${parent_prefix}.bin" "${candidate_prefix}.bin"; then
      printf 'failed to remove compared binary outputs for %s\n' "$tag" >&2
      failed=1
    fi
  else
    failed=1
  fi
done

echo "summary=$SUMMARY"
cat "$SUMMARY"
exit "$failed"

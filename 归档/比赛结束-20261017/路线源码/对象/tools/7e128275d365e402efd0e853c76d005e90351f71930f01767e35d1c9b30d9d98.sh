#!/usr/bin/env bash
set -uo pipefail

ROOT="$(cd "$(dirname "$0")" && pwd)"
DEVICE_ID="${DEVICE_ID:-2}"
RESULT_DIR="${RESULT_DIR:-$ROOT/logs/correctness-run-001}"
BUILD_DIR="${BUILD_DIR:-$ROOT/build}"
ASCEND_HOME_PATH="${ASCEND_HOME_PATH:-/usr/local/Ascend/ascend-toolkit/latest}"
if [[ -e "$RESULT_DIR" ]]; then
  printf 'result directory already exists: %s\n' "$RESULT_DIR" >&2
  exit 2
fi
export LD_LIBRARY_PATH="$ASCEND_HOME_PATH/aarch64-linux/lib64${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"

PARENT_BIN="$BUILD_DIR/selective_v001_parent_correctness"
CANDIDATE_BIN="$BUILD_DIR/selective_v001_candidate_correctness"
SUMMARY="$RESULT_DIR/correctness-summary.tsv"
mkdir -p "$RESULT_DIR"
printf 'rows\twidth\tdtype\tparent_rc\tcandidate_rc\tparent_max_abs\tcandidate_max_abs\tparent_bad\tcandidate_bad\tbyte_equal\tdispatch\ttile_width\ttile_count\tmergeRowRuns\tstatus\n' > "$SUMMARY"

failed=0
for shape in "8 16384 0" "1 32768 0" "1 16384 0" "2 16384 0"; do
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

  parent_max="$(awk -F '\t' 'NR == 2 {print $7}' "${parent_prefix}.tsv" 2>/dev/null)"
  candidate_max="$(awk -F '\t' 'NR == 2 {print $7}' "${candidate_prefix}.tsv" 2>/dev/null)"
  parent_bad="$(awk -F '\t' 'NR == 2 {print $8}' "${parent_prefix}.tsv" 2>/dev/null)"
  candidate_bad="$(awk -F '\t' 'NR == 2 {print $8}' "${candidate_prefix}.tsv" 2>/dev/null)"
  byte_equal=no
  if [[ -f "${parent_prefix}.bin" && -f "${candidate_prefix}.bin" ]] && \
     cmp -s "${parent_prefix}.bin" "${candidate_prefix}.bin"; then
    byte_equal=yes
  fi

  if [[ "$rows" == 8 && "$width" == 16384 ]] || \
     [[ "$rows" == 1 && ( "$width" == 32768 || "$width" == 16384 ) ]]; then
    dispatch=STORE_V003
    tile_width=4096
    merge=true
  else
    dispatch=R31B_V011
    tile_width=4096
    merge=false
  fi
  tile_count=$(( (width + tile_width - 1) / tile_width ))

  status=FAIL
  if [[ "$parent_rc" == 0 && "$candidate_rc" == 0 &&
        "$parent_bad" == 0 && "$candidate_bad" == 0 && "$byte_equal" == yes ]]; then
    status=PASS
  elif [[ "$parent_rc" == 3 && "$candidate_rc" == 3 &&
          "$parent_bad" == "$candidate_bad" && "$byte_equal" == yes ]]; then
    status=PARENT_GOLDEN_MISMATCH_OUTPUT_EQUAL
  fi

  printf '%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\n' \
    "$rows" "$width" "$dtype" "$parent_rc" "$candidate_rc" \
    "${parent_max:-NA}" "${candidate_max:-NA}" "${parent_bad:-NA}" \
    "${candidate_bad:-NA}" "$byte_equal" "$dispatch" "$tile_width" \
    "$tile_count" "$merge" "$status" >> "$SUMMARY"
  printf '%s\n' "--- ${rows}x${width} FP32: ${status}; Parent rc=${parent_rc}; Candidate rc=${candidate_rc}; dispatch=${dispatch}; tileWidth=${tile_width}; tileCount=${tile_count}; mergeRowRuns=${merge} ---"
  cat "${candidate_prefix}.log" 2>/dev/null

  if [[ "$status" == PASS || "$status" == PARENT_GOLDEN_MISMATCH_OUTPUT_EQUAL ]]; then
    rm -f -- "${parent_prefix}.bin" "${candidate_prefix}.bin"
  else
    failed=1
  fi
done

printf 'summary=%s\n' "$SUMMARY"
cat "$SUMMARY"
exit "$failed"

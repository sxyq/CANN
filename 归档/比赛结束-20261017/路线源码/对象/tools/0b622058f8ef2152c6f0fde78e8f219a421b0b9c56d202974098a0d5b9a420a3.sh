#!/usr/bin/env bash
set -euo pipefail
if [[ $# -ne 3 ]]; then
  echo "usage: $0 device_id lease_id run_id" >&2
  exit 2
fi
DEVICE="$1"
LEASE_ID="$2"
RUN_ID="$3"
if [[ ! "$DEVICE" =~ ^[0-7]$ || ! "$LEASE_ID" =~ ^[A-Za-z0-9._-]+$ || ! "$RUN_ID" =~ ^[A-Za-z0-9._-]+$ ]]; then
  echo "invalid device, lease id, or run id" >&2
  exit 2
fi
SUPPORT_DIR="$(cd "$(dirname "$0")" && pwd)"
REMOTE_ROOT="$(cd "$SUPPORT_DIR/.." && pwd)"
RUNNER="$REMOTE_ROOT/build-candidate/case47_timing_runner"
PARENT_SOURCE="$REMOTE_ROOT/parent/submission.asc"
CANDIDATE_SOURCE="$REMOTE_ROOT/candidate/submission.asc"
PARENT_MODULE="$REMOTE_ROOT/build-parent/libcase47_parent_kernel.so"
CANDIDATE_MODULE="$REMOTE_ROOT/build-candidate/libcase47_candidate_kernel.so"
RESULT_ROOT="$REMOTE_ROOT/results"
RESULT_DIR="$RESULT_ROOT/$RUN_ID"
MATRIX="$SUPPORT_DIR/timing-matrix.tsv"
ANALYZER="$SUPPORT_DIR/analyze_timing.py"
for required in "$RUNNER" "$PARENT_SOURCE" "$CANDIDATE_SOURCE" "$PARENT_MODULE" "$CANDIDATE_MODULE" "$MATRIX" "$ANALYZER"; do
  [[ -f "$required" ]] || { echo "missing required file: $required" >&2; exit 2; }
done
[[ -x "$RUNNER" ]] || { echo "runner is not executable: $RUNNER" >&2; exit 2; }
mkdir -p "$RESULT_ROOT"
mkdir "$RESULT_DIR"
mkdir -p "$RESULT_DIR/qualification" "$RESULT_DIR/parent-window/PRECHECK-A" \
  "$RESULT_DIR/parent-window/PRECHECK-B" "$RESULT_DIR/paired"
printf 'route\tCASE47-SMALL-CLUSTER-CHAMPION-X\nrevision\tV001\nrun_id\t%s\nlease_id\t%s\ndevice\t%s\nmethod\tACL_DEVICE_EVENT_PRIMARY_HOST_WALL_SECONDARY\nwarmups_per_process\t45\nsamples_per_process\t21\nsame_binary\tone_parent_process_two_blocks_gap_30s\nparent_window\tPRECHECK-A_6_fresh_processes_then_30s_then_PRECHECK-B_6_fresh_processes\npaired_blocks\t1_per_arm_process\npaired_order\tP,C;C,P;P,C;C,P\ninput_class\tPROXY\nofficial_case4_case7_mapping\tUNKNOWN\n' \
  "$RUN_ID" "$LEASE_ID" "$DEVICE" > "$RESULT_DIR/run-info.tsv"
sha256sum "$PARENT_SOURCE" "$CANDIDATE_SOURCE" "$PARENT_MODULE" "$CANDIDATE_MODULE" "$RUNNER" \
  > "$RESULT_DIR/sha256sums.txt"
ldd "$RUNNER" > "$RESULT_DIR/runner-linked-libraries.txt" 2>&1 || true
date -Is > "$RESULT_DIR/start.timestamp.txt"
npu-smi info > "$RESULT_DIR/npu-smi-start.txt" 2>&1
df -h /home/data4t2/lelinfeng/cann > "$RESULT_DIR/disk-start.txt" 2>&1
du -sh /home/data4t2/lelinfeng/cann/* > "$RESULT_DIR/project-usage-start.txt" 2>&1 || true
ps -eo pid,user,comm | awk 'tolower($3) ~ /vllm|python|acl|cann/ {print}' > "$RESULT_DIR/processes-start.txt"
A="$("$RUNNER" --query-cores "$DEVICE")"
[[ "$A" =~ ^[1-9][0-9]*$ ]] || { echo "invalid vector-core count: $A" >&2; exit 2; }
printf 'vector_cores\t%s\n' "$A" >> "$RESULT_DIR/run-info.tsv"
printf 'shape_id\tstage\tprocess_rep\tpid\tmodule\traw_path\trc\n' > "$RESULT_DIR/parent-processes.tsv"
printf 'shape_id\tpair\tarm\tpid\tmodule\traw_path\trc\n' > "$RESULT_DIR/pairs.tsv"
printf 'order\tshape_id\trole\tstage\tqualification\n' > "$RESULT_DIR/qualification/ordered-results.tsv"

run_one() {
  local stage="$1" rep="$2" shape_id="$3" module="$4" rows="$5" width="$6" dtype="$7" prefix="$8"
  local blocks gap rc pid arm
  if [[ "$stage" == "SAME_BINARY" ]]; then blocks=2; gap=30; else blocks=1; gap=0; fi
  date -Is > "$prefix.timestamp.txt"
  if "$RUNNER" "$module" "$DEVICE" "$rows" "$width" "$dtype" "$prefix" \
      "$stage" "$rep" 45 21 "$blocks" "$gap" > "$prefix.stdout" 2> "$prefix.stderr"; then
    rc=0
  else
    rc=$?
  fi
  printf '%s\n' "$rc" > "$prefix.rc"
  pid="$(awk -F '\t' '$1=="pid"{print $2}' "$prefix.stats.tsv" 2>/dev/null || true)"
  if [[ "$stage" == "PRECHECK-A" || "$stage" == "PRECHECK-B" ]]; then
    printf '%s\t%s\t%s\t%s\t%s\t%s.raw.tsv\t%s\n' \
      "$shape_id" "$stage" "$rep" "$pid" "$module" "$prefix" "$rc" >> "$RESULT_DIR/parent-processes.tsv"
  elif [[ "$stage" == "PAIR-P" || "$stage" == "PAIR-C" ]]; then
    if [[ "$stage" == "PAIR-P" ]]; then arm=P; else arm=C; fi
    printf '%s\t%s\t%s\t%s\t%s\t%s.raw.tsv\t%s\n' \
      "$shape_id" "$rep" "$arm" "$pid" "$module" "$prefix" "$rc" >> "$RESULT_DIR/pairs.tsv"
  fi
  return "$rc"
}

while IFS=$'\t' read -r order shape_id role dtype dtype_id width rows_rule input_class; do
  [[ "$order" == "order" ]] && continue
  if [[ "$rows_rule" == "2A" ]]; then rows=$((2 * A)); else rows="$A"; fi
  prefix="$RESULT_DIR/qualification/$shape_id.same-binary"
  if ! run_one SAME_BINARY 1 "$shape_id" "$PARENT_MODULE" "$rows" "$width" "$dtype_id" "$prefix"; then
    echo "same-binary runner failed for $shape_id; evidence retained in $RESULT_DIR" >&2
    exit 1
  fi
  same_status="$(python3 "$ANALYZER" same-binary --matrix "$MATRIX" --shape "$shape_id" \
      --raw "$prefix.raw.tsv" --out "$RESULT_DIR/qualification/$shape_id.same-binary.tsv")"
  printf '%s\t%s\t%s\tSAME_BINARY\t%s\n' "$order" "$shape_id" "$role" "$same_status" \
      >> "$RESULT_DIR/qualification/ordered-results.tsv"
  if [[ "$same_status" != "PASS" ]]; then
    printf '%s\tPRECHECK-A,PRECHECK-B,PAIRED\tSAME_BINARY_%s\n' "$shape_id" "$same_status" \
      >> "$RESULT_DIR/qualification/skipped.tsv"
    continue
  fi
  for stage in PRECHECK-A PRECHECK-B; do
    if [[ "$stage" == "PRECHECK-B" ]]; then sleep 30; fi
    for rep in 1 2 3 4 5 6; do
      prefix="$RESULT_DIR/parent-window/$stage/$shape_id.rep$(printf '%02d' "$rep")"
      if ! run_one "$stage" "$rep" "$shape_id" "$PARENT_MODULE" "$rows" "$width" "$dtype_id" "$prefix"; then
        echo "$stage runner failed for $shape_id rep $rep; evidence retained in $RESULT_DIR" >&2
        exit 1
      fi
    done
    status="$(python3 "$ANALYZER" parent-window --shape "$shape_id" --stage "$stage" \
        --index "$RESULT_DIR/parent-processes.tsv" \
        --out "$RESULT_DIR/qualification/$shape_id.$stage.tsv")"
    printf '%s\t%s\t%s\t%s\t%s\n' "$order" "$shape_id" "$role" "$stage" "$status" \
      >> "$RESULT_DIR/qualification/ordered-results.tsv"
  done
done < "$MATRIX"

while IFS=$'\t' read -r order shape_id role dtype dtype_id width rows_rule input_class; do
  [[ "$order" == "order" ]] && continue
  same_status="$(awk -F '\t' 'NR==2{print $NF}' "$RESULT_DIR/qualification/$shape_id.same-binary.tsv" 2>/dev/null || true)"
  a_status="$(awk -F '\t' 'NR==2{print $NF}' "$RESULT_DIR/qualification/$shape_id.PRECHECK-A.tsv" 2>/dev/null || true)"
  b_status="$(awk -F '\t' 'NR==2{print $NF}' "$RESULT_DIR/qualification/$shape_id.PRECHECK-B.tsv" 2>/dev/null || true)"
  [[ -n "$same_status" ]] || same_status=MISSING
  [[ -n "$a_status" ]] || a_status=MISSING
  [[ -n "$b_status" ]] || b_status=MISSING
  if [[ "$same_status" != "PASS" || "$a_status" != "PASS" || "$b_status" != "PASS" ]]; then
    printf '%s\tSKIPPED\tsame-binary=%s PRECHECK-A=%s PRECHECK-B=%s\n' \
      "$shape_id" "$same_status" "$a_status" "$b_status" >> "$RESULT_DIR/paired/skipped.tsv"
    continue
  fi
  if [[ "$rows_rule" == "2A" ]]; then rows=$((2 * A)); else rows="$A"; fi
  for pair in 1 2 3 4; do
    npu-smi info > "$RESULT_DIR/paired/$shape_id-pair-$pair-npu-smi-before.txt" 2>&1
    if (( pair % 2 == 1 )); then
      prefix="$RESULT_DIR/paired/$shape_id-pair-$pair-P"
      run_one PAIR-P "$pair" "$shape_id" "$PARENT_MODULE" "$rows" "$width" "$dtype_id" "$prefix" || exit 1
      prefix="$RESULT_DIR/paired/$shape_id-pair-$pair-C"
      run_one PAIR-C "$pair" "$shape_id" "$CANDIDATE_MODULE" "$rows" "$width" "$dtype_id" "$prefix" || exit 1
    else
      prefix="$RESULT_DIR/paired/$shape_id-pair-$pair-C"
      run_one PAIR-C "$pair" "$shape_id" "$CANDIDATE_MODULE" "$rows" "$width" "$dtype_id" "$prefix" || exit 1
      prefix="$RESULT_DIR/paired/$shape_id-pair-$pair-P"
      run_one PAIR-P "$pair" "$shape_id" "$PARENT_MODULE" "$rows" "$width" "$dtype_id" "$prefix" || exit 1
    fi
    npu-smi info > "$RESULT_DIR/paired/$shape_id-pair-$pair-npu-smi-after.txt" 2>&1
  done
done < "$MATRIX"

python3 "$ANALYZER" summarize --matrix "$MATRIX" --result-dir "$RESULT_DIR" \
  > "$RESULT_DIR/summary.stdout.txt"
npu-smi info > "$RESULT_DIR/npu-smi-end.txt" 2>&1
df -h /home/data4t2/lelinfeng/cann > "$RESULT_DIR/disk-end.txt" 2>&1
du -sh /home/data4t2/lelinfeng/cann/* > "$RESULT_DIR/project-usage-end.txt" 2>&1 || true
ps -eo pid,user,comm | awk 'tolower($3) ~ /vllm|python|acl|cann/ {print}' > "$RESULT_DIR/processes-end.txt"
date -Is > "$RESULT_DIR/end.timestamp.txt"
cat "$RESULT_DIR/summary.stdout.txt"

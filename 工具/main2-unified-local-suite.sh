#!/usr/bin/env bash

# Run the frozen Main-2 calibration set through one shared AscendC probe.
# This script only copies historical source packages into a fresh run root;
# it never edits Candidate source or shared ledgers.
set -u -o pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
TEMPLATE="$ROOT/归档/历史工作区/COEFF-LOCALITY-X"
SUITE="${SUITE:-$ROOT/研究/主代理/MAIN-2/MAIN2-UNIFIED-LOCAL-SUITE-V1.tsv}"
MANIFEST="${MANIFEST:-$ROOT/研究/主代理/MAIN-2/MAIN2-OFFICIAL-CALIBRATION-SET.tsv}"
ASCEND_HOME_PATH="${ASCEND_HOME_PATH:-/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002}"
DEVICE="${DEVICE:-7}"
WARMUP="${WARMUP:-20}"
SAMPLES="${SAMPLES:-11}"
RUNS="${RUNS:-3}"
RUN_ROOT="${RUN_ROOT:-$ROOT/研究/主代理/MAIN-2/calibration-v2/runs/$(date -u +%Y%m%dT%H%M%SZ)}"
ONLY="${ONLY:-ALL}"

export ASCEND_HOME_PATH
export PATH="$ASCEND_HOME_PATH/bin:$ASCEND_HOME_PATH/aarch64-linux/ccec_compiler/bin:$PATH"
set +u
source "$ASCEND_HOME_PATH/bin/setenv.bash" >/dev/null 2>&1 || true
set -u

KIT="$ASCEND_HOME_PATH/aarch64-linux/tikcpp/ascendc_kernel_cmake"
HCC="$ASCEND_HOME_PATH/toolkit/toolchain/hcc/aarch64-target-linux-gnu/include/c++/7.3.0"
export CPLUS_INCLUDE_PATH="$HCC:$HCC/aarch64-target-linux-gnu:$HCC/backward:${CPLUS_INCLUDE_PATH:-}"
export C_INCLUDE_PATH="$HCC:$HCC/aarch64-target-linux-gnu:${C_INCLUDE_PATH:-}"
export CMAKE_PREFIX_PATH="$KIT:${CMAKE_PREFIX_PATH:-}"
export ASC_DIR="$KIT"

mkdir -p "$RUN_ROOT/stages" "$RUN_ROOT/evidence"
printf 'run_root\t%s\ndevice\t%s\nwarmup\t%s\nsamples\t%s\nruns\t%s\n' \
  "$RUN_ROOT" "$DEVICE" "$WARMUP" "$SAMPLES" "$RUNS" > "$RUN_ROOT/run-config.tsv"
date -u +%Y-%m-%dT%H:%M:%SZ > "$RUN_ROOT/start-time.txt"
npu-smi info > "$RUN_ROOT/npu-smi-start.txt" 2>&1

MEASUREMENTS="$RUN_ROOT/measurements.tsv"
RAW="$RUN_ROOT/raw-samples.tsv"
META="$RUN_ROOT/version-status.tsv"
printf 'route\trevision\tcase_id\trun\tside\tdevice_median_us\tdevice_mean_us\tdevice_mad_us\tdevice_cv\twall_median_us\tbad\tcommand_rc\tdevice\n' > "$MEASUREMENTS"
printf 'route\trevision\tsource_sha\tcase_id\trun\tside\tdevice_us\twall_us\tdevice\n' > "$RAW"
printf 'route\trevision\tsource_sha\tofficial_score\tsource_path\tbuild_status\tcorrectness_status\tcases_attempted\tevidence_path\treason\n' > "$META"

copy_support_files() {
  local stage="$1"
  mkdir -p "$stage"
  cp "$TEMPLATE/CMakeLists.txt" "$stage/CMakeLists.txt"
  cp "$TEMPLATE/local_types.h" "$stage/local_types.h"
  cp "$TEMPLATE/main.asc" "$stage/main.asc"
  cp "$TEMPLATE/submission_shim.asc" "$stage/submission_shim.asc"
  cp "$TEMPLATE/runner_ref.inc" "$stage/runner_ref.inc"
  cp "$TEMPLATE/runner_parent.asc" "$stage/runner_parent.asc"
  cp "$TEMPLATE/runner_candidate.asc" "$stage/runner_candidate.asc"
  cp "$TEMPLATE/runner_ref_parent.asc" "$stage/runner_ref_parent.asc"
  cp "$TEMPLATE/runner_ref_candidate.asc" "$stage/runner_ref_candidate.asc"
  # The project-approved FP16 correctness tolerance is 0.004. The archived
  # generic runner used 0.001 and falsely rejected known-good wide FP16.
  sed -i 's/dtype == 1 ? 1.0e-3f : 2.0e-2f/dtype == 1 ? 4.0e-3f : 2.0e-2f/' "$stage/runner_ref.inc"
}

read_stat() {
  local file="$1" tag="$2" key="$3"
  awk -F '\t' -v t="$tag" -v k="$key" '$1 == t && $2 == k { print $3; found=1 } END { if (!found) print "NA" }' "$file"
}

read_summary() {
  local file="$1" key="$2"
  awk -F '\t' -v k="$key" '$1 == k { print $2; found=1 } END { if (!found) print "NA" }' "$file"
}

run_one_side() {
  local route="$1" rev="$2" source_sha="$3" stage="$4" case_id="$5" rows="$6" width="$7" dtype="$8" run="$9" side="${10}" order="${11}"
  local binary prefix raw_file stats_file rc
  if [[ "$side" == "parent" ]]; then
    binary="$stage/build/clx_ref_parent_probe"
  else
    binary="$stage/build/clx_ref_candidate_probe"
  fi
  local actual_sha
  actual_sha="$(sha256sum "$source" | awk '{print $1}')"
  if [[ "$actual_sha" != "$source_sha" ]]; then
    printf '%s\t%s\t%s\t%s\t%s\tNOT_RUN\tNOT_RUN\t0\t%s\tSOURCE_SHA_MISMATCH_%s\n' \
      "$route" "$rev" "$source_sha" "$official" "$source" "$evidence" "$actual_sha" >> "$META"
    return 0
  fi
  prefix="$stage/evidence/${case_id}/run${run}-${order}-${side}"
  mkdir -p "$(dirname "$prefix")"
  if [[ ! -x "$binary" ]]; then
    printf '%s\t%s\t%s\t%s\t%s\tNA\tNA\tNA\tNA\tNA\tNA\t127\t%s\n' \
      "$route" "$rev" "$case_id" "$run" "$side" "$DEVICE" >> "$MEASUREMENTS"
    return 127
  fi
  "$binary" "$DEVICE" "$rows" "$width" "$dtype" "$prefix" "$WARMUP" "$SAMPLES" 1 0 \
    > "$prefix.stdout.log" 2>&1
  rc=$?
  raw_file="$prefix-raw.tsv"
  stats_file="$prefix-stats.txt"
  if [[ -f "$raw_file" ]]; then
    awk -F '\t' -v r="$route" -v v="$rev" -v s="$source_sha" -v c="$case_id" -v n="$run" -v z="$side" -v d="$DEVICE" \
      'NR > 1 && NF >= 4 { print r "\t" v "\t" s "\t" c "\t" n "\t" z "\t" $3 "\t" $4 "\t" d }' "$raw_file" >> "$RAW"
  fi
  printf '%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\n' \
    "$route" "$rev" "$case_id" "$run" "$side" \
    "$(read_stat "$stats_file" B1_DEVICE median_us)" \
    "$(read_stat "$stats_file" B1_DEVICE mean_us)" \
    "$(read_stat "$stats_file" B1_DEVICE MAD_us)" \
    "$(read_stat "$stats_file" B1_DEVICE CV)" \
    "$(read_stat "$stats_file" B1_WALL median_us)" \
    "$(read_summary "$stats_file" bad)" "$rc" "$DEVICE" >> "$MEASUREMENTS"
  return "$rc"
}

run_version() {
  local route="$1" rev="$2" source_sha="$3" official="$4" source="$5"
  local id="${route}__${rev}" stage="$RUN_ROOT/stages/${route}__${rev}" evidence="$RUN_ROOT/evidence/${route}__${rev}"
  local build_status=PASS correctness_status=PASS attempted=0 reason=OK
  local case_id rows width dtype dtype_code path source_note index=0 run side order rc
  echo "=== VERSION $route $rev ==="
  if [[ ! -f "$source" ]]; then
    printf '%s\t%s\t%s\t%s\t%s\tBUILD_NOT_RUN\tNOT_RUN\t0\t%s\tSOURCE_MISSING\n' \
      "$route" "$rev" "$source_sha" "$official" "$source" "$evidence" >> "$META"
    return 0
  fi
  mkdir -p "$evidence"
  copy_support_files "$stage"
  cp "$ROOT/线上结果/R31B/V011/submission.asc" "$stage/parent.asc"
  cp "$source" "$stage/submission.asc"
  sha256sum "$stage/submission.asc" > "$evidence/submission.sha256"
  printf 'route\t%s\nrevision\t%s\nsource_sha\t%s\nofficial_score\t%s\nsource_path\t%s\n' \
    "$route" "$rev" "$source_sha" "$official" "$source" > "$evidence/source-meta.tsv"
  cmake -S "$stage" -B "$stage/build" \
    -DCMAKE_MODULE_PATH="$KIT/ASC_CMake;$KIT" \
    -DASC_DIR="$KIT" -DCMAKE_PREFIX_PATH="$KIT" \
    -DCMAKE_CXX_FLAGS="-I$HCC -I$HCC/aarch64-target-linux-gnu -I$HCC/backward" \
    -DCMAKE_C_FLAGS="-I$HCC -I$HCC/aarch64-target-linux-gnu" \
    -DSOC_VERSION=Ascend910B3 -DNPU_ARCH=dav-2201 \
    > "$evidence/cmake-configure.log" 2>&1
  if [[ $? -ne 0 ]]; then
    build_status=FAIL
    reason=CMAKE_CONFIGURE_FAILED
  else
    cmake --build "$stage/build" --target clx_ref_parent_probe clx_ref_candidate_probe -j2 \
      > "$evidence/build.log" 2>&1
    if [[ $? -ne 0 ]]; then
      build_status=FAIL
      reason=PROBE_BUILD_FAILED
    fi
  fi
  npu-smi info > "$evidence/npu-smi-pre.txt" 2>&1
  if [[ "$build_status" == PASS ]]; then
    while IFS=$'\t' read -r case_id rows width dtype dtype_code path source_note; do
      [[ "$case_id" == CASE_ID || -z "$case_id" ]] && continue
      index=$((index + 1))
      for run in $(seq 1 "$RUNS"); do
        if (( (index + run) % 2 == 0 )); then
          order=PC
          run_one_side "$route" "$rev" "$source_sha" "$stage" "$case_id" "$rows" "$width" "$dtype_code" "$run" parent "$order"; rc=$?
          [[ "$rc" -ne 0 ]] && correctness_status=FAIL
          run_one_side "$route" "$rev" "$source_sha" "$stage" "$case_id" "$rows" "$width" "$dtype_code" "$run" candidate "$order"; rc=$?
        else
          order=CP
          run_one_side "$route" "$rev" "$source_sha" "$stage" "$case_id" "$rows" "$width" "$dtype_code" "$run" candidate "$order"; rc=$?
          [[ "$rc" -ne 0 ]] && correctness_status=FAIL
          run_one_side "$route" "$rev" "$source_sha" "$stage" "$case_id" "$rows" "$width" "$dtype_code" "$run" parent "$order"; rc=$?
        fi
        [[ "$rc" -ne 0 ]] && correctness_status=FAIL
        attempted=$((attempted + 1))
      done
    done < "$SUITE"
  fi
  npu-smi info > "$evidence/npu-smi-post.txt" 2>&1
  printf '%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\n' \
    "$route" "$rev" "$source_sha" "$official" "$source" "$build_status" "$correctness_status" "$attempted" "$evidence" "$reason" >> "$META"
}

should_run() {
  local requested="$1"
  [[ "$ONLY" == ALL ]] && return 0
  local item
  IFS=',' read -r -a requested_items <<< "$ONLY"
  for item in "${requested_items[@]}"; do
    [[ "$item" == "$requested" ]] && return 0
  done
  return 1
}

while IFS=$'\t' read -r route rev source_sha official source recoverable local_runnable correctness_valid status_reason; do
  [[ "$route" == ROUTE || -z "$route" ]] && continue
  should_run "$route/$rev" || continue
  run_version "$route" "$rev" "$source_sha" "$official" "$source"
done < "$MANIFEST"

npu-smi info > "$RUN_ROOT/npu-smi-end.txt" 2>&1
date -u +%Y-%m-%dT%H:%M:%SZ > "$RUN_ROOT/end-time.txt"
echo "RUN_ROOT=$RUN_ROOT"
echo "MEASUREMENTS=$MEASUREMENTS"
echo "RAW_SAMPLES=$RAW"
echo "VERSION_STATUS=$META"

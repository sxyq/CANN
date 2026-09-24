#!/usr/bin/env bash
set -u
set -o pipefail

# Explicit lease and load-window markers are required before any remote runner call.
if [ "${MAIN_DEVICE_LEASE:-}" != "R31B" ]; then
    echo "MAIN_DEVICE_LEASE must be R31B; latency probes were not started" >&2
    exit 64
fi
if [ "${R31B_LOAD_WINDOW:-}" != "CLEAR" ]; then
    echo "R31B_LOAD_WINDOW must be CLEAR; latency probes were not started" >&2
    exit 64
fi

EVIDENCE_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
STAMP="$(date '+%Y%m%dT%H%M%S%z')"
LOG="${EVIDENCE_DIR}/paired-probe-rerun-${STAMP}.txt"
if [ -e "$LOG" ]; then
    echo "Refusing to overwrite existing evidence: $LOG" >&2
    exit 65
fi

printf 'ROUTE=R31B\nREVISION=V016\nDIRECT_PARENT=R31B-V011\n' | tee "$LOG"
printf 'MAIN_DEVICE_LEASE=%s\nR31B_LOAD_WINDOW=%s\n' "$MAIN_DEVICE_LEASE" "$R31B_LOAD_WINDOW" | tee -a "$LOG"
printf 'HOST_TARGET=cann-server3\nDEVICE=4\nROWS=2\nWARMUP=1\nREPEATS=3\n' | tee -a "$LOG"
printf 'PAIR_ORDER=P1,C1,P2,C2\n' | tee -a "$LOG"

if ssh cann-server3 'bash -s' <<'REMOTE' | tee -a "$LOG"
set -u
set -o pipefail

PROBE_DIR=/home/data4t2/lelinfeng/phase4-review-repro/R31B-V016/probe-build
V011="$PROBE_DIR/probe_v011"
V016="$PROBE_DIR/probe_v016"
V011_SOURCE=/home/data4t2/lelinfeng/phase4-workspaces/R31B/R31B-V011-LP-ROW-PIPELINE_kernel.asc
V016_SOURCE=/home/data4t2/lelinfeng/phase4-review-repro/R31B-V016/probe-src/R31B-V016-WIDE-TILE-SEED_kernel.asc
ENV_SCRIPT=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/script/set_env.sh
DEVICE=4
ROWS=2
WARMUP=1
REPEATS=3
EXPECTED_V011_SOURCE_SHA=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
EXPECTED_V016_SOURCE_SHA=9f5c353e65a13a740fe97dc7e6415df032d27560831a3ad142c77592b8208eb5
EXPECTED_V011_EXEC_SHA=78917340dab5c0723c2d18faf493dca32f0e3412f78971c3ef7daa4db1427891
EXPECTED_V016_EXEC_SHA=81a2ebfac7d3ec0cf1cf65105c7b81cc995d16ca8d57ed072eca3bdba40218b2

printf 'REMOTE_HOST=%s\n' "$(hostname)"
printf 'CAPTURED_AT=%s\n' "$(TZ=Asia/Shanghai date '+%Y-%m-%dT%H:%M:%S%z')"
printf 'V011_SOURCE=%s\nV016_SOURCE=%s\n' "$V011_SOURCE" "$V016_SOURCE"
printf 'V011_EXECUTABLE=%s\nV016_EXECUTABLE=%s\n' "$V011" "$V016"

for path in "$V011" "$V016" "$V011_SOURCE" "$V016_SOURCE"; do
    if [ ! -f "$path" ]; then
        printf 'MISSING=%s\n' "$path" >&2
        exit 67
    fi
done

actual_v011_source=$(sha256sum "$V011_SOURCE" | awk '{print $1}')
actual_v016_source=$(sha256sum "$V016_SOURCE" | awk '{print $1}')
actual_v011_exec=$(sha256sum "$V011" | awk '{print $1}')
actual_v016_exec=$(sha256sum "$V016" | awk '{print $1}')
printf 'V011_SOURCE_SHA=%s\nV016_SOURCE_SHA=%s\n' "$actual_v011_source" "$actual_v016_source"
printf 'V011_EXECUTABLE_SHA=%s\nV016_EXECUTABLE_SHA=%s\n' "$actual_v011_exec" "$actual_v016_exec"

[ "$actual_v011_source" = "$EXPECTED_V011_SOURCE_SHA" ] || exit 68
[ "$actual_v016_source" = "$EXPECTED_V016_SOURCE_SHA" ] || exit 68
[ "$actual_v011_exec" = "$EXPECTED_V011_EXEC_SHA" ] || exit 69
[ "$actual_v016_exec" = "$EXPECTED_V016_EXEC_SHA" ] || exit 69

source "$ENV_SCRIPT"
export LD_LIBRARY_PATH=/usr/lib/aarch64-linux-gnu:${LD_LIBRARY_PATH:-}

capture_load() {
    local label="$1"
    printf 'SNAPSHOT=%s\n' "$label"
    TZ=Asia/Shanghai date '+%Y-%m-%dT%H:%M:%S%z'
    npu-smi info -t usages -i "$DEVICE"
    printf 'PROCESSES_BEGIN\n'
    ps -eo pid=,comm=,args= | grep -E '[Vv][Ll][Ll][Mm]|EngineCore|Worker_TP' | grep -v grep | sort -n
    printf 'PROCESSES_END\n'
}

run_probe() {
    local label="$1"
    local binary="$2"
    local dtype="$3"
    local width="$4"
    local raw
    local rc=0
    capture_load "BEFORE_${label}"
    printf 'COMMAND_%s=%s %s %s %s %s %s %s\n' "$label" "$binary" "$dtype" "$ROWS" "$width" "$DEVICE" "$WARMUP" "$REPEATS"
    raw=$("$binary" "$dtype" "$ROWS" "$width" "$DEVICE" "$WARMUP" "$REPEATS" 2>&1) || rc=$?
    printf 'RAW_%s_BEGIN\n%s\nRAW_%s_END\n' "$label" "$raw" "$label"
    LAST_MEDIAN=$(printf '%s\n' "$raw" | awk -F'median_us=' '/median_us=/{split($2, a, " "); print a[1]; exit}')
    if [ -z "${LAST_MEDIAN:-}" ] || [ "$LAST_MEDIAN" = "NA" ]; then
        printf 'MEDIAN_PARSE_%s=FAIL\n' "$label" >&2
        return 70
    fi
    printf 'MEDIAN_%s_US=%s\n' "$label" "$LAST_MEDIAN"
    printf 'RETURN_%s=%s\n' "$label" "$rc"
    capture_load "AFTER_${label}"
    return "$rc"
}

abs_delta() {
    awk -v a="$1" -v b="$2" 'BEGIN { d = a - b; if (d < 0) d = -d; printf "%.3f", d }'
}

percent_jitter() {
    awk -v a="$1" -v b="$2" 'BEGIN { d = a - b; if (d < 0) d = -d; m = (a + b) / 2; if (m == 0) print "NA"; else printf "%.3f", 100 * d / m }'
}

run_case() {
    local case_name="$1"
    local dtype="$2"
    local width="$3"
    local p1 p2 c1 c2 delta1 delta2 median_delta worst_delta worst_abs
    printf 'CASE=%s\nDTYPE=%s\nWIDTH=%s\n' "$case_name" "$dtype" "$width"
    run_probe "${case_name}_P1" "$V011" "$dtype" "$width" || exit $?
    p1="$LAST_MEDIAN"
    run_probe "${case_name}_C1" "$V016" "$dtype" "$width" || exit $?
    c1="$LAST_MEDIAN"
    run_probe "${case_name}_P2" "$V011" "$dtype" "$width" || exit $?
    p2="$LAST_MEDIAN"
    run_probe "${case_name}_C2" "$V016" "$dtype" "$width" || exit $?
    c2="$LAST_MEDIAN"
    delta1=$(awk -v p="$p1" -v c="$c1" 'BEGIN { printf "%.3f", c - p }')
    delta2=$(awk -v p="$p2" -v c="$c2" 'BEGIN { printf "%.3f", c - p }')
    median_delta=$(awk -v a="$delta1" -v b="$delta2" 'BEGIN { printf "%.3f", (a + b) / 2 }')
    worst_delta=$(awk -v a="$delta1" -v b="$delta2" 'BEGIN { if (a > b) printf "%.3f", a; else printf "%.3f", b }')
    worst_abs=$(abs_delta "$delta1" "$delta2")
    printf 'PARENT_MEDIAN_1_US=%s\nPARENT_MEDIAN_2_US=%s\n' "$p1" "$p2"
    printf 'CANDIDATE_MEDIAN_1_US=%s\nCANDIDATE_MEDIAN_2_US=%s\n' "$c1" "$c2"
    printf 'PARENT_JITTER_US=%s\nPARENT_JITTER_PCT=%s\n' "$(abs_delta "$p2" "$p1")" "$(percent_jitter "$p2" "$p1")"
    printf 'CANDIDATE_JITTER_US=%s\nCANDIDATE_JITTER_PCT=%s\n' "$(abs_delta "$c2" "$c1")" "$(percent_jitter "$c2" "$c1")"
    printf 'PAIR_1_DELTA_US=%s\nPAIR_2_DELTA_US=%s\n' "$delta1" "$delta2"
    printf 'MEDIAN_DELTA_US=%s\nWORST_DELTA_US=%s\nWORST_ABS_DELTA_US=%s\n' "$median_delta" "$worst_delta" "$worst_abs"
}

printf 'LOAD_LABEL=REVIEW_REQUIRED_FROM_SNAPSHOTS\n'
run_case fp16-tail-d12288 1 12288
run_case fp16-wide-d32768 1 32768
run_case bf16-tail-d12288 2 12288
run_case bf16-wide-d32768 2 32768
printf 'REMOTE_RUN=PASS\n'
REMOTE
then
    :
else
    printf 'REMOTE_RUN=FAIL\n' | tee -a "$LOG"
    exit 66
fi

printf 'EVIDENCE_LOG=%s\n' "$LOG" | tee -a "$LOG"

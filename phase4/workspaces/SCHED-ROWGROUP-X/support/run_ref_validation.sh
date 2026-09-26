#!/usr/bin/env bash
# Unified reference harness validation — SAME-BINARY only (no Candidate).
# Blocks: in-process A/B (2 x 31 samples), warmup study, cold-process diag.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")" && pwd)"
BUILD="$ROOT/build"
BIN="$BUILD/srx_ref_parent_probe"
DEV="${ASCEND_DEVICE_ID:-4}"
ROWS="${ROWS:-17}"
WIDTH="${WIDTH:-256}"
DTYPE="${DTYPE:-0}"
OUT="${OUT:-$ROOT/results-ref-harness/d${DEV}}"
mkdir -p "$OUT"
export LD_LIBRARY_PATH="/usr/lib/aarch64-linux-gnu:${LD_LIBRARY_PATH:-}"

if [ ! -x "$BIN" ]; then
  echo "MISSING_BINARY $BIN"
  exit 2
fi

# preflight load snapshot
npu-smi info > "$OUT/npu-smi-start.txt" 2>&1 || true
date -Is > "$OUT/start.timestamp.txt"

run_block() {
  local tag="$1" warm="$2" samp="$3" blocks="$4" gap="$5"
  local prefix="$OUT/${tag}"
  echo "=== $tag warmup=$warm samples=$samp blocks=$blocks ==="
  date -Is > "${prefix}.timestamp.txt"
  "$BIN" "$DEV" "$ROWS" "$WIDTH" "$DTYPE" "$prefix" \
    "$warm" "$samp" "$blocks" "$gap" \
    > "${prefix}.stdout.txt" 2> "${prefix}.stderr.txt"
  echo "  done: $(grep -E 'ALL_DEVICE|median' "${prefix}-stats.txt" | head -2 || true)"
}

# 1) SAME-BINARY: warmup 10, 2 blocks x 31 samples, gap 2s, no process exit between
run_block same-binary-w10-s31 10 31 2 2

# 2) WARMUP STUDY (reference binary only)
run_block warmup-study-w0-s21 0 21 1 0
run_block warmup-study-w3-s21 3 21 1 0
run_block warmup-study-w10-s21 10 21 1 0
run_block warmup-study-w20-s21 20 21 1 0

# 3) PROCESS-LIFETIME A: already have in-process 31 from block 1 of same-binary
#    B: 31 cold processes, each 1 post-warmup sample (warmup 10, samples 1)
COLD_DIR="$OUT/cold-processes"
mkdir -p "$COLD_DIR"
echo "rep	device_us	host_wall_us" > "$OUT/cold-process-medians.tsv"
for i in $(seq -w 1 31); do
  prefix="$COLD_DIR/cold-${i}"
  "$BIN" "$DEV" "$ROWS" "$WIDTH" "$DTYPE" "$prefix" \
    10 1 1 0 > "${prefix}.stdout.txt" 2> "${prefix}.stderr.txt" || true
  # extract single sample from raw (block1 rep1)
  line=$(awk -F'\t' 'NR==2{print $3"\t"$4}' "${prefix}-raw.tsv")
  echo "${i}	${line}" >> "$OUT/cold-process-medians.tsv"
done

npu-smi info > "$OUT/npu-smi-end.txt" 2>&1 || true
date -Is > "$OUT/end.timestamp.txt"
echo "REF_HARNESS_RUNS_DONE out=$OUT"

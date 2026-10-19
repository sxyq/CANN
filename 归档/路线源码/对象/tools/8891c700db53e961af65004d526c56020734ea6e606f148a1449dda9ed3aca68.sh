#!/usr/bin/env bash
# EPILOGUE-ARITH-CHAMPION-X V001 NORM-HOIST timing.
# Protocol: warmup=45, samples=41, same-binary blocks=2, interleaved P/C pairs.
set -uo pipefail
ROOT="$(cd "$(dirname "$0")" && pwd)"
DEV="${DEV:-5}"
OUT="${OUT:-$ROOT/results-timing-v001-20260929}"
WARMUP=45
SAMPLES=41
GAP=2
PAIRS=6
ASCEND_HOME_PATH=${ASCEND_HOME_PATH:-/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002}
set +u; source "${ASCEND_HOME_PATH}/bin/setenv.bash" >/dev/null 2>&1 || true; set -u
mkdir -p "$OUT"
date -u +%Y-%m-%dT%H:%M:%SZ > "$OUT/timing.timestamp.txt"
npu-smi info > "$OUT/npu-smi-pre.txt" 2>&1
P="$ROOT/build/se_ref_parent_probe"
C="$ROOT/build/se_ref_candidate_probe"
# rows width dtype tag  (dtype 0=fp32)
# primary = change-domain wide FP32 (rowWidth>8192); control = 2x8192 identical-code
SHAPES=(
  "1 32768 0 1x32768_fp32"
  "1 16384 0 1x16384_fp32"
  "2 16384 0 2x16384_fp32"
  "2 8192 0 2x8192_fp32"
  "3 6144 0 3x6144_fp32"
)
PHASE="${PHASE:-all}"
if [ "$PHASE" = "all" ] || [ "$PHASE" = "sb" ]; then
  echo "=== PHASE 1: same-binary parent (blocks=2, samples=$SAMPLES, warmup=$WARMUP) ==="
  for shape in "${SHAPES[@]}"; do
    read -r rows width dtype tag <<< "$shape"
    prefix="$OUT/sb-P-$tag"
    echo "--- same-binary P $tag ---"
    "$P" "$DEV" "$rows" "$width" "$dtype" "$prefix" "$WARMUP" "$SAMPLES" 2 "$GAP" \
      > "$prefix.stdout.txt" 2>&1
    echo "rc=$?"
  done
fi
if [ "$PHASE" = "all" ] || [ "$PHASE" = "pc" ]; then
  echo "=== PHASE 2: interleaved P/C pairs x$PAIRS ==="
  for shape in "${SHAPES[@]}"; do
    read -r rows width dtype tag <<< "$shape"
    for pair in $(seq 1 "$PAIRS"); do
      if [ $((pair % 2)) -eq 1 ]; then order="P C"; else order="C P"; fi
      for variant in $order; do
        bin=$([ "$variant" = P ] && echo "$P" || echo "$C")
        prefix="$OUT/pc$pair-$variant-$tag"
        echo "--- pair$pair $variant $tag ---"
        "$bin" "$DEV" "$rows" "$width" "$dtype" "$prefix" "$WARMUP" "$SAMPLES" 1 0 \
          > "$prefix.stdout.txt" 2>&1
        echo "rc=$?"
      done
    done
  done
fi
npu-smi info > "$OUT/npu-smi-post.txt" 2>&1
echo "=== TIMING_DONE ==="

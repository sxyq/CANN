#!/usr/bin/env bash
# EPILOGUE-ARITH V001 window-3 (MAIN-1 ruling: tighter window, same binaries).
# Shapes: 1x32768 + 2x16384 (change-domain) + 2x8192 (identical-code control).
# Per run: warmup=45, 6 blocks x 11 samples, gap=5s between blocks.
set -uo pipefail
ROOT="$(cd "$(dirname "$0")" && pwd)"
DEV="${DEV:-4}"
OUT="${OUT:-$ROOT/results-timing-v001c-20260929}"
WARMUP=45
SAMPLES=11
BLOCKS=6
GAP=5
PAIRS=6
ASCEND_HOME_PATH=${ASCEND_HOME_PATH:-/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002}
set +u; source "${ASCEND_HOME_PATH}/bin/setenv.bash" >/dev/null 2>&1 || true; set -u
mkdir -p "$OUT"
date -u +%Y-%m-%dT%H:%M:%SZ > "$OUT/timing.timestamp.txt"
npu-smi info > "$OUT/npu-smi-pre.txt" 2>&1
sha256sum "$ROOT/build/se_ref_parent_probe" "$ROOT/build/se_ref_candidate_probe" "$ROOT/parent.asc" "$ROOT/submission.asc" > "$OUT/module-sha.txt"
P="$ROOT/build/se_ref_parent_probe"
C="$ROOT/build/se_ref_candidate_probe"
SHAPES=(
  "1 32768 0 1x32768_fp32"
  "2 16384 0 2x16384_fp32"
  "2 8192 0 2x8192_fp32"
)
PHASE="${PHASE:-all}"
if [ "$PHASE" = "all" ] || [ "$PHASE" = "sb" ]; then
  echo "=== PHASE 1: same-binary parent blocks=$BLOCKS samples=$SAMPLES gap=$GAP ==="
  for shape in "${SHAPES[@]}"; do
    read -r rows width dtype tag <<< "$shape"
    prefix="$OUT/sb-P-$tag"
    echo "--- same-binary P $tag ---"
    "$P" "$DEV" "$rows" "$width" "$dtype" "$prefix" "$WARMUP" "$SAMPLES" "$BLOCKS" "$GAP" \
      > "$prefix.stdout.txt" 2>&1
    echo "rc=$?"
  done
fi
if [ "$PHASE" = "all" ] || [ "$PHASE" = "pc" ]; then
  echo "=== PHASE 2: interleaved P/C pairs x$PAIRS (each side blocks=$BLOCKS samples=$SAMPLES) ==="
  for shape in "${SHAPES[@]}"; do
    read -r rows width dtype tag <<< "$shape"
    for pair in $(seq 1 "$PAIRS"); do
      if [ $((pair % 2)) -eq 1 ]; then order="P C"; else order="C P"; fi
      for variant in $order; do
        bin=$([ "$variant" = P ] && echo "$P" || echo "$C")
        prefix="$OUT/pc$pair-$variant-$tag"
        echo "--- pair$pair $variant $tag ---"
        "$bin" "$DEV" "$rows" "$width" "$dtype" "$prefix" "$WARMUP" "$SAMPLES" "$BLOCKS" 0 \
          > "$prefix.stdout.txt" 2>&1
        echo "rc=$?"
      done
    done
  done
fi
npu-smi info > "$OUT/npu-smi-post.txt" 2>&1
echo "=== TIMING_DONE -> $OUT ==="

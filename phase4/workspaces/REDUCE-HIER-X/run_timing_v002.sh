#!/usr/bin/env bash
# REDUCE-HIER-X V002 timing — lease R2-REDUCE-V002-TIMING, device 4.
# H2 VectorReduceTo8 vs frozen parent. Priority: large-D multi-tile.
# Protocol: warmup=45, samples=41, same-binary blocks=2, interleaved P/C x6.
set -uo pipefail

ROOT="$(cd "$(dirname "$0")" && pwd)"
DEV="${DEV:-4}"
OUT="${OUT:-$ROOT/results-timing-v002-20260927}"
WARMUP=45
SAMPLES=41
GAP=2
PAIRS=6
ASCEND_HOME_PATH=${ASCEND_HOME_PATH:-/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002}
set +u; source "${ASCEND_HOME_PATH}/bin/setenv.bash" >/dev/null 2>&1 || true; set -u

mkdir -p "$OUT"
date -u +%Y-%m-%dT%H:%M:%SZ > "$OUT/timing.timestamp.txt"
npu-smi info > "$OUT/npu-smi-pre.txt" 2>&1

P="$ROOT/build/rhx_ref_parent_probe"
C="$ROOT/build/rhx_ref_candidate_probe"

# rows width tag   (priority order per Main)
SHAPES=(
  "1 32768 1x32768_fp32"
  "1 16384 1x16384_fp32"
  "1 8192 1x8192_fp32"
  "1 4096 1x4096_fp32"
  "8 8192 8x8192_fp32"
)

echo "=== PHASE 1: same-binary (blocks=2, samples=$SAMPLES, warmup=$WARMUP) ==="
for shape in "${SHAPES[@]}"; do
  read -r rows width tag <<< "$shape"
  for variant in P C; do
    bin=$([ "$variant" = P ] && echo "$P" || echo "$C")
    prefix="$OUT/sb-$variant-$tag"
    echo "--- same-binary $variant $tag ---"
    "$bin" "$DEV" "$rows" "$width" 0 "$prefix" "$WARMUP" "$SAMPLES" 2 "$GAP" \
      > "$prefix.stdout.txt" 2>&1
    echo "rc=$?"
  done
done

echo "=== PHASE 2: interleaved P/C pairs x$PAIRS ==="
for shape in "${SHAPES[@]}"; do
  read -r rows width tag <<< "$shape"
  for pair in $(seq 1 "$PAIRS"); do
    if [ $((pair % 2)) -eq 1 ]; then order="P C"; else order="C P"; fi
    for variant in $order; do
      bin=$([ "$variant" = P ] && echo "$P" || echo "$C")
      prefix="$OUT/pc$pair-$variant-$tag"
      echo "--- pair$pair $variant $tag ---"
      "$bin" "$DEV" "$rows" "$width" 0 "$prefix" "$WARMUP" "$SAMPLES" 1 0 \
        > "$prefix.stdout.txt" 2>&1
      echo "rc=$?"
    done
  done
done

npu-smi info > "$OUT/npu-smi-post.txt" 2>&1
echo "=== TIMING RUN DONE -> $OUT ==="
ls "$OUT" | wc -l

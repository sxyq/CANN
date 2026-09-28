#!/usr/bin/env bash
# COEFF-LOCALITY-X V004 timing round 2 — d5, focused re-measure of ambiguous shapes.
set -uo pipefail
ROOT="$(cd "$(dirname "$0")" && pwd)"
DEV="${DEV:-5}"
OUT="${OUT:-$ROOT/results-timing-v004b-d5}"
WARMUP=45
SAMPLES=41
GAP=2
PAIRS=8
ASCEND_HOME_PATH=${ASCEND_HOME_PATH:-/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002}
set +u; source "${ASCEND_HOME_PATH}/bin/setenv.bash" >/dev/null 2>&1 || true; set -u
mkdir -p "$OUT"
date -u +%Y-%m-%dT%H:%M:%SZ > "$OUT/timing.timestamp.txt"
npu-smi info > "$OUT/npu-smi-pre.txt" 2>&1
P="$ROOT/build/clx_ref_parent_probe"
C="$ROOT/build/clx_ref_candidate_probe"
SHAPES=(
  "1 8192 1x8192_fp32"
  "1 6144 1x6144_fp32"
  "2 8192 2x8192_fp32"
  "1 4096 1x4096_fp32"
  "1 32768 1x32768_fp32"
)
echo "=== PHASE 1: same-binary ==="
for shape in "${SHAPES[@]}"; do
  read -r rows width tag <<< "$shape"
  for variant in P C; do
    bin=$([ "$variant" = P ] && echo "$P" || echo "$C")
    prefix="$OUT/sb-$variant-$tag"
    "$bin" "$DEV" "$rows" "$width" 0 "$prefix" "$WARMUP" "$SAMPLES" 2 "$GAP" > "$prefix.stdout.txt" 2>&1
    echo "sb $variant $tag rc=$?"
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
      "$bin" "$DEV" "$rows" "$width" 0 "$prefix" "$WARMUP" "$SAMPLES" 1 0 > "$prefix.stdout.txt" 2>&1
      echo "pc$pair $variant $tag rc=$?"
    done
  done
done
npu-smi info > "$OUT/npu-smi-post.txt" 2>&1
echo "=== DONE -> $OUT ==="

#!/usr/bin/env bash
# INTEGRATION-X V001 timing — stack (SCHED V002 ownership + VECTOR V001 denominator)
# vs frozen parent. Protocol: warmup=45, samples=41, same-binary blocks=2,
# 6 interleaved P/C pairs (odd pair P->C, even pair C->P).
set -uo pipefail

ROOT="$(cd "$(dirname "$0")" && pwd)"
DEV="${DEV:-4}"
OUT="${OUT:-$ROOT/results-timing-v001-20260927}"
WARMUP=45
SAMPLES=41
GAP=2
PAIRS=6
ASCEND_HOME_PATH=${ASCEND_HOME_PATH:-/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002}
set +u; source "${ASCEND_HOME_PATH}/bin/setenv.bash" >/dev/null 2>&1 || true; set -u

mkdir -p "$OUT"
date -u +%Y-%m-%dT%H:%M:%SZ > "$OUT/timing.timestamp.txt"
npu-smi info > "$OUT/npu-smi-pre.txt" 2>&1

P="$ROOT/build/ix_ref_parent_probe"
C="$ROOT/build/ix_ref_candidate_probe"

# rows width dtype tag (dtype 0=fp32 1=fp16 2=bf16)
SHAPES=(
  "33 100 0 33x100_fp32"
  "8 256 0 8x256_fp32"
  "32 256 0 32x256_fp32"
  "17 257 1 17x257_fp16"
  "8 8192 0 8x8192_fp32"
)

echo "=== PHASE 1: same-binary (blocks=2, samples=$SAMPLES, warmup=$WARMUP) ==="
for shape in "${SHAPES[@]}"; do
  read -r rows width dtype tag <<< "$shape"
  for variant in P C; do
    bin=$([ "$variant" = P ] && echo "$P" || echo "$C")
    prefix="$OUT/sb-$variant-$tag"
    echo "--- same-binary $variant $tag ---"
    "$bin" "$DEV" "$rows" "$width" "$dtype" "$prefix" "$WARMUP" "$SAMPLES" 2 "$GAP" \
      > "$prefix.stdout.txt" 2>&1
    echo "rc=$?"
  done
done

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

npu-smi info > "$OUT/npu-smi-post.txt" 2>&1
echo "=== TIMING RUN DONE -> $OUT ==="
ls "$OUT" | wc -l

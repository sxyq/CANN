#!/bin/bash
# SCHED-CHAMPION-X V001 timing — lease R2-SCHED-V001-TIMING, device 4
# Protocol: warmup=45, device-event primary, same-binary then interleaved P/C
set -uo pipefail
CANN=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002
source "$CANN/bin/setenv.bash" >/dev/null 2>&1
export ASCEND_HOME_PATH="$CANN"
export LD_LIBRARY_PATH="$CANN/lib64:$CANN/runtime/lib64:$CANN/compiler/lib64:/usr/lib/aarch64-linux-gnu:${LD_LIBRARY_PATH:-}"
DEV=4
WARMUP=45
SAMPLES=21
BIN_DIR=/home/data4t2/lelinfeng/phase4-workspaces/SCHED-CHAMPION-X/V001/support/timing/build
PARENT="$BIN_DIR/scx_ref_parent_probe"
CANDIDATE="$BIN_DIR/scx_ref_v001_probe"
OUT=/home/data4t2/lelinfeng/phase4-workspaces/SCHED-CHAMPION-X/V001/support/timing/results
mkdir -p "$OUT"

echo "=== npu-smi pre ==="
npu-smi info > "$OUT/npu-smi-pre.txt" 2>&1 || true
date -Is > "$OUT/start.timestamp.txt"

# shape list: rows width dtype tag
SHAPES=(
  "33 100 0 33x100_fp32"
  "17 256 0 17x256_fp32"
  "17 257 1 17x257_fp16"
  "7 65 0 7x65_fp32"
)

for S in "${SHAPES[@]}"; do
  read -r ROWS WIDTH DTYPE TAG <<< "$S"
  echo ""
  echo "============================================"
  echo "SHAPE $TAG  rows=$ROWS width=$WIDTH dtype=$DTYPE"
  echo "============================================"

  # --- SAME-BINARY on parent ---
  echo "--- SAME-BINARY PARENT $TAG ---"
  "$PARENT" "$DEV" "$ROWS" "$WIDTH" "$DTYPE" "$OUT/sb-parent-$TAG" \
    "$WARMUP" "$SAMPLES" 2 2 > "$OUT/sb-parent-$TAG.stdout.txt" 2> "$OUT/sb-parent-$TAG.stderr.txt"
  SB_PARENT_RC=$?
  echo "  sb_parent_rc=$SB_PARENT_RC"

  # --- SAME-BINARY on candidate ---
  echo "--- SAME-BINARY CANDIDATE $TAG ---"
  "$CANDIDATE" "$DEV" "$ROWS" "$WIDTH" "$DTYPE" "$OUT/sb-cand-$TAG" \
    "$WARMUP" "$SAMPLES" 2 2 > "$OUT/sb-cand-$TAG.stdout.txt" 2> "$OUT/sb-cand-$TAG.stderr.txt"
  SB_CAND_RC=$?
  echo "  sb_cand_rc=$SB_CAND_RC"

  # --- Interleaved P/C: 4 pairs (PC PC PC PC), each pair is one process ---
  echo "--- PAIRED P/C $TAG (4 pairs) ---"
  for I in 1 2 3 4; do
    if [ $((I % 2)) -eq 1 ]; then
      # P then C
      "$PARENT" "$DEV" "$ROWS" "$WIDTH" "$DTYPE" "$OUT/pc${I}-P-$TAG" \
        "$WARMUP" "$SAMPLES" 1 0 > "$OUT/pc${I}-P-$TAG.stdout.txt" 2>&1
      echo "  pair${I} P rc=$?"
      "$CANDIDATE" "$DEV" "$ROWS" "$WIDTH" "$DTYPE" "$OUT/pc${I}-C-$TAG" \
        "$WARMUP" "$SAMPLES" 1 0 > "$OUT/pc${I}-C-$TAG.stdout.txt" 2>&1
      echo "  pair${I} C rc=$?"
    else
      # C then P (order-robust)
      "$CANDIDATE" "$DEV" "$ROWS" "$WIDTH" "$DTYPE" "$OUT/pc${I}-C-$TAG" \
        "$WARMUP" "$SAMPLES" 1 0 > "$OUT/pc${I}-C-$TAG.stdout.txt" 2>&1
      echo "  pair${I} C rc=$?"
      "$PARENT" "$DEV" "$ROWS" "$WIDTH" "$DTYPE" "$OUT/pc${I}-P-$TAG" \
        "$WARMUP" "$SAMPLES" 1 0 > "$OUT/pc${I}-P-$TAG.stdout.txt" 2>&1
      echo "  pair${I} P rc=$?"
    fi
  done
done

echo ""
echo "=== npu-smi post ==="
npu-smi info > "$OUT/npu-smi-post.txt" 2>&1 || true
date -Is > "$OUT/end.timestamp.txt"
echo "TIMING_DONE"

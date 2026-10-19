#!/bin/bash
CANN=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002
source "$CANN/bin/setenv.bash" >/dev/null 2>&1
export ASCEND_HOME_PATH="$CANN"
export LD_LIBRARY_PATH="$CANN/lib64:$CANN/runtime/lib64:$CANN/compiler/lib64:/usr/lib/aarch64-linux-gnu"
DEV=4
WARMUP=45
SAMPLES=21
BIN=/home/data4t2/lelinfeng/phase4-workspaces/SCHED-CHAMPION-X/V001/support/timing/build
OUT=/home/data4t2/lelinfeng/phase4-workspaces/SCHED-CHAMPION-X/V001/support/timing/results
mkdir -p "$OUT"
npu-smi info > "$OUT/npu-smi-pre.txt" 2>&1
date -Is > "$OUT/start.timestamp.txt"

run_shape() {
    local ROWS=$1 WIDTH=$2 DTYPE=$3 TAG=$4
    echo "=== SHAPE $TAG ==="
    echo "--- same-binary parent ---"
    "$BIN/scx_ref_parent_probe" "$DEV" "$ROWS" "$WIDTH" "$DTYPE" "$OUT/sb-parent-$TAG" "$WARMUP" "$SAMPLES" 2 2 2>&1 | tail -2
    echo "--- same-binary candidate ---"
    "$BIN/scx_ref_v001_probe" "$DEV" "$ROWS" "$WIDTH" "$DTYPE" "$OUT/sb-cand-$TAG" "$WARMUP" "$SAMPLES" 2 2 2>&1 | tail -2
    echo "--- paired P/C 4 pairs ---"
    for I in 1 2 3 4; do
        if [ $((I % 2)) -eq 1 ]; then
            "$BIN/scx_ref_parent_probe" "$DEV" "$ROWS" "$WIDTH" "$DTYPE" "$OUT/pc${I}-P-$TAG" "$WARMUP" "$SAMPLES" 1 0 2>&1 | tail -1
            "$BIN/scx_ref_v001_probe" "$DEV" "$ROWS" "$WIDTH" "$DTYPE" "$OUT/pc${I}-C-$TAG" "$WARMUP" "$SAMPLES" 1 0 2>&1 | tail -1
        else
            "$BIN/scx_ref_v001_probe" "$DEV" "$ROWS" "$WIDTH" "$DTYPE" "$OUT/pc${I}-C-$TAG" "$WARMUP" "$SAMPLES" 1 0 2>&1 | tail -1
            "$BIN/scx_ref_parent_probe" "$DEV" "$ROWS" "$WIDTH" "$DTYPE" "$OUT/pc${I}-P-$TAG" "$WARMUP" "$SAMPLES" 1 0 2>&1 | tail -1
        fi
        echo "  pair$I done"
    done
}

run_shape 33 100 0 "33x100_fp32"
run_shape 17 256 0 "17x256_fp32"
run_shape 17 257 1 "17x257_fp16"
run_shape 7 65 0 "7x65_fp32"

npu-smi info > "$OUT/npu-smi-post.txt" 2>&1
date -Is > "$OUT/end.timestamp.txt"
echo "TIMING_ALL_DONE"

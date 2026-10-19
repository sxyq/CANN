#!/bin/bash
CANN=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002
source "$CANN/bin/setenv.bash" >/dev/null 2>&1
export ASCEND_HOME_PATH="$CANN"
export LD_LIBRARY_PATH="$CANN/lib64:$CANN/runtime/lib64:$CANN/compiler/lib64:/usr/lib/aarch64-linux-gnu"
DEV=4
WARMUP=45
SAMPLES=21
BIN=/home/data4t2/lelinfeng/phase4-workspaces/SCHED-CHAMPION-X/V003/support/timing/build
OUT=/home/data4t2/lelinfeng/phase4-workspaces/SCHED-CHAMPION-X/V003/support/timing/results
mkdir -p "$OUT"
npu-smi info > "$OUT/npu-smi-pre.txt" 2>&1

echo "=== 33x100 FP32 ==="
"$BIN/scx_ref_parent_probe" "$DEV" 33 100 0 "$OUT/sb-parent" "$WARMUP" "$SAMPLES" 2 2 2>&1 | tail -1
"$BIN/scx_ref_v001_probe" "$DEV" 33 100 0 "$OUT/sb-cand" "$WARMUP" "$SAMPLES" 2 2 2>&1 | tail -1
for I in 1 2 3 4; do
    if [ $((I % 2)) -eq 1 ]; then
        "$BIN/scx_ref_parent_probe" "$DEV" 33 100 0 "$OUT/pc${I}-P" "$WARMUP" "$SAMPLES" 1 0 2>&1 | tail -1
        "$BIN/scx_ref_v001_probe" "$DEV" 33 100 0 "$OUT/pc${I}-C" "$WARMUP" "$SAMPLES" 1 0 2>&1 | tail -1
    else
        "$BIN/scx_ref_v001_probe" "$DEV" 33 100 0 "$OUT/pc${I}-C" "$WARMUP" "$SAMPLES" 1 0 2>&1 | tail -1
        "$BIN/scx_ref_parent_probe" "$DEV" 33 100 0 "$OUT/pc${I}-P" "$WARMUP" "$SAMPLES" 1 0 2>&1 | tail -1
    fi
    echo "pair$I"
    sleep 1
done

echo "=== 17x257 FP16 ==="
"$BIN/scx_ref_parent_probe" "$DEV" 17 257 1 "$OUT/sb-parent-257" "$WARMUP" "$SAMPLES" 2 2 2>&1 | tail -1
"$BIN/scx_ref_v001_probe" "$DEV" 17 257 1 "$OUT/sb-cand-257" "$WARMUP" "$SAMPLES" 2 2 2>&1 | tail -1
for I in 1 2 3 4; do
    if [ $((I % 2)) -eq 1 ]; then
        "$BIN/scx_ref_parent_probe" "$DEV" 17 257 1 "$OUT/pc${I}-P-257" "$WARMUP" "$SAMPLES" 1 0 2>&1 | tail -1
        "$BIN/scx_ref_v001_probe" "$DEV" 17 257 1 "$OUT/pc${I}-C-257" "$WARMUP" "$SAMPLES" 1 0 2>&1 | tail -1
    else
        "$BIN/scx_ref_v001_probe" "$DEV" 17 257 1 "$OUT/pc${I}-C-257" "$WARMUP" "$SAMPLES" 1 0 2>&1 | tail -1
        "$BIN/scx_ref_parent_probe" "$DEV" 17 257 1 "$OUT/pc${I}-P-257" "$WARMUP" "$SAMPLES" 1 0 2>&1 | tail -1
    fi
    echo "pair$I"
    sleep 1
done

npu-smi info > "$OUT/npu-smi-post.txt" 2>&1
echo "V003_TIMING_DONE"

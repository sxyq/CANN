#!/bin/bash
CANN=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002
source "$CANN/bin/setenv.bash" >/dev/null 2>&1
export ASCEND_HOME_PATH="$CANN"
export LD_LIBRARY_PATH="$CANN/lib64:$CANN/runtime/lib64:$CANN/compiler/lib64:/usr/lib/aarch64-linux-gnu"
DEV=4
WARMUP=45
SAMPLES=41
BIN=/home/data4t2/lelinfeng/phase4-workspaces/SCHED-CHAMPION-X/V002/support/timing/build
OUT=/home/data4t2/lelinfeng/phase4-workspaces/SCHED-CHAMPION-X/V002/support/timing/results-remeasure
mkdir -p "$OUT"
npu-smi info > "$OUT/npu-smi-pre.txt" 2>&1
date -Is > "$OUT/start.timestamp.txt"

echo "=== 33x100 FP32 SAME-BINARY ==="
echo "--- parent attempt 1 ---"
"$BIN/scx_ref_parent_probe" "$DEV" 33 100 0 "$OUT/sb-parent-a1" "$WARMUP" "$SAMPLES" 2 2 2>&1 | tail -1
echo "--- cand attempt 1 ---"
"$BIN/scx_ref_v001_probe" "$DEV" 33 100 0 "$OUT/sb-cand-a1" "$WARMUP" "$SAMPLES" 2 2 2>&1 | tail -1

echo "=== 33x100 FP32 PAIRED 6 pairs ==="
for I in 1 2 3 4 5 6; do
    if [ $((I % 2)) -eq 1 ]; then
        "$BIN/scx_ref_parent_probe" "$DEV" 33 100 0 "$OUT/pc${I}-P" "$WARMUP" "$SAMPLES" 1 0 2>&1 | tail -1
        "$BIN/scx_ref_v001_probe" "$DEV" 33 100 0 "$OUT/pc${I}-C" "$WARMUP" "$SAMPLES" 1 0 2>&1 | tail -1
    else
        "$BIN/scx_ref_v001_probe" "$DEV" 33 100 0 "$OUT/pc${I}-C" "$WARMUP" "$SAMPLES" 1 0 2>&1 | tail -1
        "$BIN/scx_ref_parent_probe" "$DEV" 33 100 0 "$OUT/pc${I}-P" "$WARMUP" "$SAMPLES" 1 0 2>&1 | tail -1
    fi
    echo "  pair$I done"
    sleep 2
done

echo "=== 17x257 FP16 REGRESSION CONTROL ==="
echo "--- same-binary ---"
"$BIN/scx_ref_parent_probe" "$DEV" 17 257 1 "$OUT/sb-parent-257" "$WARMUP" "$SAMPLES" 2 2 2>&1 | tail -1
"$BIN/scx_ref_v001_probe" "$DEV" 17 257 1 "$OUT/sb-cand-257" "$WARMUP" "$SAMPLES" 2 2 2>&1 | tail -1
echo "--- paired 4 pairs ---"
for I in 1 2 3 4; do
    if [ $((I % 2)) -eq 1 ]; then
        "$BIN/scx_ref_parent_probe" "$DEV" 17 257 1 "$OUT/pc${I}-P-257" "$WARMUP" "$SAMPLES" 1 0 2>&1 | tail -1
        "$BIN/scx_ref_v001_probe" "$DEV" 17 257 1 "$OUT/pc${I}-C-257" "$WARMUP" "$SAMPLES" 1 0 2>&1 | tail -1
    else
        "$BIN/scx_ref_v001_probe" "$DEV" 17 257 1 "$OUT/pc${I}-C-257" "$WARMUP" "$SAMPLES" 1 0 2>&1 | tail -1
        "$BIN/scx_ref_parent_probe" "$DEV" 17 257 1 "$OUT/pc${I}-P-257" "$WARMUP" "$SAMPLES" 1 0 2>&1 | tail -1
    fi
    echo "  pair$I done"
    sleep 2
done

npu-smi info > "$OUT/npu-smi-post.txt" 2>&1
date -Is > "$OUT/end.timestamp.txt"
echo "REMEASURE_DONE"

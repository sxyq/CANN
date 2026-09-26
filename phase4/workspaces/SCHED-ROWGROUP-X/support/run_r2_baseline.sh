#!/usr/bin/env bash
# PARENT BASELINE x N on one device (shape 17x256 aligned control — mid cost, stable shape)
set -euo pipefail
ROOT="$(cd "$(dirname "$0")" && pwd)"
BUILD="$ROOT/build"
RESULT_DIR="${RESULT_DIR:-$ROOT/../results/results-round2/baseline}"
DEV="${ASCEND_DEVICE_ID:-4}"
N="${NBASE:-10}"
mkdir -p "$RESULT_DIR"
export LD_LIBRARY_PATH="/usr/lib/aarch64-linux-gnu:${LD_LIBRARY_PATH:-}"

# One representative shape for baseline gate: 17x256 FP32 (aligned control shape)
ROWS=17; WIDTH=256; DTYPE=0

echo "R2_BASELINE_START $(date -Is) device=$DEV n=$N"
npu-smi info > "$RESULT_DIR/npu-smi-before.txt" 2>&1 || true
date -Is > "$RESULT_DIR/start.timestamp.txt"

: > "$RESULT_DIR/parent_medians.tsv"
echo -e "rep\tmedian_us\tstdev_us\tmin_us\tmax_us\tbad" >> "$RESULT_DIR/parent_medians.tsv"

for i in $(seq 1 "$N"); do
  out="$RESULT_DIR/parent_rep$(printf '%02d' $i)"
  date -Is > "${out}.timestamp.txt"
  npu-smi info > "${out}.npu-smi.txt" 2>&1 || true
  "$BUILD/srx_parent_probe" "$DEV" "$ROWS" "$WIDTH" "$DTYPE" "$out" \
    > "${out}.stdout" 2> "${out}.stderr" || { echo "REP $i FAIL"; cat "${out}.stderr"; exit 1; }
  med=$(awk -F'\t' 'NR==2{print $7}' "${out}.tsv")
  bad=$(awk -F'\t' 'NR==2{print $9}' "${out}.tsv")
  stdev=$(awk -F'\t' '$1=="stdev_us"{print $2}' "${out}-jitter.txt" 2>/dev/null || echo nan)
  mn=$(awk -F'\t' '$1=="min_us"{print $2}' "${out}-jitter.txt" 2>/dev/null || echo nan)
  mx=$(awk -F'\t' '$1=="max_us"{print $2}' "${out}-jitter.txt" 2>/dev/null || echo nan)
  echo -e "${i}\t${med}\t${stdev}\t${mn}\t${mx}\t${bad}" >> "$RESULT_DIR/parent_medians.tsv"
  echo "REP $i med=$med stdev=$stdev bad=$bad"
done
echo "R2_BASELINE_END $(date -Is)"
echo R2_BASELINE_DONE

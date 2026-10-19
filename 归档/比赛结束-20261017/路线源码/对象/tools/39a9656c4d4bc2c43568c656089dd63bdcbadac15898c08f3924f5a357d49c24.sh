#!/usr/bin/env bash
# Parent-only baseline block x N for window qualification
set -euo pipefail
ROOT="$(cd "$(dirname "$0")" && pwd)"
BUILD="$ROOT/build"
DEV="${ASCEND_DEVICE_ID:-6}"
BLOCK="${BLOCK:-A}"
N="${NPRE:-6}"
RESULT_DIR="${RESULT_DIR:-$ROOT/../results/results-window-qual/precheck-${BLOCK}}"
mkdir -p "$RESULT_DIR"
export LD_LIBRARY_PATH="/usr/lib/aarch64-linux-gnu:${LD_LIBRARY_PATH:-}"
ROWS=17; WIDTH=256; DTYPE=0
echo "PRECHECK_${BLOCK}_START $(date -Is) device=${DEV} n=${N}"
npu-smi info > "$RESULT_DIR/npu-smi-before.txt" 2>&1 || true
date -Is > "$RESULT_DIR/start.timestamp.txt"
: > "$RESULT_DIR/parent_medians.tsv"
printf 'rep\tmedian_us\tstdev_us\tmin_us\tmax_us\tbad\n' >> "$RESULT_DIR/parent_medians.tsv"
i=1
while [ "$i" -le "$N" ]; do
  out="$RESULT_DIR/parent_rep$(printf '%02d' "$i")"
  date -Is > "${out}.timestamp.txt"
  "$BUILD/srx_parent_probe" "$DEV" "$ROWS" "$WIDTH" "$DTYPE" "$out" \
    > "${out}.stdout" 2> "${out}.stderr" || { echo FAIL; cat "${out}.stderr"; exit 1; }
  med=$(awk -F'\t' 'NR==2{print $7}' "${out}.tsv")
  bad=$(awk -F'\t' 'NR==2{print $9}' "${out}.tsv")
  stdev=$(awk -F'\t' '$1=="stdev_us"{print $2}' "${out}-jitter.txt" 2>/dev/null || echo nan)
  mn=$(awk -F'\t' '$1=="min_us"{print $2}' "${out}-jitter.txt" 2>/dev/null || echo nan)
  mx=$(awk -F'\t' '$1=="max_us"{print $2}' "${out}-jitter.txt" 2>/dev/null || echo nan)
  printf '%s\t%s\t%s\t%s\t%s\t%s\n' "$i" "$med" "$stdev" "$mn" "$mx" "$bad" >> "$RESULT_DIR/parent_medians.tsv"
  echo "  rep${i} med=${med} bad=${bad}"
  i=$((i+1))
done
echo "PRECHECK_${BLOCK}_END $(date -Is)"
echo "PRECHECK_${BLOCK}_DONE"

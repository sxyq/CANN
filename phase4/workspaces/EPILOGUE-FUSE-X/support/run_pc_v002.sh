#!/usr/bin/env bash
set -uo pipefail
cd "$(dirname "$0")"
export LD_LIBRARY_PATH="/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/lib64:${LD_LIBRARY_PATH:-}"
export ASCEND_DEVICE_ID=5
P=../build/efx_ref_parent
C=../build/efx_ref_v002
RES=results-timing-v002
WARM=45
SAMP=41
echo "tag,rows,width,dtype,pair,order,P_med_us,C_med_us,delta_pct" > "$RES/paired-deltas.csv"
run() {
  local bin=$1 tag=$2 rows=$3 width=$4 dtype=$5
  "$bin" 5 "$rows" "$width" "$dtype" "$RES/$tag" "$WARM" "$SAMP" 1 > "$RES/$tag.stdout.txt" 2> "$RES/$tag.stderr.txt"
}
for spec in "2 8192 0 fp32-2x8192" "4 8192 0 fp32-4x8192" "8 8192 0 fp32-8x8192" "2 4096 0 fp32-2x4096" "2 256 0 fp32-2x256-ctrl" "64 8192 0 fp32-64x8192-vmla"; do
  read -r rows width dtype tag <<< "$spec"
  for pair in 1 2 3 4 5 6; do
    if [ $((pair % 2)) -eq 1 ]; then order=PC; A=$P; B=$C; At=p; Bt=c
    else order=CP; A=$C; B=$P; At=c; Bt=p; fi
    run "$A" "${tag}-${At}-p${pair}" "$rows" "$width" "$dtype"
    run "$B" "${tag}-${Bt}-p${pair}" "$rows" "$width" "$dtype"
    pm=$(awk -F'\t' '$1=="ALL_DEVICE" && $2=="median_us"{print $3}' "$RES/${tag}-p-p${pair}-stats.txt" 2>/dev/null)
    cm=$(awk -F'\t' '$1=="ALL_DEVICE" && $2=="median_us"{print $3}' "$RES/${tag}-c-p${pair}-stats.txt" 2>/dev/null)
    d=$(python3 -c "p=float('${pm:-0}');c=float('${cm:-0}');print(round(100.0*(c-p)/p,4) if p>0 else 999)" 2>/dev/null)
    echo "${tag},${rows},${width},${dtype},${pair},${order},${pm:-NA},${cm:-NA},${d:-NA}" >> "$RES/paired-deltas.csv"
    echo "  $tag pair$pair $order P=${pm:-NA} C=${cm:-NA} delta=${d:-NA}%"
  done
done
echo "PC_DONE"

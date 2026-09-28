#!/usr/bin/env bash
set -uo pipefail
cd "$(dirname "$0")"
export LD_LIBRARY_PATH="/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/lib64:${LD_LIBRARY_PATH:-}"
export ASCEND_DEVICE_ID=4
P=../build/vmx_ref_parent
C=../build/vmx_ref_v002
RES=results-timing-v002
WARM=45
SAMP=41
PAIRS=6
rm -rf "$RES"; mkdir -p "$RES"
npu-smi info > "$RES/npu-smi-start.txt" 2>&1
date -Is > "$RES/start.timestamp.txt"

run() {
  local bin=$1 tag=$2 rows=$3 width=$4 dtype=$5
  "$bin" 4 "$rows" "$width" "$dtype" "$RES/$tag" "$WARM" "$SAMP" 1 0 > "$RES/$tag.stdout.txt" 2> "$RES/$tag.stderr.txt"
}
med() { awk -F'\t' -v t="$2" '$1==t && $2=="median_us"{print $3}' "$1" 2>/dev/null; }
fld() { awk -F'\t' -v t="$2" -v f="$3" '$1==t && $2==f{print $3}' "$1" 2>/dev/null; }

# tag rows width dtype class
SHAPES=(
  "fp32-8x256 8 256 0 SMALL"
  "fp32-32x256 32 256 0 SMALL"
  "fp32-2x100 2 100 0 SMALL"
  "fp32-2x4096 2 4096 0 MEDIUM"
  "fp32-4x2048 4 2048 0 MEDIUM"
  "fp32-8x1024 8 1024 0 MEDIUM"
  "fp32-8x8192 8 8192 0 LARGE"
  "fp32-2x8192 2 8192 0 LARGE"
  "fp32-1x32768 1 32768 0 LARGE"
  "fp16-8x256 8 256 1 SMALL"
  "bf16-8x256 8 256 2 SMALL"
)

echo "=== PHASE 1: SAME-BINARY (parent, 2x${SAMP}, warmup=${WARM}) ==="
echo "tag,class,rows,width,dtype,B1,B2,all_med,MAD_med,drift,verdict" > "$RES/samebinary-verdict.csv"
declare -A SB_OK
for spec in "${SHAPES[@]}"; do
  read -r tag rows width dtype cls <<< "$spec"
  run "$P" "sb-${tag}" "$rows" "$width" "$dtype"
  st="$RES/sb-${tag}-stats.txt"
  b1=$(med "$st" "B1_DEVICE"); b2=$(med "$st" "B2_DEVICE")
  am=$(med "$st" "ALL_DEVICE"); mad=$(fld "$st" "ALL_DEVICE" "MAD_us")
  drift=$(python3 -c "b1=float('${b1:-0}');b2=float('${b2:-0}');m=float('${am:-1}');print(abs(b1-b2)/m if m>0 else 9)" 2>/dev/null)
  mr=$(python3 -c "m=float('${mad:-9}');med=float('${am:-1}');print(m/med if med>0 else 9)" 2>/dev/null)
  v=$(python3 -c "print('PASS' if float('${mr:-9}')<=0.10 and float('${drift:-9}')<=0.10 else 'FAIL')" 2>/dev/null)
  SB_OK[$tag]=$v
  echo "${tag},${cls},${rows},${width},${dtype},${b1},${b2},${am},${mad},${drift},${v}" >> "$RES/samebinary-verdict.csv"
  echo "  $tag [$cls] B1=$b1 B2=$b2 MAD/med=$mr drift=$drift -> $v"
done

echo "=== PHASE 2: INTERLEAVED P/C (${PAIRS} pairs, ${SAMP} samples) ==="
echo "tag,class,rows,width,dtype,pair,order,P_med,C_med,delta_pct" > "$RES/paired-deltas.csv"
for spec in "${SHAPES[@]}"; do
  read -r tag rows width dtype cls <<< "$spec"
  for pair in $(seq 1 "$PAIRS"); do
    if [ $((pair % 2)) -eq 1 ]; then order=PC; A=$P; B=$C; At=p; Bt=c
    else order=CP; A=$C; B=$P; At=c; Bt=p; fi
    run "$A" "${tag}-${At}-p${pair}" "$rows" "$width" "$dtype"
    run "$B" "${tag}-${Bt}-p${pair}" "$rows" "$width" "$dtype"
    pm=$(med "$RES/${tag}-p-p${pair}-stats.txt" "ALL_DEVICE")
    cm=$(med "$RES/${tag}-c-p${pair}-stats.txt" "ALL_DEVICE")
    d=$(python3 -c "p=float('${pm:-0}');c=float('${cm:-0}');print(round(100.0*(c-p)/p,4) if p>0 else 999)" 2>/dev/null)
    echo "${tag},${cls},${rows},${width},${dtype},${pair},${order},${pm:-NA},${cm:-NA},${d:-NA}" >> "$RES/paired-deltas.csv"
    echo "  $tag [$cls] pair$pair $order P=${pm:-NA} C=${cm:-NA} delta=${d:-NA}%"
  done
done

npu-smi info > "$RES/npu-smi-end.txt" 2>&1
date -Is > "$RES/end.timestamp.txt"
echo "TIMING_DONE out=$RES"

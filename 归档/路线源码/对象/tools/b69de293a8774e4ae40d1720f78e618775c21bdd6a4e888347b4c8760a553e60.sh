#!/usr/bin/env bash
# EPILOGUE-FUSE-X V002 timing: same-binary then interleaved P/C.
set -uo pipefail
ROOT="$(cd "$(dirname "$0")" && pwd)"
BUILD="$ROOT/../build"
P="$BUILD/efx_ref_parent"
C="$BUILD/efx_ref_v002"
DEV=5
WARM=45
SB_SAMP=41
PC_SAMP=41
PC_PAIRS=6
export LD_LIBRARY_PATH="/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/lib64:/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/lib64:${LD_LIBRARY_PATH:-}"
export ASCEND_DEVICE_ID=$DEV

SHAPES=(
  "2 8192 0 fp32-2x8192"
  "4 8192 0 fp32-4x8192"
  "8 8192 0 fp32-8x8192"
  "2 4096 0 fp32-2x4096"
  "2 256 0 fp32-2x256-ctrl"
  "64 8192 0 fp32-64x8192-vmla"
)

RES="$ROOT/results-timing-v002"
rm -rf "$RES"; mkdir -p "$RES"
npu-smi info > "$RES/npu-smi-start.txt" 2>&1 || true
date -Is > "$RES/start.timestamp.txt"

sb_csv="$RES/samebinary-verdict.csv"
echo "tag,rows,width,dtype,B1_med,B2_med,all_med,MAD_med,drift,verdict" > "$sb_csv"

run_one() {
  local bin="$1" tag="$2" rows="$3" width="$4" dtype="$5" warm="$6" samp="$7" blocks="$8"
  "$bin" "$DEV" "$rows" "$width" "$dtype" "$RES/${tag}" "$warm" "$samp" "$blocks" 2 \
    > "$RES/${tag}.stdout.txt" 2> "$RES/${tag}.stderr.txt"
  echo $?
}
med() { awk -F'\t' -v t="$2" '$1==t && $2=="median_us"{print $3}' "$1"; }
fld() { awk -F'\t' -v t="$2" '$1==t && $2==F{print $3}' F="$3" "$1"; }

echo "=== PHASE 1: SAME-BINARY (parent, 2x${SB_SAMP}, warmup=${WARM}) ==="
declare -A SB_OK
for spec in "${SHAPES[@]}"; do
  read -r rows width dtype tag <<< "$spec"
  rc=$(run_one "$P" "sb-${tag}" "$rows" "$width" "$dtype" "$WARM" "$SB_SAMP" 2)
  st="$RES/sb-${tag}-stats.txt"
  b1=$(med "$st" "B1_DEVICE"); b2=$(med "$st" "B2_DEVICE")
  am=$(med "$st" "ALL_DEVICE")
  mad=$(fld "$st" "ALL_DEVICE" "MAD_us")
  drift=$(python3 -c "b1=float('${b1}');b2=float('${b2}');m=float('${am}');print(abs(b1-b2)/m if m>0 else 9)")
  mr=$(python3 -c "m=float('${mad}');med=float('${am}');print(m/med if med>0 else 9)")
  v=$(python3 -c "print('PASS' if float('${mr}')<=0.10 and float('${drift}')<=0.10 else 'FAIL')")
  SB_OK[$tag]=$v
  echo "${tag},${rows},${width},${dtype},${b1},${b2},${am},${mad},${drift},${v}" >> "$sb_csv"
  echo "  $tag rc=$rc B1=$b1 B2=$b2 MAD/med=$mr drift=$drift -> $v"
done

echo "=== PHASE 2: INTERLEAVED P/C (>=${PC_PAIRS} pairs, ${PC_SAMP} samples) ==="
pc_csv="$RES/paired-deltas.csv"
echo "tag,rows,width,dtype,pair,order,P_med_us,C_med_us,delta_pct" > "$pc_csv"

for spec in "${SHAPES[@]}"; do
  read -r rows width dtype tag <<< "$spec"
  sbv=${SB_OK[$tag]}
  if [ "$sbv" != "PASS" ]; then
    echo "  SKIP $tag (same-binary $sbv)"
    echo "${tag},${rows},${width},${dtype},0,SKIP,0,0,0" >> "$pc_csv"
    continue
  fi
  for pair in $(seq 1 "$PC_PAIRS"); do
    if [ $((pair % 2)) -eq 1 ]; then order="PC"; A="$P"; B="$C"; Atag="p"; Btag="c"
    else order="CP"; A="$C"; B="$P"; Atag="c"; Btag="p"; fi
    run_one "$A" "${tag}-${Atag}-p${pair}" "$rows" "$width" "$dtype" "$WARM" "$PC_SAMP" 1 >/dev/null
    run_one "$B" "${tag}-${Btag}-p${pair}" "$rows" "$width" "$dtype" "$WARM" "$PC_SAMP" 1 >/dev/null
    pmed=$(med "$RES/${tag}-p-p${pair}-stats.txt" "ALL_DEVICE")
    cmed=$(med "$RES/${tag}-c-p${pair}-stats.txt" "ALL_DEVICE")
    d=$(python3 -c "p=float('${pmed}');c=float('${cmed}');print(round(100.0*(c-p)/p,4) if p>0 else 999)")
    echo "${tag},${rows},${width},${dtype},${pair},${order},${pmed},${cmed},${d}" >> "$pc_csv"
    echo "  $tag pair$pair $order P=$pmed C=$cmed delta=${d}%"
  done
done

npu-smi info > "$RES/npu-smi-end.txt" 2>&1 || true
date -Is > "$RES/end.timestamp.txt"
echo "TIMING_DONE out=$RES"

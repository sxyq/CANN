#!/usr/bin/env bash
# EPILOGUE-FUSE-X V001 timing: same-binary qual then interleaved P/C.
# Protocol: warmup=45, device-event primary, same-binary 2x31, P/C >=4 pairs.
set -uo pipefail
ROOT="$(cd "$(dirname "$0")" && pwd)"
BUILD="$ROOT/../build"
P="$BUILD/efx_ref_parent"
C="$BUILD/efx_ref_v001"
DEV=5
WARM=45
SB_SAMP=31
PC_SAMP=21
PC_PAIRS=4
export LD_LIBRARY_PATH="/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/lib64:/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/lib64:${LD_LIBRARY_PATH:-}"
export ASCEND_DEVICE_ID=$DEV

# shape rows width dtype tag
SHAPES=(
  "8 256 0 sb-fp32-8x256"
  "2 4096 0 sb-fp32-2x4096"
  "4 8192 0 sb-fp32-4x8192"
  "8 256 1 sb-fp16-8x256"
  "8 256 2 sb-bf16-8x256"
  "2 16384 0 sb-ctrl-wide-2x16384"
)

OUT="$ROOT"
RES="$OUT/results-timing-v001"
mkdir -p "$RES"
npu-smi info > "$RES/npu-smi-start.txt" 2>&1 || true
date -Is > "$RES/start.timestamp.txt"

sb_pass_csv="$RES/samebinary-verdict.csv"
echo "tag,rows,width,dtype,B1_med,B2_med,MAD_med,drift,verdict" > "$sb_pass_csv"

run_one() {
  local bin="$1" tag="$2" rows="$3" width="$4" dtype="$5" warm="$6" samp="$7" blocks="$8"
  local prefix="$RES/${tag}"
  "$bin" "$DEV" "$rows" "$width" "$dtype" "$prefix" "$warm" "$samp" "$blocks" 2 \
    > "${prefix}.stdout.txt" 2> "${prefix}.stderr.txt"
  echo $?
}

extract_med() {
  # args: statsfile blocktag  -> prints median_us
  awk -F'\t' -v t="$2" '$1==t && $2=="median_us"{print $3}' "$1"
}

extract_field() {
  awk -F'\t' -v t="$2" '$1==t{print $3}' "$1"
}

echo "=== PHASE 1: SAME-BINARY (parent binary, 2 blocks x ${SB_SAMP}, warmup=${WARM}) ==="
declare -A SB_OK
for spec in "${SHAPES[@]}"; do
  read -r rows width dtype tag <<< "$spec"
  rc=$(run_one "$P" "${tag}" "$rows" "$width" "$dtype" "$WARM" "$SB_SAMP" 2)
  stats="$RES/${tag}-stats.txt"
  b1=$(extract_med "$stats" "B1_DEVICE")
  b2=$(extract_med "$stats" "B2_DEVICE")
  mad=$(extract_field "$stats" "ALL_DEVICE" )
  # recompute from raw: use ALL_DEVICE MAD and median
  all_med=$(extract_med "$stats" "ALL_DEVICE")
  all_mad=$(awk -F'\t' '$1=="ALL_DEVICE" && $2=="MAD_us"{print $3}' "$stats")
  drift=$(python3 -c "b1=float('${b1}');b2=float('${b2}');m=float('${all_med}');print(abs(b1-b2)/m if m>0 else 9)" 2>/dev/null)
  madratio=$(python3 -c "m=float('${all_mad}');med=float('${all_med}');print(m/med if med>0 else 9)" 2>/dev/null)
  verdict="FAIL"
  python3 -c "
d=float('${drift}'); r=float('${madratio}')
print('PASS' if r<=0.10 and d<=0.10 else 'FAIL')" > /tmp/sbv.txt
  verdict=$(cat /tmp/sbv.txt)
  SB_OK[$tag]=$verdict
  echo "${tag},${rows},${width},${dtype},${b1},${b2},${all_mad},${drift},${verdict}" >> "$sb_pass_csv"
  echo "  $tag rc=$rc B1=$b1 B2=$b2 MAD/med=$madratio drift=$drift -> $verdict"
done

echo "=== PHASE 2: INTERLEAVED P/C (>=${PC_PAIRS} pairs, ${PC_SAMP} samples) ==="
pc_csv="$RES/paired-deltas.csv"
echo "tag,rows,width,dtype,pair,order,P_med_us,C_med_us,delta_pct" > "$pc_csv"

for spec in "${SHAPES[@]}"; do
  read -r rows width dtype tag <<< "$spec"
  sbv=${SB_OK[$tag]}
  if [ "$sbv" != "PASS" ]; then
    echo "  SKIP $tag (same-binary $sbv) -> MEASUREMENT_PROTOCOL_BLOCKED_FOR_SHAPE"
    echo "${tag},${rows},${width},${dtype},0,SKIP,0,0,0" >> "$pc_csv"
    continue
  fi
  for pair in $(seq 1 "$PC_PAIRS"); do
    if [ $((pair % 2)) -eq 1 ]; then order="PC"; A="$P"; B="$C"; Atag="p"; Btag="c"
    else order="CP"; A="$C"; B="$P"; Atag="c"; Btag="p"; fi
    rcA=$(run_one "$A" "${tag}-${Atag}-p${pair}" "$rows" "$width" "$dtype" "$WARM" "$PC_SAMP" 1)
    rcB=$(run_one "$B" "${tag}-${Btag}-p${pair}" "$rows" "$width" "$dtype" "$WARM" "$PC_SAMP" 1)
    pstats="$RES/${tag}-p-p${pair}-stats.txt"
    cstats="$RES/${tag}-c-p${pair}-stats.txt"
    # with order CP the filenames still use p/c tags from Atag/Btag
    if [ "$Atag" = "c" ]; then
      cstats="$RES/${tag}-c-p${pair}-stats.txt"
      pstats="$RES/${tag}-p-p${pair}-stats.txt"
    fi
    pmed=$(extract_med "$pstats" "ALL_DEVICE")
    cmed=$(extract_med "$cstats" "ALL_DEVICE")
    delta=$(python3 -c "p=float('${pmed}');c=float('${cmed}');print(round(100.0*(c-p)/p,4) if p>0 else 999)")
    echo "${tag},${rows},${width},${dtype},${pair},${order},${pmed},${cmed},${delta}" >> "$pc_csv"
    echo "  $tag pair$pair $order P=$pmed C=$cmed delta=${delta}%"
  done
done

npu-smi info > "$RES/npu-smi-end.txt" 2>&1 || true
date -Is > "$RES/end.timestamp.txt"
echo "TIMING_DONE out=$RES"

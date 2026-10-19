#!/usr/bin/env bash
set -uo pipefail
cd "$(dirname "$0")"
export LD_LIBRARY_PATH="/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/lib64:${LD_LIBRARY_PATH:-}"
P=../build/vmx_ref_parent
C=../build/vmx_ref_v003
RES=results-s1-v003
WARM=45; SAMP=41; PAIRS=6; DEV=${1:-5}
rm -rf "$RES"; mkdir -p "$RES"
npu-smi info > "$RES/npu-smi-start.txt" 2>&1; date -Is > "$RES/start.txt"
run() { "$1" "$DEV" "$2" "$3" 0 "$RES/$4" "$WARM" "$SAMP" 2 0 > "$RES/$4.out" 2> "$RES/$4.err"; }
med() { awk -F"\t" -v t="$2" "\$1==t && \$2==\"median_us\"{print \$3}" "$1" 2>/dev/null; }
fld() { awk -F"\t" -v t="$2" -v f="$3" "\$1==t && \$2==f{print \$3}" "$1" 2>/dev/null; }
SHAPES=("768x256 768 256" "1024x256 1024 256" "512x512 512 512" "1024x128 1024 128" "768x128 768 128" "2048x64 2048 64")
echo "=== S1 SAME-BINARY (parent blocks=2) ==="
echo "tag,med,MAD,b1,b2,mad_ratio,drift,verdict" > "$RES/sb.csv"
for spec in "${SHAPES[@]}"; do
  set -- $spec; tag=$1; r=$2; w=$3
  run "$P" "$r" "$w" "sb-$tag"
  st="$RES/sb-$tag-stats.txt"
  am=$(med "$st" ALL_DEVICE); mad=$(fld "$st" ALL_DEVICE MAD_us)
  b1=$(med "$st" B1_DEVICE); b2=$(med "$st" B2_DEVICE)
  mr=$(python3 -c "print(round(float()/float(),4))")
  dr=$(python3 -c "print(round(abs(float()-float())/float(),4))")
  v=$(python3 -c "print(PASS if float()<=0.10 and float()<=0.10 else FAIL)")
  echo "$tag,$am,$mad,$b1,$b2,$mr,$dr,$v" >> "$RES/sb.csv"
  echo "  $tag med=$am MAD/med=$mr drift=$dr -> $v"
done
echo "=== S1 P/C ($PAIRS pairs) ==="
echo "tag,pair,order,P_med,C_med,delta_pct" > "$RES/pc.csv"
for spec in "${SHAPES[@]}"; do
  set -- $spec; tag=$1; r=$2; w=$3
  for pair in $(seq 1 $PAIRS); do
    if [ $((pair % 2)) -eq 1 ]; then order=PC; A=$P; B=$C; At=p; Bt=c
    else order=CP; A=$C; B=$P; At=c; Bt=p; fi
    run "$A" "$r" "$w" "$tag-$At-p$pair"
    run "$B" "$r" "$w" "$tag-$Bt-p$pair"
    pm=$(med "$RES/$tag-p-p$pair-stats.txt" ALL_DEVICE)
    cm=$(med "$RES/$tag-c-p$pair-stats.txt" ALL_DEVICE)
    d=$(python3 -c "print(round(100.0*(float()-float())/float(),3))")
    echo "$tag,$pair,$order,$pm,$cm,$d" >> "$RES/pc.csv"
    echo "  $tag pair$pair $order P=$pm C=$cm d=$d%"
  done
done
date -Is > "$RES/end.txt"; echo "S1_DONE"

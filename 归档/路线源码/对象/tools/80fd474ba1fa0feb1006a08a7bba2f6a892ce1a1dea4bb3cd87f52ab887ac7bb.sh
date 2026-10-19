#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")" && pwd)"
BUILD="$ROOT/build"
RESULT_DIR="${RESULT_DIR:-$ROOT/../results/probes}"
DEV="${DEVICE_ID:-6}"
mkdir -p "$RESULT_DIR"
export LD_LIBRARY_PATH="/usr/lib/aarch64-linux-gnu:${LD_LIBRARY_PATH:-}"

PROBES=(
  "unaligned_d65 7 65 0"
  "unaligned_d100 33 100 0"
  "unaligned_d257 17 257 0"
  "aligned_d256 17 256 0"
)

for p in 01 02 03 04; do
  date -Is > "$RESULT_DIR/pair-$p.timestamp.txt"
  npu-smi info > "$RESULT_DIR/pair-$p.npu-smi.txt" 2>&1 || true
  idx=$(( (10#$p - 1) % 4 ))
  read -r label rows width dtype <<< "${PROBES[$idx]}"
  if [ $(( 10#$p % 2 )) -eq 1 ]; then
    order="parent v001"
  else
    order="v001 parent"
  fi
  echo "PAIR $p label=$label rows=$rows width=$width dtype=$dtype order=$order"
  for v in $order; do
    out="$RESULT_DIR/pair-$p-${v}_${label}"
    "$BUILD/srx_${v}_probe" "$DEV" "$rows" "$width" "$dtype" "$out" \
      > "${out}.stdout" 2> "${out}.stderr" || {
        echo "  $v FAILED"; cat "${out}.stderr"; exit 1;
      }
    echo "  $v $(tail -1 "${out}.tsv")"
    cp "${out}.tsv" "$RESULT_DIR/pair-$p-${v}.tsv"
  done
done
echo PAIRED_PROBES_DONE

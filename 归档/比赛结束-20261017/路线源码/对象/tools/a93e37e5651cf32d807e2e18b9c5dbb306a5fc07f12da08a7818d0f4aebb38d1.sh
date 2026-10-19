#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")" && pwd)"
BUILD="$ROOT/build"
RESULT_DIR="${RESULT_DIR:-$ROOT/../results/results-controlled-dev4}"
DEV="${ASCEND_DEVICE_ID:-4}"
mkdir -p "$RESULT_DIR"
export LD_LIBRARY_PATH="/usr/lib/aarch64-linux-gnu:${LD_LIBRARY_PATH:-}"

# 3 unaligned + 1 aligned control (17x256)
# Orders for pairs 1..4: PC, PC, CP, CP
PAIRS=(
  "01 unaligned_d65 7 65 0 PC"
  "02 unaligned_d100 33 100 0 PC"
  "03 unaligned_d257 17 257 0 CP"
  "04 aligned_d256 17 256 0 CP"
)

echo "L004_START $(date -Is) device=$DEV"
npu-smi info > "$RESULT_DIR/npu-smi-before-session.txt" 2>&1 || true

for spec in "${PAIRS[@]}"; do
  read -r pid label rows width dtype order <<< "$spec"
  date -Is > "$RESULT_DIR/pair-$pid.timestamp.txt"
  npu-smi info > "$RESULT_DIR/pair-$pid.npu-smi.txt" 2>&1 || true
  if [ "$order" = "PC" ]; then
    seq="parent v001"
  else
    seq="v001 parent"
  fi
  echo "PAIR $pid label=$label rows=$rows width=$width dtype=$dtype order=$order dev=$DEV"
  for v in $seq; do
    out="$RESULT_DIR/pair-$pid-${v}_${label}"
    "$BUILD/srx_${v}_probe" "$DEV" "$rows" "$width" "$dtype" "$out" \
      > "${out}.stdout" 2> "${out}.stderr" || {
        echo "  $v FAILED"; cat "${out}.stderr"; exit 1;
      }
    echo "  $v $(tail -1 "${out}.tsv")"
    cp "${out}.tsv" "$RESULT_DIR/pair-$pid-${v}.tsv"
    [ -f "${out}-jitter.txt" ] && cp "${out}-jitter.txt" "$RESULT_DIR/pair-$pid-${v}-jitter.txt"
    [ -f "${out}-samples.tsv" ] && cp "${out}-samples.tsv" "$RESULT_DIR/pair-$pid-${v}-samples.tsv"
  done
  npu-smi info > "$RESULT_DIR/pair-$pid.npu-smi-after.txt" 2>&1 || true
done
echo "L004_END $(date -Is)"
echo L004_DONE

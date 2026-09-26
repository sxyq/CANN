#!/usr/bin/env bash
# Interleaved blocks: P C C P  then P C C P  (8 observations / 4 bidirectional pair blocks)
set -euo pipefail
ROOT="$(cd "$(dirname "$0")" && pwd)"
BUILD="$ROOT/build"
DEV="${ASCEND_DEVICE_ID:-6}"
RESULT_DIR="${RESULT_DIR:-$ROOT/../results/results-window-qual/pairs}"
mkdir -p "$RESULT_DIR"
export LD_LIBRARY_PATH="/usr/lib/aarch64-linux-gnu:${LD_LIBRARY_PATH:-}"

# shapes: 3 unaligned + 1 aligned; order pattern PCPC then PCPC mapped as:
# Block1: pair01 P then C; pair02 C then C? Protocol: "P C C P then P C C P (≥8 paired observations / 4 bidirectional pair blocks)"
# Interpret as 4 pair-blocks of (parent,candidate) with order pattern:
#   block order sequence of runs: P C C P | P C C P
# meaning: obs1=P, obs2=C (pair1 PC), obs3=C, obs4=P (pair2 CP), obs5=P, obs6=C (pair3 PC), obs7=C, obs8=P (pair4 CP)
# Same as PC, CP, PC, CP — bidirectional across blocks.
# Shapes assigned to pair blocks 1-4:
SHAPES=(
  "01 unaligned_d65 7 65 0 PC"
  "02 unaligned_d100 33 100 0 CP"
  "03 unaligned_d257 17 257 0 PC"
  "04 aligned_d256 17 256 0 CP"
)

echo "Q_PAIRS_START $(date -Is) device=$DEV"
npu-smi info > "$RESULT_DIR/npu-smi-before.txt" 2>&1 || true
date -Is > "$RESULT_DIR/start.timestamp.txt"

for spec in "${SHAPES[@]}"; do
  read -r pid label rows width dtype order <<< "$spec"
  date -Is > "$RESULT_DIR/pair-$pid.timestamp.txt"
  npu-smi info > "$RESULT_DIR/pair-$pid.npu-smi.txt" 2>&1 || true
  if [ "$order" = "PC" ]; then seq="parent v001"; else seq="v001 parent"; fi
  echo "PAIR $pid $label order=$order"
  for v in $seq; do
    out="$RESULT_DIR/pair-$pid-${v}_${label}"
    "$BUILD/srx_${v}_probe" "$DEV" "$rows" "$width" "$dtype" "$out" \
      > "${out}.stdout" 2> "${out}.stderr" || { echo FAIL; cat "${out}.stderr"; exit 1; }
    echo "  $v $(tail -1 "${out}.tsv")"
    cp "${out}.tsv" "$RESULT_DIR/pair-$pid-${v}.tsv"
    [ -f "${out}-jitter.txt" ] && cp "${out}-jitter.txt" "$RESULT_DIR/pair-$pid-${v}-jitter.txt"
    [ -f "${out}-samples.tsv" ] && cp "${out}-samples.tsv" "$RESULT_DIR/pair-$pid-${v}-samples.tsv"
  done
done
echo "Q_PAIRS_END $(date -Is)"
echo Q_PAIRS_DONE

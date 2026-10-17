#!/usr/bin/env bash
# REDUCE-INVSCALE-X V001: NPU correctness matrix, parent vs V001.
set -uo pipefail

ROOT="$(cd "$(dirname "$0")" && pwd)"
RESULT_DIR="${RESULT_DIR:-$ROOT/results}"
DEVICE_ID="${DEVICE_ID:-6}"
ASCEND_HOME_PATH="${ASCEND_HOME_PATH:-/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002}"
set +u; source "${ASCEND_HOME_PATH}/bin/setenv.bash" >/dev/null 2>&1 || true; set -u

mkdir -p "$RESULT_DIR"
date -u +%Y-%m-%dT%H:%M:%SZ > "$RESULT_DIR/correctness.timestamp.txt"
npu-smi info > "$RESULT_DIR/correctness.npu-smi.txt" 2>&1

SHAPES=(
  "1 64 0"
  "1 256 0"
  "1 1024 0"
  "1 4096 0"
  "1 6144 0"
  "1 8192 0"
  "1 16384 0"
  "1 32768 0"
  "3 256 0"
  "3 8192 0"
  "1 65 0"
  "1 100 1"
  "1 256 1"
  "1 256 2"
  "1 8192 1"
  "1 8192 2"
)

SUMMARY="$RESULT_DIR/correctness-summary.tsv"
echo -e "variant\trows\twidth\tdtype\texit\tmedian_us\tmax_abs\tbad\tgross_bad" > "$SUMMARY"

for shape in "${SHAPES[@]}"; do
  read -r rows width dtype <<< "$shape"
  for variant in parent candidate; do
    bin="$ROOT/build/reduce_invscale_${variant}_probe"
    prefix="$RESULT_DIR/${variant}_r${rows}_d${width}_t${dtype}"
    out="$("$bin" "$DEVICE_ID" "$rows" "$width" "$dtype" "$prefix" 2>&1)"
    rc=$?
    echo "$out" | tee "${prefix}.stdout.txt" >/dev/null
    tsv="${prefix}.tsv"
    if [[ -f "$tsv" ]]; then
      line=$(tail -n 1 "$tsv")
      median=$(echo "$line" | cut -f7)
      maxabs=$(echo "$line" | cut -f8)
      bad=$(echo "$line" | cut -f9)
      gross=$(echo "$line" | cut -f10)
    else
      median=NA; maxabs=NA; bad=NA; gross=NA
    fi
    echo -e "${variant}\t${rows}\t${width}\t${dtype}\t${rc}\t${median}\t${maxabs}\t${bad}\t${gross}" | tee -a "$SUMMARY"
  done
done

echo "=== correctness matrix done ==="
column -t -s$'\t' "$SUMMARY"

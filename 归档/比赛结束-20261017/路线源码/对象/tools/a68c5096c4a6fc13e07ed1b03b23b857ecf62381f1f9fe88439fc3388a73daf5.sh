#!/usr/bin/env bash
# MULTIROW-DMA-CHAMPION-X V001: NPU correctness, R31B-V011 parent vs V001.
set -uo pipefail
ROOT="$(cd "$(dirname "$0")" && pwd)"
RESULT_DIR="${RESULT_DIR:-$ROOT/results}"
DEVICE_ID="${DEVICE_ID:-0}"
ASCEND_HOME_PATH=${ASCEND_HOME_PATH:-/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002}
set +u; source "${ASCEND_HOME_PATH}/bin/setenv.bash" >/dev/null 2>&1 || true; set -u
mkdir -p "$RESULT_DIR"
date -u +%Y-%m-%dT%H:%M:%SZ > "$RESULT_DIR/correctness.timestamp.txt"
npu-smi info > "$RESULT_DIR/correctness.npu-smi.txt" 2>&1
SHAPES=(
  "48 16384 1"
  "64 16384 1"
  "48 12288 1"
  "96 12288 1"
  "48 12288 2"
  "8 16384 1"
  "16 16384 1"
  "8 32768 1"
  "12 12288 1"
  "12 12288 2"
  "32 32768 1"
  "48 20000 1"
  "48 8202 1"
  "1 256 1"
  "2 4096 1"
  "2 4096 2"
  "1 8192 0"
  "2 8192 0"
  "8 16384 0"
  "2 32768 0"
  "33 100 0"
  "17 257 1"
)
SUMMARY="$RESULT_DIR/correctness-summary.tsv"
echo -e "variant\trows\twidth\tdtype\texit\tmedian_us\tmax_abs\tbad\tgross_bad" > "$SUMMARY"
fail=0
for shape in "${SHAPES[@]}"; do
  read -r rows width dtype <<< "$shape"
  for variant in parent candidate; do
    bin="$ROOT/build/mrx_${variant}_probe"
    prefix="$RESULT_DIR/${variant}_r${rows}_d${width}_t${dtype}"
    out="$("$bin" "$DEVICE_ID" "$rows" "$width" "$dtype" "$prefix" 2>&1)"
    rc=$?
    echo "$out" > "${prefix}.stdout.txt"
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
    if [[ "$variant" == "candidate" && "$rc" != "0" ]]; then
      fail=1
    fi
  done
done
echo "=== correctness matrix done ==="
column -t -s$'\t' "$SUMMARY"
exit $fail

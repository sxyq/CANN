#!/usr/bin/env bash
# REDUCE-INVSCALE-X V002: paired local probes, Direct Parent V001 vs V002.
# Alternating order, load snapshot per pair.
#
# Shape: FP32 rows=1 D=32768 (6 tiles) -- the regime the V002 repair operates
# in.  NOTE: Direct Parent V001 still fails NPU correctness on this shape (the
# defect V002 repairs), so only the timing delta is comparable here; this is a
# cost-of-repair measurement, not a promotion comparison.
set -uo pipefail

ROOT="$(cd "$(dirname "$0")" && pwd)"
RESULT_DIR="${RESULT_DIR:-$ROOT/results}"
DEVICE_ID="${DEVICE_ID:-6}"
ROWS="${ROWS:-1}"
WIDTH="${WIDTH:-32768}"
DTYPE="${DTYPE:-0}"
ASCEND_HOME_PATH="${ASCEND_HOME_PATH:-/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002}"
set +u; source "${ASCEND_HOME_PATH}/bin/setenv.bash" >/dev/null 2>&1 || true; set -u

mkdir -p "$RESULT_DIR"
PROBE_LOG="$RESULT_DIR/probes.tsv"
echo -e "pair\ttimestamp_utc\torder\tvariant\tmedian_us\tmax_abs\tbad\tgross_bad" > "$PROBE_LOG"

for pair in 01 02 03 04; do
  ts=$(date -u +%Y-%m-%dT%H:%M:%SZ)
  npu-smi info > "$RESULT_DIR/probe-pair-$pair.npu-smi.txt" 2>&1
  if [[ "$pair" == "02" || "$pair" == "04" ]]; then
    order="candidate,v001ref"
    variants=(candidate v001ref)
  else
    order="v001ref,candidate"
    variants=(v001ref candidate)
  fi
  for variant in "${variants[@]}"; do
    bin="$ROOT/build/reduce_invscale_${variant}_probe"
    prefix="$RESULT_DIR/probe-pair-$pair-${variant}"
    "$bin" "$DEVICE_ID" "$ROWS" "$WIDTH" "$DTYPE" "$prefix" > "${prefix}.stdout.txt" 2>&1
    rc=$?
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
    echo -e "${pair}\t${ts}\t${order}\t${variant}\t${median}\t${maxabs}\t${bad}\t${gross}" >> "$PROBE_LOG"
    echo "pair=$pair ts=$ts order=$order variant=$variant median_us=$median bad=$bad rc=$rc"
  done
done

echo "=== probes done ==="
cat "$PROBE_LOG"

#!/usr/bin/env bash
# REDUCE-INVSCALE-X V001: paired local probes, parent vs V001.
# Alternating order, load snapshot per pair. Shape chosen so BOTH variants
# pass NPU correctness (D <= kReduceTileElems = 6144).
set -uo pipefail

ROOT="$(cd "$(dirname "$0")" && pwd)"
RESULT_DIR="${RESULT_DIR:-$ROOT/results}"
DEVICE_ID="${DEVICE_ID:-6}"
ROWS="${ROWS:-1}"
WIDTH="${WIDTH:-6144}"
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
    order="v001,parent"
    variants=(v001 parent)
  else
    order="parent,v001"
    variants=(parent v001)
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

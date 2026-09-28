#!/usr/bin/env bash
# LEASE L005 -- controlled repair/orthogonality measurement, device 4 exclusive.
# Candidates (Main-assigned):
#   parent    = FULL-R006-V001-REDUCTION-ARCH  (R006, Duplicate+Div, no invscale)
#   candidate = REDUCE-INVSCALE-X V002         (R006 + R019 invscale + sync fix)
# Shape: FP32 rows=1 D=6144 -- BOTH variants pass NPU correctness here.
# Interleaved PC/CP order, ≥4 pairs, load snapshot per pair + pre/post lease.
# No candidate source is read or written by this script.
set -uo pipefail

ROOT="$(cd "$(dirname "$0")" && pwd)"
RESULT_DIR="${RESULT_DIR:-$ROOT/results_L005}"
DEVICE_ID="${DEVICE_ID:-4}"
ROWS="${ROWS:-1}"
WIDTH="${WIDTH:-6144}"
DTYPE="${DTYPE:-0}"
PAIRS="${PAIRS:-6}"
ASCEND_HOME_PATH="${ASCEND_HOME_PATH:-/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002}"
set +u; source "${ASCEND_HOME_PATH}/bin/setenv.bash" >/dev/null 2>&1 || true; set -u

snapshot() {
  local tag="$1"
  local ts aic hbm proc free
  ts=$(date -u +%Y-%m-%dT%H:%M:%SZ)
  npu-smi info > "$RESULT_DIR/${tag}.npu-smi.txt" 2>&1
  aic=$(grep -B1 "0000:01:00.0" "$RESULT_DIR/${tag}.npu-smi.txt" | grep -A1 "910B3" | tail -1 | awk -F'|' '{print $4}' | tr -d ' ')
  aic=$(npu-smi info 2>/dev/null | grep -A1 "0000:01:00.0" | tail -1 | awk -F'|' '{print $4}' | tr -d ' ')
  hbm=$(grep "0000:01:00.0" "$RESULT_DIR/${tag}.npu-smi.txt" | awk -F'|' '{print $5}' | tr -d ' ')
  proc=$(sed -n '/Process id/,$p' "$RESULT_DIR/${tag}.npu-smi.txt" | grep -E '^\| '"$DEVICE_ID"' ' | awk -F'|' '{print $3 $4}' | tr -s ' ' | tr '\n' ';' )
  echo -e "${tag}\t${ts}\tDEVICE_ID=${DEVICE_ID}\tAICORE=${aic}\tHBM=${hbm}\tPROCS=${proc:-none}" >> "$RESULT_DIR/lease.tsv"
}

mkdir -p "$RESULT_DIR"
echo -e "tag\ttimestamp_utc\tdevice\taicore\thbm\tprocs" > "$RESULT_DIR/lease.tsv"
snapshot preflight

# Abort guard: no other next6 probe anywhere
if npu-smi info 2>/dev/null | sed -n '/Process id/,$p' | grep -qiE 'prob|probe|invscale|rowgroup|liveness|align_tail|async_triple|batch_resident|sched_row'; then
  echo "ABORT: another next6 probe detected at preflight"
  exit 9
fi

PROBE_LOG="$RESULT_DIR/probes.tsv"
echo -e "pair\ttimestamp_utc\torder\tvariant\tmedian_us\tmax_abs\tbad\tgross_bad" > "$PROBE_LOG"

for pair in $(seq -w 1 "$PAIRS"); do
  n=$((10#$pair))
  ts=$(date -u +%Y-%m-%dT%H:%M:%SZ)
  npu-smi info > "$RESULT_DIR/probe-pair-$pair.npu-smi.txt" 2>&1
  if (( n % 2 == 0 )); then
    order="candidate,parent"
    variants=(candidate parent)
  else
    order="parent,candidate"
    variants=(parent candidate)
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

snapshot postlease
echo "=== probes done ==="
cat "$PROBE_LOG"
echo "=== lease ==="
cat "$RESULT_DIR/lease.tsv"

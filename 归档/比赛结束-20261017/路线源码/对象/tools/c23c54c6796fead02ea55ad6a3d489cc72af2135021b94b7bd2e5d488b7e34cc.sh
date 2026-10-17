#!/usr/bin/env bash
# ROUND-2 baseline: repeated PARENT-only runs to gate measurement stability.
# No candidate runs here. No source is read or written.
set -uo pipefail

ROOT="$(cd "$(dirname "$0")" && pwd)"
DEVICE_ID="${DEVICE_ID:-4}"
RUNS="${RUNS:-10}"
ROWS="${ROWS:-1}"
WIDTH="${WIDTH:-6144}"
DTYPE="${DTYPE:-0}"
RESULT_DIR="${RESULT_DIR:-$ROOT/results-round2/baseline-d${DEVICE_ID}}"
ASCEND_HOME_PATH="${ASCEND_HOME_PATH:-/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002}"
set +u; source "${ASCEND_HOME_PATH}/bin/setenv.bash" >/dev/null 2>&1 || true; set -u

mkdir -p "$RESULT_DIR"
bin="$ROOT/build/reduce_invscale_parent_probe"

chip_line() { grep -B1 "$1" "$2" | head -1; }
dev_bus() {
  case "$1" in
    0) echo '0000:C1:00.0';; 1) echo '0000:C2:00.0';; 2) echo '0000:81:00.0';; 3) echo '0000:82:00.0';;
    4) echo '0000:01:00.0';; 5) echo '0000:02:00.0';; 6) echo '0000:41:00.0';; 7) echo '0000:42:00.0';;
  esac
}

snap() {
  local tag="$1"
  local f="$RESULT_DIR/${tag}.npu-smi.txt"
  npu-smi info > "$f" 2>&1
  local bus ts row field aic hbm used total procs
  bus=$(dev_bus "$DEVICE_ID")
  ts=$(date -u +%Y-%m-%dT%H:%M:%SZ)
  row=$(grep "$bus" "$f")
  field=$(echo "$row" | awk -F'|' '{print $4}')
  aic=$(echo "$field" | tr -d ' ' | grep -oE '^[0-9]+')
  hbm=$(echo "$field" | grep -oE '[0-9]+/ *[0-9]+' | tail -1 | tr -d ' ')
  used=${hbm%/*}; total=${hbm#*/}
  procs=$(sed -n '/Process id/,$p' "$f" | awk -F'|' -v d="$DEVICE_ID" '$2 ~ "^[ ]*"d"[ ]+$" {print $3 $4 $5}' | tr -s ' ' | tr '\n' ';')
  echo -e "${tag}\t${ts}\tDEVICE_ID=${DEVICE_ID}\tAICORE=${aic}%\tHBM=${hbm}\tFREE_HBM=$((total-used))MB\tPROCS=${procs:-none}" >> "$RESULT_DIR/lease.tsv"
}

echo -e "tag\ttimestamp_utc\tdevice\taicore\thbm\tfree\tprocs" > "$RESULT_DIR/lease.tsv"
snap preflight

if npu-smi info 2>/dev/null | sed -n '/Process id/,$p' | grep -qiE 'prob|probe|invscale|rowgroup|liveness|align_tail|async_triple|batch_resident|sched_row'; then
  echo "ABORT_R2: another next6 probe detected"; exit 9
fi

echo -e "run\ttimestamp_utc\tdevice\tmedian_us\tmax_abs\tbad\tgross_bad" > "$RESULT_DIR/baseline.tsv"
for r in $(seq 1 "$RUNS"); do
  ts=$(date -u +%Y-%m-%dT%H:%M:%SZ)
  prefix="$RESULT_DIR/run-${r}"
  "$bin" "$DEVICE_ID" "$ROWS" "$WIDTH" "$DTYPE" "$prefix" > "${prefix}.stdout.txt" 2>&1
  rc=$?
  line=$(tail -n 1 "${prefix}.tsv")
  echo -e "${r}\t${ts}\t${DEVICE_ID}\t$(echo "$line" | cut -f7)\t$(echo "$line" | cut -f8)\t$(echo "$line" | cut -f9)\t$(echo "$line" | cut -f10)" >> "$RESULT_DIR/baseline.tsv"
  echo "run=$r ts=$ts device=$DEVICE_ID median_us=$(echo "$line" | cut -f7) bad=$(echo "$line" | cut -f9) rc=$rc"
done

snap postlease
echo "=== baseline done ==="
cat "$RESULT_DIR/baseline.tsv"
echo "=== lease ==="
cat "$RESULT_DIR/lease.tsv"

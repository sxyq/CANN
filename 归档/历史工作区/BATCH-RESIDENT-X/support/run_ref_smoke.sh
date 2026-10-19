#!/usr/bin/env bash
# Functional smoke for the BATCH-RESIDENT-X unified device-event runner.
# Plumbing check only: proves the runner starts, warms up, and emits
# >=21 device-event samples. NOT a measurement — do not read performance
# numbers from this run and do not quote them anywhere.
#
# argc contract (runner): device rows width dtype prefix warmup samples blocks gap_sec [batch_n]
# This script uses: warmup=10, samples=21, blocks=1, gap=0, batch_n omitted (=1).
set -euo pipefail
ROOT="$(cd "$(dirname "$0")" && pwd)"
BUILD="$ROOT/build"
BIN="$BUILD/brx_ref_probe"
DEV="${ASCEND_DEVICE_ID:-5}"
ROWS="${ROWS:-8}"
WIDTH="${WIDTH:-256}"
DTYPE="${DTYPE:-0}"
OUT="${OUT:-$ROOT/results-smoke/d${DEV}}"
mkdir -p "$OUT"
export LD_LIBRARY_PATH="/usr/lib/aarch64-linux-gnu:/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/lib64:${LD_LIBRARY_PATH:-}"

if [ ! -x "$BIN" ]; then
  echo "MISSING_BINARY $BIN"
  exit 2
fi

PREFIX="$OUT/smoke-d${DEV}-r${ROWS}x${WIDTH}-dt${DTYPE}"
date -Is > "${PREFIX}.timestamp.txt"
rc=0
"$BIN" "$DEV" "$ROWS" "$WIDTH" "$DTYPE" "$PREFIX" \
  10 21 1 0 \
  > "${PREFIX}.stdout.txt" 2> "${PREFIX}.stderr.txt" || rc=$?

SAMPLES=$(awk 'END{print NR-1}' "${PREFIX}-raw.tsv" 2>/dev/null || echo 0)
DEV_OK=$(awk -F'\t' 'NR>1 && $3+0>0 {n++} END{print n+0}' "${PREFIX}-raw.tsv" 2>/dev/null || echo 0)
DEV_FIELD=$(grep -c "DEVICE_EVENT_PRIMARY" "${PREFIX}-stats.txt" 2>/dev/null || true)
echo "SMOKE_DONE rc=$rc samples=$SAMPLES device_us_positive=$DEV_OK device_event_field=$DEV_FIELD prefix=$PREFIX"
grep -E "^(method|route|warmup|samples_per_block|batch_n|bad)" "${PREFIX}-stats.txt" 2>/dev/null || true
cat "${PREFIX}.stdout.txt"
exit $rc

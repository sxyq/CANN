#!/usr/bin/env bash
# H6 pre-registered correctness battery (ALIGN-TAIL-X H6 DEEPENING 2026-09-25).
# Correctness only: warmup=1 samples=1 blocks=1 gap=0 -> one launch each.
# Serialized: one probe process at a time. No timing collection beyond harness stats.
# usage: run_h6_battery.sh <dev> <nrep> [start_rep]
set -uo pipefail
DEV=${1:-4}
NREP=${2:-10}
REPFROM=${3:-1}
BUILD="$HOME/phase4-workspaces/ALIGN-TAIL-X/support/build"
RES="$HOME/phase4-workspaces/ALIGN-TAIL-X/support/results-h6-correctness/d${DEV}"
mkdir -p "$RES"

set +u
source /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/set_env.sh >/dev/null 2>&1 || true
set -u
export ASCEND_HOME_PATH=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002
# set_env.sh does not export LD_LIBRARY_PATH on this host; runbook-2x100 values
export LD_LIBRARY_PATH="/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/lib64:/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/lib64:/usr/lib/aarch64-linux-gnu"

RESULTS="$RES/results.tsv"
if [ ! -f "$RESULTS" ]; then
  printf 'label\trows\tD\tdtype\tside\trep\trc\tbad\tmax_abs\n' > "$RESULTS"
fi
date -Is >> "$RES/battery-start.timestamp.txt"
if [ "$REPFROM" = "1" ]; then
  npu-smi info > "$RES/npu-smi-pre.txt" 2>&1 || true
  uptime > "$RES/host-load-pre.txt"
fi

# labels per H6 DEEPENING table: A/B required, C reduce-only control,
# D alignment-only isolator, E clean control.  dtype 0=fp32 1=fp16
MIN_SET="A 2 4100 0
B 2 4103 0
D 2 4104 1"
REST_SET="C 1 4100 0
E 1 4104 1"

run_one() {
  local label=$1 rows=$2 D=$3 dt=$4 side=$5 rep=$6
  local bin="$BUILD/atx_ref_${side}_probe"
  local prefix="$RES/${label}_r${rows}D${D}_t${dt}_${side}_rep$(printf '%02d' "$rep")"
  local rc bad maxabs
  "$bin" "$DEV" "$rows" "$D" "$dt" "$prefix" 1 1 1 0 > "${prefix}.stdout" 2> "${prefix}.stderr"
  rc=$?
  bad=$(awk -F'\t' '/^bad\t/{print $2}' "${prefix}-stats.txt" 2>/dev/null)
  maxabs=$(awk -F'\t' '/^max_abs\t/{print $2}' "${prefix}-stats.txt" 2>/dev/null)
  printf '%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\n' \
    "$label" "$rows" "$D" "$dt" "$side" "$rep" "$rc" "${bad:-NA}" "${maxabs:-NA}" >> "$RESULTS"
  echo "RUN label=$label rows=$rows D=$D dtype=$dt side=$side rep=$rep rc=$rc bad=${bad:-NA} max_abs=${maxabs:-NA} $(date -Is)"
}

for rep in $(seq "$REPFROM" "$NREP"); do
  if [ "$rep" = "1" ]; then
    echo "=== rep 1: minimal decisive set first (A/B/D), then isolators C/E ==="
    sets=("$MIN_SET" "$REST_SET")
  else
    echo "=== rep $rep: all 5 shapes ==="
    sets=("$MIN_SET"$'\n'"$REST_SET")
  fi
  for setstr in "${sets[@]}"; do
    while read -r label rows D dt; do
      [ -z "${label:-}" ] && continue
      for side in parent v001; do
        run_one "$label" "$rows" "$D" "$dt" "$side" "$rep"
      done
    done <<< "$setstr"
  done
done

npu-smi info > "$RES/npu-smi-post.txt" 2>&1 || true
uptime > "$RES/host-load-post.txt"
date -Is >> "$RES/battery-end.timestamp.txt"
echo "=== battery done: results.tsv lines=$(grep -c '' "$RESULTS") (incl header) ==="

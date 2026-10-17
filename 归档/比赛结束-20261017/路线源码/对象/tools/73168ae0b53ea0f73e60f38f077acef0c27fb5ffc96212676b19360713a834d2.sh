#!/usr/bin/env bash
set -uo pipefail
export ASCEND_HOME_PATH=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002
export LD_LIBRARY_PATH="${LD_LIBRARY_PATH:-}"
export PYTHONPATH="${PYTHONPATH:-}"
set +u
source /usr/local/Ascend/ascend-toolkit/set_env.sh >/dev/null 2>&1 || true
set -u
export ASCEND_HOME_PATH=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002
PROBE=/home/data4t2/lelinfeng/phase4-workspaces/ALIGN-TAIL-X/probe
RES=$PROBE/results
mkdir -p "$RES"
# Probe shape: non-aligned D where tail specialization applies; FP32 for stable timing.
# rows=2, width=100 (100*4=400 bytes, 400%32=16 -> non-aligned), dtype=0
ROWS=2
WIDTH=100
DTYPE=0
DEVICE=6
for pair in 01 02 03 04; do
  date -Is > "$RES/probe-pair-$pair.timestamp.txt"
  npu-smi info > "$RES/probe-pair-$pair.npu-smi.txt" || true
  if [[ "$pair" == "02" || "$pair" == "04" ]]; then
    order="v001 parent"
  else
    order="parent v001"
  fi
  echo "pair=$pair order=$order"
  for who in $order; do
    bin="$PROBE/build/atx_${who}_probe"
    "$bin" "$DEVICE" "$ROWS" "$WIDTH" "$DTYPE" "$RES/probe-$pair-$who" \
      >"$RES/probe-$pair-$who.log" 2>&1 || echo "FAIL $who pair $pair"
    awk -v p="$pair" -v w="$who" 'NR==2{printf "  %s pair=%s med=%s bad=%s\n", w, p, $7, $9}' \
      "$RES/probe-$pair-$who.tsv"
  done
done
python3 - <<'PY'
import os, statistics
res = "/home/data4t2/lelinfeng/phase4-workspaces/ALIGN-TAIL-X/probe/results"
pairs = []
for pair in ["01","02","03","04"]:
    def med(who):
        path = f"{res}/probe-{pair}-{who}.tsv"
        with open(path) as f:
            lines = f.read().strip().splitlines()
        return float(lines[1].split("\t")[6])
    p, v = med("parent"), med("v001")
    delta = (v - p) / p * 100.0
    pairs.append((pair, p, v, delta))
    print(f"PAIR {pair}: parent={p:.3f}us v001={v:.3f}us delta_pct={delta:+.3f}%")
deltas = [x[3] for x in pairs]
meds_p = statistics.median([x[1] for x in pairs])
meds_v = statistics.median([x[2] for x in pairs])
med_delta = (meds_v - meds_p) / meds_p * 100.0
print(f"MEDIAN parent={meds_p:.3f}us v001={meds_v:.3f}us median_delta_pct={med_delta:+.3f}%")
print(f"WORST_DELTA_PCT={min(deltas):+.3f}%")
# directional consistency: fraction of pairs with same sign as median_delta
if med_delta == 0:
    cons = "NEUTRAL"
else:
    same = sum(1 for d in deltas if (d > 0) == (med_delta > 0) and d != 0)
    cons = f"{same}/{len(deltas)}"
print(f"DIRECTIONAL_CONSISTENCY={cons}")
print(f"DELTAS={[round(d,3) for d in deltas]}")
PY
date -Is > "$RES/probes-end.timestamp.txt"
echo PROBES_DONE

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
RES=$PROBE/results-window-dev4
mkdir -p "$RES"
ROWS=2
WIDTH=100
DTYPE=0
DEVICE=${ASCEND_DEVICE_ID:-4}
echo "DEVICE=$DEVICE" | tee "$RES/device.txt"
date -Is > "$RES/probes-start.timestamp.txt"
npu-smi info > "$RES/npu-smi-pre.txt" || true
for pair in 01 02 03 04; do
  date -Is > "$RES/probe-pair-$pair.timestamp.txt"
  npu-smi info > "$RES/probe-pair-$pair.npu-smi.txt" || true
  if [[ "$pair" == "02" || "$pair" == "04" ]]; then
    order="v001 parent"
  else
    order="parent v001"
  fi
  echo "pair=$pair order=$order device=$DEVICE"
  for who in $order; do
    bin="$PROBE/build/atx_${who}_probe"
    "$bin" "$DEVICE" "$ROWS" "$WIDTH" "$DTYPE" "$RES/probe-$pair-$who" \
      >"$RES/probe-$pair-$who.log" 2>&1 || echo "FAIL $who pair $pair"
    awk -v p="$pair" -v w="$who" 'NR==2{printf "  %s pair=%s med=%s bad=%s\n", w, p, $7, $9}' \
      "$RES/probe-$pair-$who.tsv"
  done
done
python3 - <<'PY'
import statistics
res = "/home/data4t2/lelinfeng/phase4-workspaces/ALIGN-TAIL-X/probe/results-window-dev4"
pairs = []
for pair in ["01","02","03","04"]:
    def med(who):
        lines = open(f"{res}/probe-{pair}-{who}.tsv").read().strip().splitlines()
        return float(lines[1].split("\t")[6])
    p, v = med("parent"), med("v001")
    delta = (v - p) / p * 100.0
    pairs.append((pair, p, v, delta))
    print(f"PAIR {pair}: parent={p:.3f}us v001={v:.3f}us delta_pct={delta:+.3f}%")
deltas = [x[3] for x in pairs]
med_p = statistics.median([x[1] for x in pairs])
med_v = statistics.median([x[2] for x in pairs])
med_delta = (med_v - med_p) / med_p * 100.0
print(f"MEDIAN parent={med_p:.3f}us v001={med_v:.3f}us median_delta_pct={med_delta:+.3f}%")
print(f"DELTAS={[round(d,3) for d in deltas]}")
print(f"WORST_DELTA_PCT={max(deltas):+.3f}% BEST_DELTA_PCT={min(deltas):+.3f}%")
if med_delta > 0:
    same = sum(1 for d in deltas if d > 0)
elif med_delta < 0:
    same = sum(1 for d in deltas if d < 0)
else:
    same = 0
print(f"DIRECTIONAL_CONSISTENCY={same}/{len(deltas)}")
print(f"NOISE_MARGIN_PCT={max(deltas)-min(deltas):.3f}")
# residual load note
smi = open(f"{res}/probe-pair-01.npu-smi.txt").read()
print("HAS_VLLM", "VLLM" in smi)
# aicore on dev4 from pre
import re
pre = open(f"{res}/npu-smi-pre.txt").read().splitlines()
for i,l in enumerate(pre):
    if re.match(r"\|\s*4\s+910B3", l) and i+1 < len(pre):
        print("DEV4_PRE", pre[i+1])
PY
date -Is > "$RES/probes-end.timestamp.txt"
echo PROBES_DEV4_DONE

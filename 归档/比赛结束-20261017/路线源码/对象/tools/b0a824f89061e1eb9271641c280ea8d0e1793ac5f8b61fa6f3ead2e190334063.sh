#!/usr/bin/env bash
set -euo pipefail
export ASCEND_HOME_PATH=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002
export LD_LIBRARY_PATH="${LD_LIBRARY_PATH:-}"
export PYTHONPATH="${PYTHONPATH:-}"
set +u
source /usr/local/Ascend/ascend-toolkit/set_env.sh >/dev/null 2>&1 || true
set -u
export ASCEND_HOME_PATH=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002

PROBE=/home/data4t2/lelinfeng/phase4-workspaces/ALIGN-TAIL-X/probe
RES=$PROBE/results-controlled-dev4
mkdir -p "$RES"
DEVICE=4
ROWS=2
WIDTH=100
DTYPE=0

# Interleaved order only: P,C,P,C and P,C,C,P across pairs
# pair01: P C
# pair02: P C
# pair03: C P
# pair04: C P
# This covers both P,C,P,C (pairs 01-02) and C,P,C,P which is reverse of P,C,C,P style.
# Main said: P,C,P,C or P,C,C,P. Use explicit sequences:
# 01: P,C  02: P,C  => sequence P C P C
# 03: C,P  04: C,P  => sequence C P C P (also interleaved, not all-P-then-all-C)
# Also add pair05 as C,P,C pattern with P,C for P,C,C,P: 
# pair05: P,C  pair06: C,P -> overall not all parent then all cand.
# Stick to 4 pairs as minimum with strict interleave per pair (each pair is adjacent P and C).

echo "LEASE=L002" > "$RES/protocol.txt"
echo "DEVICE_ID=$DEVICE" >> "$RES/protocol.txt"
echo "SHA_EXPECT=f573d16d39fb54a2a75f75f90943168b96dd97d83fb6bd094f11fd73c61df840" >> "$RES/protocol.txt"
echo "SHAPE=FP32 rows=$ROWS width=$WIDTH" >> "$RES/protocol.txt"
echo "INTERLEAVE=P,C per pair; pair order: 01 PC, 02 PC, 03 CP, 04 CP" >> "$RES/protocol.txt"
echo "START=$(date -Is)" >> "$RES/protocol.txt"

# Mid-run next6 check before starting pairs
if npu-smi info -t proc-mem -i 4 2>/dev/null | grep -E 'atx_|srx_|batch_|sched_|async_|ub_|reduce_'; then
  echo "ABORT_MID=NEXT6_ON_D4" | tee -a "$RES/protocol.txt"
  exit 3
fi

run_one() {
  local who=$1 pair=$2
  local bin="$PROBE/build/atx_${who}_probe"
  "$bin" "$DEVICE" "$ROWS" "$WIDTH" "$DTYPE" "$RES/pair${pair}-${who}" \
    >"$RES/pair${pair}-${who}.log" 2>&1
}

# pair orders
declare -A ORDERS=(
  [01]="parent v001"
  [02]="parent v001"
  [03]="v001 parent"
  [04]="v001 parent"
)

for pair in 01 02 03 04; do
  date -Is > "$RES/pair${pair}.timestamp.txt"
  npu-smi info > "$RES/pair${pair}.npu-smi.txt"
  npu-smi info -t proc-mem -i 4 > "$RES/pair${pair}.proc-mem-d4.txt" 2>/dev/null || true
  # abort if concurrent next6 appears mid-run
  if grep -E 'atx_|srx_|batch_|sched_|async_|ub_|reduce_' "$RES/pair${pair}.proc-mem-d4.txt" >/dev/null 2>&1; then
    # our own probe will appear as atx_* while running - check BEFORE running this pair's bins
    # At snapshot time before run, atx should not be ours yet for first check at pair start
    # Actually after pair starts we write after run... we snapshot BEFORE run so atx shouldn't be present unless other route
    echo "ABORT_MID=NEXT6_BEFORE_PAIR_${pair}" | tee -a "$RES/protocol.txt"
    grep -E 'atx_|srx_|batch_|sched_|async_|ub_|reduce_' "$RES/pair${pair}.proc-mem-d4.txt" | tee -a "$RES/protocol.txt"
    exit 3
  fi
  order=${ORDERS[$pair]}
  echo "pair=$pair order=$order"
  for who in $order; do
    run_one "$who" "$pair"
    awk -v p="$pair" -v w="$who" 'NR==2{printf "  %s pair=%s med=%s bad=%s\n", w, p, $7, $9}' \
      "$RES/pair${pair}-${who}.tsv"
  done
done

# Postrun
date -Is > "$RES/postrun.timestamp.txt"
npu-smi info > "$RES/postrun.npu-smi.txt"
npu-smi info -t proc-mem -i 4 > "$RES/postrun.proc-mem-d4.txt" 2>/dev/null || true
python3 - <<'PY'
import json, math, statistics, re
from pathlib import Path
res = Path("/home/data4t2/lelinfeng/phase4-workspaces/ALIGN-TAIL-X/probe/results-controlled-dev4")

def load_med_bad(path):
    lines = path.read_text().strip().splitlines()
    parts = lines[1].split("\t")
    # device dtype rows width warmups repeats median max_abs bad
    return float(parts[6]), int(parts[8]), float(parts[7])

# Also extract all sample times? runner only writes median. Jitter across pairs for parent.
pairs = []
for pair in ["01","02","03","04"]:
    pm, pbad, pabs = load_med_bad(res / f"pair{pair}-parent.tsv")
    vm, vbad, vabs = load_med_bad(res / f"pair{pair}-v001.tsv")
    delta = (vm - pm) / pm * 100.0
    order = open(res / "protocol.txt").read()
    pairs.append({
        "pair": pair,
        "parent_us": pm,
        "v001_us": vm,
        "delta_pct": round(delta, 3),
        "parent_bad": pbad,
        "v001_bad": vbad,
        "timestamp": (res / f"pair{pair}.timestamp.txt").read_text().strip(),
    })
    print(f"PAIR {pair}: parent={pm:.3f} v001={vm:.3f} delta={delta:+.3f}% badP={pbad} badV={vbad}")

pt = [p["parent_us"] for p in pairs]
vt = [p["v001_us"] for p in pairs]
deltas = [p["delta_pct"] for p in pairs]

def jitter(xs):
    # CV = std/mean; also report min-max spread
    mean = statistics.mean(xs)
    if len(xs) < 2:
        return {"cv": 0.0, "spread_us": 0.0, "std_us": 0.0, "mean_us": mean}
    sd = statistics.pstdev(xs)
    cv = sd / mean if mean else float("inf")
    return {
        "cv": round(cv, 4),
        "std_us": round(sd, 4),
        "spread_us": round(max(xs) - min(xs), 4),
        "mean_us": round(mean, 4),
        "median_us": round(statistics.median(xs), 4),
        "min_us": round(min(xs), 4),
        "max_us": round(max(xs), 4),
    }

pj = jitter(pt)
vj = jitter(vt)
med_p = statistics.median(pt)
med_v = statistics.median(vt)
med_delta = (med_v - med_p) / med_p * 100.0
worst = max(deltas)  # for gain claim, worst is most positive if seeking V001 faster (negative delta)
# Main: WORST_DELTA — for improvement claim (lower better), worst is largest positive (V001 slower)
worst_delta = round(max(deltas), 3)
best_delta = round(min(deltas), 3)

if med_delta > 0:
    same = sum(1 for d in deltas if d > 0)
elif med_delta < 0:
    same = sum(1 for d in deltas if d < 0)
else:
    same = 0
directional = f"{same}/{len(deltas)}"

# LOAD_QUALITY from PARENT jitter only
# CLEAN: parent CV small and stable; MODERATE: moderate; LOAD_CONTAMINATED: parent wild
cv = pj["cv"]
spread = pj["spread_us"]
# Heuristic thresholds:
# CLEAN: cv < 0.15 and spread < 50% of median parent
# MODERATE: cv < 0.35
# else contaminated
med = pj["median_us"]
if cv < 0.15 and spread <= 0.5 * med:
    load_q = "CLEAN"
elif cv < 0.35:
    load_q = "MODERATE"
else:
    load_q = "LOAD_CONTAMINATED"

# Also check order-flipping parent (wild)
parent_sign_flips = sum(1 for i in range(1, len(pt)) if (pt[i]-pt[i-1]) * (pt[i]-statistics.median(pt)) < 0)
# simpler: parent range > 2x median
if (pj["max_us"] - pj["min_us"]) > pj["median_us"]:
    load_q = "LOAD_CONTAMINATED"

correctness = "PASS" if all(p["parent_bad"]==0 and p["v001_bad"]==0 for p in pairs) else "FAIL"

# Decision rule:
# ONLINE_CANDIDATE if correctness PASS + CLEAN/MODERATE + >=3/4 same direction + median gain >> parent jitter
# stable regression -> LOCAL_REJECTED
# else NEEDS_ONE_MORE_LOCAL

direction_ok = same >= 3
# median gain: negative delta means V001 faster (gain). positive means slower.
median_gain_us = med_p - med_v  # positive if V001 faster
# gain >> parent jitter: |median_gain| > parent std and > parent spread/2
gain_vs_jitter = abs(median_gain_us) > max(pj["std_us"], 0.5 * pj["spread_us"]) and abs(median_gain_us) > 0

if correctness == "PASS" and load_q in ("CLEAN", "MODERATE") and direction_ok and med_delta < 0 and gain_vs_jitter:
    decision = "ONLINE_CANDIDATE"
elif correctness == "PASS" and direction_ok and med_delta > 0 and gain_vs_jitter and load_q in ("CLEAN", "MODERATE"):
    # stable regression with good load
    decision = "LOCAL_REJECTED"
else:
    decision = "NEEDS_ONE_MORE_LOCAL"

# If parent contaminated, never judge candidate as reject for pollution; never technical-fail
if load_q == "LOAD_CONTAMINATED":
    decision = "NEEDS_ONE_MORE_LOCAL"

summary = {
    "lease": "L002",
    "device_id": 4,
    "correctness": correctness,
    "PARENT_TIMES": pt,
    "CANDIDATE_TIMES": vt,
    "PARENT_JITTER": pj,
    "CANDIDATE_JITTER": vj,
    "PROBE_DELTAS": deltas,
    "MEDIAN_DELTA": round(med_delta, 3),
    "WORST_DELTA": worst_delta,
    "BEST_DELTA": best_delta,
    "DIRECTIONAL_CONSISTENCY": directional,
    "LOAD_QUALITY": load_q,
    "LOAD_QUALITY_RULE": "from PARENT jitter only",
    "decision": decision,
    "pairs": pairs,
    "median_parent_us": round(med_p, 4),
    "median_v001_us": round(med_v, 4),
    "median_gain_us": round(median_gain_us, 4),
}
(res / "summary.json").write_text(json.dumps(summary, indent=2) + "\n")
print("PARENT_JITTER", pj)
print("CANDIDATE_JITTER", vj)
print("LOAD_QUALITY", load_q)
print("MEDIAN_DELTA", round(med_delta,3), "WORST", worst_delta, "DIR", directional)
print("CORRECTNESS", correctness, "DECISION", decision)
PY

echo "END=$(date -Is)" >> "$RES/protocol.txt"
echo CONTROLLED_DEV4_DONE

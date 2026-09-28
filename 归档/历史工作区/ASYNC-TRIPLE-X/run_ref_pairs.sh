#!/usr/bin/env bash
# ASYNC-TRIPLE-X Track-A — interleaved P/C pairs (P=SEED Direct Parent, C=V001 frozen candidate).
# Runs ONLY if the same-binary floor verdict is PASS. Unified device-event protocol,
# warmup=10, 2 blocks x 31 samples per process, 4 adjacent pairs (PC,PC,CP,CP), same window.
# Measurement only — no Candidate source edit. Raw samples preserved.
set -uo pipefail
WS="/home/data4t2/lelinfeng/phase4-workspaces/ASYNC-TRIPLE-X"
FLOOR_OUT="${FLOOR_OUT:-$WS/ref_results_8x8192/floor}"
OUT="${OUT:-$WS/ref_results_8x8192/pairs}"
DEVICE="${DEVICE:-4}"
ROWS="${ROWS:-8}"
WIDTH="${WIDTH:-8192}"
DTYPE="${DTYPE:-0}"
WARM="${WARM:-10}"
SAMPLES="${SAMPLES:-31}"
BLOCKS="${BLOCKS:-2}"
GAP="${GAP:-2}"
export ASCEND_HOME_PATH=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002
set +u
source "${ASCEND_HOME_PATH}/bin/setenv.bash" >/dev/null 2>&1
set -u

mkdir -p "$OUT"
PROTOCOL="$OUT/protocol.txt"
{
  echo "lease=TA-8x8192 owner=MAIN-2 route=ASYNC-TRIPLE-X track=TRACK-A"
  echo "phase=INTERLEAVED_PAIRS"
  echo "sha256_parent=f20da79c7086483c7cf0bddad630bdea92a219992a5f061c9fd3a6b8edbc3572"
  echo "sha256_candidate=2defc6c270e898c00aa151dbef00f68c72ccf77f33e54db18c75b8bf4f4b4f9c (FROZEN, unchanged)"
  echo "method=DEVICE_EVENT_PRIMARY+HOST_WALL_SECONDARY (runner_ref.inc)"
  TILECOUNT=$(( (WIDTH + 1023) / 1024 ))
  echo "shape=FP32 rows=$ROWS width=$WIDTH tileCount=$TILECOUNT device=$DEVICE"
  echo "interleave=pairs 01 PC, 02 PC, 03 CP, 04 CP (adjacent within pair, same window)"
  echo "warmup=$WARM samples=$SAMPLES blocks=$BLOCKS gap=$GAP batch_n=1"
  echo "PRE-REGISTERED DECISION RULE (written before any pair runs):"
  echo "  VALIDITY: every process bad=0 AND every block MAD/med<=0.10 AND every process drift<=0.10"
  echo "  FLOOR_NOISE_PCT = floor same-binary ALL_DEVICE MAD/med x100"
  echo "  delta% = (C_median - P_median)/P_median x100; negative = V001 faster"
  echo "  ONLINE_CANDIDATE iff VALIDITY && >=3/4 pairs delta<0 && median_delta% < -FLOOR_NOISE_PCT"
  echo "  LOCAL_REJECTED   iff VALIDITY && >=3/4 pairs delta>0 && median_delta% > +FLOOR_NOISE_PCT"
  echo "  else NEEDS_ONE_MORE_LOCAL; any VALIDITY failure -> NEEDS_ONE_MORE_LOCAL (unqualified, no judgment)"
  echo "  Local percent is NOT an Official Score."
  echo "start=$(date -Is)"
} > "$PROTOCOL"

if [ ! -f "$FLOOR_OUT/floor-verdict.json" ]; then
  echo "ABORT_NO_FLOOR_VERDICT"; exit 3
fi
if ! grep -q '"verdict": "PASS"' "$FLOOR_OUT/floor-verdict.json"; then
  echo "ABORT_FLOOR_NOT_PASS" | tee -a "$PROTOCOL"; exit 3
fi
cp "$FLOOR_OUT/floor-verdict.json" "$OUT/floor-verdict.json"

run_one() {
  local who="$1" pair="$2"
  local bin="$WS/build-ref/async_ref_${who}_probe"
  "$bin" "$DEVICE" "$ROWS" "$WIDTH" "$DTYPE" "$OUT/pair${pair}-${who}" \
    "$WARM" "$SAMPLES" "$BLOCKS" "$GAP" \
    > "$OUT/pair${pair}-${who}.stdout.txt" 2> "$OUT/pair${pair}-${who}.stderr.txt"
  grep -E 'REF_HARNESS' "$OUT/pair${pair}-${who}.stdout.txt" || true
}

for pair in 01 02 03 04; do
  date -Is > "$OUT/pair${pair}.timestamp.txt"
  npu-smi info > "$OUT/pair${pair}.npu-smi.txt" 2>&1 || true
  npu-smi info -t proc-mem -i "$DEVICE" > "$OUT/pair${pair}.proc-mem.txt" 2>&1 || true
  if grep -E 'atx_|srx_|batch_|sched_|ub_|reduce_' "$OUT/pair${pair}.proc-mem.txt" 2>/dev/null | grep -qv VLLM; then
    echo "ABORT_MID_NEXT6_BEFORE_PAIR_${pair}" | tee -a "$PROTOCOL"
    exit 3
  fi
  case "$pair" in
    01|02) order="seed v001" ;;
    03|04) order="v001 seed" ;;
  esac
  echo "pair=$pair order=$order"
  for who in $order; do
    run_one "$who" "$pair"
  done
done

npu-smi info > "$OUT/postflight.npu-smi.txt" 2>&1 || true
date -Is > "$OUT/postflight.timestamp.txt"
echo "end=$(date -Is)" >> "$PROTOCOL"

python3 - "$OUT" <<'PY'
import json, statistics, sys
from pathlib import Path
out = Path(sys.argv[1])
floor = json.loads((out / "floor-verdict.json").read_text())
floor_noise_pct = floor["all_device"]["mad_over_med"] * 100.0

def load(prefix):
    raw = Path(str(prefix) + "-raw.tsv")
    rows = [l.split("\t") for l in raw.read_text().strip().splitlines()[1:]]
    blocks = {}
    for b, r, du, wu in rows:
        blocks.setdefault(int(b), []).append(float(du))
    def bstat(v):
        med = statistics.median(v)
        m = statistics.median([abs(x - med) for x in v])
        return {"n": len(v), "median_us": round(med, 4), "MAD_us": round(m, 4),
                "mad_over_med": round(m / med, 4) if med else None}
    bs = {b: bstat(v) for b, v in sorted(blocks.items())}
    allv = [x for v in blocks.values() for x in v]
    meds = [bs[b]["median_us"] for b in sorted(bs)]
    med_all = statistics.median(allv)
    drift = round(abs(meds[0] - meds[1]) / med_all, 4) if len(meds) == 2 else None
    stats_txt = Path(str(prefix) + "-stats.txt").read_text().splitlines()
    bad = int([l for l in stats_txt if l.startswith("bad\t")][0].split("\t")[1])
    valid = (bad == 0 and drift is not None and drift <= 0.10
             and all(bs[b]["mad_over_med"] <= 0.10 for b in bs))
    return {"blocks": bs, "median_us": round(med_all, 4), "drift": drift,
            "bad": bad, "valid": valid}

pairs, deltas = [], []
for p in ["01", "02", "03", "04"]:
    pv, cv = load(out / f"pair{p}-seed"), load(out / f"pair{p}-v001")
    d = (cv["median_us"] - pv["median_us"]) / pv["median_us"] * 100.0
    deltas.append(d)
    pairs.append({"pair": p, "parent": pv, "candidate": cv,
                  "delta_pct": round(d, 3)})

n_neg = sum(1 for d in deltas if d < 0)
n_pos = sum(1 for d in deltas if d > 0)
med_delta = statistics.median(deltas)
valid = all(pr["parent"]["valid"] and pr["candidate"]["valid"] for pr in pairs)
if valid and n_neg >= 3 and med_delta < -floor_noise_pct:
    decision = "ONLINE_CANDIDATE"
elif valid and n_pos >= 3 and med_delta > floor_noise_pct:
    decision = "LOCAL_REJECTED"
else:
    decision = "NEEDS_ONE_MORE_LOCAL"

_st = (out / "pair01-seed-stats.txt").read_text().splitlines()
_rows = [l.split("\t")[1] for l in _st if l.startswith("rows\t")]
_width = [l.split("\t")[1] for l in _st if l.startswith("width\t")]
res = {"shape": (f"{_rows[0]}x{_width[0]} fp32" if _rows and _width else "unknown"),
       "floor_noise_pct": round(floor_noise_pct, 3),
       "floor_verdict": floor["verdict"], "pairs": pairs,
       "delta_pct": [round(d, 3) for d in deltas],
       "median_delta_pct": round(med_delta, 3),
       "direction_neg_faster": f"{n_neg}/4", "validity": valid,
       "decision": decision,
       "note": "local percent only; not an Official Score"}
(out / "pairs-verdict.json").write_text(json.dumps(res, indent=2) + "\n")
print("PAIRS", json.dumps(res))
PY
echo "PAIRS_RUN_DONE out=$OUT"

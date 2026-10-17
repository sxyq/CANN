#!/usr/bin/env bash
# ASYNC-TRIPLE-X Track-A — SAME-BINARY noise floor (Direct Parent SEED), unified device-event protocol.
# Shape PRIMARY: rows=8 width=8192 fp32 (tileCount=8). Warmup>=10, 2 blocks x 31 samples, one process.
# Measurement only — no Candidate source edit. Raw samples preserved (*-raw.tsv).
set -uo pipefail
WS="/home/data4t2/lelinfeng/phase4-workspaces/ASYNC-TRIPLE-X"
OUT="${OUT:-$WS/ref_results_8x8192/floor}"
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
  echo "phase=SAME_BINARY_FLOOR (Direct Parent SEED)"
  echo "sha256_parent=f20da79c7086483c7cf0bddad630bdea92a219992a5f061c9fd3a6b8edbc3572"
  echo "sha256_candidate_untouched=2defc6c270e898c00aa151dbef00f68c72ccf77f33e54db18c75b8bf4f4b4f9c"
  echo "method=DEVICE_EVENT_PRIMARY+HOST_WALL_SECONDARY (runner_ref.inc)"
  TILECOUNT=$(( (WIDTH + 1023) / 1024 ))
  echo "shape=FP32 rows=$ROWS width=$WIDTH tileCount=$TILECOUNT device=$DEVICE"
  echo "threshold=PRE-REGISTERED: PASS iff full-sample MAD/median<=0.10 AND block drift |B1med-B2med|/median<=0.10"
  echo "warmup=$WARM samples=$SAMPLES blocks=$BLOCKS gap=$GAP batch_n=1"
  echo "start=$(date -Is)"
} > "$PROTOCOL"

npu-smi info > "$OUT/preflight.npu-smi.txt" 2>&1 || true
npu-smi info -t proc-mem -i "$DEVICE" > "$OUT/preflight.proc-mem.txt" 2>&1 || true
date -Is > "$OUT/preflight.timestamp.txt"
if grep -E 'atx_|srx_|batch_|sched_|ub_|reduce_' "$OUT/preflight.proc-mem.txt" 2>/dev/null | grep -qv VLLM; then
  echo "ABORT_CONCURRENT_NEXT6_ROUTE" | tee -a "$PROTOCOL"
  exit 3
fi

BIN="$WS/build-ref/async_ref_seed_probe"
if [ ! -x "$BIN" ]; then echo "MISSING_BINARY $BIN"; exit 2; fi
echo "RUN floor seed device=$DEVICE shape=${ROWS}x${WIDTH} warm=$WARM samples=$SAMPLES blocks=$BLOCKS"
"$BIN" "$DEVICE" "$ROWS" "$WIDTH" "$DTYPE" "$OUT/samebin-seed" \
  "$WARM" "$SAMPLES" "$BLOCKS" "$GAP" \
  > "$OUT/samebin-seed.stdout.txt" 2> "$OUT/samebin-seed.stderr.txt"
RC=$?
echo "rc=$RC" >> "$PROTOCOL"
grep -E 'REF_HARNESS' "$OUT/samebin-seed.stdout.txt" || true

npu-smi info > "$OUT/postflight.npu-smi.txt" 2>&1 || true
date -Is > "$OUT/postflight.timestamp.txt"
echo "end=$(date -Is)" >> "$PROTOCOL"

python3 - "$OUT" <<'PY'
import json, statistics, sys
from pathlib import Path
out = Path(sys.argv[1])
raw = out / "samebin-seed-raw.tsv"
rows = [l.split("\t") for l in raw.read_text().strip().splitlines()[1:]]
blocks = {}
for b, r, du, wu in rows:
    blocks.setdefault(int(b), []).append(float(du))

def mad(v, med):
    return statistics.median([abs(x - med) for x in v])

def block_stats(v):
    med = statistics.median(v)
    m = mad(v, med)
    return {"n": len(v), "median_us": round(med, 4), "MAD_us": round(m, 4),
            "mad_over_med": round(m / med, 4) if med else None,
            "min": min(v), "max": max(v)}

bs = {b: block_stats(v) for b, v in sorted(blocks.items())}
allv = [x for v in blocks.values() for x in v]
alls = block_stats(allv)
meds = [bs[b]["median_us"] for b in sorted(bs)]
drift = round(abs(meds[0] - meds[1]) / alls["median_us"], 4) if len(meds) == 2 else None
stats_lines = (out / "samebin-seed-stats.txt").read_text().splitlines()
bad_line = [l for l in stats_lines if l.startswith("bad\t")]
bad = int(bad_line[0].split("\t")[1]) if bad_line else -1
_rows = [l.split("\t")[1] for l in stats_lines if l.startswith("rows\t")]
_width = [l.split("\t")[1] for l in stats_lines if l.startswith("width\t")]
shape_label = f"{_rows[0]}x{_width[0]} fp32" if _rows and _width else "unknown"
mad_ok = alls["mad_over_med"] is not None and alls["mad_over_med"] <= 0.10
per_block_ok = all(bs[b]["mad_over_med"] <= 0.10 for b in bs)
drift_ok = drift is not None and drift <= 0.10
verdict = "PASS" if (mad_ok and drift_ok and per_block_ok and bad == 0) else "FAIL"
res = {"shape": shape_label, "binary": "async_ref_seed_probe", "bad": bad,
       "blocks": bs, "all_device": alls, "block_drift": drift,
       "threshold": "MAD/med<=0.10 (full sample AND per block) AND drift<=0.10 AND bad=0",
       "verdict": verdict}
(out / "floor-verdict.json").write_text(json.dumps(res, indent=2) + "\n")
print("FLOOR", json.dumps(res))
PY
echo "FLOOR_RUN_DONE out=$OUT"

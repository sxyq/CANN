# H6 falsification battery — correctness only (ALIGN-TAIL-X)

Date: 2026-09-25 (UTC). Worker: MAIN-2 correctness-probe. Pre-registration:
`phase4/research/ALIGN-TAIL-X/next-hypotheses.md` H6 DEEPENING 2026-09-25 §2.

## Method

- Host: cann-server3 (dav-2201 / Ascend910B3), device **d4** (d5/d6 untouched, d7 not used),
  serialized — one probe process at a time, no other probe processes observed on the host.
- Binaries: `atx_ref_parent_probe` SHA256 `a8bd66a6b6da5bdf1acf350edd8e6f55c8044a8db48d9421a650637285d3187c`,
  `atx_ref_v001_probe` SHA256 `bfb2988a482337a21e1ef2d2f5da3c81aaf7695956d1604f7e8c0b2e59a04955`
  (both verified on server before running).
- CLI per pre-registration: `<dev> <rows> <D> <dtype> <prefix> 1 1 1 0`
  (warmup=1, samples=1, blocks=1, gap=0 → one launch per run).
- Coverage: **10 runs per (shape × binary) pair, 100 runs total** (5 shapes × 2 binaries × 10 reps).
  Rep 1 ran minimal decisive set first (A/B/D), then isolators C/E; reps 2–10 covered all shapes.
  Driver: `run_h6_battery.sh` (this directory), server copy `~/phase4-workspaces/ALIGN-TAIL-X/support/run_h6_battery.sh`.
- Window: rep1 16:36:30–16:37:23Z, reps2–10 16:37:16–16:43:52Z. Host load ~22–23 (idle of
  probe processes; npu-smi pre/post snapshots in `d4/`).
- One aborted setup attempt (rc=127, missing `LD_LIBRARY_PATH` from set_env.sh) produced only
  usage-error rows; the attempt directory was removed before the recorded run. Script now
  exports the runbook `LD_LIBRARY_PATH` values.
- Scope honored: no P/C timing, no Candidate kernel edits, no CANNJudge.

## Results (`d4/results.tsv`, all 100 runs)

| run | shape (dtype) | role | parent bad (of 10) | parent max_abs | V001 bad (of 10) | V001 max_abs |
|---|---|---|---|---|---|---|
| A | 2×4100 FP32 | required — alignment + reduce tail=4 | 0 | 1.66893e-06 | 0 | 1.66893e-06 |
| B | 2×4103 FP32 | required — alignment + reduce tail=7 | 0 | 1.43051e-06 | 0 | 1.43051e-06 |
| C | 1×4100 FP32 | reduce-only control (row0 aligned) | 0 | 1.19209e-06 | 0 | 1.19209e-06 |
| D | 2×4104 FP16 | alignment-only isolator (tail=8 canonical) | 0 | 1.95312e-03 | 0 | 1.95312e-03 |
| E | 1×4104 FP16 | clean control | 0 | 1.95312e-03 | 0 | 1.95312e-03 |

- All 100 runs exited rc=0 (harness exits 3 iff bad>0 — none did).
- max_abs identical across reps per (shape × binary): inputs are deterministic; values sit at
  ~1 ulp for FP32; FP16 1.95312e-03 passes atol=rtol=1e-3 tolerance (bad=0).
- No NaN/Inf, no run-to-run variance.

## Branch taken (pre-registered 5-branch tree)

**Branch (2) — REJECTED_AS_VIOLATION.** Alignment isolator D bad=0 on both sides, control
E bad=0 (harness healthy), reduce-only C bad=0, required A/B bad=0 on both sides. Combined
with the doc check (H6 DEEPENING §1: no GM-32B clause in the official `DataCopy` restriction;
call form is document-conforming), H6's premise is false: the unaligned-start full-tile edge
does not corrupt on this device/driver.

Not taken: (1) ALIGNMENT_REAL (D bad=0 both sides); (3) reduce-edge routing (C bad=0);
(4) CONFOUNDED (no failures anywhere); (5) HARNESS_FAULT (E bad=0, all rc=0).

## Main recommendation

- Close H6 as **REJECTED_AS_VIOLATION**: no correctness repair. The proposed
  `&& startAligned` predicate addition is optional defense-in-depth only, not a justified fix;
  do not open a correctness-first parent repair.
- A/B and C also clean → no evidence here for the H5 reduce-tail edge on these shapes either
  (H5 keeps its own status; this battery neither confirms nor retires it beyond these shapes).
- These 5 shapes (FP32/FP16, D∈{4100,4103,4104}) are proven correctness-clean on parent and
  V001 on dav-2201 — eligible for future P/C use if the measurement layer ever unblocks.
- Lease `H6-CORRECT-20260925` (d4, single appended line) updated in place to RELEASED.

## Files

- `results.tsv` + per-run `*-stats.txt` / `*-raw.tsv` / `*.stdout` / `*.stderr` under `d4/`
- `d4/battery-start|end.timestamp.txt`, `d4/npu-smi-pre|post.txt`, `d4/host-load-pre|post.txt`
- `run_h6_battery.sh` (driver, local + server)

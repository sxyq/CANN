# BATCH-RESIDENT-X Track-A — exact-shape same-binary + interleaved P/C (2026-09-25)

Measurement-layer run only. **No kernel edit**: `submission_v001.asc` SHA-256
`ad961c5837971bf989c9989b822cc2719a6a285c60969d35e1530463b7c8721d` verified
unchanged before and after this run (worktree + server copy both match).
Direct Parent `A001/submission_v017.asc` SHA-256
`c906bc45c0bfa0a1fb35710abac637a7c784b265d2658a66daa1d739da4054c3`.

Authority: `phase4/control/local-timing-protocol.md` (device events primary,
warmup ≥10, ≥11 in-process samples, interleaved pairs, MAD/median + block-drift
acceptance, outlier policy) and `execution-contract.md` §F/§R.

## Device / lease

- Lease `TA-SAMEBIN-D5`, device **5** (secondary). d4 was claimed by
  ALIGN-TAIL-X `SV-ALIGN-2x100` at 2026-09-25T14:21:27Z one minute before this
  run started, so this route moved to the secondary device instead of
  double-booking d4 ("one device one route"). d7 not used.
- Load at run time: VLLM resident on all NPUs (d5 `VLLMWorker_TP`,
  59876/65536 MB HBM), AICore 0% before and after. Residual VLLM accepted per
  protocol; pre/post `npu-smi` snapshots kept next to each result set.

## Binary / harness

- Harness: `phase4/workspaces/BATCH-RESIDENT-X/support/` (`runner_ref.inc`
  device-event measurement layer, unchanged this run).
- New measurement target added this cycle:
  `runner_ref_parent017.asc` + `brx_parent017_probe` in `support/CMakeLists.txt`
  — same runner code, `SRX_SUBMISSION = submission_parent017.asc` (Direct
  Parent). Candidate target `brx_ref_probe` (`submission_v001.asc`) rebuilt from
  the same sources. Host measurement only; no `.asc` compute source touched.
- Server build: `cann-server3:/home/data4t2/lelinfeng/BATCH-RESIDENT-X_runs/ref-harness/support`,
  CANN 8.5.0.alpha002, Ascend910B3 / dav-2201. Build exit 0.
- Exact V001 comparison probe shape (the shape every legacy BATCH pair used):
  **rows=100 width=256 dtype=0 (FP32)**.

## 1. Same-binary noise floor — Direct Parent vs itself

`brx_parent017_probe 5 100 256 0 … 10 31 2 0` — one process, warmup 10,
**2 blocks × 31 samples**, device-event primary, exit 0, `bad=0`.

| block | n | median_us | MAD_us | MAD/median | min | max |
|---|---:|---:|---:|---:|---:|---:|
| B1 | 31 | 8.160 | 0.260 | **0.0319** | 7.06 | 279.76 |
| B2 | 31 | 7.880 | 0.220 | **0.0279** | 7.30 | 174.76 |
| all | 62 | 8.000 | 0.310 | 0.0388 | 7.06 | 279.76 |

- Block drift `|B1−B2|/median = 0.28/8.000 = 0.0350`
- Gate (protocol): MAD/median ≤ 0.10 **and** block drift ≤ 0.10 →
  **PASS** (worst MAD/median 0.0319, drift 0.0350).
- Sparse slow outliers exist (max 279.76 µs vs median 8.16 µs) but MAD is
  robust to them and the primary statistic passes; raw samples retained.
- Evidence: `harness/samebin-d5/d5/sb-parent017-d5-100x256-dt0-{raw.tsv,stats.txt,stdout.txt,stderr.txt,*timestamp.txt,*.npu-smi.txt}`.

## 2. Interleaved P/C — 4 pairs, same window, same shape

P = `brx_parent017_probe` (Direct Parent A001-V017), C = `brx_ref_probe`
(V001). Order alternates P C / C P / P C / C P. Each run: warmup 10,
31 samples, 1 block, one process, exit 0, `bad=0` on all 8 runs.

| pair | order | P median_us | P MAD/med | C median_us | C MAD/med | delta (C−P)/P |
|---|---|---:|---:|---:|---:|---:|
| 1 | P→C | 7.840 | 0.163 | 7.500 | 0.037 | **−4.34%** |
| 2 | C→P | 8.100 | 0.042 | 8.120 | 0.081 | **+0.25%** |
| 3 | P→C | 7.340 | 0.014 | 7.440 | 0.048 | **+1.36%** |
| 4 | C→P | 8.040 | 0.040 | 7.660 | 0.029 | **−4.73%** |

- Median-of-pair-medians: P 7.940 µs, C 7.580 µs → **−4.53%**.
- Direction: **2/4 faster, 2/4 slower → INCONSISTENT**.
- Same-binary noise floor for this shape/device: MAD/median ≈ 0.03 (3%).
  Cross-process spread of the *same* P binary: medians 7.34–8.10 µs,
  max/min = 1.10 → every P/C pair delta (|d| ≤ 4.73%) sits inside the
  run-to-run spread of P alone.
- Reading: **no resolvable delta**. Not a win (direction not consistent,
  magnitude inside noise), not a loss.
- Evidence: `harness/pc-d5/d5/pair{1..4}-{P,C}-{raw.tsv,stats.txt,stdout.txt,stderr.txt,timestamp.txt}`,
  `pc-summary.json`, `pre/post.npu-smi.txt`, `window-start/end.txt`.

## 3. Decision

- SHA unchanged: **YES** (V001 `ad961c58…`, Parent `c906bc45…`, no `.asc`
  compute source modified, no new Revision).
- Same-binary gate: **PASS** on the exact V001 comparison probe shape
  (100×256 FP32, d5, unified device-event protocol).
- P/C: **4 interleaved pairs run; result INCONSISTENT / within noise** →
  no ONLINE_CANDIDATE, no LOCAL_REJECTED, no architecture conclusion.
- Track-A verdict: **NOT_QUALIFIED_THIS_WINDOW** — one clean attempt spent,
  route moves to Track-B per brief. Raw samples permanent; legacy wall-clock
  numbers stay `LEGACY_TIMING_METHOD` and are never merged with these medians.
- Local % ≠ Official Score.

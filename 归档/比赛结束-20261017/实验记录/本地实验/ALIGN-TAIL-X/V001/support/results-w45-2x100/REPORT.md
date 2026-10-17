# ALIGN-TAIL-X 2x100 FP32 — warmup=45 re-validation (Track-A, lease W45-ALIGN-2x100)

**Decision: NEEDS_ONE_MORE_LOCAL**

- Lease `W45-ALIGN-2x100` (d4): 2026-09-25T17:08:46Z -> RELEASED (see `server3-device-leases.tsv`).
- Method: unified reference harness (`atx_ref_parent_probe` / `atx_ref_v001_probe`, `runner_ref.inc`
  md5 `93e6441937e9cd8394b734e124d18469`). `DEVICE_EVENT_US` primary, `HOST_WALL_US` secondary,
  **warmup=45**, 2 blocks x 31 samples (same-binary) / 31 samples per process (pairs),
  batch N=1, gap 2 s, one process per run, strictly serialized (one measurement at a time).
- Host: hwnput3 / cann-server3, load 1-min 23.5 -> 22.0 across window. d4 AICore 0%,
  HBM 90% resident (persistent VLLM, documented), no concurrent next6 probe (preflight `ps` NONE).
- Probe identity: parent sha256 `a8bd66a6…3187c`, candidate sha256 `bfb2988a…4955` (expected match).
- Kernel SHA unchanged: V001 `f573d16d39fb54a2a75f75f90943168b96dd97d83fb6bd094f11fd73c61df840`,
  Direct Parent `c8d0f010f8fc68b90d69c6d2bcbf76ec7bb927bd9dc4f0fcac32425e5288c63c`.
  No source edit. No CANNJudge submission.
- Correctness: `bad=0`, `max_abs=3.58e-07` (same-binary run).

## Stage 1 — same-binary (Direct Parent vs itself), warmup=45

Device d4, 2x100 FP32, 1 attempt, 2 blocks x 31 samples, gap 2 s, one process:

| block | n | median_us | MAD_us | p10 | p90 | min | max |
|---|---:|---:|---:|---:|---:|---:|---:|
| B1_DEVICE | 31 | 5.58 | 0.14 | 5.32 | 6.04 | 5.20 | 108.92 |
| B2_DEVICE | 31 | 6.12 | 0.50 | 5.50 | 52.90 | 5.24 | 156.18 |
| ALL_DEVICE | 62 | 5.71 | 0.31 | 5.38 | 46.31 | 5.20 | 156.18 |

- MAD/median = 0.31 / 5.71 = **0.0543** (≤0.10)
- drift |B1 − B2| / ALL_median = |5.58 − 6.12| / 5.71 = **0.0946** (≤0.10)
- **PASS** on first clean attempt. Both metrics ≤0.25, so no `MEASUREMENT_PROTOCOL_BLOCKED_FOR_SHAPE`.
- Warmup 10→45 removed the systematic B1>B2 asymmetry seen at w10 (prior qb1: MAD/med 0.2861,
  drift 0.3806; direction B1 slower). At w45 B1 (5.58) < B2 (6.12) — direction reversed and
  within threshold.

Raw: `sb1-parent-w45-s31-raw.tsv` (62 rows), `sb1-parent-w45-s31-stats.txt`.

## Stage 2 — interleaved P/C, same window, warmup=45

4 pairs, orders PC / CP / PC / CP, adjacent P and C, each cell one process
(warmup=45, 31 samples, 1 block, batch N=1), device d4. Delta = (C − P)/P on block medians.

| pair | order | P (parent) med_us | P MAD/med | C (V001) med_us | C MAD/med | delta |
|---|---|---:|---:|---:|---:|---:|
| p1 | PC | 19.58 | 0.149 | 5.94 | 0.027 | **−69.66%** (P load-contaminated) |
| p2 | CP | 6.20 | 0.068 | 6.00 | 0.043 | −3.23% |
| p3 | PC | 6.24 | 0.048 | 5.94 | 0.077 | −4.81% |
| p4 | CP | 6.34 | 0.110 | 7.26 | 0.187 | **+14.51%** (reverse) |

- P medians span 6.20–6.34 µs excluding contaminated p1 (2.2%); p1 itself is 3.4x the noise floor.
- C medians span 5.94–7.26 µs (22.1%).
- Deltas: −69.66, −3.23, −4.81, +14.51 → median **−4.02%**, favor C 3/4, one reverse pair.
- Excluding contaminated p1, all three remaining deltas sit at/below the same-binary noise floor
  (MAD/med 0.054, drift 0.095 → ~±5%). Two cells exceed MAD/med 0.10 (p1-P 0.149, p4-C 0.187;
  p4-P 0.110 borderline).
- p1 raw shows the whole parent block shifted (~19–20 µs cluster, device events, AICore snapshot 0%),
  i.e. a transient device-side contention episode at 17:09:25–17:09:32, not a kernel property.

Raw: `pair0{1..4}-{parent,v001}-w45-s31-{raw.tsv,stats.txt}`, `pc-protocol.txt`,
per-pair `npu-smi.txt` + `timestamp.txt`, preflight/postrun snapshots.

## Stage 3 — decision

**NEEDS_ONE_MORE_LOCAL**

- Same-binary re-validation at warmup=45 **PASS** (this is the headline result: the w10 B1/B2
  blocker for 2x100 is resolved).
- P/C signal is inconclusive: one contaminated pair, one reverse pair, remaining deltas inside
  the noise floor. Not a Candidate win (`ONLINE_CANDIDATE` requires consistent separation beyond
  the floor), not a Candidate loss (`LOCAL_REJECTED`), shape not blocked (floor PASS).
- No winsorize, no sample deletion; all raw samples retained.
- V001 SHA unchanged; no Candidate kernel edits; no CANNJudge submission.

# MAIN-2 Track-A HANDOFF — warmup=45 re-validation + interleaved P/C (2026-09-25)

Protocol: `cann/phase4/control/local-timing-protocol.md` (warmup 10→45).
Unified runner_ref harness: DEVICE_EVENT_US primary, HOST_WALL_US secondary, 2×31 samples,
batch N=1, one process per run, strictly serialized, one lease per stage, device d4,
host cann-server3 (hwnput3). Measurement only — no Candidate kernel edits, no CANNJudge.
Shape PASS iff full-sample MAD/median ≤0.10 AND |B1−B2|/med ≤0.10;
either >0.25 → MEASUREMENT_PROTOCOL_BLOCKED_FOR_SHAPE, shape stopped, rules not loosened.
All raw samples retained, no winsorize.

## Decisions

| stage | shape | same-binary MAD/med | drift | same-binary | P/C | decision |
|---|---|---:|---:|---|---|---|
| 1 ALIGN-TAIL-X | 2x100 FP32 | 0.0543 | 0.0946 | **PASS** | 4 pairs run | **NEEDS_ONE_MORE_LOCAL** |
| 2 REDUCE-INVSCALE-X | 1x8192 FP32 | 0.1624 | **1.7321** | FAIL (>0.25 drift) | not run | **MEASUREMENT_BLOCKED** |
| 3 BATCH-RESIDENT-X (optional) | 100x256 FP32 | 0.0383 | 0.0120 | **PASS** | 4 pairs run, cells >0.25 | **MEASUREMENT_BLOCKED** |

## Stage 1 — ALIGN-TAIL-X 2x100 FP32 (d4)

Same-binary (Direct Parent vs itself, `atx_ref_parent_probe`, warmup=45, 2×31, gap 2 s, one process):
B1 med 5.58 / B2 med 6.12 / ALL med 5.71, ALL MAD 0.31 → MAD/med **0.0543**, drift **0.0946** → PASS.
bad=0, max_abs 3.58e-07. warmup 45 removed the w10 systematic B1>B2 drift (prior qb1 0.2861/0.3806).

Interleaved P/C, orders PC/CP/PC/CP, each cell one process (warmup=45, 31 samples, 1 block):

| pair | order | P med | P MAD/med | C med | C MAD/med | delta (C−P)/P |
|---|---|---:|---:|---:|---:|---:|
| p1 | PC | 19.58 | 0.149 | 5.94 | 0.027 | −69.66% (P block load-contaminated) |
| p2 | CP | 6.20 | 0.068 | 6.00 | 0.043 | −3.23% |
| p3 | PC | 6.24 | 0.048 | 5.94 | 0.077 | −4.81% |
| p4 | CP | 6.34 | 0.110 | 7.26 | 0.187 | +14.51% (reverse) |

Median delta −4.02%, favor C 3/4; excluding contaminated p1 all deltas sit inside the same-binary
floor (~±5%); one reverse pair. → **NEEDS_ONE_MORE_LOCAL** (not a win, not a loss, shape not blocked).
Probes: parent sha256 `a8bd66a6…3187c`, candidate `bfb2988a…4955` (expected match).

## Stage 2 — REDUCE-INVSCALE-X 1x8192 FP32 (d4, after Stage-1 lease released)

Same-binary (`reduce_invscale_refdirect_probe` = Direct Parent `submission_v001_reference.asc`
`f017935d…8023`, warmup=45, 2×31, one process):
B1 med 6.60 (MAD/med 0.0515, clean) / B2 med 19.40 (MAD/med 0.6639, contaminated) /
ALL med 7.39, MAD 1.20 → MAD/med **0.1624**, drift **1.7321 > 0.25** →
**MEASUREMENT_PROTOCOL_BLOCKED_FOR_SHAPE**, shape stopped → **no P/C** → **MEASUREMENT_BLOCKED**.
Direction reversed vs w10 (w10: B1 12.78 > B2 6.60; w45: B1 6.60 < B2 19.40).
Multi-tile path confirmation from the same run: parent `bad=2048`, max_abs 5.4174 (known V001 defect,
rc=3 is the harness bad!=0 exit); timing block completed, 62 samples written.
Candidate `submission.asc` `bef271b6…ad26` unchanged (not run for timing).

## Stage 3 (optional) — BATCH-RESIDENT-X 100x256 FP32 (d4, after Stage-2 lease released)

Same-binary (`brx_parent017_probe`, warmup=45, 2×31): B1 8.42 / B2 8.32 / ALL 8.35, MAD 0.32 →
MAD/med **0.0383**, drift **0.0120** → PASS; bad=0.

| pair | order | P med | P MAD/med | C med | C MAD/med | delta |
|---|---|---:|---:|---:|---:|---:|
| p1 | PC | 8.34 | 0.055 | 11.62 | **0.434** | +39.33% |
| p2 | CP | 7.54 | 0.024 | 15.02 | **0.494** | +99.20% |
| p3 | PC | 9.04 | 0.164 | 8.40 | 0.124 | −7.08% |
| p4 | CP | 7.90 | 0.053 | 18.16 | **0.551** | +129.87% |

3/4 lean C-slower but all four C cells are bimodal (fast cluster ≈ Parent, slow 40–180 µs cluster
~40–55% of samples) and three exceed the 0.25 stop level. Candidate-side same-binary diagnostic
same window: ALL MAD/med **0.4953**, drift **0.4334** — both >0.25. Counter-evidence: same
candidate binary clean on d5 at w10 (MAD/med 0.037–0.081, deltas within ±4.73%).
Host load rose 24→34 during the stage. → **MEASUREMENT_BLOCKED**
(not LOCAL_REJECTED: deltas driven by contaminated cells; not ONLINE_CANDIDATE; a further attempt
would have to be a fresh window/device — MAIN's call).

## Lease lines (appended to `cann/phase4/control/server3-device-leases.tsv`, all released)

```
4 MAIN-2 ALIGN-TAIL-X      W45-ALIGN-2x100    LEASED   2026-09-25T17:08:46Z -      warmup=45 re-validation: same-binary 2x31 then interleaved P/C >=4 pairs on 2x100 FP32; unified ref harness (DEVICE_EVENT_US primary); no source edit; no CANNJudge
4 MAIN-2 ALIGN-TAIL-X      W45-ALIGN-2x100    RELEASED 2026-09-25T17:08:46Z 2026-09-25T17:12:21Z same-binary PASS (0.0543/0.0946) + 4 pairs deltas -69.66*/-3.23/-4.81/+14.51 -> NEEDS_ONE_MORE_LOCAL; V001 SHA f573d16d unchanged
4 MAIN-2 REDUCE-INVSCALE-X W45-REDUCE-8192    LEASED   2026-09-25T17:12:21Z -      warmup=45 re-validation on 1x8192 FP32; parent multi-tile bad=2048 only as path confirmation
4 MAIN-2 REDUCE-INVSCALE-X W45-REDUCE-8192    RELEASED 2026-09-25T17:12:21Z 2026-09-25T17:13:38Z same-binary 0.1624/1.7321 -> MEASUREMENT_PROTOCOL_BLOCKED_FOR_SHAPE; no P/C; V002 SHA bef271b6 unchanged
4 MAIN-2 BATCH-RESIDENT-X  W45-BATCH-100x256  LEASED   2026-09-25T17:14:05Z -      optional Stage-3 warmup=45 on 100x256 FP32
4 MAIN-2 BATCH-RESIDENT-X  W45-BATCH-100x256  RELEASED 2026-09-25T17:14:05Z 2026-09-25T17:20:09Z parent floor PASS 0.0383/0.0120; C cells >0.25; cand floor 0.4953/0.4334 -> MEASUREMENT_BLOCKED; load 24->34
```

One lease at a time; each released before the next stage started. No concurrent next6 probe
observed at any preflight; d4 only; d7 untouched.

## SHA verification (before and after runs)

| object | SHA256 | state |
|---|---|---|
| ALIGN V001 `V001/kernel.asc` | `f573d16d39fb54a2a75f75f90943168b96dd97d83fb6bd094f11fd73c61df840` | unchanged (local + server) |
| ALIGN Direct Parent `parent/kernel.asc` | `c8d0f010f8fc68b90d69c6d2bcbf76ec7bb927bd9dc4f0fcac32425e5288c63c` | unchanged (local + server) |
| REDUCE V002 `submission.asc` | `bef271b62a2c7f2d0b0ef23f5f3610129460f5a431d7d9b3dd7ac3ea9a80ad26` | unchanged (server) |
| REDUCE Direct Parent | `f017935d840bea5a81268d9fe137f1567df0982f4f71718f65f4672ad8da8023` | unchanged (local + server) |
| BATCH V001 `submission_v001.asc` | `ad961c5837971bf989c9989b822cc2719a6a285c60969d35e1530463b7c8721d` | unchanged (local + server) |
| BATCH Direct Parent `submission_parent017.asc` | `c906bc45c0bfa0a1fb35710abac637a7c784b265d2658a66daa1d739da4054c3` | unchanged (server) |
| harness `runner_ref.inc` (ALIGN, REDUCE, SCHED) | md5 `93e6441937e9cd8394b734e124d18469` | identical across routes |

## Evidence paths (local mirrors of server3)

- Stage 1: `cann-next6/ALIGN-TAIL-X/phase4/local/ALIGN-TAIL-X/V001/support/results-w45-2x100/`
  (56 files: `sb1-parent-w45-s31-{raw.tsv,stats.txt}`, `pair0{1..4}-{parent,v001}-w45-s31-{raw.tsv,stats.txt}`,
  `pc-protocol.txt`, preflight/postrun npu-smi + timestamps, `REPORT.md`)
  server: `cann-server3:/home/data4t2/lelinfeng/phase4-workspaces/ALIGN-TAIL-X/support/results-w45-2x100/`
- Stage 2: `cann-next6/REDUCE-INVSCALE-X/phase4/local/REDUCE-INVSCALE-X/V002/support/results-w45-8192/`
  (`sb1-directparent-w45-s31-{raw.tsv,stats.txt}`, `sb1.{stdout,stderr,rc,timestamp}.txt`,
  preflight snapshots, `REPORT.md`)
  server: `cann-server3:/home/data4t2/lelinfeng/phase4-workspaces/REDUCE-INVSCALE-X/results-w45-8192/`
- Stage 3: `cann-next6/BATCH-RESIDENT-X/phase4/local/BATCH-RESIDENT-X/harness/results-w45-100x256/`
  (59 files: `sb1-parent017-*`, `sb2-ref-*`, `pair0{1..4}-{parent017,ref}-*`, `probe.md5.txt`,
  `pc-protocol.txt`, preflight/postrun, `REPORT.md`)
  server: `cann-server3:/home/data4t2/lelinfeng/BATCH-RESIDENT-X_runs/ref-harness/support/results-w45-100x256/`
- Per-stage detail: the `REPORT.md` inside each directory above.
- Lease rows: `cann/phase4/control/server3-device-leases.tsv` (6 appended rows, all RELEASED).

No probe left running on server3 after the final stage. No Candidate kernel source touched;
no CANNJudge submission.

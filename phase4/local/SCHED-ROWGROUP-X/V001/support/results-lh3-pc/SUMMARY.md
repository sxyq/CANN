# SCHED-ROWGROUP-X V001 — Track-A exact-shape measurement (lease LH3-SCHED-33x100, 2026-09-25)

Unified local timing protocol (DEVICE_EVENT_US primary, warmup=10, 31 samples/process,
batch N=1, interleaved P/C, one clean attempt rule). Device d4 (ASCEND_DEVICE_ID=4),
host hwnput3 / cann-server3. No source edits.

## Identity verified before run

- V001 `submission.asc` SHA256 `0fae0a42e3942356fe3477cd518b17180b6104d57350cee705cba37a5895e65c` (local + server include-path copy) — matches locked value.
- Direct Parent `PARENT-R016-submission.asc` SHA256 `62de32dfc56a2b258704f658115fd01c8b224c6b814c106c049fd8cc9070c216` (local + server include-path copy) — matches `source-meta.json` PARENT_SOURCE_SHA.
- Binaries: `srx_ref_parent_probe` md5 `956821cc600c9a0a1856be8cb9b42548`, `srx_ref_v001_probe` md5 `41a9aae70eab37a19bb9825b25500427` — match expected.
- Harness: `runner_ref.inc` md5 `93e6441937e9cd8394b734e124d18469` (unchanged since LH-SCHED-33x100).
- All 18 runs printed `bad=0`.

## Host / device state

- Load average ~27.75 / 25.54 / 24.13, 34 users; root VLLM resident (d0–d3 AICore 34–35%).
- d4: AICore 0%, HBM 59187/65536 MB (~90%), only residual VLLMEngineCor pid 2999855 —
  same residual documented in lease L004. No other next6 probe running (checked before each stage).
- Load quality: `DEVICE_AICORE_IDLE_VLLM_HBM_RESIDENT`.
- Lease note: shared `server3-device-leases.tsv` NOT edited (route brief forbids shared control
  edits); lease intent recorded here as LH3-SCHED-33x100, devices verified free before each stage.

## Stage 1 — same-binary qualification (Direct Parent vs itself)

33×100 FP32, 1 clean attempt, 2 blocks × 31 samples, gap 2 s:

| block | median_us | MAD_us | MAD/med | p10 | p90 |
|---|---:|---:|---:|---:|---:|
| B1 | 49.72 | 4.02 | 0.0809 | 45.34 | 176.02 |
| B2 | 45.08 | 0.60 | 0.0133 | 44.32 | 118.90 |
| ALL | 46.87 | 2.51 | 0.0536 | 44.51 | 163.82 |

Drift |B1−B2|/ALL_median = 0.0990 (≤0.10). **PASS** (MAD/med 0.0536 ≤0.10 AND drift 0.0990 ≤0.10).
Raw: `../results-lh3-sb33/sb33-attempt1-*`.

17×256 FP32, 1 clean attempt, 2 blocks × 31 samples, gap 2 s:

| block | median_us | MAD_us | MAD/med |
|---|---:|---:|---:|
| B1 | 19.64 | 0.36 | 0.0183 |
| B2 | 19.16 | 0.46 | 0.0240 |
| ALL | 19.43 | 0.51 | 0.0262 |

Drift = 0.0247 (≤0.10). **PASS**. Raw: `sb17-attempt1-*`.

## Stage 2 — interleaved P/C (same window, orders PC/CP/PC/CP)

Each cell: one process, warmup 10, 31 samples, device-event median. Delta = (C−P)/P.

| pair | order | 33×100 P | 33×100 C | delta | 17×256 P | 17×256 C | delta |
|---|---|---:|---:|---:|---:|---:|---:|
| p1 | PC | 49.38 (MAD/med 0.093) | 23.86 (0.025) | −51.68% | 26.98 (0.308) | 20.02 (0.068) | −25.80% |
| p2 | CP | 49.42 (0.070) | 24.18 (0.034) | −51.07% | 20.20 (0.024) | 19.36 (0.028) | −4.16% |
| p3 | PC | 50.04 (0.093) | 24.00 (0.025) | −52.04% | 19.72 (0.013) | 20.12 (0.034) | +2.03% |
| p4 | CP | 48.48 (0.062) | 23.76 (0.015) | −50.99% | 19.98 (0.048) | 19.94 (0.027) | −0.20% |

- **33×100 (unaligned, mechanism shape): 4/4 favor V001, median delta −51.38%, no reverse
  pair, both orders agree.** P block medians span 48.48–50.04 µs (3.2%), C span 23.76–24.18 µs
  (1.8%). Far outside same-binary floor (MAD/med 0.054, drift 0.099).
- **17×256 (aligned control): median delta −2.18%; p1-P carries an outlier-tailed block
  (MAD/med 0.308); excluding p1 the remaining three sit at ~−0.2% — in noise floor.** Control
  consistent with scheduling-neutral behavior on naturally aligned rows.

## Decision

**ONLINE_CANDIDATE** (local status; local % ≠ Official Score).

- Same-binary exact-shape PASS both shapes in this window; prior blockers (drift 0.1999/0.1036,
  reverse pair, high C MAD) resolved under the registered protocol with one clean attempt each.
- Correctness PASS (bad=0 all runs; identical local-golden behavior as before).
- V001 SHA unchanged: `0fae0a42e3942356fe3477cd518b17180b6104d57350cee705cba37a5895e65c`.
- No kernel/runner edits; no CANNJudge submission by this route (Main disposes).
- Load-quality caveat: VLLMEngineCor HBM-resident on d4 (documented, AICore 0%); robust
  stats within registered thresholds.

## Files

- `results-lh3-sb33/` — 33×100 same-binary raw/stats/timestamps/npu-smi (server dir
  `support/results-lh3-sb33/`)
- `results-lh3-pc/` — 4 P/C pairs × 2 shapes raw/stats/timestamps + `sb17-attempt1-*` +
  npu-smi start/end (server dir `support/results-lh3-pc/`)
- raw samples permanent, no deletions.

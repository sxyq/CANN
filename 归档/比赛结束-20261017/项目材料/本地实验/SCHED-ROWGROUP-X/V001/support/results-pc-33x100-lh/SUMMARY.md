# SCHED-ROWGROUP-X 33×100 re-qualification (lease LH-SCHED-33x100, 2026-09-25)

Track-A measurement under the unified local timing protocol. Device d4 on hwnput3,
serial lease LH-SCHED-33x100. No source edits.

## Identity verified before run

- V001 `submission.asc` SHA256 `0fae0a42e3942356fe3477cd518b17180b6104d57350cee705cba37a5895e65c` — matches recorded value.
- Direct Parent `PARENT-R016-submission.asc` SHA256 `62de32dfc56a2b258704f658115fd01c8b224c6b814c106c049fd8cc9070c216` — matches `source-meta.json` PARENT_SOURCE_SHA.
- Binaries on server3: `srx_ref_parent_probe` md5 `956821cc600c9a0a1856be8cb9b42548`, `srx_ref_v001_probe` md5 `41a9aae70eab37a19bb9825b25500427` — match expected.
- `runner_ref.inc` / `runner_ref_parent.asc` / `runner_ref_v001.asc` md5 match local worktree (93e64419 / 241f57cc / 87256a92).

## Host / device state at run

- load average 23.82 / 22.83 / 21.27, 34 users.
- Root VLLM resident since Sep 22 (d0–d3 AICore 37–38%).
- d4: AICore 0%, HBM 59186/65536 MB (~90%). d5/d6 idle; not touched.
- First invocation failed at loader (`libregister.so` missing in non-login shell); rerun with
  toolkit runtime libs on LD_LIBRARY_PATH. No samples lost, not counted as an attempt.

## Same-binary qualification (Direct Parent vs itself, 33×100 FP32)

Config: warmup 10, 2 blocks × 31 samples, gap 2 s, one process per attempt,
DEVICE_EVENT primary, batch N=1. Max 2 attempts allowed; 2 used.

| attempt | block | median_us | MAD_us | MAD/med | p10 | p90 |
|---|---|---:|---:|---:|---:|---:|
| 1 | B1 | 55.60 | 6.10 | 0.1097 | 48.20 | 89.92 |
| 1 | B2 | 45.66 | 0.66 | 0.0145 | 44.96 | 158.28 |
| 1 | ALL | 49.73 | 4.77 | 0.0959 | 44.96 | 98.57 |
| 2 | B1 | 49.66 | 3.34 | 0.0673 | 45.88 | 159.96 |
| 2 | B2 | 44.90 | 0.28 | 0.0062 | 44.44 | 52.16 |
| 2 | ALL | 45.94 | 1.46 | 0.0318 | 44.52 | 102.51 |

Drift |B1−B2|/median: attempt1 **0.1999**, attempt2 **0.1036** (threshold ≤0.10).

Verdict per protocol (PASS iff MAD/med ≤0.10 AND drift ≤0.10):
- attempt1: NEEDS_VALIDATION (drift 0.1999; B1 MAD/med 0.1097 marginal)
- attempt2: NEEDS_VALIDATION (drift 0.1036, misses by 0.0036)

Both attempts < 0.25 on MAD/med and drift, so **not** MEASUREMENT_PROTOCOL_BLOCKED_FOR_SHAPE.
Same-binary verdict: **NEEDS_VALIDATION (2/2)**. Attempt1 B1 carries a slow tail
(78–137 µs samples) under host load; attempt2 B1 has p90 159.96 µs from sparse outliers.

## P/C

Not run — protocol allows interleaved P/C only after same-binary PASS on the exact shape.

## Decision

**NEEDS_ONE_MORE_LOCAL** (no ONLINE_CANDIDATE, no LOCAL_REJECTED).
V001 SHA unchanged; no kernel or runner edits; lease LH-SCHED-33x100 RELEASED 2026-09-25T09:55:27Z.
Local measurements only — never an Official Score.

## Files

- raw: `sb33-attempt1-raw.tsv`, `sb33-attempt2-raw.tsv` (+ per-attempt `-stats.txt`, stdout/stderr, timestamps)
- device state: `npu-smi-start.txt`, `npu-smi-end-sb1.txt`, `npu-smi-end-sb2.txt`
- `summary.json`, this file

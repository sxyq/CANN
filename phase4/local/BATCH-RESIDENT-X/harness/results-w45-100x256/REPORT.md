# BATCH-RESIDENT-X 100x256 FP32 — warmup=45 P/C redo (optional Stage 3, lease W45-BATCH-100x256)

**Decision: MEASUREMENT_BLOCKED** (P/C run completed, not adjudicable; no ONLINE_CANDIDATE, no LOCAL_REJECTED)

- Lease `W45-BATCH-100x256` (d4): 2026-09-25T17:13:58Z -> RELEASED (see
  `server3-device-leases.tsv`). Optional stage — Stages 1 and 2 completed first.
- Method: unified reference harness (`brx_parent017_probe` P / `brx_ref_probe` C,
  `runner_ref.inc`), `DEVICE_EVENT_US` primary, `HOST_WALL_US` secondary, **warmup=45**,
  2x31 samples (same-binary) / 31 samples per process (pairs), batch N=1, one process per run,
  strictly serialized.
- Identity: V001 `submission_v001.asc` sha256
  `ad961c5837971bf989c9989b822cc2719a6a285c60969d35e1530463b7c8721d` — local + server MATCH
  (unchanged). Direct Parent `submission_parent017.asc` sha256
  `c906bc45c0bfa0a1fb35710abac637a7c784b265d2658a66daa1d739da4054c3` MATCH.
  Probes md5: parent017 `6dba7a95…`, ref `2b0463cd…`. No source edit, no CANNJudge.
- Correctness: `bad=0` on every run (same-binary + 8 pair cells + candidate diagnostic).
- Host: hwnput3 / cann-server3, load 1-min **23.97 -> 33.77 across the window** (rising),
  d4 AICore 0% snapshots, HBM 90% VLLM-resident, no concurrent next6 probe at preflight.

## 1. Same-binary (Direct Parent vs itself), warmup=45 — PASS

100x256 FP32, 1 attempt, 2 blocks x 31 samples, gap 2 s, one process:

| block | median_us | MAD_us | MAD/med |
|---|---:|---:|---:|
| B1 | 8.42 | 0.44 | 0.052 |
| B2 | 8.32 | 0.26 | 0.031 |
| ALL (62) | 8.35 | 0.32 | **0.0383** |

- drift |B1−B2|/med = 0.10/8.35 = **0.0120**. **PASS** (0.0383 ≤ 0.10, 0.0120 ≤ 0.10),
  consistent with the w10/d5 floor (0.032/0.035).
- Raw: `sb1-parent017-w45-s31-{raw.tsv,stats.txt}`.

## 2. Interleaved P/C — 4 pairs, same window

Orders PC / CP / PC / CP, adjacent P and C, each cell one process (warmup=45, 31 samples,
1 block, batch N=1), device d4. Delta = (C − P)/P on block medians.

| pair | order | P med_us | P MAD/med | C med_us | C MAD/med | delta |
|---|---|---:|---:|---:|---:|---:|
| p1 | PC | 8.34 | 0.055 | 11.62 | **0.434** | +39.33% |
| p2 | CP | 7.54 | 0.024 | 15.02 | **0.494** | +99.20% |
| p3 | PC | 9.04 | 0.164 | 8.40 | 0.124 | −7.08% |
| p4 | CP | 7.90 | 0.053 | 18.16 | **0.551** | +129.87% |

- P cells: 3/4 at the floor (MAD/med 0.024–0.064), p3 0.164. P medians 7.54–9.04 µs.
- C cells: **all 4 above 0.10, three above the 0.25 stop threshold** (0.434 / 0.494 / 0.551).
  C raw distributions are bimodal — a fast cluster identical to Parent (~7–8 µs) plus a
  40–180 µs slow cluster occupying ~40–55% of samples (raw kept in `pair0*-ref-w45-s31-raw.tsv`).
- Deltas: +39.33, +99.20, −7.08, +129.87 → 3/4 lean C-slower, but every C median that drives
  those deltas sits on a contaminated block; magnitudes are not adjudicable.

## 3. Candidate-side same-binary diagnostic (same window, same binary vs itself)

Run to test whether the C bimodality follows the binary or the window:

| block | median_us | MAD/med |
|---|---:|---:|
| B1 | 14.92 | 0.500 |
| B2 | 8.48 | 0.125 |
| ALL (62) | 14.86 | **0.4953** |

- drift = |14.92 − 8.48|/14.86 = **0.4334**. Both metrics **> 0.25** → candidate-side
  same-binary is itself `MEASUREMENT_PROTOCOL_BLOCKED_FOR_SHAPE` level in this window.
- Counter-evidence from the prior cycle: the same candidate binary on **d5, warmup=10**
  measured clean (4 cells MAD/med 0.037–0.081, medians 7.44–8.12 µs, within ±4.73% of
  Parent — `harness/TRACK-A-RESULTS-20260925.md`). So the bimodality is **not** reproduced
  as a pure kernel property; it appears only on d4 in this rising-load window
  (load 24 → 34 during the stage).

## 4. Decision

**MEASUREMENT_BLOCKED**

- Parent same-binary gate at warmup=45 **PASS** (0.0383 / 0.0120) — warmup re-validation of
  this shape succeeded, so the P/C stage was legitimately opened and completed (4 pairs).
- The P/C result cannot be adjudicated: 3/4 candidate cells exceed the pre-registered 0.25
  stop level, the candidate-side same-binary diagnostic also exceeds it on both metrics, and
  the window's host load rose 40% mid-stage. Per the fixed rules the shape is stopped — no
  loosening, no winsorize, no sample deletion, all raw samples retained.
- Not `LOCAL_REJECTED`: the +39%…+130% deltas come from contaminated cells; rejecting on
  them would judge on blocks the protocol marks as uncompromisable.
- Not `ONLINE_CANDIDATE`: p3 is a reverse pair inside the floor and no clean C cell exists.
- Not `NEEDS_ONE_MORE_LOCAL` inside this window: the stop threshold was already crossed;
  any further attempt must be a **fresh window** (MAIN's call — d5 was clean for this shape
  last cycle, d4 was contaminated today).
- SHA unchanged (V001 `ad961c58…`, Parent `c906bc45…`); no kernel/runner edits;
  local % ≠ Official Score; no CANNJudge submission.

## Files

This directory (`phase4/local/BATCH-RESIDENT-X/harness/results-w45-100x256/`), mirrored from
server3 `.../BATCH-RESIDENT-X_runs/ref-harness/support/results-w45-100x256/`:

- `sb1-parent017-w45-s31-{raw.tsv,stats.txt,stdout.txt,stderr.txt,rc?,timestamp.txt}` — parent floor
- `sb2-ref-w45-s31-{raw.tsv,stats.txt,stdout.txt,stderr.txt,rc?}`, `sb2-ref.timestamp.txt` — candidate diagnostic
- `pair0{1..4}-{parent017,ref}-w45-s31-{raw.tsv,stats.txt,stdout.txt,stderr.txt}` + pair timestamps / npu-smi
- `pc-protocol.txt`, `probe.md5.txt`, preflight/postrun snapshots, uptimes

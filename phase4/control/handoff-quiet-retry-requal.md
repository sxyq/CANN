# HANDOFF — quiet-window same-binary re-qualification, three failed shapes (MAIN-2)

Run window: 2026-09-25T14:57:19Z – 15:01:14Z, host `cann-server3`, device **d4 only**
(preferred device; d5/d6 untouched, d7 never used). Strictly serialized: one lease at a
time, released before the next was taken. Every lease line is in
`phase4/control/server3-device-leases.tsv` (`QR-ALIGN-2x100`, `QR-ASYNC-8x8192`,
`QR-REDUCE-8192`, plus `QR-ALL-FINAL`).

Protocol: unified reference harness only — `runner_ref.inc`, `DEVICE_EVENT_US` primary /
`HOST_WALL_US` secondary, warmup=10, 2 × 31 samples, batch N=1, gap 2 s, one process,
Direct Parent vs itself. PASS iff full-sample MAD/median ≤ 0.10 **and**
|B1_med − B2_med|/median ≤ 0.10. No Candidate kernel edited, no P/C run, no CANNJudge submission.

## Decision table

| # | Route / shape | device | MAD/med | drift | bad | Decision |
|---|---|---|---:|---:|---:|---|
| 1 | ALIGN-TAIL-X 2x100 FP32 | d4 | **0.2861** | **0.3806** | 0 | **MEASUREMENT_PROTOCOL_BLOCKED_FOR_SHAPE** |
| 2 | ASYNC-TRIPLE-X 8x8192 (tileCount=8) | d4 | **0.2720** | **0.0400** | 0 | **MEASUREMENT_PROTOCOL_BLOCKED_FOR_SHAPE** |
| 2b | ASYNC-TRIPLE-X 8x4096 (documented fallback) | d4 | 0.4163 | 0.7476 | 0 | MEASUREMENT_PROTOCOL_BLOCKED_FOR_SHAPE |
| 3 | REDUCE-INVSCALE-X 1x8192 FP32 | d4 | **0.1571** | **0.8026** | 2048 (parent, expected) | **MEASUREMENT_PROTOCOL_BLOCKED_FOR_SHAPE** |

No shape reached PASS, so no interleaved P/C was started. Nothing here is a Candidate
finding; all three failures are measurement-protocol/host limited.

## What exclusivity actually changed

| Route | prior cycle (sibling leases active) | this cycle (exclusive) | reading |
|---|---|---|---|
| ALIGN | 0.2201 / 0.3724 and 0.1166 / 0.4257 | 0.2861 / 0.3806 | unchanged — same B1-slower-than-B2 direction |
| ASYNC 8x8192 | 0.4565 / 0.4427 | 0.2720 / **0.0400** | drift fixed by exclusivity; MAD/median still >0.25 |
| ASYNC 8x4096 | 0.3449 / 0.0893 | 0.4163 / 0.7476 | worse this window |
| REDUCE 1x8192 | 0.2149 / 0.8667 | 0.1571 / 0.8026 | marginally better MAD/median; drift unchanged |

Conclusion: concurrent sibling leases were **not** the root cause. ASYNC's block drift
was, but its MAD/median remains above 0.25, and ALIGN/REDUCE show the same
first-block-slower-than-second-block asymmetry with zero concurrent probes.

## Host / device state during the runs

- d4 AICore 0% throughout; d5/d6 also idle at AICore 0%; d7 unused.
- Residual VLLM resident on d4 (`VLLMEngineCor`, ~55.6 GB HBM) — expected, not waited on.
- Host load (1-min): 27.2 / 26.4 (ALIGN), 25.1 / 22.8 (ASYNC), 22.3 / 22.4 (REDUCE).
  Driven by persistent VLLM workers plus another user's `Lingma` process (~91% CPU).
  Documented, not suppressed.
- `ps` confirmed **no** other route timing process at any lease start, and none at close.

## Evidence (raw samples permanent)

| Route | report | raw |
|---|---|---|
| ALIGN | `cann-next6/ALIGN-TAIL-X/phase4/local/ALIGN-TAIL-X/V001/support/results-quiet-retry/REPORT.md` | `…/results-quiet-retry/d4/sb1-parent-w10-s31-raw.tsv` (62 rows) |
| ASYNC | `cann-next6/ASYNC-TRIPLE-X/phase4/local/ASYNC-TRIPLE-X/V001/support/results-quiet-retry/REPORT.md` | `…/results-quiet-retry/8x8192/floor/samebin-seed-raw.tsv`, `…/8x4096/floor/samebin-seed-raw.tsv` (62 rows each) |
| REDUCE | `cann-next6/REDUCE-INVSCALE-X/phase4/local/REDUCE-INVSCALE-X/V002/support/results-quiet-retry/REPORT.md` | `…/results-quiet-retry/d4/samebin-w10-s31-raw.tsv` (62 rows) + smoke raw for parent/candidate |

Server copies live under `~/phase4-workspaces/<ROUTE>/` (`results-quiet-retry*`); prior
cycle evidence directories were left untouched.

## Multi-tile confirmation (REDUCE)

Correctness-only smoke, untimed, d4:
- parent `reduce_invscale_refdirect_probe` → `bad=2048`, `max_abs=5.4174` (expected)
- candidate `reduce_invscale_refcand_probe` → `bad=0`, `max_abs=1.669e-06`

Multi-tile path confirmed on both sides, identical to the previous cycle.

## Identity / integrity

- No Candidate or Parent source was modified. Kernel SHA256 verified unchanged:
  - ALIGN candidate `f573d16d…`, parent `c8d0f010…`
  - REDUCE submission `bef271b6…`, parent `94ab0ef9…`
  - ASYNC parent `f20da79c…`, candidate `2defc6c2…` (recorded in `floor/protocol.txt`)
- Binaries reused as built: ALIGN `atx_ref_parent_probe`, ASYNC `async_ref_seed_probe`,
  REDUCE `reduce_invscale_refdirect_probe` / `reduce_invscale_refcand_probe`.
- No rebuild, no source edit, no CANNJudge submission.

## For the next round

These three shapes cannot unlock P/C under the current harness on this host. Options
belong to Main: keep the shapes `MEASUREMENT_PROTOCOL_BLOCKED_FOR_SHAPE`, or change the
measurement layer (e.g. longer in-process settling, repeat-batch calibration retest, or a
quieter host window) before spending another lease. Nothing in this round recommends a
Candidate decision either way.

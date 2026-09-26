# Measurement-layer research: why short-kernel same-binary floors fail with systematic B1 slower than B2

Date: 2026-09-25 · Host: cann-server3 · Device: d4 (secondary checks on d5 evidence) ·
Owner: MAIN-2 measurement research (lease `MR-B1B2-20260925`)
Scope: measurement layer only. No Candidate kernel edited, no P/C run, no CANNJudge
submission, pre-registered outlier policy untouched.

## 1. Question and answer in one paragraph

The registered criterion is `PASS iff full-sample MAD/median ≤ 0.10 AND |B1_med−B2_med|/median ≤ 0.10`.
Four shapes fail it even in exclusive quiet windows (ALIGN 2x100 0.286/0.381, ASYNC 8x8192
0.272/0.040, ASYNC 8x4096 0.416/0.748, REDUCE 1x8192 0.157/0.803) while SCHED 33x100 and
BATCH 100x256 passed on the same harness. Offline analysis of all 25 historical 2-block runs,
11 new same-binary diagnostics (D1–D9b, run today under `MR-B1B2`), and an independently run
parallel battery (`phase4/research/measurement-b1b2-20260925/`, warmup sweep + affinity + gap
variants) shows three separable
mechanisms: **(H1) the first measurement block runs while device/driver state is still settling —
settling completes only after ~40–45 kernel launches, and warmup=10 leaves the entire first block
inside that unsettled window for short kernels, which is the systematic B1>B2 drift; (H2) the first
sample after any inter-block gap pays a cold-wake penalty (median 11× block median, 23/25 runs),
removed entirely by gap=0; (H3) sparse, bursty, non-iid co-tenant interference (resident VLLM,
host load 21–32) inflates full-sample MAD and swings whole blocks of the long HBM-bound ASYNC
kernel regardless of warmup.** Warmup=30 already turned REDUCE into a registered-criterion PASS
in a 4-block diagnostic (MAD/med 0.044, max drift 0.034); ASYNC stays interference-limited.

## 2. Evidence base

### 2.1 Historical raw samples reviewed (all `*-raw.tsv` under `cann-next6/`)

- Failing quiet-window runs: ALIGN `results-quiet-retry/d4/`, REDUCE `results-quiet-retry/d4/`,
  ASYNC `results-quiet-retry/8x{8192,4096}/floor/`.
- Passing runs: SCHED `results-lh3-sb33/`, `results-lh3-pc/sb17-…`, BATCH `harness/samebin-d5/`.
- Prior cycles: ALIGN `results-qual-2x100/`, REDUCE `results-ref-8192/`, ASYNC `results-ref-8x{8192,4096}/`,
  SCHED `results-shape-same-binary/` (8 runs), `results-pc-33x100-lh/`, `results-ref-harness/d4/`
  (warmup study w0/w3/w10/w20, 31 cold-process runs, medium/wide shapes).
- Harness: `SCHED-ROWGROUP-X/.../support/runner_ref.inc` (SHA 89f8380a… identical copies in
  ALIGN, ASYNC, REDUCE remote workspace; BATCH copy differs only in correctness/golden-model code,
  measurement loop identical).

### 2.2 New same-binary diagnostics (d4, lease `MR-B1B2-20260925`, parent/seed only, no P/C)

Run 16:45–16:54 UTC after the H6 correctness battery finished; preflight `ps`=NONE; host load
recorded per run (quiet ≈22–25, one loud stretch ≈28–32 covering D7/D8/D9).

| ID | shape / binary | warmup | blocks | gap | host load | result (device_us medians per block) |
|---|---|---:|---:|---:|---:|---|
| D1 | REDUCE 1x8192 parent | 10 | 4 | 2s | ~24 | 13.20 / 7.58 / 6.86 / 7.04 — B1 slow, rest flat |
| D2 | REDUCE 1x8192 parent | 30 | 4 | 2s | ~24 | 7.08 / 6.96 / 7.20 / 7.18 — ALL MAD/med **0.0440**, max drift **0.0341 → registered PASS-grade** |
| D3 | REDUCE 1x8192 parent | 10 | 4 | 0 | ~24 | 19.68 / 6.84 / 6.52 / 6.50 — B1 slow persists; **no rep1 spikes** (B2 rep1=5.96, fastest sample) |
| D6 | REDUCE 1x8192 parent | 10 | 3 | 10s | ~24 | 13.30 / 7.44 / 6.96 — B1 slow unchanged by longer gap; rep1 spikes 220/485 |
| D4 | ALIGN 2x100 parent | 10 | 4 | 2s | ~24 | 9.52 / 8.28 / 9.20 / 6.76 — B1 floor up (p10 7.10 vs B4 5.78), interference in mid blocks |
| D9 | ALIGN 2x100 parent | 30 | 4 | 2s | 28–32 | B1 clean-ish (6.44); B2 caught an interference burst (24.90) — window contaminated |
| D9b | ALIGN 2x100 parent | 30 | 4 | 2s | ~22 | B1 p10 5.80 ≈ B2 p10 5.68 (**floor shift gone**); residual 0.139/0.206 from spikes only |
| D5 | ASYNC 8x8192 seed | 10 | 4 | 2s | ~24 | 103 / 183 / 115 / 109 — block swings, rep1 spikes 395–671, bimodal 55–110 vs 185–215 |
| D7 | ASYNC 8x8192 seed | 10 | 4 | 0 | 32 | no rep1 spikes (gap=0) but swings 193/133/182/186 persist — interference |
| D8 | ASYNC 8x8192 seed | 30 | 4 | 2s | 28–32 | still wild (99.7/194/242/221) — loud window |
| D8b | ASYNC 8x8192 seed | 30 | 4 | 2s | ~22 | 215/112/227/230, MAD/med 0.264 — warmup does not fix ASYNC even quiet |

Evidence directories (raw + stats + stdout/stderr + runlog):

- `cann-next6/REDUCE-INVSCALE-X/phase4/local/REDUCE-INVSCALE-X/V002/support/results-b1b2-diag/` (D1, D2, D3, D6, runlog)
- `cann-next6/ALIGN-TAIL-X/phase4/local/ALIGN-TAIL-X/V001/support/results-b1b2-diag/` (D4, D9, D9b)
- `cann-next6/ASYNC-TRIPLE-X/phase4/local/ASYNC-TRIPLE-X/V001/support/results-b1b2-diag/` (D5, D7, D8, D8b)

Server copies under `~/phase4-workspaces/<ROUTE>/results-b1b2-diag*` (ALIGN under `support/`).

### 2.3 Independent parallel battery (same lease, run earlier at 15:33 local)

`cann/phase4/research/measurement-b1b2-20260925/` contains a second, independently produced
diagnostic set on d4 (ALIGN 2x100 + REDUCE 1x8192 + ASYNC 8x8192): warmup sweep
w0/w10/w20/w25/w30/w40/w50 ×2 reps, 4-block gap2 (d1), gap0 (d2), warm30 (d3), batch N=16 (d4),
single 62-sample block (d5), gap10 (d6), REDUCE 4-block (d7), plus CPU-affinity variants
(a*=unpin, b*=pin, c*=pin+spin). It independently reproduces every mechanism reported below:

- w10 ALIGN: B1/B2 = 9.5/6.8 and 10.0/7.7 (fail); warm30 d3: **6.1/6.4, MAD/med 0.0665,
  drift 0.0411 → PASS**; w40-r2: 6.0/5.9 (0.0318/0.0201 → PASS); w50-r1: 6.1/6.4
  (0.0386/0.0386 → PASS).
- gap0 d2: B2 rep1 = 5.6 µs (ratio 0.9 — no post-gap spike); gap10 d6: B2 rep1 = 492 µs
  (ratio 80×); gap2 d1: B2/B3/B4 rep1 = 184/432/297 µs.
- CPU pinning does not remove B1 elevation: unpin 21.1/5.9 and 9.2/6.9, pin 9.6/6.5 and 9.9/5.9,
  pin+spin 9.6/6.7 and 9.6/6.4 — same direction under all affinities.
- batch N=16 (d4): 4.3/5.2, drift 0.174, MAD/med 0.275 — no help (consistent with the earlier
  REPEAT_BATCH result in the protocol).
- ASYNC at warmup=40 (asy-w40): MAD/med 0.28–0.38, drift 0.019–0.248 — warmup does not fix it.

## 3. Quantification of B1 vs B2

### 3.1 Direction is systematic, not random

Across all 25 historical 2-block same-binary runs (31×2 samples, unified harness):
**B1 median slower in 20/25**, B2 slower in 5/25. The five B2-slower runs are exactly the
four ASYNC runs plus SCHED `wide-r1-w6144` — the long / HBM-bound shapes. For every short
compute shape (ALIGN ×3, REDUCE ×2, SCHED 33x100 ×3 + sb17, BATCH, SCHED shape-study ×8,
medium, ref-harness) the direction is B1 slower, including the runs that passed:

| run | B1 med | B2 med | drift | verdict |
|---|---:|---:|---:|---|
| SCHED lh3 sb33 (pass) | 49.72 | 45.08 | 0.099 | PASS (just under 0.10) |
| SCHED pc-33x100 attempt1 | 55.60 | 45.66 | 0.200 | fail |
| SCHED pc-33x100 attempt2 | 49.66 | 44.90 | 0.104 | fail |
| SCHED sb17 (pass) | 19.64 | 19.16 | 0.025 | PASS |
| BATCH samebin d5 (pass) | 8.16 | 7.88 | 0.035 | PASS |
| REDUCE quiet | 12.78 | 6.60 | 0.803 | blocked |
| ALIGN quiet | 9.44 | 6.30 | 0.381 | blocked |

The passing runs exhibit the same first-block contamination but the block median survives it.
BATCH B1 first-half median is 68.9 µs (vs second-half 8.0) — 8 of its first 15 samples are
69–280 µs spikes — yet B1 overall median stays 8.16 because ≥16 samples are clean. The
difference between "pass" and "blocked" shapes is largely how much of block 1 the unsettled /
spiky phase covers, relative to the kernel's own noise floor.

### 3.2 Floor (p10) is systematically higher in B1

p10(B1) > p10(B2) in 23/25 runs (the two exceptions are ASYNC 8x4096 runs); for clean
short-kernel runs the absolute gap is roughly
+0.6…+1.9 µs: ALIGN quiet 7.00 vs 5.42 (+29%), REDUCE quiet 7.44 vs 6.36 (+17%), medium 9.60
vs 8.36 (+15%), SCHED 45.34 vs 44.32 (+2.3%), BATCH 7.90 vs 7.58 (+4%). The gap is roughly
fixed in absolute µs rather than proportional to kernel length, so it hurts 6–8 µs kernels
(+17–29% on the floor) far more than 45 µs kernels (+2%).

### 3.3 The first sample after every gap is a cold-wake spike

`B2 rep1 / B2 median` has median **11.09×** and exceeds 2× in **23/25** runs (B1 rep1 exceeds 2×
in only 10/25). Absolute rep1 values: 47–671 µs for kernels of 6–110 µs. Gap duration 2 s vs 10 s
changes nothing in the block floor (D1 vs D6), and gap=0 removes the spike entirely (D3 B2 rep1 =
5.96 µs = fastest sample of the run; D7 B3/B4 rep1 = 55/52 µs = fastest samples; parallel battery
d2 gap0 B2 rep1 = 5.6 µs at ratio 0.9 vs d6 gap10 B2 rep1 = 492 µs at ratio 80×). So the spike is
a wake-from-idle cost at the start of each post-gap block, not a ramp of the whole block. The
first-ever measured sample after process warmup spikes too (D2 B1 rep1 = 202.8; d2 B1 rep1 =
166.1), but at warmup=50 even that sample came out clean (w50-r1 B1 rep1 = 6.0).

### 3.4 The B1 elevation is a settling effect of the first ~40–45 launches

Sequences show a two-state pattern (REDUCE B1 alternates ~7.5 / ~13.5 µs; ALIGN B1 spreads
7–22 µs) that persists while the total launch count since process start is below ~40–45 and
disappears afterwards, independent of warmup count itself:

- D1/D3/D6 (warmup=10): unsettled through all 31 B1 samples (launches 11–41), first clean block
  is B2 (launch ≥42).
- D2 (warmup=30): B1 samples 2–10 still alternate 13.7/7.9 (launches 32–40), clean from sample 11
  (launch 41) onward; because only ~10/31 samples are unsettled the B1 median stays 7.08 and the
  whole run passes the registered criterion (0.044 / 0.034).
- D9b (ALIGN warmup=30): mild elevations until B1 sample ~14 (launch ≈45), clean afterwards;
  B1 p10 5.80 equals B2 p10 5.68 — the floor shift is gone.
- Independent prior evidence: SCHED warmup study medians w0 22.50 → w3 22.44 → w10 20.38 →
  w20 19.60 (monotone), chosen warmup=10 was set from that study where the effect is only ~10%.
- Independent warmup sweep on ALIGN 2x100 (§2.3): B1/B2 = 9.5/6.8 (w10) → 7.6/6.3 (w30) →
  6.0/5.9 (w40-r2, PASS-grade 0.032/0.020) → 6.1/6.4 (w50-r1, PASS-grade 0.039/0.039);
  d3-warm30 = 6.1/6.4 PASS-grade (0.067/0.041). The B1>B2 gap closes as warmup crosses 30–40.

Whether the threshold counts launches or elapsed activity time (~5–9 ms) cannot be separated
from these runs (the two are proportional through a fixed per-launch period); both descriptions
give the same protocol consequence: warmup=10 leaves short-kernel block 1 fully inside the
settling window.

### 3.5 Within-process drift after settling is small

REDUCE D1 B2/B3/B4 = 7.58/6.86/7.04 (max pair drift 0.098); D2 all four blocks within 0.034;
D3 B2/B3/B4 within 0.048. Once settled, in-process block-to-block agreement comfortably meets
0.10 — the registered criterion is achievable on this harness in this host.

### 3.6 Interference (spikes and block swings) is bursty, non-iid, and not load-tracking

- Spike definition (sample >3× block median): 357 device-side occurrences across reviewed runs;
  wall-only spikes are essentially absent, i.e. every device spike also shows up in host wall
  time (wall ≈ device + fixed host overhead), which cannot by itself distinguish "host deschedule
  inside the event window" from "device stalled by co-tenant" — both inflate the event span the
  same way. Structural note: `evStart` is recorded before the kernel is enqueued, so a host
  scheduling delay inside that gap is absorbed into `device_us` (harness property, runner_ref.inc
  lines 285–292).
- Spike counts do not track host load average: ~2–12 spikes per 31 samples at load 22 and at
  load 32 alike. The resident co-tenant (`VLLMEngineCor`, 55.6 GB HBM on d4, VLLM workers
  actively computing on NPUs 0–3/5–6 at 36–37% AICore in npu-smi snapshots, HBM 99% full) is
  constant; a separate `Lingma` user process drives host load swings.
- Clustering: ASYNC sequences are two-state with lag-1 autocorrelation 0.31 (B2) and 0.41 (B2 of
  8x4096); blocks of 8–15 consecutive samples sit at the slow state (e.g. quiet-retry 8x4096 B2
  samples1–14 at 274–586 µs, then 100–110 µs). Within one process, ASYNC block medians swing
  103→183→115→109 (D5) and 215→112→227→230 (D8b) — this is time-varying interference, not block
  order.
- Host wall overhead per sample (wall − device): p10 60 µs, median 131 µs, p90 224 µs. Blocks
  take ~5–11 ms of wall time; whole diagnostics take 8–60 s.

### 3.7 Hypotheses checked and rejected

- **Thermal**: NPU4 temperature flat 41–45 °C start→end in every window; runs last seconds; and
  the *cooler* block (B1, right after warmup) is the slower one — opposite of thermal drift.
- **Sibling-lease contention**: already falsified by the quiet-retry round (exclusive windows,
  same directions: ALIGN 0.381, REDUCE 0.803).
- **Event reuse across blocks**: events are created once and reused, but the reuse pattern is
  identical for every sample of every block; B2–B4 are mutually tight in D1/D2/D3 while B1 is
  not, so reuse alone cannot produce first-block-only elevation.
- **Block order as such**: warmup=30 with the same order (D2, D9b) removes the B1 elevation;
  gap=0 with the same order (D3) keeps it. Order is a proxy for "first block after warmup", and
  the quantity that matters is launches-since-process-start.
- **Host CPU scheduling / affinity as the cause of the B1 elevation**: the parallel battery ran
  ALIGN under unpin / pin / pin+spin CPU affinities — B1 stays ≈9.5–10 with B2 ≈6–7 in all six
  runs (plus one unpin outlier at 21.1). Affinity changes spike counts at most; the first-block
  elevation is invariant to host scheduling policy, supporting a device/driver-side settling
  state (H1) over host-side delay for that specific effect.
- **Batch N / repeat-batch**: previously calibrated (protocol §repeat-batch: N=10 drift 0.254/0.111,
  not adopted); consistent with drift being a block-level state problem, not an event-resolution
  problem.

## 4. Harness inspection (`runner_ref.inc`, lines 253–303)

State between block1 and block2, exactly as implemented:

1. Warmup runs **once per process**, before all blocks (line 259–263); block 1 starts immediately
   after event creation — B1 is always the first block after warmup. There is no settling sample
   budget beyond `warmup`.
2. Gap: `nanosleep(gapSec)` only for `b > 0` (line 276–280), then `aclrtSynchronizeStream`.
   With gap ≥ 1 s the device idles between blocks → rep1 cold-wake spike (§3.3). gap=0 gives
   back-to-back blocks.
3. Events `evStart/evStop` created once, reused for all samples in all blocks (line 265–267);
   per sample: record start → enqueue `batch_n` launches → record stop → sync stop →
   elapsed → sync stream → wall stop. `evStart` precedes the kernel enqueue, so host scheduling
   delay inside the window lands in `device_us`.
4. No other state: no malloc/copy/correctness inside the timed loop; D2H/golden compare runs
   once after all blocks (line 306+). CLI exposes `warmup samples blocks gap_sec batch_n` —
   every diagnostic above was produced through CLI parameters only; no harness edit, no rebuild.
5. Block order cannot be swapped without a harness edit; the warmup/gap/blocks variations used
   here answer the same questions (first-block effect vs gap effect vs time drift) without one.

## 5. Ranked root-cause hypotheses

**H1 — Insufficient pre-block settling (rank 1; explains the systematic B1>B2 drift).**
Device/driver state needs ~40–45 launches (~5–9 ms) after process start before sample times
reach their floor. warmup=10 puts the entire B1 of short kernels inside that window, so B1's
whole distribution (not just its first samples) sits 15–100% above B2's; this is exactly the
observed drift 0.38–0.86 on ALIGN/REDUCE, the mild 0.10–0.20 drifts on SCHED near the threshold,
and why BATCH (whose contamination period shrank to <50% of B1 by run-to-run luck) passed.
Fix verified: warmup=30 → REDUCE registered PASS-grade (D2), ALIGN floor shift gone (D9b).
Kernel-time dependence (fixed-µs penalty, §3.2) explains the shape split: short kernels fail,
45 µs kernels pass.

**H2 — Post-gap cold-wake on the first sample of every block (rank 2; explains rep1 spikes,
contributes to MAD/max but not to drift by itself).**
Median 11× block median in 23/25 runs; removed entirely by gap=0 (D3, D7); unaffected by gap
2 s vs 10 s. Because a single spike out of 31 cannot move a median, this alone does not fail the
registered criterion (D2 passed while carrying four 150–350 µs rep1 spikes) — it inflates max,
full-sample MAD slightly, and wall time.

**H3 — Bursty co-tenant interference, non-iid (rank 3; dominant residual for ASYNC, and the
thing that keeps ALIGN/REDUCE MAD/med near or above 0.10 even after H1 is fixed).**
Resident VLLM on the host/device plus load swings from foreign processes produce multi-ms
two-state slowdowns (ASYNC 55–110 vs 190–240 µs; lag-1 autocorr up to 0.41) and clustered
100–350 µs spikes on short kernels. It is independent of block order, unaffected by warmup
(D8b), only partially mitigated by lease exclusivity (handoff round), and does not track the
1-min load average — so "quiet window" in the lease sense is not sufficient for ASYNC.

**H4 — Event-window structure absorbs host scheduling delay (rank 4; background floor).**
`evStart`→enqueue→`evStop` means a descheduled host thread inflates `device_us` with no
wall/device asymmetry to detect it; with batch N=1 and 6–110 µs kernels, a single 100 µs
deschedule is a 2–15× outlier. Contributes to spike counts under load 21–32; not the systematic
drift (H1 persists at load 22 and 32 alike).

**Rejected**: thermal drift, sibling leases, event reuse, pure block-order mechanics (§3.7).

## 6. Recommended Main action: (a) harness change + validation plan, with (b) as a required
companion for ASYNC and a (c) fallback

**(a) Change: `warmup` default 10 → 45 in the unified protocol** (`local-timing-protocol.md`
§Warmup). Rationale: covers the observed ~40–45-launch settling window with margin; costs
~7–10 ms per run; validated same-binary today (REDUCE D2 full PASS-grade; ALIGN d3/w40-r2/
w50-r1 PASS-grade; D9b floor level fixed). This is
a protocol-parameter change only — it does not touch kernels, the registered PASS criterion, or
the pre-registered outlier policy (no sample deletion, no rep1 exclusion; D2 shows the criterion
holds with rep1 spikes present).

Validation plan before any P/C (all same-binary, Direct Parent vs itself, d4 exclusive, one
lease at a time, registered criterion unchanged):

1. REDUCE 1x8192, warmup=45, 2×31, gap 2 — expect PASS (D2 already at 0.044/0.034 with only
   30 warmups; 45 also removes D9b's residual first-10-samples exposure).
2. ALIGN 2x100, warmup=45, 2×31, gap 2 — expect PASS: three PASS-grade warmup≥30 runs already
   exist (d3 0.067/0.041, w40-r2 0.032/0.020, w50-r1 0.039/0.039). The open question is
   window-to-window spike load (D9b residual 0.139 when a burst landed in B1/B3). If a run
   lands >0.10, retry once in the quietest available window (b); if still >0.10, keep ALIGN
   `MEASUREMENT_PROTOCOL_BLOCKED_FOR_SHAPE` (c) — its residual would be interference, not
   block asymmetry.
3. ASYNC 8x8192 / 8x4096: warmup change alone does not address them (D5/D7/D8b). Sequence:
   (b) obtain a genuinely quiet co-tenant window (VLLM inference actually idle, not just
   "no sibling lease"), then same-binary with warmup=45; if the two-state interference persists,
   keep both ASYNC shapes `MEASUREMENT_PROTOCOL_BLOCKED_FOR_SHAPE` (c). Do not spend further
   quiet-retry leases on ASYNC under the current resident load — three windows already agree.
4. Optional secondary (only if Main wants it, needs its own same-binary check): gap 2 s → 0
   (or 0.5 s) to remove the rep1 cold-wake, trading inter-block idleness; and blocks=3 so drift
   is judged on more than one pair. Neither is required for (a) to work.

Explicitly **not** adopted here: any change to outlier handling, any rep1-based exclusion, any
post-hoc rule keyed to Candidate results. This document proposes parameters only; the protocol
file is edited by MAIN-2 ownership after Main approves, and no P/C runs until the route's
same-binary re-validation in step 1–3 passes.

**(b) companion**: schedule ASYNC re-validation only when VLLM inference is actually paused
(AICore of *all* NPUs near 0% over a sampled minute, not just d4; host load <10 if possible).
Lease exclusivity among route probes was already shown insufficient.

**(c) fallback**: if (a) steps 1–2 still fail after a clean window, accept
`MEASUREMENT_PROTOCOL_BLOCKED_FOR_SHAPE` for ALIGN/REDUCE under current policy rather than
loosening the criterion — the criterion itself was shown achievable (D2, and prior SCHED/BATCH
passes).

## 7. Integrity

- Kernel sources untouched; SHAs unchanged (ALIGN candidate f573d16d…, REDUCE bef271b6…,
  ASYNC parent f20da79c…). Binaries reused as built (no rebuild).
- No P/C pair run, no CANNJudge submission, no candidate revision, outlier policy file not
  modified.
- Diagnostics used only CLI parameters of the existing `runner_ref.inc` (SHA 89f8380a…).
- All raw samples retained verbatim in the evidence directories of §2.2; server copies under
  `~/phase4-workspaces/<ROUTE>/results-b1b2-diag*`.
- Lease `MR-B1B2-20260925` (device 4) covered the diagnostics; H6 correctness battery finished
  before timing started; row left as-is for Main to release.

# MAIN2-CANONICAL-V1 — frozen local scorer

Authority: C2C CONTROL — MAIN-2 CANONICAL LOCAL SCOREBOARD + ASSET CONSOLIDATION,
2026-10-05. This Planning instruction supersedes the earlier predictor-readiness
gate and formal-qualification gate **for this engineering local scoreboard**.
No Kernel change, new performance Revision, Online submission, or route selection.

## Identity and fixed suite

- `SCORER_VERSION=MAIN2-CANONICAL-V1`
- `SUITE_VERSION=CANONICAL_LOCAL_SUITE_V1`
- Source anchor: R31B/V011, SHA-256
  `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- Official anchor = 45.16, 15/15; **not a local score**.
- Local anchor = **100.000000**, normalized to this campaign's fresh raw vector.
- Exactly four equally weighted Core cases: C12, C13, C14, C16.
- Exactly three Diagnostic cases: C01, C08, C11. Diagnostics never enter the score,
  even if a Candidate is unusually fast or slow on them.
- The seven cases retain the existing V4 engineering case set. Roles are frozen
  before this campaign's measurements, using raw availability, measurement
  stability and source-path coverage, not Official labels or Candidate scores.
- The core covers the BF16 D=8192 cache boundary plus FP16/BF16 wide paths. Small,
  mid and FP32 paths are diagnostic-only. FP32-wide and unaligned-D performance
  are **not measured by the score**. A mechanism outside core scope cannot be
  declared beneficial merely because its numerical score exceeds 100.
- C16's legacy name "reduced-tail" means a reduced resident-row group, not an
  unaligned column tail. C11 D=8192 is not the FP32 wide path (wide means D>8192).

Only Planning can change cases, roles, formula or aggregation in a new version.
Scores from different scorer/suite/anchor identities are not directly ranked.

## Measurement: ENGINEERING_3RUN

Reuse the proven `runner_ref.inc` device-event harness, byte-for-byte. Retain its
source hash and its existing golden reference; do not alter input generation,
math reference, Kernel, launch policy or timing boundary.

- Same server3 Ascend 910B3 device class; d4 preferred, one predeclared safe d7
  fallback only if necessary. Check leases across current worktrees before each
  batch and hold one exclusive timing lease per device.
- Build and full seven-case preflight correctness must PASS before Candidate
  timing. Known unresolved correctness failures remain blocked, not negative.
- Fresh R31B/V011: three independent processes per case; this is not reconstructed
  from an old paired Parent log. It establishes the immutable raw local anchor.
- Candidate: three valid interleaved pairs, order P-C / C-P / P-C. P is exact V011
  for scoreboard comparability, **not** necessarily the Candidate's genealogical
  Direct Parent. Both identities are recorded separately.
- Per process: warmup 45, one block, 31 device-event samples, batch_n=1, gap=0.
  H2D/allocation precede timing, D2H/golden check follow timing. Host-wall samples
  remain secondary diagnostics. No correctness operation inside timed samples.
- At most five attempts to obtain three valid runs. A valid run has RC=0, bad=0,
  31 finite positive event samples, matching source/runner/protocol/shape identity.
  Record every failure. Never retry merely to obtain a faster or less noisy run.
- Formal qualification jitter is not a hard gate. Keep GOOD/FAIR/POOR data;
  missing identity, runtime error, correctness failure or invalid samples are not
  converted to a numerical score.
- Record per-batch HBM, AICore/AIVector, host load and observed processes, exact
  command, device, executable/source SHA and lease. HBM free<100 MB blocks a new
  job; when `npu-smi` exposes only integer usage, record the conservative free-HBM
  lower bound explicitly, never label an estimate as exact free memory.

## Formula and outliers

For a run, `run_us = median(all 31 device_event_us samples)`.
For a case, `T = median(run1_us, run2_us, run3_us)`.
Also report the mean of run medians, all raw samples, min/max/CV/MAD and throughput.
A single extreme run cannot determine T. No sample removal, winsorization,
best-of-run, per-Candidate trimming, Official-label fitting or special case weight.

```text
speedup_i = frozen_fresh_V011_T_i / candidate_T_i
CANONICAL_LOCAL_SCORE = 100 * exp(mean_i(log(speedup_i)))  [Core only]
DELTA_VS_R31B_V011 = CANONICAL_LOCAL_SCORE - 100  [local points]
THROUGHPUT_ELEMENTS_PER_SECOND = rows * width * 1e6 / T_us
```

No throughput term is added to the formula. Ratio and throughput must be inverse
consistent. The contemporaneous interleaved Parent is a drift/sign diagnostic;
it never silently replaces the frozen anchor denominator.

## Quality, uncertainty and verdicts

These are engineering classifications, not a statistical confidence guarantee.

- Per case quality: `(max(run medians)-min(run medians))/mean(run medians)`.
  GOOD<=0.10, FAIR<=0.25, otherwise POOR; include the worse anchor/paired-Parent/
  Candidate quality. Preserve within-run MAD, CV and paired sign counts too.
- Empirical case uncertainty in log space: the maximum of the within-vector
  run-median log range (anchor, Candidate, paired Parent), twice the median
  relative within-run MAD, and absolute paired-Parent drift from the frozen
  anchor. Aggregate conservatively by the mean across Core cases.
- Empirical neutral interval: score / exp(mean uncertainty) through
  score * exp(mean uncertainty). LOCAL_POSITIVE only when its lower bound>100;
  LOCAL_NEGATIVE only when upper bound<100; otherwise LOCAL_NEUTRAL. POOR core
  measurements cannot establish a reliable improvement. Do not use these bands
  as calibrated confidence intervals or as an Online gate.
- Numerical rank retains every eligible finite score, including noisy scores.
  `LOCAL_CHAMPION` denotes the numerical leader of this fixed suite, not Official
  promotion or a newly approved performance Parent. Reliability is a separate
  field; a noisy champion must be reported as provisional/fragile.
- `LOCAL_GAIN_FRAGILE=YES` if a single case contributes >half the positive log
  gain, removing one Core case eliminates the aggregate gain, a Core case is
  POOR, or paired-Parent drift/sign evidence contradicts the frozen-anchor gain.
  TOP results must include all Core ratios and the leave-one-case-out scores.
- UNSCORED_SOURCE_MISSING / UNSCORED_BUILD_BLOCKED /
  UNSCORED_CORRECTNESS_BLOCKED / UNSCORED_MEASUREMENT_INVALID remain explicit;
  UNSCORED is never interpreted as LOCAL_NEGATIVE.

## Raw reuse policy

`REUSED_VALID_RAW` is permitted only after checking exact source SHA, harness
SHA, shape/dtype, warmup=45, samples=31, blocks=1, batch_n=1, three valid runs,
same device class, correctness and raw evidence. Reaggregate raw with this
frozen scorer; never copy a historical delta or predicted score into this table.
The 20261005 V4 engineering batches are reuse candidates. V2 batches have
warmup=20/samples=11 and are **incompatible**; their binaries may be reused after
identity checks but their timings are not Canonical data.

## Ownership and canonical truth

- `技术路线/MAIN2-CANONICAL-ROUTE-REGISTRY.tsv`: architecture/identity inventory.
- `本地实验/MAIN2-CANONICAL-LOCAL-SCOREBOARD.tsv`: sole canonical numerical ranking.
- `本地实验/R31B-V011-CANONICAL-LOCAL-VECTOR.tsv`: fresh anchor vector.
- `本地实验/MAIN2-CANONICAL-V1/`: per-version raw, identity, build and correctness.
- `研究/Local-Judge-History/`: experimental predictors, NOT_CANONICAL_SCORE.
- `调度/MAIN2-NEXT-ROUTE-PLANNING-PACK.md`: advice only; MAIN_SELECTED=NONE.

Historical Local verdicts and Official results are preserved, not rewritten by
the new local ranking. Raw/code snapshots are permanent. Archive before cleanup;
delete only manifest-approved duplicates with a verified replacement and no
references. Current Agent owns only the Main-2 control worktree and its new
assets; unrelated worktrees, legacy leases and canonical-main dirty files stay
untouched. Push failure does not block local experiments.

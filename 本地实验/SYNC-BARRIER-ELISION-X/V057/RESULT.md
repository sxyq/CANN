# SYNC-BARRIER-ELISION-X V057 Result

- Direct Parent: exact `R31B-V011`, SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- Candidate: `submission.asc`, SHA256 `03db8e677e50fa456201c97426a74027a1841cfc9db5f3f4b1a0ec905c5dd4ed`.
- Single change: delete only the `PIPE_V` barrier in `ApplyFp16GammaBiasBatch` between batched FP16 gamma `Mul` and bias `Add`; exact source diff is `diff.patch`.
- Compile: PASS, target `sync_barrier_elision_v057`, `logs/compile-v057-retry-cplusplus-include-20261008.log`. The first generated-plugin build failed to find `<vector>`; that failure is retained at `logs/compile-v057-20261008.log`. The include-path-only retry passed. Compiled executable SHA256: `b6c6fdfb197411b3372494f95abd638818623a226ee841ae30e725c8a27e51d6`.
- Correctness: PASS, 7/7 FP16 Parent/Candidate bitwise cases with zero mismatches, `logs/correctness-v057-device3-20261008T050851Z.log`. Runner SHA256: `e9bdc46534b8b6e8332f81a1a11748ca0de703949a397d905915ed3ac8be8830`.
- Local: FP16 128x128, device-event timing; 60 warmups per binary per block; two 31-pair alternating Parent/Candidate blocks; all 62 raw pairs, wall latencies and throughput are retained in the two `logs/local-block*-v057-device3-*.log` files. No samples were filtered.

## Score Reconciliation

The reported Local score is the pooled ratio-of-medians device-latency reduction:

`100 * (Parent median device_us - Candidate median device_us) / Parent median device_us`

Across the exact 62 raw pairs, Parent device-event median/mean was `17.0200/18.9265 us`; Candidate was `15.2900/14.4335 us`. This gives `+10.164512%` by ratio-of-medians. Median paired delta (Candidate minus Parent) was `-2.0500 us`, which gives `+12.044653%` under the distinct estimator `-100 * median(paired delta) / Parent median`. The ratio-of-means is `+23.738751%`; it is not the reported score. The wide estimator spread is retained rather than selecting the most favorable value.

The runner's per-block `LOCAL_RESULT.local_score_percent` field uses the paired-median-delta estimator, not the ratio-of-medians above: it reports `+5.328802%` for block 1 and `+13.388625%` for block 2. The block table below independently recomputes the declared ratio-of-medians from each block's raw Parent and Candidate samples.

| Block | P median / mean (us) | C median / mean (us) | Ratio-of-medians score | Median paired delta (C-P, us) |
|---|---:|---:|---:|---:|
| 1 (31 pairs) | 17.6400 / 21.9955 | 13.9600 / 15.2761 | +20.861678% | -0.9400 |
| 2 (31 pairs) | 16.8800 / 15.8574 | 15.9800 / 13.5910 | +5.331754% | -2.2600 |
| Pooled (62 pairs) | 17.0200 / 18.9265 | 15.2900 / 14.4335 | +10.164512% | -2.0500 |

Pooled device-event stdev/CV/range: Parent `19.0911 us / 100.87% / 6.64-150.74 us`; Candidate `7.0812 us / 49.06% / 5.96-33.84 us`. The paired-delta range is `-142.12 to +25.86 us`, with mean `-4.4929 us` and stdev `20.1769 us`. Pooled wall-latency median/mean: Parent `72.4355/78.6772 us`; Candidate `72.0600/74.6975 us`. Pooled throughput median/mean: Parent `0.962633/1.190822 GElement/s`; Candidate `1.071551/1.432248 GElement/s`.

## Qualification, Load, And Verdict

- Same-binary qualification used 31 paired measurements for each binary. Parent device-event first/second CV was `59.844%/52.399%`, with median drift `18.262%`; Candidate CV was `81.034%/60.216%`, with median drift `57.840%`. Full raw logs are `logs/same-binary-parent-v057-device3-20261008T051016Z.log` and `logs/same-binary-candidate-v057-device3-20261008T051059Z.log`.
- Fresh post-assignment device snapshot: device 3 HBM `9,131/65,536 MB` used (`56,405 MB` free), AICore `13%`; listed PID `2532307` (`python`, `5,758 MB`). Pre-Local snapshot: same HBM/process, AICore `1%`. Between blocks: same HBM/process, AICore `5%`. Post-capture: same HBM/process, AICore `15%`. Snapshots are preserved in `logs/device3-*.log`. Existing processes were not disturbed.
- Main's exclusive device-3 assignment covered V057 correctness, qualification, and both Local blocks. The assignment was explicitly released after raw capture and the post-capture snapshot at `2026-10-08T05:13:43Z`; see `logs/device3-release-20261008T051343Z.log`.
- Verdict: `LOCAL_REJECTED_NOISY`. Both blocks are numerically positive, but same-binary drift and pooled Parent CV are extreme and the estimates vary materially by block and estimator. Do not promote V057; Local Best remains exact `R31B-V011`. This single-shape Local measurement is not comparable to Official `45.16`. No Online or shared-record writes were made.

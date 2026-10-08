# SYNC-BARRIER-ELISION-X V064 Result

## Revision and qualification

- Route/revision: `SYNC-BARRIER-ELISION-X` / `V064`.
- Direct Parent and Local Best: exact `R31B-V011`.
- Parent source SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- Candidate source SHA256: `4db6715c4e11a1b840998f2aeb08a97a551bf4f84dbfd83c1d630f41e5d75204`.
- Single change: delete only the `SyncVToS()` after the per-row `Sqrt` loop and before reading `reduceLocal.GetValue(...)` into `invRmsValues` in `ProcessSmallLowPrecisionContiguousBatched`.
- Compile: `PASS` for `sync_barrier_elision_v064` and `sync_barrier_elision_correctness`; see `logs/compile-v064.log`.
- Correctness: `PASS`, 7/7 FP16 cases bitwise equal; see `logs/correctness-v064.log`.
- Same-binary qualification: 31 samples each. Parent first/second device medians `15.42/15.54 us`, CV `0.41757/0.47707`, median drift `0.007752`. Candidate medians `15.60/16.22 us`, CV `0.37434/0.41363`, median drift `0.038969`. Raw logs: `logs/qualification-v064-parent.log` and `logs/qualification-v064-candidate.log`.

## Local capture

- Device: NPU 3, Ascend910B3; shape `128x128`, FP16. Device-event latency is primary; wall time is diagnostic.
- Runner: `/tmp/sync-v064-build/sync_barrier_elision_correctness`; labels verified as `PARENT_R31B_V011` and `CANDIDATE_V064`.
- Method: 60 warmups per invocation, two interleaved blocks of 31 P/C pairs each, alternating order; all 62 pairs retained without filtering.
- Raw samples: `logs/local-v064-block1.log` and `logs/local-v064-block2.log`. Values below are recomputed from the four-decimal device-event values in every raw pair.
- Block 1: Parent/Candidate device medians `15.70/16.14 us`, means `16.211613/16.066452 us`, paired-delta median `+0.48 us`; paired-median score `-3.057325%`, median-latency speedup `-2.726146%`.
- Block 2: Parent/Candidate device medians `7.10/7.38 us`, means `13.186452/13.130323 us`, paired-delta median `+0.02 us`; paired-median score `-0.281690%`, median-latency speedup `-3.794038%`.

## Pooled numeric result

- Parent device latency, `n=62`: median `15.60 us`, mean `14.699032 us`, stdev `6.653777 us`, CV `0.452668`, MAD `4.89 us`, range `6.18-35.18 us`, p10/p90 `6.46/23.126 us`.
- Candidate device latency, `n=62`: median `15.50 us`, mean `14.598387 us`, stdev `6.225772 us`, CV `0.426470`, MAD `4.92 us`, range `5.92-29.80 us`, p10/p90 `6.52/21.364 us`.
- Paired Candidate-minus-Parent delta: median `+0.23 us`, mean `-0.100645 us`, stdev `7.329105 us`, MAD `1.83 us`, range `-18.84 to +17.06 us`, p10/p90 `-12.336/+9.274 us`.
- Primary paired-median Local score: `-1.474359%`, computed as `-100 * median(paired Candidate-minus-Parent delta) / median(Parent latency)`.
- Separate pooled median-latency speedup: `+0.645161%`, computed as `100 * (Parent median / Candidate median - 1)`. Mean-latency speedup is `+0.684706%` and is only an outlier-sensitive diagnostic.
- Pooled wall-time medians: Parent/Candidate `69.924/70.199 us`; means `71.942242/70.157000 us`. Pooled per-sample throughput medians: Parent/Candidate `1.050258/1.057048 Gelem/s`.

## Noise, load, and verdict

- The pooled paired-median score is negative while the pooled ratio of medians is slightly positive. Both blocks' paired-median scores are negative; pooled parent/candidate distributions are also broad and bimodal across blocks. This is not a repeatable gain.
- Device snapshots before Correctness, qualification, and both Local blocks show device 3 at 5% HBM usage, 0% AICore, and no device process. Host load varied: post-Compile/pre-Correctness at `15:07:55Z` was `61.29/55.38/57.37`; post-Correctness/pre-qualification at `15:09:30Z` was `40.93/50.70/55.54`; pre-block-1 at `15:11:53Z` was `42.75/47.88/53.81`; pre-block-2 at `15:13:14Z` was `63.97/51.81/54.60`; post-capture at `15:14:34Z` was `46.56/49.68/53.65`. All snapshots are retained in `logs/`.
- Device 3 was explicitly released at `2026-10-08T15:14:36.468579289Z` after the post-capture snapshot; see `logs/device3-v064-post-local-release-snapshot.log`.
- Verdict: `LOCAL_REJECTED_NOISY`. No promotion; `CURRENT_LOCAL_BEST` remains exact `R31B-V011`.
- Single-shape Local is not comparable to Official Score `45.16`; no Official score was measured and no Online action was taken.

## Evidence identities

- Kernel executable SHA256: `dfb731b7642ad608b90e60e68c5416f79a61eccb6b30aaf48a9a47e0fece3bee`.
- Correctness/Local runner executable SHA256: `fff22a75ab61ea2e6ac06efcc5b1df0f8c90cf5f4c1ce84ef7a3b9bc79e739e6`.
- Candidate source, Parent source, build, qualification, correctness, raw Local samples, resource snapshots, and release evidence are retained in this revision directory.

# SYNC-BARRIER-ELISION-X V039

- ROUTE: SYNC-BARRIER-ELISION-X
- REVISION: V039
- DIRECT_PARENT: R31B-V011
- PARENT_SOURCE_SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- SINGLE_HYPOTHESIS: In generic `Process()`, remove only the `SyncMTE3ToV()` immediately after FP16 output `Store(outputGm_, rowOffset + col, outputLocal, valid)`.
- SINGLE_CHANGE_BOUNDARY: Delete one MTE3-to-V event drain from exact R31B-V011. No V001-V038 candidate edits are inherited.
- DUPLICATION_AUDIT: V001-V038 route-local actual diff patches contain no deletion of this generic `Process()` FP16 post-store drain. V032 deletes the distinct final `ProcessNarrowMidOverlap` post-loop drain; V033 deletes its distinct per-row drain. The selected call is inside generic `Process()`'s half-precision output branch and runs at width 128.
- COMPILE_TARGET: `sync_barrier_elision_v039`
- CORRECTNESS_CASES: FP16 rows=8, widths=128, 256, 1024, 2048, 4096; exact Parent/Candidate bitwise comparison.
- LOCAL_CASE: FP16 8x128, device 3, 60 warmups and two 31-pair interleaved runs using device events and wall-clock diagnostics; retain every sample without outlier filtering.
- CURRENT_LOCAL_BEST: R31B-V011
- ONLINE: NO

- CANDIDATE_SOURCE_SHA256: `cb1c13eb42c86b5d1cf625c5bfddb96d3c737c0120463a48aa9de6393859eabd`
- COMPILE: PASS; target `sync_barrier_elision_v039`; configure RC=0, build RC=0; CANN `8.5.0.alpha002`, host `hwnput3`, Ascend910B3, device 3.
- CORRECTNESS: PASS; FP16 rows=8 widths 128, 256, 1024, 2048, and 4096 all had zero bit mismatches; runner RC=0 at `2026-10-07T21:21:59Z`.
- LOCAL: 62 raw interleaved Parent/Candidate device-event pairs on FP16 8x128. Pooled `LOCAL_SCORE=+3.576375074%` geomean paired speedup. Per-run scores were `-3.251160558%` and `+10.885727781%`; paired-median delta diagnostics were `-2.712698%` and `+6.874999%`. Pooled Parent/Candidate device medians were `17.35/17.37 us`, CV `0.350689/0.414146`; raw ranges were `4.20-32.06/4.06-42.92 us`. The run scores reverse and the pooled device medians are effectively equal; all samples retained; `LOCAL_REJECTED_NOISY`.
- LOCAL_QUALITY: Pooled median-derived throughput `0.059020/0.058952 G elements/s` (Parent/Candidate, diagnostic); pooled wall medians `76.199/75.369 us`, wall CV `0.238663/0.128913`, maximum wall latency `181.188/130.538 us`. Device 3 AICore remained 0% before/after both runs; HBM snapshots were `3425/3428 MB` and `3426/3428 MB`. Other devices had active AICore users, including device 4 at 64%; host load averages after runs were `60.16/64.41/58.58` and `58.11/63.62/58.50`. This single-shape Local score is not comparable to Official 45.16.
- CURRENT_LOCAL_BEST: `R31B-V011` (unchanged).
- ONLINE: NOT_SUBMITTED.

Evidence: `compile-result.json`, `correctness-result.json`, `local-result.json`, `source-meta.json`, `submission.sha256`, `diff.patch`, and raw logs under `logs/`.

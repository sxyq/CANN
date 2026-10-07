# SYNC-BARRIER-ELISION-X V025

- ROUTE: SYNC-BARRIER-ELISION-X
- REVISION: V025
- DIRECT_PARENT: R31B-V011
- PARENT_SOURCE_SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- SINGLE_HYPOTHESIS: In `ProcessNarrowMidOverlap`, remove only the common `PIPE_V` barrier after the dtype-specific input-combine branch and before `Mul(xFp32, valueLocal, valueLocal, valid)`.
- SINGLE_CHANGE_BOUNDARY: One synchronization-operation deletion from the exact R31B-V011 parent. No V001-V024 candidate changes are inherited.
- DUPLICATION_AUDIT: V001-V024 actual source diffs contain no deletion at this common post-branch site. V004/V008 remove the distinct FP16 input-Add-to-`ToFloat` barrier; V005 removes the FP32 Add-to-square barrier inside the alternate branch; V007 removes the following square-Mul-to-ReduceSum barrier.
- COMPILE_TARGET: `sync_barrier_elision_v025`
- CORRECTNESS_CASES: FP16 rows=8, widths=128, 256, 1024, 2048, 4096; exact Parent/Candidate bitwise comparison.
- LOCAL_CASE: FP16 8x2048, device 3, 60 warmups and two 31-pair interleaved runs using device events and wall-clock diagnostics; retain every sample and no outlier filtering if Correctness passes.
- CURRENT_LOCAL_BEST: R31B-V011
- ONLINE: NO

- CANDIDATE_SOURCE_SHA256: `92afad39c7bcfe3331150f5627ff996d81645f2d1230d6cb1a612660ee91e12e`
- COMPILE: PASS on `hwnput3`, CANN `8.5.0.alpha002`, `dav-2201`; configure and build return codes were 0.
- CORRECTNESS: PASS; FP16 rows=8 widths 128, 256, 1024, 2048, and 4096 all had zero bit mismatches.
- LOCAL: 62 raw interleaved Parent/Candidate pairs across two 31-pair runs. `LOCAL_SCORE=+11.377344384%` pooled geomean device-event speedup. Per-run paired geomean scores were `+3.142580728%` and `+20.269560394%`; runner median-delta diagnostics were `-4.203824%` and `+1.595093%`. Pooled Parent/Candidate device medians were `15.81/13.53 us`, with event CV `0.483903/0.534607`; run medians and paired-delta diagnostics reverse direction and jitter is high. All samples retained; `LOCAL_REJECTED_NOISY`, no promotion.
- LOCAL_QUALITY: Pooled median-derived effective throughput was `1.036306/1.210939 G elements/s` (Parent/Candidate). Device 3 AICore load was 0% before and after both runs; other devices showed 45-50% AICore activity during the snapshots, and unrelated resident processes were left untouched. This one-shape Local metric is not comparable to Official 45.16.
- CURRENT_LOCAL_BEST: `R31B-V011` (unchanged).
- ONLINE: NOT_SUBMITTED.

Evidence: `compile-result.json`, `correctness-result.json`, `local-result.json`, `source-meta.json`, `submission.sha256`, `diff.patch`, and raw logs under `logs/`.

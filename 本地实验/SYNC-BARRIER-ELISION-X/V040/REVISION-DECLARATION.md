# SYNC-BARRIER-ELISION-X V040

- ROUTE: SYNC-BARRIER-ELISION-X
- REVISION: V040
- DIRECT_PARENT: R31B-V011
- PARENT_SOURCE_SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- SINGLE_HYPOTHESIS: In generic `Process()` first-pass FP16 processing, remove only the `PIPE_V` barrier after `ToFloat(valueTile, xLocal, valid)` and before `Mul(xFp32, valueTile, valueTile, valid)`.
- SINGLE_CHANGE_BOUNDARY: Delete one vector-pipeline barrier from exact R31B-V011. No V001-V039 candidate edits are inherited.
- DUPLICATION_AUDIT: V001-V039 route-local actual diff patches contain no deletion at this generic first-pass conversion-to-square-Mul point. V004/V008 target the distinct `ProcessNarrowMidOverlap` Add-to-`ToFloat` barrier; V012 targets a generic second-pass widening-to-Add barrier; V013 targets the later first-pass square-Mul-to-`ReduceSum` barrier. Width 128 dispatches to generic `Process()` and executes the selected first-pass FP16 conversion and barrier.
- COMPILE_TARGET: `sync_barrier_elision_v040`
- CORRECTNESS_CASES: FP16 rows=8, widths=128, 256, 1024, 2048, 4096; exact Parent/Candidate bitwise comparison.
- LOCAL_CASE: FP16 8x128, device 3, 60 warmups and two 31-pair interleaved runs using device events and wall-clock diagnostics; retain every sample without outlier filtering.
- CURRENT_LOCAL_BEST: R31B-V011
- ONLINE: NO

- CANDIDATE_SOURCE_SHA256: `fe66c884741e4dd7fc81d9b4301da7e785565eb32b32eb943f823417208ab994`
- COMPILE: PASS; target `sync_barrier_elision_v040`; configure RC=0, build RC=0; CANN `8.5.0.alpha002`, host `hwnput3`, Ascend910B3, device 3.
- CORRECTNESS: PASS; FP16 rows=8 widths 128, 256, 1024, 2048, and 4096 all had zero bit mismatches; runner RC=0 at `2026-10-07T21:29:31Z`.
- LOCAL: 62 raw interleaved Parent/Candidate device-event pairs on FP16 8x128. Pooled `LOCAL_SCORE=-11.119541502%` geomean paired speedup. Per-run scores were `-10.255617402%` and `-11.975149038%`; paired-median diagnostics were `-12.320911%` and `-3.634232%`. Pooled Parent/Candidate device medians were `9.81/15.61 us`, CV `0.676876/0.562901`; raw ranges were `3.54-41.44/3.80-34.46 us`. The paired score is negative in both runs, jitter is high, and pooled candidate median latency is slower; all samples retained; `LOCAL_REJECTED_NOISY`.
- LOCAL_QUALITY: Pooled median-derived throughput `0.104383/0.065599 G elements/s` (Parent/Candidate, diagnostic); pooled wall medians `72.9045/71.7545 us`, wall CV `0.255155/0.208220`, maximum wall latency `181.760/169.659 us`. Device 3 AICore remained 0% before/after both runs; HBM snapshots were `3426/3428 MB` and `3426/3428 MB`. Other devices had active AICore users, including device 4 at 57-64%; host load averages after runs were `56.97/57.01/56.75` and `46.68/54.50/55.93`. This single-shape Local score is not comparable to Official 45.16.
- CURRENT_LOCAL_BEST: `R31B-V011` (unchanged).
- ONLINE: NOT_SUBMITTED.

Evidence: `compile-result.json`, `correctness-result.json`, `local-result.json`, `source-meta.json`, `submission.sha256`, `diff.patch`, and raw logs under `logs/`.

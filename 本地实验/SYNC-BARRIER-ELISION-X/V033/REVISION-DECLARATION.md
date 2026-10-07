# SYNC-BARRIER-ELISION-X V033

- ROUTE: SYNC-BARRIER-ELISION-X
- REVISION: V033
- DIRECT_PARENT: R31B-V011
- PARENT_SOURCE_SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- SINGLE_HYPOTHESIS: In `ProcessNarrowMidOverlap`, remove only the per-row `SyncMTE3ToV()` after the optional `inputRelease` WaitFlag and before the next row's input-buffer setup and loads.
- SINGLE_CHANGE_BOUNDARY: Delete one per-row MTE3-to-V event handoff from exact R31B-V011. No V001-V032 candidate edits are inherited.
- DUPLICATION_AUDIT: V001-V032 actual route diff patches contain no deletion of this per-row `SyncMTE3ToV()` at the loop boundary. V032 deletes the distinct final post-loop `SyncMTE3ToV()`; the call tested here runs only before a subsequent row iteration.
- COMPILE_TARGET: `sync_barrier_elision_v033`
- CORRECTNESS_CASES: FP16 rows=8, widths=128, 256, 1024, 2048, 4096; exact Parent/Candidate bitwise comparison.
- LOCAL_CASE: FP16 8x2048, device 3, 60 warmups and two 31-pair interleaved runs using device events and wall-clock diagnostics; retain every sample without outlier filtering if Correctness passes.
- CURRENT_LOCAL_BEST: R31B-V011
- ONLINE: NO

- CANDIDATE_SOURCE_SHA256: `77879e35234b1fb4df06a1bb0fa814253bef4d9432156f039623f32a4aaaf022`
- COMPILE: PASS; target `sync_barrier_elision_v033`; configure RC=0, build RC=0; CANN `8.5.0.alpha002`, host `hwnput3`, Ascend910B3, device 3.
- CORRECTNESS: PASS; FP16 rows=8 widths 128, 256, 1024, 2048, and 4096 all had zero bit mismatches; runner RC=0 at `2026-10-07T19:38:18Z`.
- LOCAL: 62 raw interleaved Parent/Candidate device-event pairs. Pooled `LOCAL_SCORE=+13.084428858%` geomean paired speedup. Per-run scores were `+31.371219428%` and `-2.656852043%`; runner median-delta diagnostics were `+13.750000%` and `+10.989011%`. Pooled Parent/Candidate device medians were `7.44/7.09 us`, CV `0.698992/0.688679`; raw event ranges were `4.76-35.94/3.88-33.50 us`. Run directions oppose and jitter is high; all samples retained; `LOCAL_REJECTED_NOISY`.
- LOCAL_QUALITY: Pooled median-derived throughput `2.202151/2.310860 G elements/s` (Parent/Candidate, diagnostic). Pooled wall medians were `65.134/64.3735 us`, wall CV `0.307291/0.159724`; maximum wall latency `195.837/132.428 us`. Device 3 AICore remained 0% before/after both runs; HBM snapshots were 3426/3428 MB in both. This single-shape Local score is not comparable to Official 45.16.
- CURRENT_LOCAL_BEST: `R31B-V011` (unchanged).
- ONLINE: NOT_SUBMITTED.

Evidence: `compile-result.json`, `correctness-result.json`, `local-result.json`, `source-meta.json`, `submission.sha256`, `diff.patch`, and all raw logs under `logs/`.

# SYNC-BARRIER-ELISION-X V036

- ROUTE: SYNC-BARRIER-ELISION-X
- REVISION: V036
- DIRECT_PARENT: R31B-V011
- PARENT_SOURCE_SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- SINGLE_HYPOTHESIS: In generic `Process()` row-level reduction, remove only the `SyncVToS()` after `ReduceSum(scalarLocal, ...)` and before `scalarLocal.GetValue(0)` reads the row sum.
- SINGLE_CHANGE_BOUNDARY: Delete one V-to-S event handoff from exact R31B-V011. No V001-V035 candidate edits are inherited.
- DUPLICATION_AUDIT: V001-V035 actual route diff patches contain no deletion of this generic `Process()` row-level V-to-S handoff. V029 deletes a distinct `ProcessNarrowMidOverlap` handoff after its per-row reduction. Width 128 dispatches to generic `Process()` and executes the selected row-level reduction point.
- COMPILE_TARGET: `sync_barrier_elision_v036`
- CORRECTNESS_CASES: FP16 rows=8, widths=128, 256, 1024, 2048, 4096; exact Parent/Candidate bitwise comparison.
- LOCAL_CASE: FP16 8x128, device 3, 60 warmups and two 31-pair interleaved runs using device events and wall-clock diagnostics; retain every sample without outlier filtering if Correctness passes.
- CURRENT_LOCAL_BEST: R31B-V011
- ONLINE: NO

- CANDIDATE_SOURCE_SHA256: `eaa5faa4bef2ae44c4db5444421a060e210f9c90e671f72172817120c181e954`
- COMPILE: PASS; target `sync_barrier_elision_v036`; configure RC=0, build RC=0; CANN `8.5.0.alpha002`, host `hwnput3`, Ascend910B3, device 3.
- CORRECTNESS: PASS; FP16 rows=8 widths 128, 256, 1024, 2048, and 4096 all had zero bit mismatches; runner RC=0 at `2026-10-07T20:27:19Z`.
- LOCAL: 62 raw interleaved Parent/Candidate device-event pairs on FP16 8x128. Pooled `LOCAL_SCORE=+10.492092988%` geomean paired speedup. Per-run scores were `-0.325885181%` and `+22.484183932%`; runner median-delta diagnostics were `+4.783258595%` and `+6.257378985%`. Pooled Parent/Candidate device medians were `15.92/14.69 us`, CV `0.456127/0.512661`; raw ranges were `3.66-28.94/3.64-26.72 us`. Run scores oppose materially and jitter is high; all samples retained; `LOCAL_REJECTED_NOISY`.
- LOCAL_QUALITY: Pooled median-derived throughput `0.064322/0.069707 G elements/s` (Parent/Candidate, diagnostic). Pooled wall medians were `65.5255/65.6405 us`, wall CV `0.297920/0.112132`; maximum wall latency `179.001/100.331 us`. Device 3 AICore remained 0% before/after both runs; HBM snapshots were 3426/3428 MB and 3426/3427 MB. This single-shape Local score is not comparable to Official 45.16.
- CURRENT_LOCAL_BEST: `R31B-V011` (unchanged).
- ONLINE: NOT_SUBMITTED.

Evidence: `compile-result.json`, `correctness-result.json`, `local-result.json`, `source-meta.json`, `submission.sha256`, `diff.patch`, and all raw logs under `logs/`.

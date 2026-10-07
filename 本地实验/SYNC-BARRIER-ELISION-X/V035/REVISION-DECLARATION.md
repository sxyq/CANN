# SYNC-BARRIER-ELISION-X V035

- ROUTE: SYNC-BARRIER-ELISION-X
- REVISION: V035
- DIRECT_PARENT: R31B-V011
- PARENT_SOURCE_SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- SINGLE_HYPOTHESIS: In generic `Process()` first-pass tile processing, remove only the `SyncVToMTE2()` after storing the tile reduction and before advancing to the next tile.
- SINGLE_CHANGE_BOUNDARY: Delete one V-to-MTE2 event handoff from exact R31B-V011. No V001-V034 candidate edits are inherited.
- DUPLICATION_AUDIT: V001-V034 actual route diff patches contain no deletion of this generic first-pass `SyncVToMTE2()` call. V034 deletes a different MTE2-to-V call before vector consumption. Width 128 dispatches to generic `Process()` because narrow/mid requires `rowWidth > 128`; its one-tile first pass executes the selected post-ReduceSum handoff.
- COMPILE_TARGET: `sync_barrier_elision_v035`
- CORRECTNESS_CASES: FP16 rows=8, widths=128, 256, 1024, 2048, 4096; exact Parent/Candidate bitwise comparison.
- LOCAL_CASE: FP16 8x128, device 3, 60 warmups and two 31-pair interleaved runs using device events and wall-clock diagnostics; retain every sample without outlier filtering if Correctness passes.
- CURRENT_LOCAL_BEST: R31B-V011
- ONLINE: NO

- CANDIDATE_SOURCE_SHA256: `6b099237590ebf5081dce063bed8ca7215e8c3b911198293c063558744da9d8f`
- COMPILE: PASS; target `sync_barrier_elision_v035`; configure RC=0, build RC=0; CANN `8.5.0.alpha002`, host `hwnput3`, Ascend910B3, device 3.
- CORRECTNESS: PASS; FP16 rows=8 widths 128, 256, 1024, 2048, and 4096 all had zero bit mismatches; runner RC=0 at `2026-10-07T20:15:04Z`.
- LOCAL: 62 raw interleaved Parent/Candidate device-event pairs on FP16 8x128. Pooled `LOCAL_SCORE=+10.425807900%` geomean paired speedup. Per-run scores were `+11.086354677%` and `+9.769188898%`; runner median-delta diagnostics were `+1.598837209%` and `+2.476780186%`. Pooled Parent/Candidate device medians were `13.48/12.77 us`, CV `0.502455/0.604063`; raw ranges were `3.88-25.48/3.56-38.66 us`. Both run geomeans are positive, but variability/outliers remain high; retain every sample and mark `LOCAL_REJECTED_NOISY`.
- LOCAL_QUALITY: Pooled median-derived throughput `0.075964/0.080188 G elements/s` (Parent/Candidate, diagnostic). Pooled wall medians were `63.581/63.991 us`, wall CV `0.311607/0.111428`; maximum wall latency `189.262/101.841 us`. Device 3 AICore was 0% before/after both runs; HBM snapshots were 3426/3428 MB for both. This single-shape Local score is not comparable to Official 45.16.
- CURRENT_LOCAL_BEST: `R31B-V011` (unchanged).
- ONLINE: NOT_SUBMITTED.

Evidence: `compile-result.json`, `correctness-result.json`, `local-result.json`, `source-meta.json`, `submission.sha256`, `diff.patch`, and all raw logs under `logs/`.

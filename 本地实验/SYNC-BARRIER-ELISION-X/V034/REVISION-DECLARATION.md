# SYNC-BARRIER-ELISION-X V034

- ROUTE: SYNC-BARRIER-ELISION-X
- REVISION: V034
- DIRECT_PARENT: R31B-V011
- PARENT_SOURCE_SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- SINGLE_HYPOTHESIS: In generic `Process()` first-pass tile processing, remove only the `SyncMTE2ToV()` immediately after x/residual loads and before FP16 widening/add consumes those tiles.
- SINGLE_CHANGE_BOUNDARY: Delete one MTE2-to-V event handoff from exact R31B-V011. No V001-V033 candidate edits are inherited.
- DUPLICATION_AUDIT: V001-V033 actual route diff patches contain no deletion of this first-pass generic `Process()` `SyncMTE2ToV()` call. Other route revisions target distinct narrow/mid calls or different generic-path barriers. Width 128 dispatches to generic `Process()` because the narrow/mid condition is strictly `rowWidth > 128`; the call executes after its x/residual tile loads.
- COMPILE_TARGET: `sync_barrier_elision_v034`
- CORRECTNESS_CASES: FP16 rows=8, widths=128, 256, 1024, 2048, 4096; exact Parent/Candidate bitwise comparison.
- LOCAL_CASE: FP16 8x128, device 3, 60 warmups and two 31-pair interleaved runs using device events and wall-clock diagnostics; retain every sample without outlier filtering if Correctness passes.
- CURRENT_LOCAL_BEST: R31B-V011
- ONLINE: NO

- CANDIDATE_SOURCE_SHA256: `a2a190e38cf9a5fa7de578780cdcec8c5a13f2dc4c863a2078fee22751ddbf71`
- COMPILE: PASS; target `sync_barrier_elision_v034`; configure RC=0, build RC=0; CANN `8.5.0.alpha002`, host `hwnput3`, Ascend910B3, device 3.
- CORRECTNESS: FAIL; first case FP16 rows=8 width=128 had 1023 bit mismatches; runner RC=3 at `2026-10-07T20:07:42Z`. Remaining widths were not run.
- LOCAL: NOT_RUN; `LOCAL_SCORE=NONE` because Correctness failed.
- CURRENT_LOCAL_BEST: `R31B-V011` (unchanged); V034 is not promoted.
- ONLINE: NOT_SUBMITTED.

Evidence: `compile-result.json`, `correctness-result.json`, `local-result.json`, `source-meta.json`, `submission.sha256`, `diff.patch`, and raw Compile/Correctness logs under `logs/`.

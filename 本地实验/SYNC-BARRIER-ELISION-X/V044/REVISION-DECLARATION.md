# SYNC-BARRIER-ELISION-X V044

- ROUTE: SYNC-BARRIER-ELISION-X
- REVISION: V044
- DIRECT_PARENT: R31B-V011
- PARENT_SOURCE_SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- SINGLE_HYPOTHESIS: In generic `Process()` second-pass FP16 processing, remove only the guarded `SyncMTE2ToV()` after x/residual and non-cached gamma/bias loads.
- SINGLE_CHANGE_BOUNDARY: Delete one MTE2-to-V event handoff from exact R31B-V011. No prior route candidate edits are inherited.
- DUPLICATION_AUDIT: V001-V043 route-local diffs contain no deletion at this generic second-pass call. V034 removes the distinct first-pass MTE2-to-V handoff after x/residual tile loads. The 8x128 case dispatches to generic `Process()` with `cacheRow=true` and `cacheParams=false`, so the selected guard is active.
- COMPILE_TARGET: `sync_barrier_elision_v044`
- CORRECTNESS_CASES: FP16 rows=8, widths=128, 256, 1024, 2048, 4096; exact Parent/Candidate bitwise comparison.
- LOCAL_CASE: Planned FP16 8x128, device 3, 60 warmups and two 31-pair interleaved runs; NOT_RUN because Correctness failed.
- CURRENT_LOCAL_BEST: R31B-V011
- ONLINE: NO

- CANDIDATE_SOURCE_SHA256: `487a44dae3474395f7196ef2ace4c5bd6a11816879cc16421649b0dfaadcdca7`
- COMPILE: PASS; configure and target build RC=0 on `hwnput3`, CANN `8.5.0.alpha002`, Ascend910B3 / `dav-2201`.
- CORRECTNESS: FAIL; FP16 8x128 had 7 bit mismatches; runner RC=3 at `2026-10-07T22:08:43Z`. The suite stopped at the first failing case.
- LOCAL: NOT_RUN; `LOCAL_SCORE=NONE` because Correctness did not pass.
- CURRENT_LOCAL_BEST: `R31B-V011` (unchanged); V044 is not promoted.
- ONLINE: NOT_SUBMITTED.

Evidence: `compile-result.json`, `correctness-result.json`, `local-result.json`, `source-meta.json`, `submission.sha256`, `diff.patch`, and raw compile/correctness logs under `logs/`.

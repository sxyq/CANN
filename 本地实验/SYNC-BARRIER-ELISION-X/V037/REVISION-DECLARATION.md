# SYNC-BARRIER-ELISION-X V037

- ROUTE: SYNC-BARRIER-ELISION-X
- REVISION: V037
- DIRECT_PARENT: R31B-V011
- PARENT_SOURCE_SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- SINGLE_HYPOTHESIS: In generic `Process()`, remove only the `SyncSToV()` after reading `invRms` from Scalar and before the output pass.
- SINGLE_CHANGE_BOUNDARY: Delete one S-to-V event handoff from exact R31B-V011. No V001-V036 candidate edits are inherited.
- DUPLICATION_AUDIT: V001-V036 actual route diff patches contain no deletion of this generic `Process()` S-to-V handoff. V028/V030 delete distinct `ProcessNarrowMidOverlap` calls. Width 128 dispatches to generic `Process()` and executes this post-scalar handoff.
- COMPILE_TARGET: `sync_barrier_elision_v037`
- CORRECTNESS_CASES: FP16 rows=8, widths=128, 256, 1024, 2048, 4096; exact Parent/Candidate bitwise comparison.
- LOCAL_CASE: FP16 8x128, device 3, 60 warmups and two 31-pair interleaved runs using device events and wall-clock diagnostics; retain every sample without outlier filtering if Correctness passes.
- CURRENT_LOCAL_BEST: R31B-V011
- ONLINE: NO

- CANDIDATE_SOURCE_SHA256: `2a10ec9423536022be27b4173c1d87bd62149e196893957b039f33b9f099f7bc`
- COMPILE: PASS; target `sync_barrier_elision_v037`; configure RC=0, build RC=0; CANN `8.5.0.alpha002`, host `hwnput3`, Ascend910B3, device 3.
- CORRECTNESS: PASS; FP16 rows=8 widths 128, 256, 1024, 2048, and 4096 all had zero bit mismatches; runner RC=0 at `2026-10-07T20:41:36Z`.
- LOCAL: 62 raw interleaved Parent/Candidate device-event pairs on FP16 8x128. Pooled `LOCAL_SCORE=+2.675036872%` geomean paired speedup. Per-run scores were `-5.166040647%` and `+11.164431694%`; paired median deltas were `-0.22 us` and `-0.06 us`. Pooled Parent/Candidate device medians were `6.81/6.89 us`, CV `0.671010/0.651376`; raw ranges were `3.80-27.76/4.08-27.66 us`. Run scores reverse and jitter is high; all samples retained; `LOCAL_REJECTED_NOISY`.
- LOCAL_QUALITY: Pooled median-derived throughput `0.150367/0.148621 G elements/s` (Parent/Candidate, diagnostic); pooled wall medians `69.995/69.690 us`, wall CV `0.259284/0.083838`, maximum wall latency `183.181/89.861 us`. Device 3 AICore remained 0% before/after both runs; HBM snapshots were `3425/3427 MB` and `3425/3428 MB`. Other devices had active AICore users, including device 4 at 63%; this single-shape Local score is not comparable to Official 45.16.
- CURRENT_LOCAL_BEST: `R31B-V011` (unchanged).
- ONLINE: NOT_SUBMITTED.

Evidence: `compile-result.json`, `correctness-result.json`, `local-result.json`, `source-meta.json`, `submission.sha256`, `diff.patch`, and all raw logs under `logs/`.

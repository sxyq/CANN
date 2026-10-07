# SYNC-BARRIER-ELISION-X V032

- ROUTE: SYNC-BARRIER-ELISION-X
- REVISION: V032
- DIRECT_PARENT: R31B-V011
- PARENT_SOURCE_SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- SINGLE_HYPOTHESIS: In `ProcessNarrowMidOverlap`, remove only the final `SyncMTE3ToV()` after the loop's optional input-release `WaitFlag` and before releasing event IDs.
- SINGLE_CHANGE_BOUNDARY: Delete one MTE3-to-V event handoff from exact R31B-V011. No V001-V031 candidate edits are inherited.
- DUPLICATION_AUDIT: V001-V031 actual route diff patches contain no `SyncMTE3ToV()` deletion. Earlier route tests delete different PIPE_V barriers, V-to-S/S-to-V handoffs, V-to-MTE3 store handoffs, SetFlag or WaitFlag operations; this final post-loop drain handoff is a distinct point.
- COMPILE_TARGET: `sync_barrier_elision_v032`
- CORRECTNESS_CASES: FP16 rows=8, widths=128, 256, 1024, 2048, 4096; exact Parent/Candidate bitwise comparison.
- LOCAL_CASE: FP16 8x2048, device 3, 60 warmups and two 31-pair interleaved runs using device events and wall-clock diagnostics; retain every sample without outlier filtering if Correctness passes.
- CURRENT_LOCAL_BEST: R31B-V011
- ONLINE: NO

- CANDIDATE_SOURCE_SHA256: `42e4df2659dcff4e6e9f14a2bbf0616f9368286ab6439f5e7e9b324d0fac9a9a`
- COMPILE: PASS; target `sync_barrier_elision_v032`; configure RC=0, build RC=0; CANN `8.5.0.alpha002`, host `hwnput3`, Ascend910B3, device 3.
- CORRECTNESS: PASS; FP16 rows=8 widths 128, 256, 1024, 2048, and 4096 all had zero bit mismatches; runner RC=0 at `2026-10-07T19:09:47Z`.
- LOCAL: 62 raw interleaved Parent/Candidate device-event pairs. Pooled `LOCAL_SCORE=+2.472808271%` geomean paired speedup. Per-run scores were `+0.051147499%` and `+4.953083471%`; runner median-delta diagnostics were `-5.128205128%` and `-0.169779287%`. Pooled Parent/Candidate device medians were `12.77/11.64 us`, CV `0.471906/0.494505`; raw event ranges were `3.70-24.16/3.78-24.88 us`. All samples retained; jitter and inconsistent run diagnostics make this `LOCAL_REJECTED_NOISY`.
- LOCAL_QUALITY: Pooled median-derived throughput `1.283007/1.407560 G elements/s` (Parent/Candidate, diagnostic). Pooled wall medians were `70.466/71.6365 us`, wall CV `0.262776/0.127135`; maximum wall latency `173.854/100.141 us`. Device 3 AICore remained 0% before/after both runs; HBM snapshots were 3425/3428 MB and 3426/3428 MB. This single-shape Local score is not comparable to Official 45.16.
- CURRENT_LOCAL_BEST: `R31B-V011` (unchanged).
- ONLINE: NOT_SUBMITTED.

Evidence: `compile-result.json`, `correctness-result.json`, `local-result.json`, `source-meta.json`, `submission.sha256`, `diff.patch`, and all raw logs under `logs/`.

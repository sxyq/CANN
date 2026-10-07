# SYNC-BARRIER-ELISION-X V031

- ROUTE: SYNC-BARRIER-ELISION-X
- REVISION: V031
- DIRECT_PARENT: R31B-V011
- PARENT_SOURCE_SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- SINGLE_HYPOTHESIS: In `ProcessNarrowMidOverlap`, remove only the second `SyncVToS()` event handoff, immediately after scalar `Sqrt` and before `xFp32.GetValue(0)` reads `invRms`.
- SINGLE_CHANGE_BOUNDARY: Delete one V-to-S event handoff from exact R31B-V011. No V001-V030 candidate edits are inherited.
- DUPLICATION_AUDIT: V001-V030 actual route diff patches contain one `SyncVToS()` deletion, V029's first handoff after `ReduceSum`; no patch deletes the second handoff after scalar `Sqrt`. V010/V018 remove distinct `PIPE_V` barriers near scalar transfer.
- COMPILE_TARGET: `sync_barrier_elision_v031`
- CORRECTNESS_CASES: FP16 rows=8, widths=128, 256, 1024, 2048, 4096; exact Parent/Candidate bitwise comparison.
- LOCAL_CASE: FP16 8x2048, device 3, 60 warmups and two 31-pair interleaved runs using device events and wall-clock diagnostics; retain every sample without outlier filtering if Correctness passes.
- CURRENT_LOCAL_BEST: R31B-V011
- ONLINE: NO

- CANDIDATE_SOURCE_SHA256: `a7e5114500be64690f1ba77066784f98613a6da0920ed88bbc2005b2c83b4b01`
- COMPILE: PASS; target `sync_barrier_elision_v031`; Configure RC=0, build RC=0; CANN `8.5.0.alpha002`, `hwnput3`, Ascend910B3, device 3.
- CORRECTNESS: PASS; FP16 rows=8 widths 128, 256, 1024, 2048, and 4096 all had zero bit mismatches; runner RC=0 at `2026-10-07T18:51:01Z`.
- LOCAL: 62 raw interleaved Parent/Candidate device-event pairs across two 31-pair runs. Pooled `LOCAL_SCORE=+29.143349859%` geomean paired device-event speedup. Per-run scores were `+12.960135907%` and `+47.645049105%`; runner median-delta diagnostics were `+0.474496%` and `+10.461192%`. Pooled Parent/Candidate device medians were `16.98/8.41 us`, CV `0.503577/0.636494`; run 2 CV was `0.567652/0.752831` and max device latency `41.82/33.90 us`. Both runs are positive but high jitter makes the result non-repeatable enough for promotion; all 62 samples retained; `LOCAL_REJECTED_NOISY`.
- LOCAL_QUALITY: Pooled median-derived throughput `0.964900/1.948157 G elements/s` (Parent/Candidate, diagnostic). Pooled wall medians were `75.1855/76.065 us`, wall CV `0.222203/0.086061`; maximum wall latency `177.180/101.380 us`. Device 3 AICore remained 0% before/after both runs; HBM snapshots were 3427/3428 MB and 3426/3428 MB. This single-shape Local score is not comparable to Official 45.16.
- CURRENT_LOCAL_BEST: `R31B-V011` (unchanged).
- ONLINE: NOT_SUBMITTED.

Evidence: `compile-result.json`, `correctness-result.json`, `local-result.json`, `source-meta.json`, `submission.sha256`, `diff.patch`, and all raw logs under `logs/`.

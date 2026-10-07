# SYNC-BARRIER-ELISION-X V029

- ROUTE: SYNC-BARRIER-ELISION-X
- REVISION: V029
- DIRECT_PARENT: R31B-V011
- PARENT_SOURCE_SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- SINGLE_HYPOTHESIS: In `ProcessNarrowMidOverlap`, remove only the first `SyncVToS()` event handoff after `ReduceSum` and before `reduceLocal.GetValue(0)` reads the reduction result.
- SINGLE_CHANGE_BOUNDARY: Delete one V-to-S event handoff from exact R31B-V011. No V001-V028 candidate edits are inherited.
- DUPLICATION_AUDIT: V001-V028 actual route diff patches contain no `SyncVToS()` deletion. V010/V018 target a distinct `PIPE_V` barrier before the later scalar `SyncVToS()` after `Sqrt`; V028 deletes the first S-to-V event handoff after `meanSquare`, not this V-to-S handoff.
- COMPILE_TARGET: `sync_barrier_elision_v029`
- CORRECTNESS_CASES: FP16 rows=8, widths=128, 256, 1024, 2048, 4096; exact Parent/Candidate bitwise comparison.
- LOCAL_CASE: FP16 8x2048, device 3, 60 warmups and two 31-pair interleaved runs using device events and wall-clock diagnostics; retain every sample without outlier filtering if Correctness passes.
- CURRENT_LOCAL_BEST: R31B-V011
- ONLINE: NO

- CANDIDATE_SOURCE_SHA256: `548d6cdf50f701588b759547916c4d8a6f08b519e1d7f824aa1b08469b89a39c`
- COMPILE: PASS; target `sync_barrier_elision_v029`; Configure RC=0, build RC=0; CANN `8.5.0.alpha002`, `hwnput3`, Ascend910B3, device 3.
- CORRECTNESS: PASS; FP16 rows=8 widths 128, 256, 1024, 2048, and 4096 all had zero bit mismatches; runner RC=0 at `2026-10-07T17:53:49Z`.
- LOCAL: 62 raw interleaved Parent/Candidate device-event pairs across two 31-pair runs. Pooled `LOCAL_SCORE=-0.723375597%` geomean paired device-event speedup. Per-run scores were `-1.888513458%` and `+0.455599037%`; runner median-delta diagnostics were `-1.705238%` and `-3.448276%`. Pooled Parent/Candidate device medians were both `17.53 us`, CV `0.388754/0.401767`; latency extrema were `4.08–31.88/3.66–35.14 us`. Device-event variability is high and run-level results disagree; run 2 preflight also observed unrelated device-3 process PID 202260 (`w4r01_v001_pair`), although device-3 AICore read 0% and the process was absent after the run. All 62 samples retained; `LOCAL_REJECTED_NOISY`, no promotion.
- LOCAL_QUALITY: Pooled median-derived throughput `0.934626/0.934626 G elements/s` (Parent/Candidate, diagnostic). Pooled device-event wall medians `72.985/72.831 us`, wall CV `0.236706/0.094721`; max wall latency `168.191/97.491 us`. Device 3 AICore was 0% before and after both runs; HBM snapshots were 3426/3428 MB (run 1) and 3485/3427 MB (run 2). This single-shape Local score is not comparable to Official 45.16.
- CURRENT_LOCAL_BEST: `R31B-V011` (unchanged).
- ONLINE: NOT_SUBMITTED.

Evidence: `compile-result.json`, `correctness-result.json`, `local-result.json`, `source-meta.json`, `submission.sha256`, `diff.patch`, and all raw logs under `logs/`.

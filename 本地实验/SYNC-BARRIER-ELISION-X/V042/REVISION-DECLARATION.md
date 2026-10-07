# SYNC-BARRIER-ELISION-X V042

- ROUTE: SYNC-BARRIER-ELISION-X
- REVISION: V042
- DIRECT_PARENT: R31B-V011
- PARENT_SOURCE_SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- SINGLE_HYPOTHESIS: In generic `Process()` FP16 output, remove only the guarded `SyncVToMTE2()` immediately before `SyncVToMTE3()` and `Store()`.
- SINGLE_CHANGE_BOUNDARY: Delete that one V-to-MTE2 handoff from exact R31B-V011. No prior route candidate change is inherited.
- DUPLICATION_AUDIT: V001-V041 route-local diffs contain no deletion at this generic FP16 output site. V035 deletes a distinct first-pass post-`ReduceSum` `SyncVToMTE2()`. The 8x128 Local path dispatches to generic `Process()` with `localRows=1`, `cacheRow=true`, `cacheParams=false`, so the selected guard is active.
- COMPILE_TARGET: `sync_barrier_elision_v042`
- CORRECTNESS_CASES: FP16 rows=8, widths=128, 256, 1024, 2048, 4096; exact Parent/Candidate bitwise comparison.
- LOCAL_CASE: FP16 8x128, device 3, 60 warmups and two 31-pair interleaved runs using device events and wall-clock diagnostics; all samples retained without outlier filtering.
- CURRENT_LOCAL_BEST: R31B-V011
- ONLINE: NO

- CANDIDATE_SOURCE_SHA256: `25e14b74a1929811de74ed1c9240912857644a7159b516ce951deb293dcc029a`
- COMPILE: PASS; configure and target build RC=0 on `hwnput3`, CANN `8.5.0.alpha002`, Ascend910B3 / `dav-2201`.
- CORRECTNESS: PASS; all five widths had zero bit mismatches; runner RC=0 at `2026-10-07T21:46:27Z`.
- LOCAL: 62 raw Parent/Candidate device-event pairs. Pooled `LOCAL_SCORE=-2.472447252%`; run geomean scores were `-17.221932077%` and `+14.905116581%`. The score directions reverse, pooled device-event CV is `1.145177/0.654862` (Parent/Candidate), and Parent wall latency reached `6124.640 us`; retain all samples and mark `LOCAL_REJECTED_NOISY`.
- LOCAL_QUALITY: Pooled Parent/Candidate device medians `7.18/8.17 us`; wall medians `75.015/77.611 us`. Median-derived throughput is `0.142618384/0.125336597 G elements/s` (diagnostic). Device 3 AICore was 0% before/after both runs; HBM was `3426/3428 MB`. Host load averages rose from `65.48/51.90/51.78` to `73.35/54.13/52.51` during run 1, then to `81.90/56.49/53.29` during run 2; other devices had active AICore users. This single-shape Local score is not comparable to Official 45.16.
- CURRENT_LOCAL_BEST: `R31B-V011` (unchanged).
- ONLINE: NOT_SUBMITTED.

Evidence: `compile-result.json`, `correctness-result.json`, `local-result.json`, `source-meta.json`, `submission.sha256`, `diff.patch`, and raw logs under `logs/`.

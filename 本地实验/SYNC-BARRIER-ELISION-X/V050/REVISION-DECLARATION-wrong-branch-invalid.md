# SYNC-BARRIER-ELISION-X V050

- ROUTE: SYNC-BARRIER-ELISION-X
- REVISION: V050
- DIRECT_PARENT: R31B-V011
- PARENT_SOURCE_SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- SINGLE_HYPOTHESIS: In the FP16 branch of `ProcessSmallLowPrecisionContiguousBatched`, remove only the `SyncMTE3ToV()` immediately after `Store(outputGm_, batchOffset, outputLocal, totalElems)`.
- SINGLE_CHANGE_BOUNDARY: Delete one post-store MTE3-to-V event handoff from exact R31B-V011. No V049 or other Candidate change is inherited.
- DUPLICATION_AUDIT: V001-V049 actual Route diffs contain no deletion at this exact operation point. V039 removes a post-store drain in generic `Process()`; V032/V033 target distinct `ProcessNarrowMidOverlap` drains. V046-V049 remove different synchronization operations earlier in this batched function.
- COMPILE_TARGET: `sync_barrier_elision_v050`
- CORRECTNESS_CASES: FP16 Parent/Candidate bitwise comparison across the seven cases configured in the runner.
- LOCAL_CASE: FP16 128x128; device-event timing, same-binary qualification, and two 31-pair interleaved Parent/Candidate blocks; retain every raw sample and load/jitter context. Await a new exclusive Local device assignment after Correctness.
- CURRENT_LOCAL_BEST: R31B-V011
- ONLINE: NO
- CANDIDATE_SOURCE_SHA256: `d48ef8b0cfe9633e06fca4abb36839add0ff00b4b7b5a489df6da53a0babb590`
- COMPILE: PASS; corrected exact-parent sibling built as `sync_barrier_elision_v050` with CANN `8.5.0.alpha002` on `hwnput3` / Ascend910B3 / `dav-2201`.
- CORRECTNESS: PASS_BITWISE; seven FP16 cases, zero Parent/Candidate mismatches on device 3.
- LOCAL_CASE: FP16 `128x128`, device 3; 60 warmups per invocation, two 31-pair Parent and Candidate same-binary qualification runs, then two 31-pair interleaved Parent/Candidate device-event blocks. Every raw event, throughput sample, wall diagnostic and load snapshot is retained.
- LOCAL: `LOCAL_REJECTED_NOISY`; pooled raw-derived 62-pair median-paired score `+0.694444%`, median paired Candidate-minus-Parent delta `-0.150000 us`. Block scores reverse direction (`-0.274162%`, `+1.559021%`); qualification and interleaved event timing show substantial variability. No promotion.
- CURRENT_LOCAL_BEST: `R31B-V011` (unchanged). Numeric single-shape Local is not comparable to Official `45.16`.
- OFFICIAL / ONLINE: no Official score; `NOT_SUBMITTED`.
- DEVICE ASSIGNMENT: Main allocated device 3 exclusively to V050 from Correctness through Local result capture. Assignment was released after V050 result commit; no shared device TSV was modified by this Route.
- SCAFFOLD DEVIATION: The first V050 submission accidentally inherited the V049 barrier deletion. That intermediate source and its Compile/Correctness logs are preserved and excluded; the final Candidate is re-established from exact R31B-V011 and contains only the declared V050 deletion. See `SCAFFOLD-DEVIATION.md`.

Evidence: `compile-result.json`, `correctness-result.json`, `local-result.json`, `source-meta.json`, `submission.sha256`, `diff.patch`, `SCAFFOLD-DEVIATION.md`, and preserved raw logs under `logs/`.

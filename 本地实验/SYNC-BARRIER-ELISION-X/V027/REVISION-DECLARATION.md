# SYNC-BARRIER-ELISION-X V027

- ROUTE: SYNC-BARRIER-ELISION-X
- REVISION: V027
- DIRECT_PARENT: R31B-V011
- PARENT_SOURCE_SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- SINGLE_HYPOTHESIS: In the FP16 branch of `ProcessNarrowMidOverlap`, remove only the `SyncVToMTE3()` event handoff immediately before `Store`.
- SINGLE_CHANGE_BOUNDARY: One event-synchronization call deletion from the exact R31B-V011 parent. No V001-V026 candidate changes are inherited.
- DUPLICATION_AUDIT: V001-V026 actual source diffs contain no deletion of this `SyncVToMTE3()` call. V001/V002 delete adjacent `PIPE_V` barriers, and V017 removes a barrier before a different `SyncVToMTE3()` in the generic tiled path; neither deletes this event handoff in the FP16 narrow/mid output branch.
- COMPILE_TARGET: `sync_barrier_elision_v027`
- CORRECTNESS_CASES: FP16 rows=8, widths=128, 256, 1024, 2048, 4096; exact Parent/Candidate bitwise comparison.
- LOCAL_CASE: FP16 8x2048, device 3, 60 warmups and two 31-pair interleaved runs using device events and wall-clock diagnostics; retain every sample and no outlier filtering if Correctness passes.
- CURRENT_LOCAL_BEST: R31B-V011
- ONLINE: NO

- CANDIDATE_SOURCE_SHA256: `25e7144625d6f381ac74f17e9d075a4506deffc2f8ae770cf5edd24d7251bca3`
- COMPILE: PASS; target `sync_barrier_elision_v027`; Configure RC=0, build RC=0; `hwnput3`, Ascend910B3, device 3; 2026-10-07T15:56:04Z to 15:56:20Z.
- CORRECTNESS: `CORRECTNESS_RUN_FAILED_CANDIDATE_VECTOR_CORE_TIMEOUT`. FP16 rows=8 width=128 completed with zero bit mismatches. The next case (width=256 by runner order) failed during Candidate stream synchronization with ACL status `507034` / vector-core timeout (`retCode=0x30`); no Parent/Candidate comparison is available for width=256 or later cases. Runner PID 3345235 exited naturally with RC=2 at 2026-10-07T16:15:47Z. The original log contains no per-run plog path; no SSH or device retry was used.
- LOCAL: NOT_RUN because Correctness did not complete; no Local score or samples.
- CURRENT_LOCAL_BEST: `R31B-V011` (unchanged); V027 is not promoted.
- ONLINE: NOT_SUBMITTED. Single-shape Local is not comparable to Official 45.16.
- EVIDENCE: `compile-result.json`, `correctness-result.json`, `local-result.json`, `source-meta.json`, `diff.patch`, `submission.sha256`, and original logs under `logs/`.

# SYNC-BARRIER-ELISION-X V028

- ROUTE: SYNC-BARRIER-ELISION-X
- REVISION: V028
- DIRECT_PARENT: R31B-V011
- PARENT_SOURCE_SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- SINGLE_HYPOTHESIS: In `ProcessNarrowMidOverlap`, remove only the first `SyncSToV()` event handoff, after computing scalar `meanSquare` and before `Duplicate(xFp32, meanSquare, 1)` consumes it.
- SINGLE_CHANGE_BOUNDARY: Delete one event-handoff call from exact R31B-V011. No V001-V027 candidate edits are inherited.
- DUPLICATION_AUDIT: Route-local prior revision declarations and actual diff patches contain no `SyncSToV()` deletion. V010/V018 remove different `PIPE_V` barriers near the scalar `Sqrt`/`SyncVToS` handoff; V027 removes a distinct V-to-MTE3 store handoff. This is the first `SyncSToV()` call in the function; the later call after `invRms` extraction is unchanged.
- COMPILE_TARGET: `sync_barrier_elision_v028`
- CORRECTNESS_CASES: FP16 rows=8, widths=128, 256, 1024, 2048, 4096; exact Parent/Candidate bitwise comparison.
- LOCAL_CASE: FP16 8x2048, device 3, 60 warmups and two 31-pair interleaved runs using device events and wall-clock diagnostics; retain every sample without outlier filtering if Correctness passes.
- CURRENT_LOCAL_BEST: R31B-V011
- ONLINE: NO

- CANDIDATE_SOURCE_SHA256: `91936312119be6b3390a146048587902527f4e99b9e852462cbb2870f925eb73`
- COMPILE: PASS; target `sync_barrier_elision_v028`; Configure RC=0, build RC=0 with the CANN setup environment and C++ include path; `hwnput3`, Ascend910B3, device 3. Initial environment-only attempts are retained separately in `logs/`.
- CORRECTNESS: `CORRECTNESS_RUN_FAILED_PARENT_VECTOR_CORE_TIMEOUT`. FP16 rows=8 width=128 completed with zero bit mismatches. At width=256 the Parent stream synchronization failed with ACL status `507034`; no Candidate comparison exists for width=256 or later cases. Runner exited naturally with RC=2 at `2026-10-07T17:35:35Z`; device 3 post-run snapshot reported no running processes. No retry or plog retrieval was performed.
- LOCAL: NOT_RUN because Correctness did not complete; no Local score or samples.
- CURRENT_LOCAL_BEST: `R31B-V011` (unchanged); V028 is not promoted.
- ONLINE: NOT_SUBMITTED. Single-shape Local is not comparable to Official 45.16.
- EVIDENCE: `compile-result.json`, `correctness-result.json`, `local-result.json`, `source-meta.json`, `diff.patch`, `submission.sha256`, and all original logs under `logs/`.

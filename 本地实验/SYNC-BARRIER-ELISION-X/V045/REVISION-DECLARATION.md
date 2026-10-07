# SYNC-BARRIER-ELISION-X V045

- ROUTE: SYNC-BARRIER-ELISION-X
- REVISION: V045
- DIRECT_PARENT: R31B-V011
- PARENT_SOURCE_SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- SINGLE_HYPOTHESIS: In `ProcessNarrowMidOverlap`, remove only the `SyncMTE2ToV()` after the resident-parameter gamma/bias loads.
- SINGLE_CHANGE_BOUNDARY: Delete the one resident-parameter MTE2-to-V handoff from exact R31B-V011. No prior Route candidate edits are inherited.
- DUPLICATION_AUDIT: V001-V044 actual Route diff patches contain no deletion at this `ProcessNarrowMidOverlap` resident-parameter point. V034 deletes a distinct generic `Process()` first-pass handoff; V044 deletes a distinct generic `Process()` second-pass handoff guarded by `cacheRow/cacheParams`.
- COMPILE_TARGET: `sync_barrier_elision_v045`
- CORRECTNESS_CASES: FP16 (8,128), (8,256), (8,1024), (8,2048), (8,4096), and (16,2064); exact Parent/Candidate bitwise comparison.
- LOCAL_CASE: FP16 16x2064 on device 3; Parent self-qualification x2, Candidate self-qualification x2, then two 31-pair interleaved Parent/Candidate blocks; 60 warmups per invocation.
- CURRENT_LOCAL_BEST: R31B-V011
- ONLINE: NO

- CANDIDATE_SOURCE_SHA256: `501b0855cf362d400dd47fa3bf2f93e14c7996e1cf537ec950b560380d7c971e`
- COMPILE: PASS; configure and target build RC=0 on `hwnput3`, CANN `8.5.0.alpha002`, Ascend910B3 / `dav-2201`.
- CORRECTNESS: PASS; all six FP16 cases had zero Parent/Candidate bit mismatches. The first runner-build attempt failed before any device run because the versioned environment-script path was wrong; that log is preserved, and the retry used the installed toolkit entry point.
- LOCAL: `LOCAL_REJECTED_NOISY`; 62 raw Parent/Candidate pairs produce a pooled geomean score of -5.148496818%. Same-binary and interleaved timing show high jitter; no promotion.
- CURRENT_LOCAL_BEST: `R31B-V011` (unchanged).
- ONLINE: NOT_SUBMITTED.

Evidence: `compile-result.json`, `correctness-result.json`, `local-result.json`, `source-meta.json`, `submission.sha256`, `diff.patch`, and raw logs under this revision directory.

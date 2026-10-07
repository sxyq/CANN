# SYNC-BARRIER-ELISION-X V048

- ROUTE: SYNC-BARRIER-ELISION-X
- REVISION: V048
- DIRECT_PARENT: R31B-V011
- PARENT_SOURCE_SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- SINGLE_HYPOTHESIS: In `ProcessSmallLowPrecisionContiguousBatched`, remove only the `PIPE_V` barrier after squaring `valueLocal` into `xFp32` and before the per-row `ReduceSum` loop.
- SINGLE_CHANGE_BOUNDARY: Delete one vector-pipe barrier from exact R31B-V011. No V047 or other candidate change is inherited.
- DUPLICATION_AUDIT: V001-V047 actual Route diff hunks contain no deletion at this exact point. V007 targets a different function; V046 deletes a later event handoff; V047 deletes a different input-load handoff.
- COMPILE_TARGET: `sync_barrier_elision_v048`
- CORRECTNESS_CASES: FP16 Parent/Candidate bitwise comparison across the seven cases configured in the runner.
- LOCAL_CASE: FP16 128x128; device-event timing, same-binary qualification, and two 31-pair interleaved Parent/Candidate blocks; retain all raw samples and load/jitter context. Await Main's exclusive device assignment after Correctness.
- CURRENT_LOCAL_BEST: R31B-V011
- ONLINE: NO
- CANDIDATE_SOURCE_SHA256: `b4b69991de41f0cb48e5a036fd31d56cac4b2b67ef6f87e6fb7103048e35fd29`
- COMPILE: PASS; target `sync_barrier_elision_v048`, configure/build return code 0 on `hwnput3`, Ascend910B3 / `dav-2201`, CANN `8.5.0.alpha002`.
- CORRECTNESS: PASS_BITWISE; seven FP16 cases, zero Parent/Candidate mismatches.
- LOCAL_CASE: FP16 `128x128`, device 3; 60 warmups per invocation, two Parent and two Candidate 31-pair same-binary qualification runs, then two 31-pair interleaved Parent/Candidate Local blocks. Device-event latency is primary, wall latency is diagnostic, and all samples are retained.
- LOCAL: `LOCAL_REJECTED_NOISY`; pooled raw-derived 62-pair median-paired score `+1.505017%`, pooled median paired Candidate-minus-Parent delta `-0.270 us`; block scores `-0.662983%` and `+2.834467%` reverse direction. Same-binary and interleaved event timing have high spread; no promotion.
- CURRENT_LOCAL_BEST: `R31B-V011` (unchanged).
- OFFICIAL / ONLINE: no Official score; `NOT_SUBMITTED`. Numeric single-shape Local is not comparable to Official `45.16`.
- DEVICE ASSIGNMENT: Main's exclusive device-3 allocation covered V048 qualification, Local and result capture. Shared device TSV was not modified by this Route.

Evidence: `compile-result.json`, `correctness-result.json`, `local-result.json`, `source-meta.json`, `submission.sha256`, `diff.patch`, and all qualification/Local raw logs under `logs/`. A toolkit shell nounset setup failure occurred before any device command and is preserved separately; the retry completed qualification successfully.

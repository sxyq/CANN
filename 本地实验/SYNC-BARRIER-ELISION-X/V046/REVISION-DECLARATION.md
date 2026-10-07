# SYNC-BARRIER-ELISION-X V046

- ROUTE: `SYNC-BARRIER-ELISION-X`
- REVISION: `V046`
- DIRECT_PARENT: `R31B-V011`
- PARENT_SOURCE_SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- CANDIDATE_SOURCE_SHA256: `dafe51fc1ea4582c3605222e282f3e68da556c8388ba3ca697fd00185e1f05f4`
- SINGLE_CHANGE: remove the one `SyncVToMTE2()` after the per-row `ReduceSum` loop in `ProcessSmallLowPrecisionContiguousBatched`.
- CHANGE_BOUNDARY: the only Candidate source diff is that synchronization removal. The measurement-runner correction changes `CANDIDATE_V045` to `CANDIDATE_V046`; Candidate source is unchanged by that correction.
- COMPILE: PASS; `sync_barrier_elision_v046`, Ascend910B3 / `dav-2201`, CANN `8.5.0.alpha002`.
- CORRECTNESS: PASS_BITWISE; seven FP16 cases, zero Parent/Candidate mismatches.
- LOCAL_CASE: FP16 `128x128`, device 3; 60 warmups per invocation; two Parent and two Candidate 31-pair same-binary qualification runs, followed by two 31-pair interleaved Parent/Candidate Local blocks. Device-event latency is primary; wall latency is diagnostic; all raw samples are retained.
- LOCAL: `LOCAL_REJECTED_NOISY`; raw-derived 62-pair score `-4.676489%`, median paired Candidate-minus-Parent delta `+0.730 us`. Same-binary qualification and Local samples have high jitter and large outliers.
- CURRENT_LOCAL_BEST: `R31B-V011` (unchanged).
- OFFICIAL / ONLINE: no Official score; `NOT_SUBMITTED`. This numeric single-shape Local score is not comparable to Official `45.16`.
- DEVICE ASSIGNMENT: Main's exclusive device-3 allocation covered V046 measurement/result closeout. Shared device TSV was not modified by this Route.

Evidence: `compile-result.json`, `correctness-result.json`, `local-result.json`, `source-meta.json`, `submission.sha256`, `diff.patch`, and the raw logs under `logs/`. Failed runner-build and verification attempts are retained and are not timing evidence.

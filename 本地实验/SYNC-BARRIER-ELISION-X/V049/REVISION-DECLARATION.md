# SYNC-BARRIER-ELISION-X V049

- ROUTE: SYNC-BARRIER-ELISION-X
- REVISION: V049
- DIRECT_PARENT: R31B-V011
- PARENT_SOURCE_SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- SINGLE_HYPOTHESIS: In `ProcessSmallLowPrecisionContiguousBatched`, remove only the `PIPE_V` barrier after `FromFloat(outputLocal, valueLocal, totalElems)` and before FP16 gamma/bias application.
- SINGLE_CHANGE_BOUNDARY: Delete one vector-pipe barrier from exact R31B-V011. No V048 or other candidate change is inherited.
- DUPLICATION_AUDIT: V001-V048 actual Route diff hunks contain no deletion at this exact point. V003/V009 target a conversion-to-gamma barrier in `ProcessNarrowMidOverlap`; V014 targets a different pre-conversion point in the generic tiled path. V046-V048 target distinct operations earlier in this function.
- COMPILE_TARGET: `sync_barrier_elision_v049`
- CORRECTNESS_CASES: FP16 Parent/Candidate bitwise comparison across the seven cases configured in the runner.
- LOCAL_CASE: FP16 128x128; device-event timing, same-binary qualification, and two 31-pair interleaved Parent/Candidate blocks; retain all raw samples and load/jitter context. Await renewed Main exclusive device assignment after Correctness.
- CURRENT_LOCAL_BEST: R31B-V011
- ONLINE: NO
- CANDIDATE_SOURCE_SHA256: `83e4f9248e939a102171b3ac931bfa80cfa9128a347107c00ac627ce8a542225`
- COMPILE: PASS; target `sync_barrier_elision_v049`, CANN `8.5.0.alpha002`, Ascend910B3 / `dav-2201` on `hwnput3`.
- CORRECTNESS: PASS_BITWISE; seven FP16 cases, zero Parent/Candidate mismatches. An earlier preflight exited before any device command and is preserved; the retry completed.
- LOCAL_CASE: FP16 `128x128`, device 3; 60 warmups per invocation, two 31-pair Parent and Candidate same-binary qualification runs, then two 31-pair interleaved Parent/Candidate device-event blocks. Every raw sample and load snapshot remains in the logs.
- LOCAL: `LOCAL_REJECTED_NOISY`; pooled raw-derived 62-pair median-paired score `+1.684717%`, pooled median paired Candidate-minus-Parent delta `-0.140000 us`. Block scores reverse direction (`-7.204612%`, `+3.464203%`); qualification median drift reached `87.1355%` for Parent run 2 and `75.0218%` for Candidate run 1. No promotion.
- CURRENT_LOCAL_BEST: `R31B-V011` (unchanged). Numeric single-shape Local is not comparable to Official `45.16`.
- OFFICIAL / ONLINE: no Official score; `NOT_SUBMITTED`.
- DEVICE ASSIGNMENT: Main allocated device 3 exclusively through V049 qualification, paired timing and result capture. The shared device TSV was not modified by this Route.

Evidence: `compile-result.json`, `correctness-result.json`, `local-result.json`, `source-meta.json`, `submission.sha256`, `diff.patch`, and preserved qualification/Local logs under `logs/`. The initial correctness preflight failure is retained separately from the passing retry.

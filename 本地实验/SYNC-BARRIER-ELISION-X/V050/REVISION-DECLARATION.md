# SYNC-BARRIER-ELISION-X V050

- ROUTE: SYNC-BARRIER-ELISION-X
- REVISION: V050
- DIRECT_PARENT: R31B-V011
- PARENT_SOURCE_SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- SINGLE_HYPOTHESIS: In the FP16 branch of `ProcessSmallLowPrecisionContiguousBatched`, remove only the `SyncMTE3ToV()` immediately after `Store(outputGm_, batchOffset, outputLocal, totalElems)`.
- SINGLE_CHANGE_BOUNDARY: Exactly one post-store MTE3-to-V event deletion in the FP16 branch of the exact R31B-V011 source. The FP32 `else` branch is unchanged.
- DUPLICATION_AUDIT: V001-V049 actual Route diffs contain no deletion at this exact operation point. V039 removes a post-store drain in generic `Process()`; V032/V033 target distinct `ProcessNarrowMidOverlap` drains; V046-V049 target different operations earlier in this batched function.
- COMPILE_TARGET: `sync_barrier_elision_v050`
- CANDIDATE_SOURCE_SHA256: `a8c12738f9937f1f6c2d9a08ad7f84fd182ecf1ed08065d196c787b3d270573c`
- CANDIDATE_EXECUTABLE_SHA256: `b7f4e810fe34fb2375c07fbda706a54f68c78b3a003b83b6991b5e7d79bbccaf`
- COMPILE: PASS on `hwnput3`, Ascend910B3 / `dav-2201`, CANN `8.5.0.alpha002`. The immediate post-edit build is `logs/compile-v050-fp16-point-final-20261008T002733Z.log`; the source-hash-bound rebuild and executable identity are in `logs/compile-v050-correct-fp16-20261008T003200Z.log`.
- CORRECTNESS_CASES: Seven FP16 Parent/Candidate bitwise cases.
- CORRECTNESS: PASS_BITWISE; seven cases, zero mismatches on device 3. Runner source SHA `4e31ae6977846dd4f80eff5d0c7359b1851be000e49e24b3b7dd405c210b67fd`; executable SHA `159a9b00a60ff1713e89b7d7c51bf4fa0238ab35fd1e2a51cf08c47bf2364bae`; `logs/correctness-v050-correct-fp16-20261008T003227Z.log`.
- LOCAL_CASE: FP16 `128x128`, device-event timing; two 31-pair same-binary runs each for Parent and Candidate, then two 31-pair interleaved Parent/Candidate blocks. 60 warmups per invocation. All raw event, throughput, wall, process and load data are retained in `logs/local-v050-correct-fp16-env-20261008T003726Z.log`.
- LOCAL: `LOCAL_REJECTED_NOISY`; 62-pair raw-derived median-paired score `+1.866978%`, median paired Candidate-minus-Parent delta `-0.160000 us`. Block scores reverse direction (`-4.096383%`, `+2.748413%`). Pooled Parent/Candidate event CVs are `0.557879` / `0.501228`; Candidate same-binary qualification has high within-run dispersion. No promotion.
- CURRENT_LOCAL_BEST: `R31B-V011` (unchanged). Numeric single-shape Local is not comparable to Official `45.16`.
- OFFICIAL / ONLINE: no Official score; `NOT_SUBMITTED`.
- DEVICE ASSIGNMENT: Main allocated device 3 exclusively to V050 through Correctness, qualification, paired timing, and result capture. Released at `2026-10-08T00:47:12Z`; the Route did not modify the shared device TSV.
- PROCESS_DEVIATION: The initial scaffold inherited V049 and is preserved separately. A later Candidate deleted the FP32 `else` branch operation (SHA `d48ef8b0cfe9633e06fca4abb36839add0ff00b4b7b5a489df6da53a0babb590`); its Compile/Correctness/Local are invalid for this declared FP16 change and remain in `*-wrong-branch-invalid` artifacts. The associated old logs are listed in `SCAFFOLD-DEVIATION.md`. A loader failure before any samples is separately preserved at `logs/local-v050-correct-fp16-20261008T003323Z.log`. The corrected FP16 Candidate was recompiled, passed Correctness, and completed the valid qualification/Local sequence.

Evidence: `compile-result.json`, `correctness-result.json`, `local-result.json`, `source-meta.json`, `submission.sha256`, `diff.patch`, `SCAFFOLD-DEVIATION.md`, and the preserved logs under `logs/`.

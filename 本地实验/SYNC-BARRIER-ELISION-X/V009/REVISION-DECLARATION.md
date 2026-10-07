# SYNC-BARRIER-ELISION-X V009

- ROUTE: SYNC-BARRIER-ELISION-X
- REVISION: V009
- DIRECT_PARENT: R31B-V011
- PARENT_SOURCE_SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- SINGLE_HYPOTHESIS: In the FP16 branch of `ProcessNarrowMidOverlap`, remove only the `PIPE_V` barrier after `FromFloat(outputLocal, valueLocal, valid)` and before `Mul(outputLocal, outputLocal, gammaLocal, valid)`.
- SINGLE_CHANGE_BOUNDARY: One synchronization-operation deletion; all source starts from the R31B-V011 parent.
- DISTINCTION: This is separate from V005's pre-square conversion barrier, V006's post-`Muls` barrier, V007's square-to-`ReduceSum` barrier, and V008's residual-add-to-conversion barrier.
- PREVIOUS_LOCAL_END: `2026-10-07T06:43:34Z`
- LOCAL_RESULT_TO_EDIT_SECONDS: `1242` (historical timing fact; retained)
- EDIT_START_AT: `2026-10-07T07:04:16Z` (source mtime)
- EDIT_TO_FIRST_COMPILE_SECONDS: `37` (Compile started at `07:04:53Z`)
- EDIT_TO_COMPILE_PASS_SECONDS: `765` to the first Compile PASS for final source SHA (`07:17:01Z`; retained compile sequence is in `compile-result.json`)
- COMPILE: PASS for final source SHA `e2e9ed435e36855f493010aaa2d6ae10b8108922e631d36ee3acf3df604506b7` at `07:17:01Z`, before the exact-source Correctness and Local stages; a post-Local exact-source rebuild also passed at `07:36:29Z`. Earlier CMake/include failures and subsequent repairs remain in `compile-result.json` and `logs/`.
- CORRECTNESS: PASS for final source SHA on device 3, five FP16 widths (128, 256, 1024, 2048, 4096), zero bit mismatches; exact-source run `07:24:45Z` to `07:24:53Z`.
- LOCAL: final-source 62-pair pooled device-event geomean `+22.234668%` (run geomeans `+5.965208%` and `+41.002074%`); `LOCAL_REJECTED_NOISY`. Device-event CV was 1.45-1.96 for Parent and 1.79-1.83 for Candidate, maxima reached 382.92/246.96 us against 6.16-8.92 us medians, and run medians disagree in direction. Raw samples: `logs/local-exact-source-device3-20261007T073236Z.log`; result summary: `local-result-exact-source.json`.
- CURRENT_LOCAL_BEST: `R31B-V011` (unchanged)
- LOCAL_START_AT: `2026-10-07T07:32:36Z`
- LOCAL_END_AT: `2026-10-07T07:32:52Z`
- ONLINE_CANDIDATE: NO; Local score is single-shape and not comparable to Official 45.16.
- SOURCE_IDENTITY: `submission.asc`, Compile, Correctness, and final-source Local all use SHA-256 `e2e9ed435e36855f493010aaa2d6ae10b8108922e631d36ee3acf3df604506b7`.
- RUNNER_LABEL_CAVEAT: The copied harness prints stale `CANDIDATE_V008` / `CANDIDATE_V006` labels; candidate source identity is bound by the exact SHA in the Compile, Correctness, and Local logs.
- ONLINE_CANDIDATE: NO

Evidence: `compile-result.json`, `correctness-result.json`, `local-result-exact-source.json`, `diff.patch`, `submission.sha256`, and `logs/`. The earlier `local-result.json` and its logs are retained unchanged as superseded evidence for the pre-normalization source SHA `214afec5339a3ecd9af57a2fe0f1b361a0c4119c05fac235966a95f59107b98d`.

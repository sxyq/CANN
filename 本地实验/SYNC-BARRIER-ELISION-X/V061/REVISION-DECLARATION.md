# SYNC-BARRIER-ELISION-X V061

- ROUTE: `SYNC-BARRIER-ELISION-X`
- REVISION: `V061`
- DIRECT_PARENT: `R31B-V011`
- PARENT_SOURCE_SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- CANDIDATE_SOURCE_SHA256: `4efb391c70b8ef033a3b4675f32b39a282760e55f5222364773ea28d31c15977`.
- SINGLE_HYPOTHESIS: In the low-precision batched output path, remove only the guarded MTE3-to-V wait before reusing the alternating output buffer.
- SINGLE_CHANGE_BOUNDARY: Delete only `AscendC::WaitFlag<AscendC::HardEvent::MTE3_V>(outputRelease)` after the per-row `Muls` and before `FromFloat`. Keep the guard, release-state updates, producer SetFlag, and all other operations unchanged.
- DUPLICATION_AUDIT: `NO_MATCH_IN_SCOPED_DIFFS` in this Route's available V001-V060 diffs/declarations for the exact outputRelease wait at this source site. See `RULE_REFRESH_RECEIPT.md`.
- FOCUS_AXIS: Output buffer MTE3-to-V event wait elision.
- FOCUS_VALUE: One guarded `WaitFlag<MTE3_V>(outputRelease)` in the low-precision batched output path.
- COMPILE_TARGET: `sync_barrier_elision_v061`.
- CORRECTNESS_CASES: Existing seven FP16 Parent/Candidate cases.
- LOCAL_CASE: FP16 128x128; same-binary qualification and two 31-pair interleaved event-timing blocks; retain every raw sample and load snapshot.
- CURRENT_LOCAL_BEST: exact `R31B-V011`; V060 remains rejected/noisy and is not inherited.
- COMPILE: PASS; see `logs/compile-v061-support-fix.md`.
- CORRECTNESS: PASS, 7/7 FP16 cases bitwise equal.
- LOCAL: `LOCAL_REJECTED_NOISY`, pooled paired-median score +4.912068%, 62 pairs; see `RESULT.md`.

# SYNC-BARRIER-ELISION-X V064

- ROUTE: `SYNC-BARRIER-ELISION-X`
- REVISION: `V064`
- DIRECT_PARENT: exact `R31B-V011`
- PARENT_SOURCE_SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- CANDIDATE_SOURCE_SHA256: `4db6715c4e11a1b840998f2aeb08a97a551bf4f84dbfd83c1d630f41e5d75204`
- SINGLE_HYPOTHESIS: test whether the post-Sqrt V-to-S handoff is required before reading the per-row inverse-RMS scalars in the active small-batched FP16 path.
- SINGLE_CHANGE_BOUNDARY: delete only `SyncVToS()` after the per-row `Sqrt` loop and before the `invRmsValues` scalar extraction loop in `ProcessSmallLowPrecisionContiguousBatched`. Preserve `PIPE_V`, the later `SyncSToV()`, all arithmetic, all other synchronization, and all other paths.
- DUPLICATION_AUDIT: no exact match in this Route's revision declarations or available V046-V063 diff patches. V031 is a distinct function; V053, V055, and V063 target distinct synchronization operations. See `RULE_REFRESH_RECEIPT.md` for the audit boundary.
- FOCUS_AXIS: batched FP16 post-Sqrt V-to-S scalar-read handoff.
- FOCUS_VALUE: one `SyncVToS()` before `invRmsValues` reads.
- CURRENT_LOCAL_BEST: exact `R31B-V011`; no rejected Candidate is inherited.
- DEVICE: user-assigned device 3 for V064 Correctness/Local through result capture; snapshot required after Compile and before runtime.
- COMPILE: `PASS` for kernel and correctness targets; see `logs/compile-v064.log`.
- CORRECTNESS: `PASS`, 7/7 FP16 Parent/Candidate bitwise cases with zero mismatches; see `logs/correctness-v064.log`.
- LOCAL: 62 raw interleaved device-event pairs. Paired-median score `-1.474359%`; separate pooled median-latency ratio `+0.645161%`; `LOCAL_REJECTED_NOISY`, not promoted. Full raw-derived summary in `RESULT.md`.
- CURRENT_LOCAL_BEST: exact `R31B-V011` (unchanged).
- DEVICE_RELEASE: device 3 explicitly released after post-capture snapshot at `2026-10-08T15:14:36.468579289Z`.
- OFFICIAL / ONLINE: none; `NOT_SUBMITTED`.

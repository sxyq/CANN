# Superseded V002 Timing Probe

The first V002 harness was accidentally created by copying V001 `submission.asc` rather than the unchanged `R31B-V011` parent. Its source therefore contained two deletions: the V001 post-Add barrier deletion and the intended V002 Mul-to-Add deletion, plus an unintended wide-path deletion from the patch context.

The probe was compiled, passed the five-case correctness runner, and produced two 31-pair Local logs. Those raw logs remain intact:

- `local-device3-superseded-two-change-run1.log`
- `local-device3-superseded-two-change-repeat.log`

They are excluded from the V002 result. The corrected V002 source is directly derived from `parent.asc` and has exactly one deleted source line, recorded in `diff.patch`.

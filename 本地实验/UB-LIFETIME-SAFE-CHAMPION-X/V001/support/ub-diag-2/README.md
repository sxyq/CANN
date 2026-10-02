# UB-DIAG-2 exact-source diagnostic harness

This harness is a read-only diagnostic wrapper for the preserved V001 evidence.
It compiles two independent executables from the byte-bound sources:

- `ub_diag_2_parent`: `parent-submission.asc` (`R31B V011`)
- `ub_diag_2_candidate`: `submission.asc` (`UB-LIFETIME-SAFE-CHAMPION-X V001`)

Both targets include the same `diagnostic_runner.cpp`, use the same deterministic
eight-case correctness matrix, and accept `DEVICE RUNS`. The wrappers do not
edit or copy either Candidate/Parent kernel. The harness fixes the pre-existing
FP32 host protocol defect in the prior runner: dtype `0` is allocated, encoded,
decoded, and referenced as 4-byte FP32. FP16 and BF16 retain the prior input
generation and staged reference rounding protocol.

Every case prints source identity, first mismatch index, actual/reference output
values, max absolute/relative error, NaN/Inf counts, reference `invRms` (actual
`invRms` is not exposed by the kernel and is recorded as unavailable),
tail/non-tail geometry, and run-to-run byte repeatability. This is correctness
diagnostics only: no timing, profiling, Online submission, or shared-ledger
update is part of UB-DIAG-2.

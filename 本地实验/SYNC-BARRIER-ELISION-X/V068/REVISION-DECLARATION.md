# SYNC-BARRIER-ELISION-X V068

- ROUTE: `SYNC-BARRIER-ELISION-X`
- REVISION: `V068`
- DIRECT_PARENT: exact `R31B-V011`
- PARENT_SOURCE_SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- SINGLE_HYPOTHESIS: test whether the vector-pipeline barrier after BF16 row-wise gamma `Mul` is required before bias `Add` in the small-contiguous batched fallback.
- SINGLE_CHANGE_BOUNDARY: delete only `AscendC::PipeBarrier<PIPE_V>()` immediately after `AscendC::Mul(valueRow, valueRow, gammaFp32, width)` and before `AscendC::Add(valueRow, valueRow, biasFp32, width)` in the `width > kFp32RepeatMaxWidth` branch of `ProcessSmallLowPrecisionContiguousBatched`. Preserve Mul, Add, the post-Add barrier, conversion, every other sync operation, and all other paths.
- DUPLICATION_AUDIT: all 67 Route-local Candidate sources V001-V067 retain this exact BF16 pre-Add barrier; V067 removes only the distinct post-Add barrier. See `RULE_REFRESH_RECEIPT.md`.
- FOCUS_AXIS: BF16 row-wise fallback gamma-to-bias vector-pipeline dependency.
- FOCUS_VALUE: one `PIPE_V` barrier between BF16 row-wise gamma `Mul` and bias `Add` when width exceeds 192.
- CASE: BF16 `128x256`, which selects the aligned multi-row batched path and its width>192 row-wise FP32-staged gamma/bias fallback.
- CURRENT_LOCAL_BEST: exact `R31B-V011`; V067 remains rejected/noisy and is not inherited.
- COMPILE / CORRECTNESS / LOCAL: pending.
- OFFICIAL / ONLINE: none; `NOT_SUBMITTED`.

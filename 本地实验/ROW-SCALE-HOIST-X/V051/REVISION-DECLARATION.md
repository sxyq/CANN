# ROW-SCALE-HOIST-X V051

- Direct parent: V026, exact source SHA-256 `7c2569d9e3830c845f2b5dc4587a73a05a0bdf041168614cd55ad84204e063f9`; sibling of V050.
- Focus axis: FP32 gamma operand placement in the narrow contiguous-batched consumer.
- Single change: in `ProcessSmallFp32ContiguousBatched` when `rowWidth <= kFp32RepeatMaxWidth`, form the per-row scaled gamma vector in the dead-after-reduction `xBuf_` row, then multiply the retained value row by that vector. The wide subcase and every other consumer retain V026 behavior.
- Target: FP32 `[128,128]`, 40 vector cores/blocks, 3-4 rows/core; observed dispatch `ProcessSmallFp32ContiguousBatched`.
- Distinctness: V050 applies gamma-scratch scaling in `ProcessSmallFp32Batched` and fails Candidate correctness at `[128,3072]`. V039/V045 use other consumers. The superseded V051 order-swap attempt is retained under `attempt-gamma-placement/` and was never run for correctness or Local.
- Candidate source SHA-256: `5aad6db8fb2be25d65cd39c42c6a5ce72b75fd6e485b4495905c0c1d46f25bba`.
- Compile: PASS for `device` and `submission` in fresh build `/tmp/cann-row-scale-hoist-x-v051-canonical-compile.PtQs44`; see `attempt-gamma-placement/compile-canonical.log` and `compile-result.json`.
- Correctness: Parent and Candidate PASS on FP32 `[128,128]`; both matched ratio `1.0`, max absolute error `7.15255737e-07`. Candidate source delta executed. See attempt-local raw logs and `correctness-result.json`.
- Local: 128 samples per arm in four interleaved P/C blocks. Pooled medians Parent `19.6799995 us`, Candidate `19.21 us`; descriptive score `102.446639770953`, median delta `-2.388208902139%`. Pooled means instead indicate Candidate `+16.896410709656%` slower; Candidate CV is `127.189512%` with a `289.679993 us` maximum, and three of four paired block medians are slower. Verdict `MEASUREMENT_BLOCKED`; no promotion.
- Current Local Best: V026.
- Device 0: fresh V051 snapshots show `53,170 MB` free before Correctness and `53,169 MB` before Local. Device explicitly released after raw capture at `2026-10-08T05:38:02Z`; see `attempt-gamma-placement/device-release.log`.
- Online: NOT_RUN / NOT_AUTHORIZED.
- Complete evidence: `attempt-gamma-placement/RESULT.md` and adjacent route-local artifacts. The original V051 root compile log and source variant are preserved as superseded compile-only evidence; they do not identify the current Candidate.

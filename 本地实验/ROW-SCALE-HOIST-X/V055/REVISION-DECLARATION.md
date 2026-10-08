# ROW-SCALE-HOIST-X V055

- Direct parent: V026, exact source SHA-256 `7c2569d9e3830c845f2b5dc4587a73a05a0bdf041168614cd55ad84204e063f9`.
- Current Local Best at selection: V026.
- Focus axis: row-scale operand placement/order in the BF16 resident full-row output consumer.
- Single change: only in `ProcessBf16FullRowOutputPipelined`, after the existing per-row `invRms` is computed, use the dead-after-reduction `xFp32` tile as scratch for `gammaFp32[col] * invRms`, then multiply the retained FP32 value tile by that scaled gamma scratch before the unchanged bias addition and BF16 conversion.
- Target: BF16 `[128,8192]`, `ProcessBf16FullRowOutputPipelined`, resident multi-row path.
- Distinction: V025 changes the order of scale and gamma multiplication on the value operand in this consumer; V055 moves the scale onto a reusable FP32 gamma operand scratch. V053 uses gamma-scratch scaling in the separate `ProcessSmallLowPrecisionContiguousBatched` consumer. V055 is a sibling directly from V026, not from V054.
- Unchanged: exact V026 parent base; dtype and precision; reduction and inverse-RMS formula; one row-scale operation and one value/gamma operation; bias placement; dispatch and extents; allocations; synchronization; stores; all other consumers.
- Correctness tolerance: BF16 mixed tolerance `atol=rtol=1/64`, matched ratio `>=0.99`, max absolute error `<=1.0`.
- Local protocol: 45 warmups, 32 timed samples per invocation, 32-sample Parent stability, four interleaved Parent/Candidate blocks.
- Device: device 0 assigned to this Route for V055; preserve live snapshots and release after raw result capture.
- Current stage: selected; Candidate edit and Compile not yet run.
- Online/shared records: not run/not authorized.

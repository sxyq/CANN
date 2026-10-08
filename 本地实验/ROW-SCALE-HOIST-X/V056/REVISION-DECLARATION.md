# ROW-SCALE-HOIST-X V056

- Direct parent: V026, exact source SHA-256 `7c2569d9e3830c845f2b5dc4587a73a05a0bdf041168614cd55ad84204e063f9`.
- Current Local Best: V026. V055 remains committed and `MEASUREMENT_BLOCKED`; it is not the parent.
- Focus axis: BF16 row-scale operand placement in the resident full-tile batched output consumer.
- Single change: in `ProcessBf16FullTileBatchedOutputPipelined`, use the dead-after-reduction `xFp32Buf_` tile to form `gammaFp32 * invRmsValues[batchRow]`; multiply `valueRow` by this scaled-gamma scratch before the existing bias addition and BF16 conversion.
- Target: BF16 `[128,4096]`, multiple resident rows, expected branch `ProcessBf16FullTileBatchedOutputPipelined`.
- Distinctness: V023 changes scale/gamma order on the value operand in this consumer; V055 uses gamma-scratch scaling in `ProcessBf16FullRowOutputPipelined`; V053 uses it in `ProcessSmallLowPrecisionContiguousBatched`. V056 isolates the gamma-scratch placement in the full-tile batched consumer.
- Scratch lifetime: `xFp32Buf_` holds the last input row's FP32 value/square data during the batched reduction loop. All row reductions and inverse-RMS extraction finish before the epilogue; the tile is dead until the next batch, so it can be reused per output row without new storage or synchronization.
- Unchanged: exact V026 base; dispatch and batch extents; reduction and inverse-RMS calculation; operation count; cached `gammaFp32`; bias operand and position; BF16 conversion; buffers, synchronization, stores, and every other consumer.
- Correctness target: BF16 mixed tolerance `atol=rtol=1/64`, matched ratio `>=0.99`, max absolute error `<=1.0`.
- Local protocol: 45 warmups, 32 timed samples per invocation, 32-sample Parent stability, four interleaved P/C blocks.
- Stage at declaration: selected, not edited. The next action after the one Candidate edit is Compile.
- Device 0 is assigned to this Route for V056; require fresh live snapshots, preserve all processes, and release explicitly after numeric capture.
- Online/shared-record changes: not authorized and not run.

## Closed result

- Compile: PASS for `device` and `submission`; candidate source SHA-256 `9bfd5afed03e679ffbc5849b97164de9224e15d784ac4407b19519f344050096` matches the compiled input.
- Correctness: Parent and Candidate PASS on BF16 `[128,4096]`; both matched ratio `1.0`, max absolute error `0.00781393051`; the selected branch and Candidate source delta were confirmed by runner audits.
- Local: pooled-median score index `87.3437497`, Candidate median delta `+14.4901614%` slower; pooled-mean delta `+26.5282049%` slower; Candidate slower in `4/4` paired blocks.
- Verdict: `MEASUREMENT_BLOCKED` due high stability/jitter (`Parent stability CV=0.538813`, Candidate pooled CV `0.801675`). Numeric samples remain descriptive; no promotion. `CURRENT_LOCAL_BEST=V026`.
- Device 0 released at `2026-10-08T15:41:18Z` after post-capture snapshot and numeric result capture.

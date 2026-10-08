# ROW-SCALE-HOIST-X V053

- Direct parent: V026, exact source SHA-256 `7c2569d9e3830c845f2b5dc4587a73a05a0bdf041168614cd55ad84204e063f9`.
- Focus axis: FP32 row-scale operand placement in the BF16 wide subcase of `ProcessSmallLowPrecisionContiguousBatched`.
- Single change: only when `width > kFp32RepeatMaxWidth` in the BF16 arm, apply the existing per-row `invRms` to a reusable FP32 gamma scratch in `xFp32`, then multiply the retained FP32 value row by that scaled gamma before adding bias. This replaces the same row scale on the value operand; it does not add or remove an arithmetic operation.
- Target: BF16 `[128,256]`, aligned width, multiple rows per core, expected dispatch `ProcessSmallLowPrecisionContiguousBatched` wide subcase.
- Distinctness: V052 tests the same consumer only at `width <= kFp32RepeatMaxWidth` (BF16 `[128,128]`); V051 is the FP32 contiguous-batched consumer. V043/V046/V047 exercise `ProcessNarrowMidOverlap`; V027/V050 and V040 exercise different FP32 consumers. No recorded probe applies gamma-scratch scaling in this BF16 wide subcase.
- Scratch bound: at width 256, each batch row's `xFp32[valueOffset]` region is width elements; the region is reused after reduction, when converted input scratch is dead. No new buffer, allocation, synchronization, dispatch, or store change is planned.
- Unchanged: exact V026 parent base; reduction and inverse-RMS calculation; FP32 arithmetic and BF16 output conversion; gamma/bias loads and bias position; dispatch and row/batch extents; buffer allocation; synchronization; FP16 arm; BF16 narrow subcase; all other consumers.
- Compile: PASS for device/submission and Local Parent/Candidate runners. Candidate source SHA-256 `45c4f948ce3fc10633e5d39a47239df1a55673d13f98348006b58a37edbb9ee1` matches the compiled input.
- Correctness: Parent and Candidate PASS on BF16 `[128,256]`; dispatch audit confirmed the `ProcessSmallLowPrecisionContiguousBatched` wide subcase and Candidate delta execution.
- Local: descriptive pooled-median index `110.4347791052`; Candidate median delta `-9.44881602497%` (faster), pooled mean delta `-28.0558607052%` (faster), with 128 raw samples per arm. Paired block deltas: `-36.2275%`, `+47.6070%`, `+10.8311%`, `-24.4139%` (positive means slower).
- Local classification: `MEASUREMENT_BLOCKED`. Parent stability CV `0.600320` and MAD/median `0.382562`; paired direction is 2/4 each way. Numeric data is retained as descriptive only; no Local Best promotion. Device 0 was released after capture (receipt: `local/device-release.log`).
- Current Local Best: V026. Local numeric result is not Official-comparable; no Online action.

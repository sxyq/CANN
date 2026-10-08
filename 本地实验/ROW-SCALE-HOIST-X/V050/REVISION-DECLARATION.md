# ROW-SCALE-HOIST-X V050

- Direct parent: V026 (`7c2569d9e3830c845f2b5dc4587a73a05a0bdf041168614cd55ad84204e063f9`); this is a sibling of V049.
- Focus axis: FP32 row-scale operand placement in `ProcessSmallFp32Batched`.
- Single change: for each retained FP32 value row, apply its existing `invRms` scalar to a reusable FP32 gamma scratch, then multiply the row value by that scaled scratch. Preserve the original cached gamma and all other epilogue operations.
- Rationale: test moving the existing scale operation from the retained value operand to the row-specific gamma scratch in this small-row batched consumer.
- Target: FP32 `[128,3072]`, 40 vector cores/blocks, 3-4 rows/core; expected dispatch `ProcessSmallFp32Batched`.
- Duplicate boundary: V027 tests value-scale/gamma ordering in `ProcessSmallFp32Batched`; V029 tests scale/gamma ordering in other output consumers. V039 tests gamma-operand scaling in `ProcessFp32FullRowOutputPipelined`; V045 tests it in `ProcessNarrowMidOverlap`. No prior V026 sibling applies the scale to a gamma scratch in `ProcessSmallFp32Batched`.
- Unchanged: exact V026 parent base; FP32 dtype/precision; reduction and inverse-RMS formula; row scale value; cached gamma/bias loads; batch, dispatch, buffers except reusing the now-dead x scratch after reductions; synchronization, stores, and all other consumers.
- Compile: PASS for `device` and `submission`; see `compile-result.json` and `compile.log`.
- Correctness: Parent PASS; Candidate FAIL on FP32 `[128,3072]`, 40 blocks, 3-4 rows/core. The dispatch audit confirms `ProcessSmallFp32Batched` and `candidate_source_delta_executed=true`; see `correctness-result.json` and per-arm logs.
- Local: NOT_RUN because Candidate correctness failed. Device 0 was released after the captured result; see `device-release.log`.
- Current Local Best: V026.
- Online: NOT_RUN / NOT_AUTHORIZED.

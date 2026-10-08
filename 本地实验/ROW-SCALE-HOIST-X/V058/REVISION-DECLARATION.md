# ROW-SCALE-HOIST-X V058

- Direct parent: V026, exact source SHA-256 `7c2569d9e3830c845f2b5dc4587a73a05a0bdf041168614cd55ad84204e063f9`.
- Current Local Best: V026. V057 is a failed-correctness sibling and is not the parent.
- Focus axis: FP16 row-scale placement across the output conversion boundary in `ProcessSmallLowPrecisionContiguousBatched`.
- Single change: in the FP16 arm only, move the existing per-row `invRms` scale from the retained FP32 value row before `FromFloat` to the converted FP16 output row after `FromFloat` and before the unchanged gamma multiplication. Apply the scalar as `half(invRms)` at the new placement. This is one placement/rounding-boundary change; no other consumer is modified.
- Target: FP16 `[128,128]`, 40 vector cores/blocks, 3-4 rows per block; dispatch `ProcessSmallLowPrecisionContiguousBatched`, FP16 branch, `width <= kFp32RepeatMaxWidth`.
- Distinctness: V021 changes FP16 placement in `ProcessNarrowMidOverlap`; V024 changes placement in `ProcessFp16FullTileBatchedOutputPipelined`; V034 changes scale/gamma order in that full-tile consumer; V035 changes scale/gamma order in `ProcessNarrowMidOverlap`; V047 changes gamma operand placement in resident FP16 `ProcessNarrowMidOverlap`. No recorded revision moves FP16 row-scale across `FromFloat` in `ProcessSmallLowPrecisionContiguousBatched`.
- Unchanged: exact V026 parent base; FP16 dispatch, reduction and inverse-RMS formula; gamma and bias values and order; one scale and one gamma multiplication per output element; row/batch extents; all buffers, synchronization, stores, FP32 and BF16 arms, and every other consumer.
- Correctness target: Parent and Candidate FP16 `[128,128]`; intended contiguous-batched FP16 dispatch must be observed. Local runs only if both pass.
- Local protocol: use the canonical route warmup/sample and interleaved Parent/Candidate protocol in the available route-local runner; retain every raw sample and before/after device snapshot. Report numeric score/delta separately from measurement quality and promotion.
- Online/shared-record changes: not authorized and not run. Do not push.

## Stage

The fresh rule refresh is recorded in RULE_REFRESH_RECEIPT.md.

- Candidate edit: complete as the single declared FP16 row-scale placement change.
- Compile: PASS for device and submission; Candidate and compile-input SHA-256 6c4a11c570226ec17821063c33c6e5f48f1f9a0ec7c9f5b825c38c766ef7b20d.
- Correctness: Parent and Candidate PASS on FP16 [128,128]; dispatch and Candidate delta confirmed in correctness-result.json.
- Local: descriptive pooled-median index 130.8988764045, Candidate median delta -23.6051502146%; pooled-mean delta -17.2937608671%. Verdict MEASUREMENT_BLOCKED due Parent stability CV 0.5272795511, baseline drift, and concurrent non-Route load. All 128 raw samples per arm are preserved.
- Current Local Best remains V026. Device 3 was released after numeric capture. No Online, shared-record edits, or push.

# ROW-SCALE-HOIST-X V059

- Direct parent: V026, exact source SHA-256 `7c2569d9e3830c845f2b5dc4587a73a05a0bdf041168614cd55ad84204e063f9`.
- Current Local Best: V026. V057 failed Candidate correctness; V058 is `MEASUREMENT_BLOCKED`; neither is the parent.
- Focus axis: BF16 row-scale placement/order in the wide subcase of `ProcessSmallLowPrecisionContiguousBatched`.
- Single change: only when `width > kFp32RepeatMaxWidth` in the BF16 arm, move the existing FP32 `invRms` multiplication on the retained value row from before gamma multiplication to after gamma multiplication and before bias addition. Preserve the same value operand and operation count.
- Target: BF16 `[128,256]`, `width=256 > 192`; expected dispatch is the `ProcessSmallLowPrecisionContiguousBatched` wide subcase. V053 already verified this shape and dispatch.
- Distinctness: V052 tests BF16 gamma-scratch placement only in the narrow subcase; V053 tests BF16 gamma-scratch placement in this wide subcase, not value/gamma operation order. V058 changes the FP16 conversion-boundary placement in this consumer. No recorded revision tests this BF16 wide-subcase value-operand order swap.
- Unchanged: exact V026 parent base; reduction and inverse-RMS calculation; dtype/precision; gamma/bias loads and bias placement; row and batch extents; dispatch; buffers, synchronization, stores; FP16 arm; BF16 narrow subcase; all other consumers.
- Correctness: Parent and Candidate BF16 `[128,256]`; run Local only if both pass.
- Local protocol: canonical route protocol, 45 warmups, 32 timed samples per invocation, Parent stability sample, and four interleaved Parent/Candidate blocks; preserve every raw sample and before/after device snapshot. Report numeric result independently of measurement quality and promotion.
- Online/shared-record changes: not authorized and not run. Do not push.
- Candidate source SHA-256: `36dca7a4b01f6ff723cea2a72fd9d0e956f719974d8542fb46ea9c55f367ed92`; exact compile input match recorded in `source-meta.json`.
- Compile: PASS for `device` and `submission`; see `compile-result.json` and `compile/` logs.
- Correctness: Parent and Candidate PASS on BF16 `[128,256]`; target wide subcase and Candidate delta execution confirmed. See `correctness-result.json` and retained raw logs.
- Local: descriptive score index `106.0884549110`; Candidate median `5.7390362750%` faster, pooled mean delta `-18.5045058761%`; 128 raw samples per arm. Verdict `MEASUREMENT_BLOCKED` due Parent pooled CV `1.1239997`, baseline drift, and paired direction `2/4`. See `local/local-result.json` and all nine raw invocation logs.
- Current Local Best remains V026. Device 3 was released after result capture; see `device-release.log`. No Online, shared-record change, or push.

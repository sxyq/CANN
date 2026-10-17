# ROW-SCALE-HOIST-X V062

- Route: `ROW-SCALE-HOIST-X`
- Revision: `V062`
- Exact source: `4042019061242d3ec3a6c47cd6057b5e1ef2e9c1:本地实验/ROW-SCALE-HOIST-X/V062/submission.asc`
- Candidate source SHA-256: `1dd6fb4b2a29d752b0d8a190cc5c8e5310a7725a8aee05d708bf0a0f43955996`
- Direct Parent: `V026`; Parent source SHA-256: `7c2569d9e3830c845f2b5dc4587a73a05a0bdf041168614cd55ad84204e063f9`
- Scoped evidence commit: `36984e5f83000f3331a15d46d979c4fcbc97583e`

## Source change

In the BF16 narrow `ProcessSmallLowPrecisionContiguousBatched` branch for `width <= kFp32RepeatMaxWidth`, replace the prior combined epilogue with the explicit order gamma multiplication, row `invRms` scaling, then bias addition. The BF16 wide branch, FP16 branch, dispatch predicates, reduction, dtype policy, and unrelated synchronization are unchanged in the declared hypothesis.

## Local

Current retest scope: BF16 shape `[128,128]`, `ProcessSmallLowPrecisionContiguousBatched`, device 0; 20 warmups, 32 device-event samples across 9 invocations, and four interleaved blocks. Formula: `score_index = 100 * (P_median / C_median)` and `candidate_delta_pct = 100 * (C_median / P_median - 1)`. Parent/Candidate medians were `19.7300005/20.8500005 us`, giving score `94.6282974909` and candidate `+5.6766344228%` slower; 2/4 blocks favored Candidate; CV was `0.876/1.330`, with maximum Candidate sample `339.08 us`. Verdict: `MEASUREMENT_BLOCKED`; numeric Local data is not an Official-equivalent score. Current Local Best remains V026.

## Target path and correctness

Target path executed: YES. Parent and Candidate reached the intended BF16 contiguous-batched dispatch. Compile: PASS. Correctness: PASS for BF16 `[128,128]`, matched ratio `1.0`, maximum absolute error `0.00390625`; this is not an official-suite result.

## Risks and Official history

- Risk: one BF16 shape/dtype only; high timing variation, an extreme Candidate sample, and a negative retest signal leave the measurement blocked.
- Previous-round route Official score: none recorded. The prior V047 package was experimental and not Official-comparable; no Official result is claimed here.
- Online: NO; no Online action was executed.

# ROW-SCALE-HOIST-X V058

- Route: `ROW-SCALE-HOIST-X`
- Revision: `V058`
- Exact source: `909f8641d76baa3b0643afcaba890f4b5b8733d2:本地实验/ROW-SCALE-HOIST-X/V058/submission.asc`
- Candidate source SHA-256: `6c4a11c570226ec17821063c33c6e5f48f1f9a0ec7c9f5b825c38c766ef7b20d`
- Direct Parent: `V026`; Parent source SHA-256: `7c2569d9e3830c845f2b5dc4587a73a05a0bdf041168614cd55ad84204e063f9`
- Scoped evidence commit: `88dedc72f163aa654302490cc15aba202218831c`

## Source change

In the FP16 `ProcessSmallLowPrecisionContiguousBatched` branch, move the existing per-row `invRms` scale from the retained FP32 row before `FromFloat` to the converted FP16 row after `FromFloat` and before the unchanged gamma multiplication. The scalar is applied as `half(invRms)`; no other dtype arm or consumer is claimed changed.

## Local

Current retest scope: FP16 shape `[128,128]`, `ProcessSmallLowPrecisionContiguousBatched_FP16`, device 3; 45 warmups, 32 device-event samples across 9 invocations, and four interleaved blocks. Formula: `score_index = 100 * (P_median / C_median)` and `candidate_delta_pct = 100 * (C_median / P_median - 1)`. Parent/Candidate medians were `18.38/18.53 us`, giving score `99.1904964922` and candidate `+0.8161099465%` slower; 2/4 blocks favored Candidate; CV was `0.424/0.572`. Verdict: `MEASUREMENT_BLOCKED`; numeric Local data is not an Official-equivalent score. Current Local Best remains V026.

## Target path and correctness

Target path executed: YES. Parent and Candidate reached the intended FP16 contiguous-batched dispatch and matched ratio `1.0` at `[128,128]`. Compile: PASS. Correctness: PASS for the stated Parent/Candidate FP16 scope; this is not an official-suite result.

## Risks and Official history

- Risk: one FP16 shape/dtype only; the retest is measurement-blocked, slightly slower, and not a promotion signal.
- Previous-round route Official score: none recorded. The prior V047 package was experimental and not Official-comparable; no Official result is claimed here.
- Online: NO; no Online action was executed.

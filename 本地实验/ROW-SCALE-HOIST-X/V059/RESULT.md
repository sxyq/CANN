# ROW-SCALE-HOIST-X V059 Result

- Direct parent and Current Local Best: V026 (`7c2569d9e3830c845f2b5dc4587a73a05a0bdf041168614cd55ad84204e063f9`).
- Single change: in the BF16 wide subcase of `ProcessSmallLowPrecisionContiguousBatched`, move the existing FP32 row `invRms` multiplication on the retained value from before gamma to after gamma and before bias.
- Candidate SHA-256: `36dca7a4b01f6ff723cea2a72fd9d0e956f719974d8542fb46ea9c55f367ed92`.
- Compile: PASS for `device` and `submission`.
- Correctness: Parent and Candidate PASS on BF16 `[128,256]`; `matched_ratio=1.0` on both, Candidate delta executed in the intended wide subcase.
- Local: 128 samples per arm. Descriptive median score index `106.0884549110`; Parent median `18.47 us`, Candidate median `17.41 us`, median delta `-5.7390362750%`. Pooled means are `24.9854688672 us` Parent and `20.3620313125 us` Candidate, mean delta `-18.5045058761%`.
- Paired block median deltas (positive means Candidate slower): block 1 `+4.8304666783%`; block 2 `-38.3044554455%`; block 3 `+44.0205482032%`; block 4 `-33.2279924779%`.
- Quality verdict: `MEASUREMENT_BLOCKED`. Parent pooled CV is `1.1239997`; Parent stability CV is `0.3926844`; the Parent stability median (`21.09 us`) is `14.19%` above the pooled Parent median; paired direction is only `2/4`. The external device-3 training process remained untouched. Numeric result is descriptive only.
- Current Local Best remains V026. No Official comparison, Online, shared-record edit, or push.
- Correctness stdout recovery after a tee path typo is documented in `correctness/output-recovery-note.md`; no runner was repeated. All nine Local outputs are preserved in `local/`.
- Device 3 was explicitly released after the post-capture snapshot; see `device-release.log`.

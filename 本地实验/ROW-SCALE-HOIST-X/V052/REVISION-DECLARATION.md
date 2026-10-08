# ROW-SCALE-HOIST-X V052

- Direct parent: V026, exact source SHA-256 `7c2569d9e3830c845f2b5dc4587a73a05a0bdf041168614cd55ad84204e063f9`.
- Focus axis: row-scale operand placement in the BF16 small contiguous-batched consumer.
- Single change: only in the BF16 arm of `ProcessSmallLowPrecisionContiguousBatched` when `width <= kFp32RepeatMaxWidth`, reuse the now-dead `xFp32Buf_` input-staging tile to form `gammaFp32 * invRmsValues[batchRow]`, then multiply the retained FP32 value row by that scratch before adding bias.
- Target: BF16 `[128,128]`, 40 vector cores/blocks, 3-4 rows/core; aligned width, multiple rows/core, narrow subcase.
- Dispatch predicates: BF16, `rowWidth <= kSmallLowPrecisionContiguousMaxWidth` (2048), `rowWidth % kSmallLowPrecisionAlignment` (16) == 0, and `localRows > 1`; target width also satisfies `kFp32RepeatMaxWidth` (192).
- Duplicate audit: V027/V050 cover `ProcessSmallFp32Batched`; V028/V051 cover FP32 `ProcessSmallFp32ContiguousBatched`; V040 covers FP32 `ProcessSmallFp32FullTileBatched`; V043/V046 cover BF16 `ProcessNarrowMidOverlap`; V047 covers FP16 `ProcessNarrowMidOverlap`. V029's broad order change does not touch this BF16 contiguous-batched epilogue. No earlier revision tests gamma-scratch scaling in this BF16 consumer/subcase.
- Unchanged: V026 parent math/reduction, FP32 arithmetic, gamma and bias values, dispatch, row/batch extents, buffers allocated, synchronization, stores, FP16 arm, width > `kFp32RepeatMaxWidth` subcase, and every other consumer.
- Compile: PASS for `device` and `submission`; Candidate source and compile input SHA-256 `69eeccade066c7865f28d997932f573131d395df3354abec34b6f1f5f411b237`; build directory `/tmp/cann-row-scale-hoist-x-v052-canonical-compile`.
- Correctness runners: Parent/Candidate built successfully in `/tmp/cann-row-scale-hoist-x-v052-correctness-verified`; source and executable identities are in `correctness-build-identity.txt`.
- Correctness: PASS for Parent and Candidate on BF16 `[128,128]`; intended dispatch and Candidate delta confirmed in `correctness-result.json`.
- Local: `LOCAL_REJECTED`; descriptive score `84.015216840742`, Candidate `19.0260571362%` slower by pooled medians; noisy samples retained, with all four paired block medians slower for Candidate.
- Current Local Best: V026.
- Online: NOT_RUN / NOT_AUTHORIZED.

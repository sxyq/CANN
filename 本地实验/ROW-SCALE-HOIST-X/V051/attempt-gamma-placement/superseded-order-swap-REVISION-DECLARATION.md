# ROW-SCALE-HOIST-X V051

- Direct parent: V026 (`7c2569d9e3830c845f2b5dc4587a73a05a0bdf041168614cd55ad84204e063f9`); sibling of V050.
- Focus axis: FP32 row-scale consumption order in the narrow subcase of `ProcessSmallFp32ContiguousBatched`.
- Single change: for `rowWidth <= kFp32RepeatMaxWidth`, move the existing per-row `invRms` multiply from before gamma multiplication to between gamma multiplication and bias addition. Leave the wider direct-loop subcase and all other paths unchanged.
- Target: FP32 `[128,128]`, 40 vector cores/blocks, 3-4 rows/core; expected dispatch `ProcessSmallFp32ContiguousBatched`, narrow helper subcase.
- Distinctness: V027 changes `ProcessSmallFp32Batched` at width 3072. V028 changes only the `rowWidth > kFp32RepeatMaxWidth` direct-loop subcase of this contiguous consumer at width 2048, but correctness was tooling-blocked before producing a binary. No prior V026 sibling changes the `rowWidth <= kFp32RepeatMaxWidth` subcase.
- Unchanged: reduction and inverse-RMS formula/value; FP32 precision; data, gamma and bias loads; contiguous batching, row assignment, dispatch, buffers, barriers, stores, all other consumers, and the wider subcase.
- Compile: NOT_RUN.
- Correctness: NOT_RUN; requires a fresh exclusive device assignment.
- Local: NOT_RUN; only after Parent and Candidate correctness pass and a fresh exclusive assignment.
- Current Local Best: V026.
- Online: NOT_RUN / NOT_AUTHORIZED.

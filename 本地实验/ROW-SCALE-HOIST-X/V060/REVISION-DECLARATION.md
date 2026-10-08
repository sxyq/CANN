# ROW-SCALE-HOIST-X V060

- Direct parent: V026, exact parent source SHA-256 `7c2569d9e3830c845f2b5dc4587a73a05a0bdf041168614cd55ad84204e063f9`.
- Current Local Best before and after this revision: V026.
- Focus axis: FP16 row-scale placement in the wide `ProcessSmallLowPrecisionContiguousBatched` path.
- Single change: for the FP16 wide branch (`width > kFp16RepeatMaxWidth`), move the existing row `invRms` multiplication from the FP32 value before conversion to native FP16 after gamma multiplication and before bias.
- Preserved: reduction, dispatch, buffers, synchronization, stores, dtype policy, scalar/narrow branch, and all non-FP16 paths.
- Target case: FP16 `[128,256]`, device 3, 40 vector cores, wide branch (`width=256 > 192`).
- Candidate source SHA-256: `144e93799f98ee8595c52c7bbbae1323ab46a4139c89f2712f22253ef18c2df8`.
- Compile: PASS for `device` and `submission`; revalidated from the exact V060 compile input. See `compile-result.json` and `compile/`.
- Correctness: Parent and Candidate PASS; candidate delta executed in the intended FP16 wide branch. See `correctness-result.json` and retained logs.
- Local: numeric descriptive score index `103.0031612223`; Candidate median `2.9156010230%` faster; paired direction `3/4`. Result is `MEASUREMENT_BLOCKED` and descriptive only because of mixed block behavior and concurrent device load. See `local/local-result.json` and all raw logs.
- Current Local Best remains V026. V060 is not a performance Parent for V061.
- No shared-record edit, Online action, or other Route change.


# ROW-SCALE-HOIST-X V066 Revision Declaration

- Direct Parent: V026 (`7c2569d9e3830c845f2b5dc4587a73a05a0bdf041168614cd55ad84204e063f9`)
- Single change: in non-full-tile `ProcessSmallFp32Batched`, multiply each value row by gamma before multiplying by its row `invRms`, then add bias
- Focus axis: row-scale/row-level scale order only
- Target branch: `ProcessSmallFp32Batched`
- Target shape: FP32 `[128,3072]`
- Candidate source: `submission.asc`
- Compile: required `device` and `submission` targets PASS; `full_link` host-object architecture diagnostic retained
- Correctness: Parent and Candidate PASS on FP32 `[128,3072]`
- Local: 9/9 invocations exit 0; descriptive numeric result retained; `MEASUREMENT_BLOCKED`
- Online: forbidden

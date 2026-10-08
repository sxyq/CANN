# ROW-SCALE-HOIST-X V065 Revision Declaration

- Direct Parent: V026 (`7c2569d9e3830c845f2b5dc4587a73a05a0bdf041168614cd55ad84204e063f9`)
- Single change: in non-full-tile `ProcessSmallFp32Batched`, stage `gamma * invRms` into the existing FP32 `xBuf_` scratch tile before multiplying each value row; preserve bias, reduction, dispatch, buffers, and synchronization otherwise
- Focus axis: row-scale/row-level scale hoist only
- Target branch: `ProcessSmallFp32Batched`
- Target shape: FP32 `[128,3072]` (non-full-tile path)
- Candidate source: `submission.asc`
- Compile: not run
- Correctness: not run
- Local: not run
- Online: forbidden


# ROW-SCALE-HOIST-X V067

- Direct Parent: exact V026 (`7c2569d9e3830c845f2b5dc4587a73a05a0bdf041168614cd55ad84204e063f9`); V066 is not a parent.
- Current Local Best: V026.
- Single change: in non-full-tile `ProcessSmallFp32Batched`, move the existing per-row FP32 `invRms` multiplication from before gamma to after the unchanged gamma multiplication and bias addition.
- Focus: row-scale/row-level scale placement/order only.
- Target: FP32 `[128,3072]`, `ProcessSmallFp32Batched` non-full-tile branch.
- Distinctness: V027/V066 use gamma -> `invRms` -> bias in this consumer; V064 uses gamma -> bias -> `invRms` in the contiguous-batched narrow consumer. V067 tests the latter order in this separate batched consumer, directly from V026.
- Unchanged: reduction/rsqrt, dtype, dispatch, core ownership, UB/buffer allocation, DMA, synchronization, bias operation itself, other consumers, and other routes.
- Correctness gate: Parent and Candidate must pass before Local.
- Local protocol: canonical warmup/sample protocol with Parent stability and interleaved Parent/Candidate blocks; retain raw samples and load/process snapshots.
- Online: FORBIDDEN. Shared records: NOT MODIFIED.

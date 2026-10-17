# ROW-SCALE-HOIST-X V051

- Direct parent: V026, exact source SHA-256 `7c2569d9e3830c845f2b5dc4587a73a05a0bdf041168614cd55ad84204e063f9`.
- Focus axis: FP32 row-scale placement in the contiguous batched consumer.
- Single change: only in `ProcessSmallFp32ContiguousBatched` when `rowWidth <= kFp32RepeatMaxWidth`, use the per-row `invRms` to form a scaled gamma vector in the dead-after-reduction `xBuf_` row, then multiply the retained value row by that vector. The wide subcase retains the parent value-scale path.
- Target: FP32 `[128,128]`, 40 vector cores/blocks, 3-4 rows/core; expected dispatch `ProcessSmallFp32ContiguousBatched` narrow subcase.
- Buffer boundary: `xBuf_` is `kTileElems` floats for FP32 and is no longer consumed after the batch reductions; at this target the four-row gamma scratch is 512 floats, within the existing 4096-float buffer.
- Distinctness: V050 tests gamma-scratch scaling in `ProcessSmallFp32Batched` and failed Candidate correctness at `[128,3072]`. V039 and V045 exercise gamma-scale placement in different consumers. The prior untracked V051 order-swap source/compile remains preserved at the parent V051 paths and is superseded by this current directive; it has no correctness or Local result.
- Unchanged: reduction and inverse-RMS formula/value; FP32 arithmetic; gamma/bias loads; dispatch, row assignment, buffers allocated, synchronization, stores, wide subcase, and all other consumers.
- Pre-edit Candidate SHA-256: `7c2569d9e3830c845f2b5dc4587a73a05a0bdf041168614cd55ad84204e063f9`.
- Compile: pending a fresh attempt-specific build; the old V051 compile validates only the superseded source SHA `ccf35a52e8e53a87b08740f18d1c2e558bf5b2e3b358bc9af1d54c3c7ff8774b`.
- Correctness: NOT_RUN until exact candidate/build identity is verified and a fresh device snapshot is captured.
- Local: NOT_RUN; only if Parent and Candidate correctness both pass.
- Current Local Best: V026.
- Online: NOT_RUN / NOT_AUTHORIZED.

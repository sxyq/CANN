# D001 architecture metadata

## Target and mapping

- Target: Ascend910B3, DAV_2201 (`dav-2201`), CANN 8.5.0.alpha002.
- The launch uses AIV-only blocks. `blockIdx` is the fixed D-stripe owner.
- `usedCores = min(40, D)` and `blockDim = usedCores`; every block owns one non-empty contiguous D stripe.
- One block persists for every row batch and sweeps all rows in that batch for its stripe; no row is migrated between cores.
- The hot path loads gamma/bias into 128-element local tiles once per stripe tile and reuses those values for every row in the batch.

## Reduction and synchronization

- Each owner computes a partial row scalar `sum(u * u)` over its complete D stripe.
- Partial scalars are written to the current workspace slot. After the first synchronization, block 0 performs the deterministic cross-stripe row-scalar merge. After the second synchronization, every owner uses that merged scalar for its output stripe.
- V004 writes each block's row partials into a contiguous 512-byte-stride slot and flushes those GM lines before the first barrier. Block 0 flushes the merged row scalars before the second barrier, making the cross-core scalar exchange visible on DAV_2201.
- The selected DAV_2201 API is `AscendC::SyncAll<true>()` with signature `template <bool isAIVOnly = true> __aicore__ inline void SyncAll()`. The installed header resolves this to `ffts_cross_core_sync(PIPE_MTE3, ...)` followed by `wait_flag_dev(...)` in `dav_c220/kernel_operator_sync_impl.h`.
- The same header confirms `CrossCoreSetFlag(uint16_t flagId)` and `CrossCoreWaitFlag(uint16_t flagId)`; these were not selected because the row-batch protocol needs an all-AIV barrier rather than a point-to-point flag.
- Synchronization count is exactly two per persistent row batch: one after scalar partials are published and one after the merged row scalar is visible. All `usedCores` blocks execute both calls in the same order, including the final tail batch. A launch with `usedCores == 1` uses the same call sequence.

## Resources

- `usedCores`: `min(40, D)` and supplied in `D001TilingData`.
- `blockDim`: exactly `usedCores` on the hot path; no inactive block enters a wait. The generic fallback launches one block and does not call a synchronization API.
- Workspace layout per double-buffer slot: `kPartialStride=128` floats reserved for each block, followed by `rowBatch` float merged scalars. Slots alternate by row batch, so the maximum workspace is `2 * (128 * 40 + 32) * 4 = 41,216` bytes. Each block's partial region begins on a 512-byte cache-line stride, and V004 uses `DataCacheCleanAndInvalid<..., CacheLine::SINGLE_CACHE_LINE, DcciDst::CACHELINE_OUT>` before the barriers.
- Local working set: two 128-element float gamma/bias tiles (1,024 bytes) plus 32 float row accumulators (128 bytes). There are no TQue slots or temporary tensors in this scalar-compatible implementation; 32-byte alignment loss is bounded by 64 bytes. The fixed workspace, local buffers, and a 8 KiB framework/synchronization reserve are all below the 184 KiB candidate budget.
- The source keeps a private scalar generic fallback for all legal ranks, D tails, unaligned D, NaN/Inf, and epsilon values. The fallback is correctness-only; the persistent stripe path is the hot route.
- V003/V004 use the target `bfloat16_t` conversion helpers (`ToFloat` / `ToBfloat16`) instead of reinterpreting BF16 storage through `uint16_t`; stripe ownership and synchronization are unchanged.

## Compile evidence

- Status: PASS.
- Source: `phase4/workspaces/D001/src/d001_add_rms_norm_bias.cpp`.
- Build file: `phase4/workspaces/D001/CMakeLists.txt`.
- Remote build workspace: `/home/data4t2/lelinfeng/phase4-workspaces/D001` on `cann-server3`.
- Compile log: `phase4/workspaces/D001/compile.log`.

## V004 handoff

- Changes: cache-visible block-major reduction slots, exact proportional stripe bounds, and restored `__builtin_sqrtf` in both hot and fallback paths.
- Source: `phase4/workspaces/D001/submission_v004.asc` (also copied to `kernel_v004.asc`).
- V004 server compile log: `phase4/workspaces/D001/compile_v004.log`.

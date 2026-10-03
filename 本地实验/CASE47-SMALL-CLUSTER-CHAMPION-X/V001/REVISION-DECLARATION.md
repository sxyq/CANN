# CASE47-SMALL-CLUSTER-CHAMPION-X V001

DATE: 2026-10-03
ROUTE: CASE47-SMALL-CLUSTER-CHAMPION-X
REVISION: V001
DIRECT_PARENT: `线上结果/R31B/V011/submission.asc`
PARENT_SOURCE_SHA: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
PARENT_SOURCE_COMMIT: `43a1049a1e08e518c88e354a754fdebb85a96f99`
PARENT_SCORE: 45.16 (Official)
CONTEXT_CLASS: GUIDED_FRESH
MAIN_SELECTED: YES (H1 for V001; approved 2026-10-03)

## SINGLE_HYPOTHESIS

Only when `localRows == 2` in the existing non-aligned `ProcessNarrowMidOverlap` path, group the two existing V-to-scalar reads and scalar-to-vector handoffs. Keep one Level-2 `ReduceSum` per row with count exactly `rowWidth`. After both row reductions, read each row's square sum and compute its mean-square in the original order, restore each row's mean-square with `Duplicate` and `Sqrt`, then read each reciprocal RMS value through the second V/S round. Preserve the per-row scalar formula and output arithmetic order. `localRows == 1` and `localRows > 2` retain the exact Parent path.

Do not change block count or row ownership, input/MTE2 load order, tile or GM byte counts, epsilon, output addressing/stores, any arithmetic formula, or any path outside `ProcessNarrowMidOverlap`.

## PROXY_AND_CONTROLS

- Primary PROXY: FP32, `D=257`, `M=2*A`, where `A` is read at runtime from `ACL_DEV_ATTR_VECTOR_CORE_NUM`. Parent dispatch uses `blockCount=min(A,M)=A`; each block owns exactly two rows. This is a synthetic proxy only and is never identified as Official case4 or case7.
- One-row fallback PROXY: FP32, `D=257`, `M=A`; each block owns one row and retains the Parent scalar path.
- Aligned-width control: FP32, `D=256`, `M=2*A`; it must continue through the existing aligned small-row path, unchanged.
- Case4/case7 input shape and dtype remain unknown in the retained Official records.
- The Support-A raw-chat C2C claim that case4/7 share a cluster or help each other has no stated mechanism and no shape/dtype. Treat it only as a weak research lead, not as input-map evidence. Other chat items do not affect this Route's mechanism.

## BUFFER_AND_DEPENDENCY_BASIS

- The Parent allocates `kCacheElems=8192` FP32 values for `valueFp32Buf_` and `kTileElems=4096` FP32 values for `reduceFp32Buf_`.
- For `D=257`, `valueFp32Buf_` uses an 8-FP32-element padded row stride: row 0 starts at offset 0 and row 1 at offset 264. Their valid elements are `[0..256]` and `[264..520]`; the two-row span is 528 floats, within the existing 8192-float allocation. Each Level-2 `ReduceSum` count remains exactly 257. The two row-reduction destinations use the existing `kSmallFp32ScalarStride=8` spacing at offsets 0 and 8, 32 bytes apart.
- Keep the two reciprocal RMS values in two existing local scalar slots through their respective output rows; they require no TBuf.
- Existing x/residual work buffers remain single-row and are reused in row order after the existing input-release wait. Gamma/bias remain resident for `localRows > 1`. No UB allocation or GM transfer is added.
- GM input and output offsets remain `row * rowWidth`; padding changes only the internal value-buffer row stride.
- The grouping follows the existing aligned `ProcessSmallFp32Batched` V/S pattern. That prior path already groups scalar reads; V001 applies the boundary grouping only to the distinct non-aligned `ProcessNarrowMidOverlap` path.

## WHY_NOT_DUPLICATE

The Parent already batches scalar reads and V/S boundaries in aligned small-row paths, so V001 does not claim that sync batching is unprecedented. Its single change is to apply the existing group boundary pattern to non-aligned rows that reach `ProcessNarrowMidOverlap`, while keeping each row's Level-2 reduction and scalar/math sequence unchanged. It does not add the older Track-B H1 padded-input/epilogue proposal, a new reduction form, a multi-row DMA, an ownership rule, or an issue-order change.

## RISKS_AND_STOP_CONDITIONS

- Confirm the rounded FP32 row stride and two-row offsets stay inside the existing value buffer for every reached width; `D=257` uses 528 of 8192 values.
- Keep the two ReduceSum result slots 8 FP32 elements apart, and do not read either result before the group's V-to-scalar wait.
- Preserve the input-release wait before reusing x/residual buffers and the MTE3 wait before reusing any output staging buffer. Preserve the Parent's per-row output address and conversion path.
- FP16/BF16 follow the same source path but have their existing type-specific Add/convert/store behavior; target correctness across dtypes is required before any timing.
- No build, correctness, device execution, or timing before Main's live resource preflight and assignment. Timing additionally requires Main's lease.

## STATUS

Candidate source now implements the two-row `ProcessNarrowMidOverlap` scalar-handoff grouping. The rounded FP32 value-row stride is used for both rows; at D=257 it is 264 elements, and the existing value/reduction buffers are reused. Build and runtime behavior remain unverified.

# DTYPE-SPECIAL-X Future Dtype-Path Studies

Scope: V001 remains the pending FP32 Candidate. The items below are read-only research for Main review; none authorizes a source edit or a new revision. All shape references stay inside the approved 24-shape matrix. No wide-row specialization is proposed.

## DTYPE-FP32-01: Remove Same-Type Helper Copies at Call Sites

- MECHANISM: In FP32-only call paths, pass an existing `LocalTensor<float>` through as the working tensor when lifetime and aliasing permit, instead of invoking `ToFloat` or `FromFloat` to copy between two FP32 tensors. Keep conversion helpers for non-FP32 instantiations.
- BOTTLENECK: Repeated local-to-local copies remain at helper call sites after V001 changes the aligned copy primitive.
- EXPECTED_SHAPES: Widths 64, 128, and 1024 across prefixes `[12]`, `[3,4]`, and `[1,2,6]`.
- WHY_IT_MAY_HELP: It can remove the copy itself, not merely change the instruction used to perform it.
- WHY_IT_MAY_FAIL: Callers may still need the original tensor for square, reduction, or output work; aliasing can extend a live range or overwrite a value still in use.
- ASCEND_FEASIBILITY: `LocalTensor<float>` handle reuse appears compatible with the existing typed buffers, but each call site needs lifetime and alias review against the installed CANN 8.5 compiler.
- UB/CORE/DMA_IMPACT: Fewer UB reads/writes; no intended change to core assignment or GM transfer count.
- SYNC_IMPACT: No new synchronization is intended; existing barriers must remain at their current data-dependency points.
- PRECISION_RISK: Low if values and operation order are unchanged; alias mistakes can cause incorrect output.
- DUPLICATE_CHECK: V001 changes aligned `DataCopy` versus `Adds` inside the shared helpers. This study skips the helper copy at selected call sites, a distinct mechanism.
- MINIMAL_OFAT_DIFF: One FP32 call-site family, with a compile-time FP32 path and no arithmetic or scheduling edits.
- EXPECTED_LOCAL_PROBES: Compile/link; CPU/static alias cases; then exact-shape correctness before any measurement.
- READINESS: NEEDS_MORE_EVIDENCE.

## DTYPE-FP32-02: Aligned Direct Output Transfer

- MECHANISM: For complete aligned FP32 output tiles, compare the current `DataCopyPad` store with the matching aligned global/local `DataCopy` form; retain the padded store for tails and misaligned addresses.
- BOTTLENECK: The output stage uses the padded-copy helper even when a full tile has no tail.
- EXPECTED_SHAPES: Widths 64, 128, 1024, and 4096 across all three prefixes; exclude width 8192 from this study.
- WHY_IT_MAY_HELP: Removing padding metadata from full-tile stores may reduce MTE3 setup work on aligned rows.
- WHY_IT_MAY_FAIL: The global/local overload may lower identically, or the operation may be hidden by preceding vector arithmetic.
- ASCEND_FEASIBILITY: Confirm the exact CANN 8.5 overload, address alignment, byte-count constraints, and tail behavior before editing.
- UB/CORE/DMA_IMPACT: Same output bytes and core layout; only the full-tile store form changes.
- SYNC_IMPACT: No synchronization changes.
- PRECISION_RISK: None expected for a byte-preserving copy; address or count errors are correctness risks.
- DUPLICATE_CHECK: V001 changes only local-to-local FP32 conversion copies; it does not alter GM output stores.
- MINIMAL_OFAT_DIFF: One store-helper branch selected only for FP32 full tiles satisfying the verified alignment conditions.
- EXPECTED_LOCAL_PROBES: Compile/link; exact task-domain correctness; same-binary floor and paired blocks only after Main grants a fresh device lease.
- READINESS: NEEDS_MORE_EVIDENCE.

## DTYPE-FP32-03: Batch Irregular Narrow Rows

- MECHANISM: Add a small-row FP32 batch path for widths 65, 127, and 129 using padded per-row input staging and a separate valid-element tail, while preserving each row's current reduction and output operation order.
- BOTTLENECK: `ProcessSmallFp32Batched` is selected only when width is divisible by `kSmallFp32ScalarStride`; the adjacent irregular widths fall through to the per-row path.
- EXPECTED_SHAPES: Widths 65, 127, and 129 across all three prefixes.
- WHY_IT_MAY_HELP: Grouping independent rows may amortize scalar V/S handoffs without changing the row reduction formula.
- WHY_IT_MAY_FAIL: Padding can add UB traffic, and the task has only 12 rows, limiting the group size on some cores.
- ASCEND_FEASIBILITY: Reuse the existing `Load`/`DataCopyPad` behavior only after confirming the per-row staging layout and valid tail lengths; do not read across a row boundary.
- UB/CORE/DMA_IMPACT: More padded UB elements and fewer per-row setup events; block count stays at eight.
- SYNC_IMPACT: Grouped scalar/vector handoffs must preserve the current dependence between each row's square sum and output.
- PRECISION_RISK: Low if valid elements retain the current FP32 arithmetic order; tail masking and padding initialization need correctness proof.
- DUPLICATE_CHECK: The current aligned small-row batches cover multiples of the scalar stride. The proposed work covers only the excluded irregular widths and does not reuse a wide-row design.
- MINIMAL_OFAT_DIFF: One new irregular-width branch; no changes to aligned batches, reduction arithmetic, or other dtypes.
- EXPECTED_LOCAL_PROBES: Host-side layout model; compile/link; focused 65/127/129 correctness for all prefixes, followed by the full approved matrix if Main authorizes a revision.
- READINESS: NEEDS_MORE_EVIDENCE.

## DTYPE-FP32-04: Fused Output Affine

- MECHANISM: Evaluate a supported FP32 vector fused multiply-add for `value * (invRms * gamma) + bias`, with gain preparation kept within the row output path.
- BOTTLENECK: The pointwise output path currently issues separate scale, gamma multiply, and bias add operations.
- EXPECTED_SHAPES: Widths 64, 128, and 1024 across all three prefixes.
- WHY_IT_MAY_HELP: A valid fused instruction could reduce pointwise instruction count after the row reciprocal has been computed.
- WHY_IT_MAY_FAIL: Gain preparation may erase the instruction saving; the compiler may already fuse an equivalent sequence.
- ASCEND_FEASIBILITY: Needs confirmation of the exact installed CANN 8.5 FP32 vector API and generated instruction sequence before implementation.
- UB/CORE/DMA_IMPACT: No planned buffer or transfer changes; same core mapping.
- SYNC_IMPACT: No planned synchronization changes.
- PRECISION_RISK: Material. FMA and gain reassociation change FP32 rounding; compare against the task tolerance on all three prefixes and selected widths.
- DUPLICATE_CHECK: V001 preserves the pointwise affine sequence and changes only same-type conversion copies.
- MINIMAL_OFAT_DIFF: One pointwise affine formulation, with no reduction, copy-helper, or row-scheduling edits.
- EXPECTED_LOCAL_PROBES: API/codegen evidence; CPU numerical model; compile/link; exact-shape correctness before any lease-backed performance work.
- READINESS: NEEDS_MORE_EVIDENCE.

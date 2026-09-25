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

- MECHANISM: Keep the existing `Muls(value, invRms)` stage and replace only the following `Mul(value, gamma)` plus `Add(value, bias)` with one supported FP32 vector fused multiply-add.
- BOTTLENECK: The pointwise output path currently issues separate scale, gamma multiply, and bias add operations.
- EXPECTED_SHAPES: Widths 64, 128, and 1024 across all three prefixes.
- WHY_IT_MAY_HELP: One fused instruction could replace the separate pointwise multiply and add after the row reciprocal has been applied.
- WHY_IT_MAY_FAIL: The vector API may not lower to a fused instruction, or instruction issue may not limit these shapes.
- ASCEND_FEASIBILITY: Confirm the exact installed CANN 8.5 FP32 vector API and generated instruction sequence before implementation.
- UB/CORE/DMA_IMPACT: No planned buffer or transfer changes; same core mapping.
- SYNC_IMPACT: No planned synchronization changes.
- PRECISION_RISK: Fused rounding differs from the existing separate multiply and add; compare against the task tolerance on all three prefixes and selected widths.
- DUPLICATE_CHECK: V001 preserves the pointwise affine sequence and changes only same-type conversion copies.
- MINIMAL_OFAT_DIFF: Replace only the adjacent output multiply and add; keep `Muls(value, invRms)`, reduction, copies, and row scheduling unchanged.
- EXPECTED_LOCAL_PROBES: API/codegen evidence; CPU numerical model; compile/link; exact-shape correctness before any lease-backed performance work.
- READINESS: NEEDS_MORE_EVIDENCE.

## Track-B Addendum (2026-09-26)

The following three studies are additional, read-only directions for V001 follow-up review. They do not authorize Candidate edits or a new revision.

## DTYPE-FP32-05: Keep RMS Reciprocal in the Scalar Path

- MECHANISM: After reading the row square sum, compute inverse RMS in the scalar path and avoid writing a one-element square-root result to UB and reading it back; keep the row reduction and output `Muls` unchanged.
- BOTTLENECK: The current path performs a scalar-to-vector handoff for one-element `Sqrt`, then a vector-to-scalar handoff to read that result before returning to vector output work.
- EXPECTED_SHAPES: All three prefixes `[12]`, `[3,4]`, and `[1,2,6]` at widths 64, 65, 127, 128, 129, 1024, 4096, and 8192.
- WHY_IT_MAY_HELP: A supported scalar square-root operation could remove one one-element vector operation and one V/S round trip per row or row batch.
- WHY_IT_MAY_FAIL: The scalar square-root may lower to a slower sequence, may not be supported in this AICore context, or may differ numerically from the current vector `Sqrt` path.
- ASCEND_FEASIBILITY: The installed route notes confirm one-element vector `Sqrt` and scalar reciprocal use, but do not establish a supported scalar square-root intrinsic. Confirm the CANN 8.5 device API and generated code before implementation.
- UB/CORE/DMA_IMPACT: No additional UB buffer, core mapping, or GM transfer is intended; one scalar result remains live until the output `Muls`.
- SYNC_IMPACT: Intended to remove the intermediate S-to-V, V-to-S, and return S-to-V crossings around the one-element `Sqrt`; preserve ordering from reduction completion through output scaling.
- PRECISION_RISK: Scalar square-root rounding may differ; compare maximum and relative error with the task tolerance for every selected width.
- DUPLICATE_CHECK: This changes only the RMS scalar path. It does not change the same-type copies, output store, irregular-row dispatch, or output affine operation in DTYPE-FP32-01 through DTYPE-FP32-04.
- MINIMAL_OFAT_DIFF: Replace only the one-element vector square-root/readback sequence in the FP32 path with a verified scalar inverse-RMS calculation.
- EXPECTED_LOCAL_PROBES: Verify the scalar API and generated code; run a CPU numerical comparison; then compile/link and task-domain correctness if Main authorizes a revision. Any performance probe still needs a fresh exclusive Main lease.
- READINESS: NEEDS_MORE_EVIDENCE.

## DTYPE-FP32-06: Batch Aligned Small-Row Reductions with Pattern AR

- MECHANISM: In the existing aligned small-row FP32 batch path, replace the per-row Level 2 `ReduceSum` loop with one row-axis Pattern `ReduceSum` over the already contiguous batch.
- BOTTLENECK: The contiguous batch path forms multiple rows together but still issues one `ReduceSum` call for each row before the grouped scalar handoff.
- EXPECTED_SHAPES: Widths 64, 128, and 1024 across prefixes `[12]`, `[3,4]`, and `[1,2,6]`; the current 8-block split gives four two-row groups and four single-row groups.
- WHY_IT_MAY_HELP: A single Pattern AR call may reduce vector instruction setup for each two-row group while retaining the existing grouped scalar phase.
- WHY_IT_MAY_FAIL: Pattern setup and temporary storage may cost more than two Level 2 calls for these short rows; most blocks have only one row and cannot combine work.
- ASCEND_FEASIBILITY: The local API guide documents Pattern AR reduction for aligned columns and A2/A3 use with inner padding enabled. Confirm the installed CANN 8.5 `ReduceSum` overload and required temporary-buffer size; these FP32 widths meet 32-byte row alignment.
- UB/CORE/DMA_IMPACT: Needs a contiguous square-value view and Pattern temporary storage; no GM traffic or core mapping change is intended. Confirm the additional live UB footprint against the 192 KiB budget.
- SYNC_IMPACT: Keep the existing completion point before reading per-row results; Pattern AR must not mix values between rows.
- PRECISION_RISK: Reduction order may differ from the per-row Level 2 calls; validate each output against the task tolerance.
- DUPLICATE_CHECK: DTYPE-FP32-03 adds a dispatch path for irregular widths. This study retains the existing aligned path and changes only the reduction call granularity/API.
- MINIMAL_OFAT_DIFF: Change only the reductions for an already selected two-row aligned batch; retain batching, scalar math, output, and synchronization boundaries.
- EXPECTED_LOCAL_PROBES: Confirm API and temporary-size constraints; compile/link; exact-shape correctness on the three widths and prefixes; then per-shape same-binary and paired probes only under an exclusive Main lease.
- READINESS: NEEDS_MORE_EVIDENCE.

## DTYPE-FP32-07: Cap Blocks for the Twelve-Row Small/Medium Shapes

- MECHANISM: Cap the launched block count at four for FP32 widths 64, 128, and 1024 while leaving `availableCoreNum=8` as the caller-provided upper bound.
- BOTTLENECK: With 12 rows and 8 blocks, four blocks own one row each and cannot enter the existing multi-row small FP32 path; four blocks own two rows.
- EXPECTED_SHAPES: Widths 64, 128, and 1024 across prefixes `[12]`, `[3,4]`, and `[1,2,6]`.
- WHY_IT_MAY_HELP: Four blocks would each own three rows, making the current contiguous batch path available to every block and potentially reducing block-level scalar-phase setup.
- WHY_IT_MAY_FAIL: Halving active blocks may reduce parallel throughput; each core receives more work, and the current split may already balance these short rows better.
- ASCEND_FEASIBILITY: The entry point treats `availableCoreNum` as a requested maximum and launches `min(rowCount, availableCoreNum)` blocks. Confirm that a shape-limited cap stays at or below the caller-provided core limit and does not alter tensor metadata or numerical behavior.
- UB/CORE/DMA_IMPACT: No extra buffer or data movement; active blocks fall from eight to four and rows per active block rise from two/one to three.
- SYNC_IMPACT: No new cross-core synchronization; existing per-block row ordering and local V/S dependencies remain.
- PRECISION_RISK: None expected from assignment alone; verify outputs to catch row-offset or block-coverage mistakes.
- DUPLICATE_CHECK: This changes only block allocation for already aligned small/medium shapes. It does not add an irregular-width batch path or change the per-row reduction primitive.
- MINIMAL_OFAT_DIFF: Add one FP32 shape predicate for the block-count cap; leave all kernel math and local batch limits unchanged.
- EXPECTED_LOCAL_PROBES: Static block-coverage model for 12 rows; compile/link; exact-shape correctness; then compare against 8 blocks with same-binary and paired runs under an exclusive Main lease.
- READINESS: NEEDS_MORE_EVIDENCE.

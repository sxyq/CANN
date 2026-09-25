# MIX-A Next-Hypothesis Research

Status: design-only; Main review returned `NEEDS_ONE_MORE_LOCAL`. V007 remains unchanged. No Candidate source or new revision was created.

Route identity: V007 source SHA-256 `a63ad29a997ae2fe8a1238c1a47a9d5ddfb14d16f725fca975ce5c55523f28eb`; direct parent V003 source SHA-256 `1a1857a945ce1f04e3877437882b21a3d5071dddcd697103d7b539a0feb5c706`; parent Official Score `44.69`.

## MIX-A-H01: Combine MTE2-to-V waits in the narrow-mid prologue

- MECHANISM: In `ProcessNarrowMidFast`, issue gamma/bias and x/residual copies before one `SyncMTE2ToV`, instead of waiting once after parameters and again after row inputs. Keep the current arithmetic order after that wait.
- BOTTLENECK: Two MTE2-to-V event waits serialize parameter readiness and the first input path for a single-row invocation.
- EXPECTED_SHAPES: `rows=1`, `129 < width <= 4096`, under the existing dispatch condition; start with `1x256 FP32`, then dtype 1 and 2 at the same shape.
- WHY_IT_MAY_HELP: Removes one event set/wait pair and lets input copies enter the MTE2 queue before the parameter wait completes.
- WHY_IT_MAY_FAIL: DMA time may dominate; the later combined wait may erase the gain, or the event ordering may not cover every issued copy as assumed.
- ASCEND_FEASIBILITY: Uses the existing `Load` and `SyncMTE2ToV` helpers. Confirm that one wait after the queued copies makes all four buffers V-visible on the selected CANN release before implementation.
- UB/CORE/DMA_IMPACT: No added UB or core count; same bytes transferred, with a changed issue order.
- SYNC_IMPACT: Removes one MTE2-to-V wait in the Fast path prologue; no new event type.
- PRECISION_RISK: Low if all four copies complete before any consumer reads them; correctness must cover FP32, FP16, and BF16.
- DUPLICATE_CHECK: V001-V007 do not isolate this prologue wait merge. V007 changes the V-to-MTE2 release before the input load, a different event direction and dependency.
- MINIMAL_OFAT_DIFF: Change only the parameter/input copy and wait sequence in `ProcessNarrowMidFast`; leave dispatch, reduction, arithmetic, and output store unchanged.
- EXPECTED_LOCAL_PROBES: Same-binary noise qualification for `1x256` first; then at least four interleaved V003/Vcandidate pairs per dtype with device events and retained raw rows. Compare pair deltas without combining the old `LOAD_CONTAMINATED` data.
- CLASSIFICATION: `READY_FOR_MAIN_REVIEW`.

## MIX-A-H02: Use aligned DataCopy for fully aligned narrow-mid rows

- MECHANISM: Add a narrow-mid copy path using `DataCopy` only when GM/UB addresses and the full byte count satisfy the 32-byte alignment requirement; retain `DataCopyPad` for every other shape.
- BOTTLENECK: `ProcessNarrowMidFast` moves x, residual, gamma, bias, and output through the shared `DataCopyPad` helpers, including aligned rows.
- EXPECTED_SHAPES: Single-row widths whose `width * sizeof(dtype)` is a multiple of 32; begin with `1x256` FP32, FP16, and BF16.
- WHY_IT_MAY_HELP: A direct aligned copy may lower address/padding work for a short kernel with several transfers.
- WHY_IT_MAY_FAIL: The compiler may lower both APIs to equivalent instructions; the copy portion may be too small to affect total latency. Misjudged UB alignment or byte count would corrupt data.
- ASCEND_FEASIBILITY: Route-local API guidance allows `DataCopy` for strictly 32-byte-aligned GM-to-UB and UB-to-GM transfers. Every source offset and UB start address still needs proof; keep `DataCopyPad` as the fallback.
- UB/CORE/DMA_IMPACT: No UB or core change; identical bytes and transfer count, with different copy instructions on the aligned branch.
- SYNC_IMPACT: Unchanged.
- PRECISION_RISK: None for exact byte copies; high correctness risk if an alignment predicate is incomplete.
- DUPLICATE_CHECK: The V001-V007 implementations use `DataCopyPad`; no Route revision switches the narrow-mid copies to aligned `DataCopy`.
- MINIMAL_OFAT_DIFF: Add an alignment-conditioned copy helper used only by `ProcessNarrowMidFast`; do not change shared `Load`/`Store` behavior for other paths.
- EXPECTED_LOCAL_PROBES: Verify exact outputs for all three dtypes on `1x256`; qualify each same-binary shape before four or more interleaved pairs. Keep unaligned widths on the existing copy path as a control.
- CLASSIFICATION: `INFEASIBLE`.
- REVIEW_NOTE: The local CANN API guidance says aligned `DataCopy` and `DataCopyPad` use the same transfer instruction and that aligned-path performance differences are negligible. This route has no evidence for a distinct speed mechanism, so H02 is excluded from the active set.

## MIX-A-H03: Remove the terminal MTE3-to-V wait for one-row output

- MECHANISM: Test omitting the final `SyncMTE3ToV` in `ProcessNarrowMidFast` when exactly one local row is processed and no later vector operation consumes the output staging buffer.
- BOTTLENECK: The final store waits for MTE3 completion to V even though the single-row path exits immediately afterward.
- EXPECTED_SHAPES: Only the existing single-row Fast dispatch domain; start at `1x256 FP32`.
- WHY_IT_MAY_HELP: Removes a terminal event pair from a short single-row kernel.
- WHY_IT_MAY_FAIL: Kernel completion may not provide the required store visibility, or the wait may be needed for runtime completion semantics. The saved work may be below the shape noise floor.
- ASCEND_FEASIBILITY: The event helper is already used in the Route, but kernel-exit store completion semantics for this exact path are not established by the local sources. Require documentation or a focused device correctness result before any timing.
- UB/CORE/DMA_IMPACT: No UB, core, or DMA count change.
- SYNC_IMPACT: Removes only the terminal MTE3-to-V pair; retain V-to-MTE3 before `Store`.
- PRECISION_RISK: Low numerical risk; high output-completion risk until store visibility is demonstrated.
- DUPLICATE_CHECK: V007 removes a V-to-MTE2 release before the first input load. No V001-V007 change removes the terminal MTE3-to-V wait in this method.
- MINIMAL_OFAT_DIFF: One conditional omission of the final wait in `ProcessNarrowMidFast`; do not alter the pre-store V-to-MTE3 wait.
- EXPECTED_LOCAL_PROBES: Correctness first on FP32/FP16/BF16 `1x256`, including repeated launches and post-kernel D2H visibility; only then same-binary qualification and interleaved pairs.
- CLASSIFICATION: `NEEDS_MORE_EVIDENCE`.

## MIX-A-H04: Keep inverse RMS in the vector pipeline

- MECHANISM: Replace the `SyncVToS` / `GetValue(0)` / scalar reciprocal / `SyncSToV` sequence after reduction with a vector reciprocal-square-root operation on the reduction result.
- BOTTLENECK: Scalar extraction and two cross-pipe waits serialize the normalization epilogue.
- EXPECTED_SHAPES: Begin with one-row FP32 widths 129-4096, especially `1x256`; expand to FP16/BF16 only after FP32 passes.
- WHY_IT_MAY_HELP: Avoids the V-to-S and S-to-V handoff and keeps the inverse RMS computation in the vector pipeline.
- WHY_IT_MAY_FAIL: The available `Rsqrt` entry in the local API index does not establish its exact CANN 8.5.0 signature or DAV_2201 availability. Approximation error may change official outputs, and the scalar tail may not dominate runtime.
- ASCEND_FEASIBILITY: `Rsqrt` appears in a local API category index, but no CANN 8.5.0 DAV_2201 signature was found in the available local references or the inspected server3 AscendC interface headers. A vector result must also broadcast the reduced scalar to all elements without retaining the current scalar handoff; establish both details before implementation.
- UB/CORE/DMA_IMPACT: No added UB, core count, or DMA; reduction output stays in its existing buffer.
- SYNC_IMPACT: Intended to remove one V-to-S and one S-to-V handoff.
- PRECISION_RISK: Medium to high; compare all output elements with the existing formula and dtype tolerances before performance work.
- DUPLICATE_CHECK: V001-V007 use scalar extraction followed by scalar reciprocal; no Route revision uses vector reciprocal-square-root for this path.
- MINIMAL_OFAT_DIFF: Change only the inverse RMS calculation after `ReduceSum`; retain reduction inputs, normalization order, dtype casts, and dispatch.
- EXPECTED_LOCAL_PROBES: Confirm API compatibility first; then correctness on `1x256` FP32 and wider single-row widths, followed by shape qualification and at least four interleaved pairs.
- CLASSIFICATION: `NEEDS_MORE_EVIDENCE`.

## Stop

These are research candidates only. Main returned `NEEDS_ONE_MORE_LOCAL` for V007; any next revision still requires Main authorization. No V008 is created here.

## Track B Review (2026-09-25)

Four distinct ideas remain active: H01 (MTE2 wait placement), H03 (terminal MTE3 completion wait), H04 (inverse-RMS vector path), and H05 (FP32 affine-tail fusion). Their full field records are above, except the new H05 record below. H02 is excluded because the local API guidance gives no expected copy-instruction speed difference.

### Review dispositions

- H01: Retain as `READY_FOR_MAIN_REVIEW`. It changes only MTE2 wait placement; prove that the final wait covers all four queued copies before any implementation.
- H03: Retain as `NEEDS_MORE_EVIDENCE`. Output visibility after kernel exit must be established before considering removal of the final wait.
- H04: Retain as `NEEDS_MORE_EVIDENCE`. Confirm the CANN 8.5.0 API and a vector broadcast path that removes both scalar-pipeline waits.
- H05: Add as `NEEDS_MORE_EVIDENCE`; the local references did not establish `FusedMulAdd` availability for this target.

### MIX-A-H05: Fuse the FP32 affine tail

- MECHANISM: In the FP32 branch of `ProcessNarrowMidFast`, replace the consecutive `Mul(u, u, gammaLocal)` and `Add(u, u, biasLocal)` with one fused multiply-add after the existing inverse-RMS `Muls`.
- BOTTLENECK: The affine tail has two dependent vector operations after normalization.
- EXPECTED_SHAPES: FP32, `rows=1`, `129 < width <= 4096`; begin with `1x256`.
- WHY_IT_MAY_HELP: A supported fused instruction could remove one vector instruction and shorten the dependent tail.
- WHY_IT_MAY_FAIL: CANN 8.5.0 DAV_2201 support was not confirmed in the local references or inspected server3 interface headers. Fusion changes FP32 rounding, and the short tail may be below the shape noise floor.
- ASCEND_FEASIBILITY: Treat `FusedMulAdd` availability and signature as unverified. Confirm the exact target API and compile support before implementation; do not substitute an API from a different architecture family.
- UB/CORE/DMA_IMPACT: No added UB, core, or DMA; same tensors and element count.
- SYNC_IMPACT: No intended synchronization change.
- PRECISION_RISK: Medium; fused rounding differs from separate multiply then add. Compare every output against the existing tolerance before any timing.
- DUPLICATE_CHECK: The current V007 `ProcessNarrowMidFast` FP32 affine tail uses separate `Mul` and `Add`; this idea does not overlap H01/H03 event changes or H04 inverse-RMS handling.
- MINIMAL_OFAT_DIFF: Change only the FP32 affine-tail pair in `ProcessNarrowMidFast`; leave FP16/BF16, dispatch, reduction, and output transfer unchanged.
- EXPECTED_LOCAL_PROBES: After API confirmation and Main authorization, run FP32 `1x256` correctness first. If it passes, qualify that exact parent shape with same-binary event samples before any interleaved pair; retain all raw rows.
- CLASSIFICATION: `NEEDS_MORE_EVIDENCE`.

## Stop Point

V007 remains unchanged under Main's `NEEDS_ONE_MORE_LOCAL` decision. Research is design-only; no Candidate source was changed and no V008 was created.

## Track B Research Cycle (2026-09-26)

This cycle adds three mechanisms distinct from H01/H03/H04/H05 and from V007's removed V-to-MTE2 release wait. All are design-only and require a new Main decision before implementation.

### MIX-A-H06: Store the FP32 result tensor directly

- MECHANISM: In the FP32 branch of `ProcessNarrowMidFast`, pass the already-computed `u` tensor directly to `Store` instead of calling `FromFloat(xLocal, u, valid)` and storing `xLocal`. The float specialization of `FromFloat` is an `Adds(..., 0.0f, ...)` vector copy.
- BOTTLENECK: One full-width vector copy and its following vector barrier remain between the final affine add and the output DMA.
- EXPECTED_SHAPES: `rows=1`, `width=256`, FP32 (`dtype=0`), with the existing single-row Fast-path conditions unchanged.
- WHY_IT_MAY_HELP: Removes a vector pass over all 256 output elements while preserving the same result tensor and output transfer.
- WHY_IT_MAY_FAIL: The direct source tensor must stay live until MTE3 has consumed it. The current end-of-row MTE3-to-V wait must remain; signed-zero behavior from adding `+0.0f` may differ.
- ASCEND_FEASIBILITY: `Store` already accepts `LocalTensor<T>` and the FP32 branch has `T=float`; `u` is a `LocalTensor<float>` with `valid` computed for the row. Keep `SyncVToMTE3` before the store.
- UB/CORE/DMA_IMPACT: No added UB, core, or DMA; one fewer vector copy, unchanged GM bytes.
- SYNC_IMPACT: Remove only the barrier attached to the removed copy; retain the preceding affine dependency barriers, V-to-MTE3 handoff, and MTE3-to-V reuse wait.
- PRECISION_RISK: Low; compare every output and explicitly include signed zero and non-finite edge cases in host reference checks if those inputs are supported.
- DUPLICATE_CHECK: H05 changes the affine multiply/add instruction pair. H06 changes only the subsequent FP32 staging copy; neither changes V007's synchronization hypothesis.
- MINIMAL_OFAT_DIFF: Change the FP32 output source from `xLocal` to `u` and remove the now-unused `FromFloat` call; leave FP16/BF16 branches unchanged.
- EXPECTED_LOCAL_PROBES: After Main authorization, run FP32 `1x256` correctness against V003 first. Then qualify that exact Parent shape using same-binary blocks before any paired run; keep all raw samples.
- CLASSIFICATION: `READY_FOR_MAIN_REVIEW`.

### MIX-A-H07: Overlap BF16 parameter widening with row-input DMA

- MECHANISM: In the BF16 `ProcessNarrowMidFast` path, retain the parameter-copy wait. At the start of the single-row loop, issue x/residual copies, widen gamma/bias to FP32 while MTE2 transfers the row inputs, then keep the existing input-copy wait before any input consumer.
- BOTTLENECK: Two parameter `ToFloat` operations and their vector barrier currently complete before the x/residual DMA is issued.
- EXPECTED_SHAPES: `rows=1`, `width=256`, BF16 (`dtype=2`), with the existing Fast-path conditions unchanged.
- WHY_IT_MAY_HELP: Overlaps independent Vector conversion work with MTE2 input transfers without adding a copy or changing arithmetic.
- WHY_IT_MAY_FAIL: The transfers may be too short to hide; CANN scheduling may serialize the pipes, or event placement may fail to order both buffers as required.
- ASCEND_FEASIBILITY: Reuses existing `Load`, `ToFloat`, and `SyncMTE2ToV` helpers. Preserve the first wait before reading gamma/bias and the second wait before reading x/residual; only issue order changes.
- UB/CORE/DMA_IMPACT: No additional buffers, core count, or bytes transferred.
- SYNC_IMPACT: Same two MTE2-to-V waits; the second wait moves after parameter widening so it also covers the row inputs.
- PRECISION_RISK: Low if both existing waits remain at those dependencies; verify all BF16 outputs against the direct-parent reference.
- DUPLICATE_CHECK: H01 removes a wait by issuing parameter and input copies before one combined wait. H07 keeps the parameter wait and overlaps the later input DMA specifically with BF16 parameter conversion; it does not combine waits.
- MINIMAL_OFAT_DIFF: Move the BF16 gamma/bias `ToFloat` block into the existing row loop after the two input `Load` calls and before their wait; leave both wait points, buffers, and arithmetic unchanged.
- EXPECTED_LOCAL_PROBES: After Main authorization, test V003/V007 reference correctness at BF16 `1x256`. If correct, establish a BF16 Parent same-binary floor before paired device-event samples.
- CLASSIFICATION: `NEEDS_MORE_EVIDENCE`.

### MIX-A-H08: Double-buffer row inputs in the multi-row narrow-mid path

- MECHANISM: For FP32 cores with `localRows>=2`, add a second x/residual input-buffer pair to `ProcessNarrowMidOverlap`; after row N's reduction releases its input slot, prefetch row N+1 into the other slot while row N completes scalar normalization and affine output, then swap slots.
- BOTTLENECK: The current loop waits for the input-release event at the next iteration before issuing the next row's two MTE2 copies, leaving input DMA outside most of the preceding row's scalar/output tail.
- EXPECTED_SHAPES: FP32, `width=256`, with row count and tiling confirmed to give at least two local rows per core. This is a separate multi-row shape, not the pending V007 `rows=1` qualification shape.
- WHY_IT_MAY_HELP: Hides next-row input latency behind work that no longer reads the current x/residual buffers.
- WHY_IT_MAY_FAIL: Extra UB may reduce occupancy; event-ID reuse, output staging, and row-buffer lifetime may create a race. The launch may not have enough rows per core to amortize the extra state.
- ASCEND_FEASIBILITY: The current path already has `V_MTE2` input-release and `MTE2_V` input-ready events. A two-slot schedule is plausible but needs a per-slot event/lifetime table and an UB budget for the exact tiling before coding.
- UB/CORE/DMA_IMPACT: Adds up to `2 * width * sizeof(float)` UB for a second input pair; core count and bytes transferred stay unchanged.
- SYNC_IMPACT: Requires independent readiness/release tracking per slot; do not reuse one event ID while its prior transfer or consumer remains outstanding.
- PRECISION_RISK: Low arithmetic risk; high synchronization and buffer-alias risk until targeted correctness passes.
- DUPLICATE_CHECK: The Candidate has adjacent-row overlap in its wide FP32 batch path, but `ProcessNarrowMidOverlap` still serializes next-row input issue. H08 targets the narrow-mid multi-row schedule, not V007's single-row path.
- MINIMAL_OFAT_DIFF: Add one paired input slot and alternate slots only in `ProcessNarrowMidOverlap`; leave Fast dispatch, arithmetic, and single-row behavior unchanged.
- EXPECTED_LOCAL_PROBES: After Main authorization, verify the selected tiling gives `localRows>=2`, then compare V003 and the new path on FP32 width 256 for output correctness. Qualify the exact Parent shape before any paired measurement.
- CLASSIFICATION: `NEEDS_MORE_EVIDENCE`.

## Current Stop Point

Main's V007 decision is `NEEDS_ONE_MORE_LOCAL`. The pending qualification shape is Parent V003, `rows=1`, `D=256`, FP32; existing targeted correctness also covers FP16 and BF16 at `rows=1`, `D=256`. Do not run timing until Main authorizes a current exclusive lease.

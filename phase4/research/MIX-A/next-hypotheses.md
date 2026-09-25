# MIX-A Next-Hypothesis Research

Status: design-only; V007 remains pending Main review. No Candidate source or new revision was created.

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
- CLASSIFICATION: `READY_FOR_MAIN_REVIEW`.

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
- ASCEND_FEASIBILITY: `Rsqrt` appears in the local API category index, but the version-specific API page was unavailable in this review. Confirm signature, supported data types, vector length, and DAV_2201 support before implementation.
- UB/CORE/DMA_IMPACT: No added UB, core count, or DMA; reduction output stays in its existing buffer.
- SYNC_IMPACT: Intended to remove one V-to-S and one S-to-V handoff.
- PRECISION_RISK: Medium to high; compare all output elements with the existing formula and dtype tolerances before performance work.
- DUPLICATE_CHECK: V001-V007 use scalar extraction followed by scalar reciprocal; no Route revision uses vector reciprocal-square-root for this path.
- MINIMAL_OFAT_DIFF: Change only the inverse RMS calculation after `ReduceSum`; retain reduction inputs, normalization order, dtype casts, and dispatch.
- EXPECTED_LOCAL_PROBES: Confirm API compatibility first; then correctness on `1x256` FP32 and wider single-row widths, followed by shape qualification and at least four interleaved pairs.
- CLASSIFICATION: `NEEDS_MORE_EVIDENCE`.

## Stop

These are research candidates only. Main must review V007 and authorize any next revision before implementation. No V008 is created here.

# MIX-A Next-Hypothesis Research

Status: design-only; Main review returned `NEEDS_ONE_MORE_LOCAL`. V007 remains unchanged. Current Track-B screen has five active hypotheses; no Candidate source or new revision was created.

Route identity: V007 source SHA-256 `a63ad29a997ae2fe8a1238c1a47a9d5ddfb14d16f725fca975ce5c55523f28eb`; direct parent V003 source SHA-256 `1a1857a945ce1f04e3877437882b21a3d5071dddcd697103d7b539a0feb5c706`; parent Official Score `44.69`.

## MIX-A-H01: Combine MTE2-to-V waits in the narrow-mid prologue

- MECHANISM: In `ProcessNarrowMidFast`, issue gamma/bias and x/residual copies before one `SyncMTE2ToV`, instead of waiting once after parameters and again after row inputs. Keep the current arithmetic order after that wait.
- BOTTLENECK: Two MTE2-to-V event waits serialize parameter readiness and the first input path for a single-row invocation.
- EXPECTED_SHAPES: `rows=1`, `129 < width <= 4096`, under the existing dispatch condition; start with `1x256 FP32`, then dtype 1 and 2 at the same shape.
- WHY_IT_MAY_HELP: Removes one event set/wait pair and lets input copies enter the MTE2 queue before the parameter wait completes.
- WHY_IT_MAY_FAIL: DMA time may dominate; the later combined wait may erase the gain, or the event ordering may not cover every issued copy as assumed.
- ASCEND_FEASIBILITY: Uses the existing `Load` and `SyncMTE2ToV` helpers. V007 already compiles on CANN 8.5.0.alpha002 / Ascend910B3 / dav-2201; the helper is `SetFlag/WaitFlag<HardEvent::MTE2_V>`. This confirms the target APIs, not that the proposed ordering covers all four pending copies.
- SOURCE_EVIDENCE: `submission.asc` `ProcessNarrowMidFast` loads gamma/bias, waits, then loads x/residual and waits again (around lines 1153-1168); `Load` uses `DataCopyPad` and the helper uses MTE2_V events (around lines 3030-3070). The local API guide describes DataCopy as asynchronous.
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
- ASCEND_FEASIBILITY: V007 compiles and passes correctness with `Store` followed by `SyncMTE3ToV`; that proves the event APIs are available on the target. The local API guide describes MTE3 copies as asynchronous, and neither the Route source nor retained build record establishes that kernel exit alone completes the final store. Keep output-completion risk high.
- SOURCE_EVIDENCE: `ProcessNarrowMidFast` issues `SyncVToMTE3`, `Store`, then `SyncMTE3ToV` for each row (around lines 1208-1232); the `Store` helper uses `DataCopyPad` (around lines 3030-3040). No retained result tests omission of the final event.
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
- CURRENT_SCREEN: Deferred from the active set until the exact CANN 8.5.0 DAV_2201 `Rsqrt` API and reduced-scalar broadcast path are established.

## Stop

These are research candidates only. Main returned `NEEDS_ONE_MORE_LOCAL` for V007; any next revision still requires Main authorization. No V008 is created here.

## Track B Review (2026-09-25)

As of the 2026-09-25 review, four ideas were retained: H01 (MTE2 wait placement), H03 (terminal MTE3 completion wait), H04 (inverse-RMS vector path), and H05 (FP32 affine-tail fusion). The 2026-09-26 screen below supersedes that active set. H02 remains excluded because the local API guidance gives no expected copy-instruction speed difference.

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
- ASCEND_FEASIBILITY: The FP32 `FromFloat` specialization is `Adds(dst, src, 0.0f, count)`. `Store` accepts `LocalTensor<T>`; the exact V007 target build already compiles FP32 calls that pass a `LocalTensor<float>` to it. This supports the direct-call type/API path, while the modified call itself still needs an authorized build. Keep `SyncVToMTE3` before the store and the final MTE3_V wait after it.
- SOURCE_EVIDENCE: `FromFloat` and `Store` are defined around lines 3030-3060; `ProcessNarrowMidFast` runs the FP32 add-zero copy and its following `PipeBarrier<PIPE_V>` before `Store` (around lines 1200-1212). Other FP32 paths in the same compiled source already store their float result tensor directly.
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
- CURRENT_SCREEN: Deferred from the active set due to partial prologue overlap with H01. Keep its narrower conversion-versus-input-DMA question distinct for later review; it is not an exact duplicate.

### MIX-A-H08: Double-buffer row inputs in the multi-row narrow-mid path

- MECHANISM: For FP32 cores with `localRows>=2`, add a second x/residual input-buffer pair to `ProcessNarrowMidOverlap`; once row N's reduction stops reading x/residual, issue row N+1 into the alternate slot while row N completes scalar normalization and affine output, then swap slots. Before either slot is written again, wait for the release from its prior consumer.
- BOTTLENECK: The current loop waits for the input-release event at the next iteration before issuing the next row's two MTE2 copies, leaving input DMA outside most of the preceding row's scalar/output tail.
- EXPECTED_SHAPES: FP32, `129 <= D <= 4095`, `D % 8 != 0`, and `localRows >= 2`; start with `D=257`. For FP32, aligned widths through 4096 are consumed by the earlier small-row batches when a block has multiple rows, so the narrow-mid multi-row path is reachable here only at widths not divisible by 8. `localRows >= 2` selects `ProcessNarrowMidOverlap`. For the one-row case, `ProcessNarrowMidFast` is selected when `rowCount <= floor(262144 / D)`; at D=257 that limit is 1020. `blockCount` is the host-clamped value `B = min(max(availableCoreNum, 1), rowCount, UINT32_MAX)`, and every launched block has at least two rows exactly when `rowCount >= 2 * B`. If `rowCount=2B+r` with `0 <= r < B`, the first r blocks receive 3 rows and the rest receive 2. D=256 is an aligned dispatch control, not an H08 probe.
- WHY_IT_MAY_HELP: Hides next-row input latency behind work that no longer reads the current x/residual buffers.
- WHY_IT_MAY_FAIL: Extra UB may reduce occupancy; event-ID reuse, output staging, and row-buffer lifetime may create a race. The launch may not have enough rows per core to amortize the extra state.
- ASCEND_FEASIBILITY: The retained CANN 8.5.0.alpha002 / Ascend910B3 build compiles the current `TBuf`, `AllocEventID`, `SetFlag`, and `WaitFlag` use. The same Candidate source initializes a `TBuf` from `rowWidth * sizeof(float)` in its wide-row branch, establishing that the target build accepts shape-sized buffer requests. The current narrow-mid path has one x slot and one residual slot, so H08 needs explicit second slots and per-slot event lifetimes; it cannot rely on queue rotation. The added schedule and its allocation have not been built.
- SOURCE_EVIDENCE: `Process` sends FP32 rows divisible by 8 and no wider than 4096 through the small-row batches before the mid path; D=257 bypasses them. For D in 129..4095 with `D % 8 != 0`, multi-row calls reach `ProcessNarrowMidOverlap`; a one-row call reaches `ProcessNarrowMidFast` only when `rowCount <= floor(262144 / D)`, because the FP32 `fastUb` expression remains below 160000 throughout this width range. At D=257, the one-row Fast limit is 1020. The host wrapper clamps the launch block count to `UINT32_MAX` after limiting it by `rowCount` (around lines 200-270 and 3180-3205).
- UB/CORE/DMA_IMPACT: The current FP32 narrow-mid allocations total 176 KiB: x/residual 32 KiB, gamma/bias 64 KiB, xFP32/residualFP32 32 KiB, value 32 KiB, and reduction scratch 16 KiB. The source documents 192 KiB UB with an 8 KiB system reservation, leaving 8 KiB nominally available. A second pair allocated at the existing 4096-element size adds 32 KiB, projecting 208 KiB and exceeding the nominal 184 KiB allowance by 24 KiB; reject that sizing. At D=257, two exact-width float slots add 2056 bytes before allocator alignment, leaving about 6 KiB nominally. No retained build output reports actual per-kernel UB high-water or allocator overhead, so exact-width feasibility is a sizing result, not a target allocation result. Core count and GM bytes stay unchanged.
- SYNC_IMPACT: Use a ready event per input slot and a release event per slot. Wait for slot N's V-to-MTE2 release before refilling that slot; wait for its MTE2-to-V readiness before consuming it. The current `valueFp32Buf_` is shared by rows and is also the source of asynchronous Store, so retain MTE3-to-V completion before reusing it. The single-slot path currently signals input release immediately after `ReduceSum`, which marks the point when vector code stops reading x/residual; that release does not make the shared output buffer reusable.
- PRECISION_RISK: Low arithmetic risk; high synchronization and buffer-alias risk until targeted correctness passes.
- DUPLICATE_CHECK: The Candidate has adjacent-row overlap in its wide FP32 batch path, but `ProcessNarrowMidOverlap` still serializes next-row input issue. H08 targets the narrow-mid multi-row schedule, not V007's single-row path.
- MINIMAL_OFAT_DIFF: Add one paired input slot and alternate slots only in `ProcessNarrowMidOverlap`; leave Fast dispatch, arithmetic, and single-row behavior unchanged.
- EXPECTED_LOCAL_PROBES: After Main authorizes implementation, record the actual block count and per-block `localRows` for D=257. Probe `rowCount=2B-1`, `2B`, and `2B+1`: at `2B-1`, one block has a single row and may take Fast depending on the global row-count limit; at and above `2B`, every block reaches Overlap. Include D=256 as the aligned dispatch control. Then validate that neither input slot is refilled before its V-to-MTE2 release, each slot's data is consumed only after MTE2-to-V readiness, and the shared value buffer is not reused before MTE3-to-V completion. Correctness and exact-shape qualification require Main authorization; timing additionally requires a current exclusive Main-1 lease.
- CLASSIFICATION: `NEEDS_MORE_EVIDENCE`.

### Static Dependency And Allocation Filter

- H01 remains eligible. In `ProcessNarrowMidFast`, parameter loads target gamma/bias storage and row loads target x/residual storage; all four copies precede their Vector consumers. The target build confirms the existing `DataCopyPad` and MTE2-to-V event APIs compile, but it contains no generated schedule for the combined wait. No source-level alias or consumer dependency directly rules out the proposed issue order.
- H03 remains unresolved. The Fast path is restricted to one local row, so no later row reuses its input/output locals. `Store` uses asynchronous `DataCopyPad`, however, and neither the retained source nor build output establishes that kernel exit alone guarantees output completion. The final MTE3-to-V wait cannot be discarded from static evidence.
- H06 remains eligible as a type/lifetime simplification. FP32 `FromFloat` is `Adds(dst, src, 0.0f, count)`, while the same compiled Candidate has FP32 paths that store a float `LocalTensor` directly. The current MTE3 completion wait keeps the source live through Store. Existing build output has no instruction listing, and add-zero may alter signed-zero results, so neither speed nor bitwise equivalence is established.
- H08's full-4096-element second input pair is excluded by the UB arithmetic above. Exact-width slots at D=257 remain plausible within nominal capacity, but allocator alignment and emitted resource use are not present in retained build evidence. No whole H01, H03, H06, or exact-width H08 mechanism is eliminated by the available source/build record.

## Current Stop Point

Main's V007 decision is `NEEDS_ONE_MORE_LOCAL`. The pending qualification shape is Parent V003, `rows=1`, `D=256`, FP32; existing targeted correctness also covers FP16 and BF16 at `rows=1`, `D=256`. Do not run timing until Main authorizes a current exclusive lease.

## Current Track-B Screen (2026-09-26)

Exactly five hypotheses are active in this screen: H01, H03, H05, H06, and H08. Existing full field records above define each mechanism, bottleneck, shape, expected benefit/failure, Ascend C feasibility, UB/core/DMA/synchronization impact, precision risk, duplicate review, OFAT diff, and local probes. H02 stays `INFEASIBLE`; H04 stays deferred pending API evidence; H07 stays deferred with a partial-overlap note against H01. Deferred entries are retained and are not counted in the active five.

### Active Hypothesis Falsification And Value

- H01: FALSIFICATION_TEST: After Main authorizes a revision, verify FP32/FP16/BF16 `1x256` output against V003 and use a compiler schedule or device trace to confirm the single MTE2-to-V wait makes all four copies visible. Any stale output falsifies the schedule. If the qualified device-event paired delta stays within the exact-shape floor, the performance premise is falsified. EXPECTED_INFORMATION_GAIN: High; it distinguishes event overhead from transfer latency in the common single-row path. LIKELY_GLOBAL_UPSIDE: Medium, conditional on the narrow-mid path's share of scored inputs.
- H03: FALSIFICATION_TEST: After Main authorizes a revision, run repeated single-row output checks with immediate synchronized D2H and inspect store-completion ordering. Any missing or stale output falsifies removal of the terminal wait. If correctness holds but qualified event measurements remain within the same-binary floor, the saved wait is not useful for scoring. EXPECTED_INFORMATION_GAIN: High; it resolves whether kernel exit supplies the required MTE3 visibility on this target. LIKELY_GLOBAL_UPSIDE: Low to medium because the change is limited to the one-row path.
- H05: FALSIFICATION_TEST: First establish the exact fused API and signature for CANN 8.5.0 DAV_2201. If unavailable, stop. If available, compare all FP32 outputs, including cancellation and rounding-boundary inputs, against current tolerances; any mismatch falsifies numerical feasibility. A qualified device-event result within the same-binary floor falsifies useful speedup. EXPECTED_INFORMATION_GAIN: Medium; it tests both target API support and whether the dependent affine pair is material. LIKELY_GLOBAL_UPSIDE: Low to medium, limited to single-row FP32 narrow-mid inputs.
- H06: FALSIFICATION_TEST: Compare generated instructions to confirm the FP32 add-zero staging copy disappears, then test ordinary values, signed zero, and supported non-finite inputs against V003. Any required-output mismatch or no instruction reduction falsifies the proposed simplification; a qualified result within the noise floor falsifies its performance value. EXPECTED_INFORMATION_GAIN: High; it directly tests whether an explicit FP32 copy remains on the hot path. LIKELY_GLOBAL_UPSIDE: Low to medium, conditional on frequency of single-row FP32 cases.
- H08: FALSIFICATION_TEST: First verify width 257 reaches `ProcessNarrowMidOverlap` and the selected tiling gives every core `localRows>=2`; width 256 is a dispatch control, not a probe. After authorization, test row counts at and just above `2 * blockCount`, including a tail assignment, while auditing each slot's ready/release event lifetime. Any slot reuse before release or stale output falsifies the schedule. If the trace shows no MTE2/Vector overlap, occupancy falls materially, or qualified timing stays within its shape floor, stop pursuing the design. EXPECTED_INFORMATION_GAIN: High; it tests whether the reachable unaligned narrow-mid path hides input-DMA time after UB cost. LIKELY_GLOBAL_UPSIDE: Low to medium until scored-shape frequency for width 257-class rows is known.

### Ranked Track-B Screen And Authorization

1. **H01 — first to review.** Existing target CANN support for the constituent MTE2_V event and DataCopyPad operations is confirmed by the V007 build. It adds no UB; the open question is whether one post-copy event covers all four transfers in the new order. Requires Main approval before a code revision. Any device correctness or same-binary work also needs fresh Main authorization; timing needs a current exclusive Main-1 lease and exact-shape qualification.
2. **H06 — second.** Source evidence identifies the FP32 add-zero copy, and the target build already compiles direct FP32 LocalTensor-to-Store calls elsewhere in V007. The missing evidence is whether the altered call lowers to less Vector work and preserves required bit-level behavior. Requires Main approval before a code revision; device tests and timing need the corresponding Main authorization and lease.
3. **H08 — third.** Existing target event primitives and TBuf allocation compile, and width 257 reaches the intended path. The second-slot schedule, per-slot events, occupancy, and scored-shape frequency remain unknown. Requires Main approval before implementation; static UB sizing is a prerequisite, then exact-shape correctness and qualification need authorization.
4. **H03 — fourth.** The target event API works in the current sequence, but the local API guidance confirms asynchronous MTE3 movement and gives no evidence that kernel exit replaces the final wait. Requires Main approval plus targeted output-visibility correctness authorization before any performance qualification.
5. **H05 — fifth.** The CANN 8.5.0 DAV_2201 fused API and signature remain unverified, with an additional FP32 rounding risk. API confirmation and Main approval are both prerequisites; otherwise do not implement.

No new implementation or device run is authorized by this screen. Same-binary qualification and P/C remain unavailable until Main grants a fresh exclusive Main-1 lease; for V007 this starts with Parent V003 at `rows=1, D=256, FP32`.

### Overlap Map And Track-A Hold

- H01 and H07 touch the same prologue. H01 combines two input-readiness waits; H07 retains both and overlaps BF16 parameter widening with the later input transfer. Keep only H01 active now; reconsider H07 after H01 has a Main disposition.
- H05 and H06 touch the FP32 epilogue but alter different operations (affine arithmetic versus output staging). They remain separate single-hypothesis options and must not be combined in one revision.
- H03 changes the terminal MTE3-to-V dependency; V007 removes a pre-load V-to-MTE2 dependency. Their event directions and consumers differ.
- H08 uses `ProcessNarrowMidOverlap`; the existing wide-path row overlap is a neighboring pattern, not the same buffer schedule.
- TRACK-A: the unified runner is built and linked and its host lease tests are recorded PASS, but the runner has not been executed. Parent V003 same-binary qualification for `rows=1, D=256, FP32` is absent. The current timing protocol requires 45 warmups; the runner CLI default is 10, so any later authorized qualification must pass 45 explicitly. The shared lease table currently has no active lease. Do not run runner, NPU, or timing without a fresh Main-1 lease and passing exact-shape qualification.

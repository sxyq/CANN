# MODE-X-R015C Future Hypotheses

Research only, recorded 2026-09-25. R015C-r4 source and implementation remain unchanged. No timing was run for this research.

Route review 2026-09-25: retain the four distinct ideas below as research only. No implementation or new revision is authorized by this review.

## R015C-H01: Row-segment task mapping

- MECHANISM: Assign one existing 2048-element maximum segment to each block, mapping `blockIdx` to `(row, segment)`.
- BOTTLENECK: Low row counts provide few blocks while wide rows contain independent segments.
- EXPECTED_SHAPES: Small `R` with `D=4096` or `D=8192`; include a tail-segment case such as `(3,2056)`.
- WHY_IT_MAY_HELP: At `D=8192`, task count grows from `R` to `4R`; each block issues one segment's existing input and output copies and skips the per-row segment loop.
- WHY_IT_MAY_FAIL: Total bytes and DMA requests remain unchanged. More block scheduling may cost more than it saves, especially for small tensors or larger `R`.
- ASCEND_FEASIBILITY: Host and kernel can derive `segmentsPerRow=ceil(D/2048)` and decode the row and segment from the block index. Keep the current 256-by-32-byte transfer cap.
- UB/CORE/DMA_IMPACT: Retain the 64 KiB per-block allocation; increase available block tasks by up to four times for the supported widths; keep total DMA bytes and segment count unchanged.
- SYNC_IMPACT: Preserve the current input-copy, `PIPE_ALL`, output-copy, `PIPE_ALL` order within each task; no cross-block synchronization is needed.
- PRECISION_RISK: Incorrect index decoding can omit or overlap a row segment; the copy itself remains bit-exact FP32 movement.
- DUPLICATE_CHECK: R015C-r4 uses one block per row. This proposal uses one block per row segment while preserving the copy path, segment size, and synchronization order.
- MINIMAL_OFAT_DIFF: Change only task indexing and launch block count; keep transfer descriptors, scratch size, and barriers unchanged.
- EXPECTED_LOCAL_PROBES: Exact checks for `(2,256)`, `(5,4096)`, `(3,8192)`, and `(3,2056)`. Any future timing requires a fresh shape-specific same-binary qualification first.
- READINESS: READY_FOR_MAIN_REVIEW.

## R015C-H02: Strictly aligned DataCopy path

- MECHANISM: Replace the paired `DataCopyPad` operations with `DataCopy` for shapes whose GM and UB addresses and byte lengths satisfy the 32-byte alignment rule.
- BOTTLENECK: Per-segment copy descriptor and padding-path overhead.
- EXPECTED_SHAPES: All accepted FP32 widths where `D % 8 == 0`; emphasize `(5,4096)` and `(3,8192)`.
- WHY_IT_MAY_HELP: A direct aligned copy path may reduce transfer setup work for these rows.
- WHY_IT_MAY_FAIL: The current aligned `DataCopyPad` path may already have equivalent device cost; transfer parameter units or a tail alignment assumption may invalidate the alternative.
- ASCEND_FEASIBILITY: The accepted shape rule makes row starts 32-byte aligned, and each full segment is 2048 FP32 values. Confirm DAV_2201 overloads and parameter units by compiling the exact CANN 8.5 toolchain before any device run.
- UB/CORE/DMA_IMPACT: Keep current 64 KiB staging, block mapping, and DMA byte count; change only the transfer primitive.
- SYNC_IMPACT: Preserve both current barriers and their positions.
- PRECISION_RISK: No arithmetic is introduced. Incorrect byte length or final-segment handling can corrupt copied values.
- DUPLICATE_CHECK: R015C-r4 deliberately retains `DataCopyPad`; this proposal changes only the aligned transfer primitive.
- MINIMAL_OFAT_DIFF: Replace the input and output copy calls while retaining segmentation, task mapping, buffer allocation, and synchronization.
- EXPECTED_LOCAL_PROBES: Compile first, then exact checks for `(2,256)`, `(5,4096)`, `(3,8192)`, and `(3,2056)`. Any future timing requires a fresh shape-specific same-binary qualification first.
- READINESS: NEEDS_MORE_EVIDENCE.

## R015C-H03: Segment-sized VECCALC staging

- MECHANISM: Allocate one maximum segment of VECCALC storage (`2048 * sizeof(float)`, 8 KiB) instead of the current 64 KiB.
- BOTTLENECK: Per-block UB allocation may limit resident work for larger row counts.
- EXPECTED_SHAPES: Larger `R` with `D=8192`; retain the three existing exact-copy shapes for correctness coverage.
- WHY_IT_MAY_HELP: Each copy call addresses only `local[0]` and transfers at most 2048 FP32 elements. A smaller allocation may permit more resident blocks.
- WHY_IT_MAY_FAIL: Small `R` may not benefit from occupancy; allocation granularity or compiler resource use may dominate.
- ASCEND_FEASIBILITY: The segment cap gives an 8 KiB payload, aligned to 32 bytes. Confirm the compiler and runtime allocate the VECCALC buffer at that size and that no access exceeds the last segment's extent.
- UB/CORE/DMA_IMPACT: Reduce requested VECCALC storage from 64 KiB to 8 KiB per block; leave block count, bytes copied, and transfer count unchanged.
- SYNC_IMPACT: No change to copy order or barriers.
- PRECISION_RISK: Exact copy semantics remain; an undersized buffer could corrupt data at the maximum segment boundary.
- DUPLICATE_CHECK: R015C-r4 preserves 64 KiB as approved. This proposal changes only the staging allocation size.
- MINIMAL_OFAT_DIFF: Define the maximum segment capacity before `InitBuffer` and allocate exactly that payload; keep all transfers and synchronization unchanged.
- EXPECTED_LOCAL_PROBES: Exact checks at all three current shapes plus a larger-row `(64,8192)` case. Any future timing requires a fresh shape-specific same-binary qualification first.
- READINESS: READY_FOR_MAIN_REVIEW.

## R015C-H04: Two-slot segment transfer pipeline

- MECHANISM: Use two aligned segment slots so MTE2 can load the next independent segment while MTE3 writes the previous segment.
- BOTTLENECK: Serial input-copy, full barrier, output-copy sequencing across two to four segments per wide row.
- EXPECTED_SHAPES: `D=8192` first, then `D=4096`, with enough rows to keep several blocks active.
- WHY_IT_MAY_HELP: Independent segments can overlap input and output DMA when their staging slots have separate lifetimes.
- WHY_IT_MAY_FAIL: This copy-only kernel has no Vector computation to hide DMA latency; queue setup, barriers, or only two segments may erase the benefit.
- ASCEND_FEASIBILITY: A two-slot design can use TQue or explicit event ownership, subject to DAV_2201 queue-position support and valid MTE2/MTE3 ordering. Verify with a small compiled prototype only after Main authorizes an implementation revision.
- UB/CORE/DMA_IMPACT: Two segment payloads require at least 16 KiB staging per block, plus queue and alignment overhead; block mapping and total bytes remain fixed.
- SYNC_IMPACT: Replaces the current serial full-barrier lifetime with explicit slot ownership and producer/consumer synchronization; race risk is material.
- PRECISION_RISK: No arithmetic is needed; stale-slot reuse or ordering errors can produce data mismatches.
- DUPLICATE_CHECK: R015C-r4 keeps one slot and serial DMA. This proposal changes only the transfer schedule and staging lifetime.
- MINIMAL_OFAT_DIFF: Keep one row per block and the 2048-element segment size; introduce two slots and pipeline adjacent segment copies.
- EXPECTED_LOCAL_PROBES: Exact checks on `(3,8192)` and `(5,4096)`, including last-slot reuse; no performance run before fresh shape-specific same-binary qualification and an explicit current lease.
- READINESS: NEEDS_MORE_EVIDENCE.

## Research batch 2026-09-26

R015C-r4 remains unchanged. The following ideas are separate research candidates; none is authorized for implementation.

## R015C-H05: Batch adjacent segments in one copy descriptor

- MECHANISM: Keep one block per row and the 2048-FP32 segment limit, but describe adjacent full segments with one `DataCopyExtParams` using `blockCount`; use a separate final descriptor only for a partial segment.
- BOTTLENECK: Per-segment copy-call setup and repeated full barriers on wide rows.
- EXPECTED_SHAPES: `(5,4096)` and `(3,8192)` first; include `(3,2056)` to exercise a partial final segment and `(2,256)` as a short-row control.
- WHY_IT_MAY_HELP: A wide row can use one batched copy-in and one batched copy-out for its full segments, reducing API-call and barrier count while retaining the observed per-block byte limit.
- WHY_IT_MAY_FAIL: The device may not handle the proposed block layout or stride units as expected; DMA work and bytes remain unchanged, and short rows may see no benefit.
- ASCEND_FEASIBILITY: The local DataCopy guide documents multi-block `DataCopyExtParams` and contiguous blocks with zero stride. Confirm DAV_2201 behavior for both GM-to-UB and UB-to-GM, the 8192-byte block length, and a separate tail descriptor before any implementation.
- UB/CORE/DMA_IMPACT: Retain one row per block and 64 KiB staging; keep total bytes and the maximum 8192-byte transfer block unchanged. Reduce full-segment descriptors from four to one for `D=8192`.
- SYNC_IMPACT: Keep serial copy-in, `PIPE_ALL`, copy-out, `PIPE_ALL` ordering; place each barrier after its complete batched descriptor.
- PRECISION_RISK: Incorrect block count, stride, or tail offset could omit or repeat data; comparison remains bit-exact FP32.
- DUPLICATE_CHECK: H01 creates one task per segment; H02 changes the copy primitive; H03 changes staging capacity; H04 overlaps segments with two slots. This idea batches adjacent segments in a descriptor while retaining one row per block and the current copy primitive.
- MINIMAL_OFAT_DIFF: Replace only the per-segment descriptor loop with one descriptor for full segments plus the existing-style tail descriptor; preserve mapping, staging, primitive, and barrier order.
- EXPECTED_LOCAL_PROBES: Exact checks for `(2,256)`, `(5,4096)`, `(3,8192)`, and `(3,2056)`. Require fresh same-binary qualification for each measured shape and an explicit current lease before timing.
- READINESS: NEEDS_MORE_EVIDENCE.

## R015C-H06: Specialize the fixed segment-count paths

- MECHANISM: Retain one block per row and separate `DataCopyPad` calls, but use fixed control-flow paths for the supported one-, two-, and four-segment row lengths instead of the dynamic column loop.
- BOTTLENECK: Loop tests, minimum selection, and offset updates repeated around each short DMA operation.
- EXPECTED_SHAPES: `(2,256)`, `(5,4096)`, and `(3,8192)`; include a non-full final segment such as `(3,2056)`.
- WHY_IT_MAY_HELP: Fixed paths can remove loop-control work for the current common widths without changing transfer count, segment size, or byte traffic.
- WHY_IT_MAY_FAIL: The compiler may already unroll or simplify the loop, making the extra branches and code size a net loss; DMA latency may dominate.
- ASCEND_FEASIBILITY: The shape arrives through the existing tiling structure, so a device-side switch can select fixed paths without changing the host ABI. Check generated code before considering any device run.
- UB/CORE/DMA_IMPACT: Keep the same block count, 64 KiB staging, per-segment DMA descriptors, and total bytes; only device control flow changes.
- SYNC_IMPACT: Preserve both `PIPE_ALL` barriers after every individual copy.
- PRECISION_RISK: A fixed path can skip a segment or mishandle the tail if its shape branch is incomplete; exact comparison is required for each path.
- DUPLICATE_CHECK: H01 changes task mapping, H02 changes the copy primitive, H03 changes buffer size, H04 changes the schedule, and H05 batches descriptors. This idea retains the same mapping, per-segment calls, buffer, and schedule while specializing only loop control.
- MINIMAL_OFAT_DIFF: Replace the dynamic segment loop with fixed-count paths selected by `shape->cols`; leave all copy arguments and barriers unchanged.
- EXPECTED_LOCAL_PROBES: Exact checks for all three current shapes plus `(3,2056)`. Any timing requires fresh exact-shape same-binary qualification and an explicit current lease.
- READINESS: NEEDS_MORE_EVIDENCE.

## R015C-H07: Narrow serial copy barriers

- MECHANISM: Keep one staging buffer and the current serial sequence, replacing the two `PIPE_ALL` barriers with pipeline-specific MTE2 and MTE3 barriers after the corresponding copies.
- BOTTLENECK: A full-pipeline stall after each input and output transfer may wait for unrelated engines.
- EXPECTED_SHAPES: `(2,256)`, `(5,4096)`, and `(3,8192)`; include `(3,2056)` for the tail path.
- WHY_IT_MAY_HELP: A narrower wait may preserve the required DMA completion while reducing stalls across unrelated pipelines.
- WHY_IT_MAY_FAIL: The MTE2-to-MTE3 local-buffer dependency may require stronger ordering on DAV_2201; the current full barrier may already compile to an equally narrow operation.
- ASCEND_FEASIBILITY: Local guidance documents `PipeBarrier<PIPE_MTE2>` and `PipeBarrier<PIPE_MTE3>`. Confirm cross-engine visibility and target compiler support, then run exact correctness before any measurement.
- UB/CORE/DMA_IMPACT: No change to the 64 KiB buffer, block mapping, descriptors, bytes, or core count.
- SYNC_IMPACT: Retain copy-in then copy-out ordering and wait for output completion before reusing the same local buffer; do not overlap transfers.
- PRECISION_RISK: An insufficient fence can expose incomplete local data to MTE3 or permit buffer reuse before writeback completes.
- DUPLICATE_CHECK: H04 introduces two-slot overlap and new buffer lifetimes. This idea retains one slot and serial ownership, changing only barrier scope.
- MINIMAL_OFAT_DIFF: Replace the post-input and post-output barrier template arguments only; retain all data movement and loop logic.
- EXPECTED_LOCAL_PROBES: Exact checks for all three current shapes plus `(3,2056)`. Do not time before fresh same-binary qualification for the exact shape and an explicit current lease.
- READINESS: NEEDS_MORE_EVIDENCE.

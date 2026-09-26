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

## Research batch 2026-09-26 (continued)

R015C-r4 remains unchanged. These three ideas are distinct from H01-H07 and remain research-only; none is authorized for implementation.

### Screening summary

This batch was screened against the Route's r3/r4 sources and the local Ascend C DataCopy, buffer, and synchronization guidance only. The device-load report is a measurement constraint and does not change the hypothesis ranking or authorize implementation.

| ID | Distinct mechanism | Main uncertainty | First falsification target | Readiness |
|---|---|---|---|---|
| H08 | Group two adjacent rows into one block and batch rows in the DataCopyPad descriptor | Reduced block parallelism may outweigh fewer block and descriptor setups; confirm two-row UB layout and odd tail | Exact-copy even and odd row counts; a future qualified result within noise or slower rejects the expected benefit | NEEDS_MORE_EVIDENCE |
| H09 | Stage all segments of one row, then copy the row out in a second phase | Multiple in-flight copies and phase barriers may cost more than the current serial loop | Exact-copy the one-, two-, four-segment and partial-tail paths; any stale or omitted segment rejects feasibility | NEEDS_MORE_EVIDENCE |
| H10 | Pass row width as a scalar instead of reading the tiling structure in each block | DAV_2201 direct-launch scalar ABI and wrapper parity are unverified; saved metadata traffic may be negligible | Build the exact scalar entry/wrapper first; ABI failure or qualified results within noise rejects the idea | NEEDS_MORE_EVIDENCE |

Duplicate screening: H08 groups rows, distinct from H05's within-row descriptor batching and H01's per-segment task split. H09 changes serial staging and barrier placement without H04's two-slot overlap or H07's narrower barrier scope. H10 changes shape metadata delivery only, leaving block mapping and DMA schedule fixed. These ideas remain separate OFAT candidates if Main later authorizes one.

No implementation or device probe was run. Timing remains disallowed under the reported load and the d7 exclusion.

## R015C-H08: Two-row block with batched row copies

- MECHANISM: Starting from r3's existing two-rows-per-block mapping, batch corresponding row transfers in one `DataCopyPad` Ext descriptor. A whole-row descriptor with zero strides is within the retained 8192-byte per-request boundary only for `D<=2048`. For wider rows, retain the 2048-FP32 segment loop and use `blockCount=rowsThisBlock` for each matching segment across the rows.
- BOTTLENECK: Per-row copy descriptor setup for high-row-count workloads, especially short rows.
- EXPECTED_SHAPES: Start with high-row-count short widths `(64,256)` and `(64,1024)`, where one descriptor can cover both rows; then `(64,4096)` to exercise segmented row batching. Include odd-row `(5,256)` and max-width `(3,8192)` boundary cases. Low-row-count existing cases offer less room for reducing descriptor submissions.
- WHY_IT_MAY_HELP: It batches up to two row transfers per descriptor and can reduce descriptor/API submissions and per-segment barriers while preserving the same block count and total bytes as r3.
- WHY_IT_MAY_FAIL: The device may still perform the same per-row DMA work, making descriptor submission overhead negligible; stride setup or cross-row access may also offset the saved host/device command work.
- ASCEND_FEASIBILITY: The local API guide documents multi-block `DataCopyPad` Ext descriptors and specifies GM stride in bytes and UB stride in 32-byte dataBlocks. Accepted widths are divisible by 8, so row bytes and the 2048-FP32 segments are 32-byte multiples. For a wide-row segment, the GM gap is `rowBytes-blockBytes` and the UB gap is `(rowBytes-blockBytes)/32`; swap source/destination stride roles for copy-out. Odd final row blocks use `blockCount=1`. The route's observed per-request boundary remains 256 dataBlocks (8192 bytes), while the API field's larger range does not establish a larger DAV_2201 request limit.
- UB/CORE/DMA_IMPACT: Keep r3's 64 KiB allocation, two rows per block, and total bytes; two maximum-width rows occupy the allocation exactly. Block count is unchanged from r3. For `D>2048`, descriptor count becomes `ceil(D/2048)` per direction and block rather than twice that count from serial per-row descriptors.
- SYNC_IMPACT: Retain copy-in, full barrier, copy-out, full barrier ordering for each segment descriptor; one descriptor covering two rows removes the duplicate barrier pair from r3's per-row loop. No cross-block synchronization is added.
- PRECISION_RISK: Incorrect `blockCount`, row extent, or final odd-row handling can omit or repeat values; there is no arithmetic, so exact-copy comparison is suitable.
- DUPLICATE_CHECK: Direct Parent r3 already uses two rows per block, but still issues one-row descriptors in its row loop. H05 batches column segments for one row. H08's distinct change is cross-row descriptor batching with explicit strides; row grouping alone is not new relative to r3.
- MINIMAL_OFAT_DIFF: Starting from r3, keep two rows per block, `DataCopyPad`, segment cap, 64 KiB allocation, and barrier ordering. Change the loop to visit matching segments across the available rows and issue one strided descriptor for that segment; use one full-row descriptor only when `D<=2048`.
- FALSIFICATION_TEST: Cheapest pre-device falsifier is a host-side descriptor arithmetic table for `D=256, 2048, 2056, 4096, 8192`, both copy directions, and odd/even row tails; reject any case whose segment exceeds 8192 bytes, stride is not representable, or final UB extent exceeds 64 KiB. A build can establish compiler/API acceptance but not device addressing. Exact-copy correctness on DAV_2201 is required to validate GM/UB stride semantics; a later qualified paired run on high-R short shapes must show that lower descriptor/barrier overhead exceeds any stride-related cost.
- EXPECTED_INFORMATION_GAIN: High; isolates cross-row descriptor/stride effects from H05's within-row descriptor batching while retaining r3's row mapping.
- LIKELY_GLOBAL_UPSIDE: Low to medium and shape-dependent; most plausible for large R with short D, uncertain for the route's existing low-R probes.
- EXPECTED_LOCAL_PROBES: Use host-only runner plans for `(64,256)`, `(64,1024)`, and `(64,4096)` first. After a future authorized implementation, add exact-copy coverage for odd and even row counts; any timing requires exact-shape same-binary qualification and a fresh lease.
- READINESS: NEEDS_MORE_EVIDENCE.

## R015C-H09: Stage a complete row before copy-out

- MECHANISM: Keep one row per block and the existing segment descriptors, but copy all segments of a row into distinct offsets in the existing staging buffer, issue one full barrier, copy all segments out, then issue one full barrier.
- BOTTLENECK: Repeated synchronization between input and output copies for rows spanning multiple segments.
- EXPECTED_SHAPES: Wide rows `(5,4096)` and `(3,8192)`; include `(3,2056)` to exercise an aligned partial final segment and `(2,256)` as a one-segment control.
- WHY_IT_MAY_HELP: A four-segment row would use two barriers instead of eight while retaining the same number and size of DMA descriptors.
- WHY_IT_MAY_FAIL: The kernel has no Vector work to overlap with DMA; multiple outstanding same-direction copies may add queue pressure, and the barriers may already be inexpensive after compiler lowering.
- ASCEND_FEASIBILITY: The 64 KiB staging buffer holds the largest complete row (32 KiB). Accepted D values and the 2048-element segment cap keep segment offsets 32-byte aligned. Static bounds establish that every `local[col]` segment fits in the buffer. `DataCopyPad` is asynchronous; the existing API notes support waiting after a set of copies, but only a DAV_2201 correctness run can confirm the proposed multi-copy phase ordering for this kernel.
- UB/CORE/DMA_IMPACT: Keep one block per row, 64 KiB allocation, segment size, and total DMA bytes. At most 32 KiB of one row is live in the staging buffer; descriptor count is unchanged.
- SYNC_IMPACT: Change synchronization from per-segment input/output alternation to one full barrier after all input segments and one after all output segments; do not overlap buffers or rows.
- PRECISION_RISK: A wrong local segment offset or a missing phase barrier can expose stale data; test every segment count and tail length with exact comparison.
- DUPLICATE_CHECK: H05 batches multiple segments in a descriptor; H07 narrows barrier scope but retains the per-segment schedule. H09 retains each descriptor and `PIPE_ALL`, changing only staging layout and barrier placement by phase.
- MINIMAL_OFAT_DIFF: Split the existing segment loop into a copy-in loop and copy-out loop, address `local[col]`, and move the two existing full barriers to the phase boundaries.
- FALSIFICATION_TEST: Cheapest non-device falsifier is compile-time or host arithmetic for segment offsets and last-byte bounds at `D=256, 2056, 4096, 8192`, followed by a build of the two-phase source. These steps cannot prove asynchronous completion. Exact-compare `(2,256)`, `(5,4096)`, `(3,8192)`, and `(3,2056)` on DAV_2201; any stale or missing segment falsifies correctness. A qualified paired run is still needed to establish benefit, especially for `(3,8192)` where four input/output segments become two synchronization points.
- EXPECTED_INFORMATION_GAIN: High; separates synchronization frequency and full-row staging effects from descriptor-count changes.
- LIKELY_GLOBAL_UPSIDE: Low to medium, concentrated on low-row-count wide shapes with multiple segments; short rows should be neutral.
- EXPECTED_LOCAL_PROBES: Use host-only plans for the three recorded shapes and `(3,2056)`. After a future authorized implementation, exact-compare all segment-count and tail paths; qualify each timed shape and obtain a fresh lease before paired runs.
- READINESS: NEEDS_MORE_EVIDENCE.

## R015C-H10: Pass row width as a kernel scalar

- MECHANISM: Remove the device-resident tiling pointer from the R4 kernel entry and pass `cols` as a scalar kernel argument; launch exactly `rows` blocks so the kernel can derive its row from `blockIdx` without reading `shape->rows` from GM.
- BOTTLENECK: Per-block GM reads of the shared tiling structure and its dependent row/column control values in a copy kernel with little other work.
- EXPECTED_SHAPES: Many short rows such as `(64,256)` and `(128,256)`, with `(64,1024)` as a medium-width comparison.
- WHY_IT_MAY_HELP: It removes the tiling-structure GM load and bounds check from each block without changing row-level DMA traffic.
- WHY_IT_MAY_FAIL: The compiler/runtime may already cache or cheaply handle the tiling reads; kernel scalar argument passing may not be supported by the exact direct-launch ABI or may add equivalent launch overhead.
- ASCEND_FEASIBILITY: The current paired runner's Parent and Candidate wrapper prototypes both take `(grid,input,output,tiling,stream)`; the Candidate template forwards the tiling pointer, and `Launch` currently discards its already-available `cols` value. A scalar Candidate entry therefore needs a distinct Candidate wrapper signature and a corresponding host call that forwards `cols`; the Parent wrapper and Parent tiling path can remain unchanged. The existing build and host identity tests cover only the tiling-pointer entry, so they do not establish scalar ABI support.
- UB/CORE/DMA_IMPACT: No change to block count, 64 KiB staging, copy descriptors, or DMA bytes; the Candidate kernel avoids per-block tiling GM reads. The paired runner still needs the Parent tiling buffer, so any host allocation saving applies only to a standalone Candidate invocation.
- SYNC_IMPACT: No change to copy order, barriers, or stream behavior.
- PRECISION_RISK: No arithmetic is added; a bad scalar value or grid mismatch can shift row offsets or leave output rows unwritten.
- DUPLICATE_CHECK: H01 changes task mapping; H08 batches cross-row descriptors while retaining r3's existing two-row mapping; H10 retains the one-row r4 mapping and changes only how row width reaches the kernel.
- MINIMAL_OFAT_DIFF: Change only the Candidate kernel entry and Candidate launch wrapper/host call to pass `cols`; derive row start from the one-row grid and leave r3 and Candidate DMA logic unchanged. Keep the Parent wrapper, tiling allocation, and launch path intact for paired execution.
- FALSIFICATION_TEST: Cheapest first falsifier is a host-only compile/link and wrapper argument test for the split Parent/Candidate signatures using the exact toolchain; failure rejects feasibility without device access. Passing that test does not prove the scalar reaches the DAV_2201 kernel correctly. Exact-copy NPU cases `(2,256)`, `(5,4096)`, `(3,8192)`, `(64,256)`, and `(64,1024)` are needed for launch correctness. Only a later same-binary-qualified paired run on high-R short shapes can test whether removing an 8-byte tiling read and row-bound branch helps.
- EXPECTED_INFORMATION_GAIN: Medium; tests whether repeated shape metadata loads matter independently of DMA and row scheduling.
- LIKELY_GLOBAL_UPSIDE: Low, with possible gains limited to many short rows; likely negligible for wide DMA-dominated shapes.
- EXPECTED_LOCAL_PROBES: Establish host wrapper and scalar-entry ABI feasibility first; then use host-only plans for `(64,256)` and `(64,1024)`. After a future authorized implementation, exact-compare current and high-row-count shapes; timing requires exact-shape qualification and a fresh lease.
- READINESS: NEEDS_MORE_EVIDENCE.

## Track-B handoff 2026-09-26 (static continuation)

- Scope: the current three-item shortlist is H08-H10. H01-H07 remain retained research history and are not folded into this batch. R015C-r4 remains unchanged; no kernel or runner work, device access, correctness run, or timing was performed for this handoff.
- Current measurement state: Main-provided latest server3 `npu-smi` output at `2026-09-26T10:06:13Z`: d0-d3 HBM respectively `59976/65536`, `60026/65536`, `59969/65536`, `59971/65536 MB`; AICore `36, 36, 37, 37%`; each has `VLLMWorker_TP` using approximately 56.45 GB. d4 HBM `59186/65536 MB`, AICore 0%, `VLLMEngineCor` PID 2999855 using 55664 MB. d5/d6 HBM respectively `59876/65536` and `59875/65536 MB`, AICore 0% each, `VLLMWorker_TP` using `56354/56355 MB`. d7 HBM `3430/65536 MB`, AICore 0%, no running process, excluded by the timing protocol. No timing was run and no lease is active. The scheduler row remains `NEEDS_ONE_MORE_LOCAL`; shared control was not edited.
- Source identities checked locally: r3 kernel SHA-256 `e1786bec2673519f41887fdffa0e11751f515a81037d854c88ae7edc0e4af903`, r3 tiling SHA-256 `5e4ad750f5c63da357324c0a829674aff81b7c0b655e8ff08072b1c289c66c41`; r4 kernel SHA-256 `9367db4ebb4edf6b7bf6cde97f4846f987e6ab230c2aeee44d942d5e881a1c74`, r4 tiling SHA-256 `0939ba8498426fcd77645d826fcc45a8eb65c78996ad0ce1da1a00d1ac01a250`.
- Parent and OFAT boundary: direct Parent r3 has `kRowsPerBlock=2`; r4 has `kRowsPerBlock=1`. H08 is single-mechanism only when evaluated from r3 and changes cross-row descriptor batching, not rows per block. H09 and H10 are single-mechanism relative to r4's one-row mapping; they must remain research-only until Main resolves r4 and explicitly permits a suitable parent. If the next authorized parent is r3, reframe either idea to retain r3's two-row mapping or do not implement it as currently written.
- H08 static boundary: each request must remain at or below 8192 bytes (256 32-byte dataBlocks), consistent with the Route's observed boundary. For `D<=2048`, a full-row multi-block descriptor fits that bound. For wider rows, issue one descriptor per existing segment across up to two rows; the API guide defines GM stride in bytes and UB stride in 32-byte dataBlocks. The cheapest non-device falsifier is an offset/stride and 64 KiB end-address table covering `D=256, 2048, 2056, 4096, 8192`, both directions, and an odd final row. DAV_2201 exact-copy execution is still required to establish actual stride behavior; a leased, shape-qualified paired run is needed to judge benefit.
- H09 static boundary: the largest row is 32768 bytes and fits within the existing 65536-byte staging allocation. Segment starts remain 32-byte aligned for the listed widths. Host arithmetic and compilation can reject bad offsets or API forms, but cannot prove completion of several asynchronous input copies before output begins. Exact-copy device runs are required for that ordering; paired timing is needed for benefit.
- H10 launch boundary: the Route runner's Candidate launch still accepts a tiling pointer, and its `Launch` function discards the available `cols` scalar. The existing build and identity tests therefore do not validate a scalar entry. The CANN 8.5.0.alpha002 installation is not present locally, and the documentation search returned no exact-version scalar-entry result; the only indexed kernel-parameter page found was for CANN 9.2 beta and its fetch did not expose usable details. Treat exact CANN 8.5 compile/link as the cheapest later feasibility test, device correctness as necessary for runtime argument delivery, and leased qualified timing as necessary for benefit.
- API references reviewed: repository-local `.agents/skills/ascendc-api-best-practices/references/api-datacopy.md` documents multi-block copies and stride units; `api-pipeline.md` describes asynchronous DataCopy completion and barriers; `api-buffer.md` documents TBuf allocation. These guide mechanism screening, while DAV_2201 behavior remains subject to the device-dependent evidence listed above.
- Device-dependent conclusions for all three ideas: hardware copy/stride execution and correctness; actual scheduling, DMA and synchronization cost; and any performance direction. No current load observation qualifies timing, and d7 remains excluded by the timing protocol.

## Track-B source cross-check 2026-09-26

- Read-only source evidence: `phase4/local/MODE-X-R015C/R015C-r3/submission.asc:9-50` maps two rows per block and serializes each row's copy-in, barrier, copy-out, barrier sequence; its tiling header sets `kRowsPerBlock=2`. This supports treating H08 as cross-row descriptor batching while retaining the r3 mapping.
- `phase4/local/MODE-X-R015C/R015C-r4/submission.asc:14-50` sets `rowsThisBlock=1` and retains per-segment copy/barrier order; the r4 tiling header sets `kScratchRows=2` and `kRowsPerBlock=1`. The complete-row staging in H09 is therefore a research proposal, not current r4 behavior.
- H10's wrapper boundary is visible in `phase4/workspaces/MODE-X-R015C/support/paired_runner/op_kernel/candidate_kernel.asc.in:6-12` and `phase4/workspaces/MODE-X-R015C/support/paired_runner/op_host/paired_runner.asc:166-176`: the Candidate wrapper forwards a tiling pointer, while `Launch` discards `cols`. Host-only ABI coverage therefore does not establish scalar delivery to the device kernel.
- This pass read only MODE-X-R015C sources and the canonical shared status rows. It ran no build, runner, device command, correctness case, or timing sample. The three existing falsification tests remain the next evidence required; hardware stride behavior, asynchronous completion, scalar ABI delivery, and any benefit still require their explicitly listed later checks.

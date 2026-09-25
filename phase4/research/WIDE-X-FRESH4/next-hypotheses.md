# WIDE-X-FRESH4 Next Hypotheses

Research only. V001 remains the current Candidate and is unchanged. No later performance revision is opened here. Shape expectations below come from the operator domain and this Route's source; the official scored-shape distribution is not present in Route-local evidence.

## Provenance boundary

- The retained V001 runner note says an archived problem-analysis document containing material beyond task semantics was opened while locating the problem statement.
- Route-local records do not identify its exact path or the architecture/performance details it exposed, and do not say whether that exposure influenced the V001 tile-width idea or source. No archive was reopened for this research round.
- Fresh Blind provenance remains unconfirmed for Main to decide. This round's screening uses only WIDE-X-FRESH4 V001 source and its own build/correctness evidence, plus public Ascend C API and hardware facts.
- Candidate identity remains `f7628795e6699288669dbff8963181e376e10741ba4d46c768af51cf09bf6895`.

Hardware/API basis: local correctness records identify server device 4 as 910B3; the target compile architecture is DAV_2201 with approximately 192 KiB UB. The current kernel maps one block to a row, uses 4096-element wide tiles, stores float conversion buffers separately, and allocates its five TQue instances with one buffer each. Wide rows are scanned once for squared-sum reduction and again for output.

## H1: Native-width input queues

- STATUS: NEEDS_MORE_EVIDENCE
- MECHANISM: Size `xQueue_`, `rQueue_`, `gQueue_`, and `bQueue_` for `TILE * sizeof(T)`; retain the separate float conversion buffers.
- BOTTLENECK: UB reservation for raw FP16/BF16 inputs is currently sized as though each source element occupied four bytes.
- EXPECTED_SHAPES: FP16/BF16, D=16384 or 32768; smaller D is a lower-priority case.
- WHY_IT_MAY_HELP: At TILE=4096 this can release 32 KiB of nominal queue space for half-width inputs while leaving the FP32 arithmetic path unchanged.
- WHY_IT_MAY_FAIL: The compiler or queue allocator may reserve memory differently; the freed space may not change active work or kernel latency.
- ASCEND_FEASIBILITY: TQue tensors retain element type T and copy lengths use `sizeof(T)`; conversion to float uses distinct `xFloat_`, `rFloat_`, `gFloat_`, and `bFloat_` buffers.
- UB/CORE/DMA_IMPACT: Lower per-block UB reservation for half types; no change to tile count, core mapping, or DMA bytes.
- SYNC_IMPACT: None intended.
- PRECISION_RISK: Low if queue capacity remains at least the copied byte count and the float conversion path is unchanged.
- DUPLICATE_CHECK: Different from V001's tile-width and `tmp_` capacity change. No other Route implementation was consulted.
- MINIMAL_OFAT_DIFF: Change only the four raw-input queue allocation expressions from float-sized to T-sized elements.
- EXPECTED_LOCAL_PROBES: Compile/link; all nine existing dtype/width correctness cases; then exact-shape same-binary noise-floor qualification before any Main-authorized P/C.

## H2: Retain tile reduction partials in UB

- STATUS: NEEDS_MORE_EVIDENCE
- MECHANISM: Store each wide-tile sum in an aligned local partial-sum vector, then reduce those partials once after the input scan instead of reading and adding one scalar after every tile.
- BOTTLENECK: The wide path has a loop-carried scalar accumulation and one scalar extraction per tile; D=32768 has eight tiles at TILE=4096.
- EXPECTED_SHAPES: Wide path D=16384 and 32768, especially rows-per-block near one.
- WHY_IT_MAY_HELP: A local final reduction may shorten the serial dependency chain across tile iterations.
- WHY_IT_MAY_FAIL: The extra local reduction can cost more than a few scalar additions; the compiler may serialize the same work.
- ASCEND_FEASIBILITY: FP32 ReduceSum destinations require 8-byte alignment. Eight partials in every-other-float slots need 64 bytes including padding; the current 32-byte scalar allocation is insufficient for that layout. Verify supported LocalTensor offset/slice access and the final local reduction in a compile probe.
- UB/CORE/DMA_IMPACT: A 64-byte partial area (32 bytes above the current scalar allocation), with no global traffic or core-count change.
- SYNC_IMPACT: No cross-core synchronization; one local reduction is added after the first scan.
- PRECISION_RISK: Medium because the summation order changes; verify FP32 and BF16 at maximum width against the Route tolerances.
- DUPLICATE_CHECK: Distinct from V001's tile size change; built only from this Route's current reduction loop and documented ReduceSum constraints.
- MINIMAL_OFAT_DIFF: Change only placement/storage of per-tile scalar partials and the required local buffer capacity; preserve tile width and output pass.
- EXPECTED_LOCAL_PROBES: CANN compile/link; 9-case correctness matrix with repeated maximum-width inputs; exact-shape same-binary noise-floor qualification before any Main-authorized P/C.

## H3: Split a very wide row across vector cores

- STATUS: NEEDS_MORE_EVIDENCE
- MECHANISM: Add a partial-sum stage over column ranges, followed by an output stage whose blocks independently combine the row partials and write disjoint column ranges.
- BOTTLENECK: Current launch count is `min(rows, availableCoreNum)` and each block walks its assigned row serially across all wide tiles.
- EXPECTED_SHAPES: D=16384 or 32768 with rows substantially below the 910B3 vector-core count; shape eligibility remains unconfirmed.
- WHY_IT_MAY_HELP: More cores can cooperate on a small number of long rows instead of leaving most vector cores idle.
- WHY_IT_MAY_FAIL: An extra launch, workspace traffic, and repeated partial-sum reads may exceed the saved per-row work; small widths are poor candidates.
- ASCEND_FEASIBILITY: Requires a new workspace path and two ordered kernel launches through the Route host wrapper; no global cross-core barrier is assumed inside one kernel.
- UB/CORE/DMA_IMPACT: Small workspace proportional to rows times column partitions; more active cores and additional global-memory traffic.
- SYNC_IMPACT: Kernel boundary orders partial generation before output; no device-wide in-kernel barrier.
- PRECISION_RISK: Medium because partial grouping changes FP32 accumulation order; test all dtypes at D=32768.
- DUPLICATE_CHECK: Different from V001's per-row tile-width change; no source from another Route was read.
- MINIMAL_OFAT_DIFF: Change only row-to-core decomposition and the partial-sum workspace/second launch required by that decomposition.
- EXPECTED_LOCAL_PROBES: Verify one-row and two-row cases plus 2D/3D/4D flattening; confirm parent and Candidate correctness for every compared case; only then seek shape-specific same-binary qualification and Main-authorized P/C.

## H4: Output-only TQue double buffer

- STATUS: NEEDS_MORE_EVIDENCE
- MECHANISM: Double-buffer `outQueue_` and schedule the next output tile's vector work while the prior tile's MTE3 copy is in flight.
- BOTTLENECK: The output pass serializes vector arithmetic and the per-tile output copy with a single output buffer.
- EXPECTED_SHAPES: FP16/BF16 wide rows with at least four output tiles; FP32 is excluded from the first feasibility probe because the estimated UB reservation reaches the 192 KiB limit.
- WHY_IT_MAY_HELP: It can overlap MTE3 with vector work across adjacent tiles without changing arithmetic or input traffic.
- WHY_IT_MAY_FAIL: The output copy may be too short to hide; queue scheduling may retain a dependency; double buffering adds live UB.
- ASCEND_FEASIBILITY: A2/A3 TQue supports `InitBuffer(..., 2, size)`; double-buffer only this one TQue, within the documented limit of four double-buffered queues.
- UB/CORE/DMA_IMPACT: Adds one output tile buffer; no core mapping or DMA byte-count change.
- SYNC_IMPACT: Queue ownership and final-tile drain must be explicit; no cross-core sync is introduced.
- PRECISION_RISK: Low if output order and arithmetic remain unchanged.
- DUPLICATE_CHECK: Separate from V001's tile-width change and H1's raw-input allocation sizing; this changes only output transfer overlap.
- MINIMAL_OFAT_DIFF: Change only output queue depth/allocation and the producer-consumer order needed to overlap adjacent tile output copies.
- EXPECTED_LOCAL_PROBES: Compile/link; all nine existing correctness cases; verify tail-tile handling; exact-shape same-binary noise-floor qualification before any Main-authorized P/C.

## Track-B Screen Round 2026-09-26

Research only. No source was changed, no new performance revision was opened, and no runner, correctness executable, or timing process was run. The local CANN API index lists `Rsqrt`, but the workstation has no `ASC_DEVKIT_DIR`; exact CANN 8.5.0.alpha002 and DAV_2201 support remains unverified.

## H5: Reuse gamma/bias tiles across a small row group

- STATUS: NEEDS_MORE_EVIDENCE
- MECHANISM: For a small fixed group of rows assigned to one block, compute and retain each row's inverse RMS scalar first; in the output pass, make tile position the outer loop, load one gamma/bias tile, and reuse it while producing that tile for every row in the group.
- BOTTLENECK: The current row-outer output pass reloads the same gamma and bias vectors for every row and tile.
- EXPECTED_SHAPES: Wide rows where one block owns at least two rows; official scored-shape eligibility is unknown.
- WHY_IT_MAY_HELP: Reuse can reduce repeated gamma/bias GM-to-UB transfers and queue operations without changing per-element arithmetic or adding global workspace.
- WHY_IT_MAY_FAIL: Gamma/bias may already be served efficiently from cache; row-group scheduling and scalar retention may add control work, and there is no benefit when a block owns one row.
- ASCEND_FEASIBILITY: Can use the existing single-buffer gamma/bias queues and float tile buffers. A fixed small group needs bounded per-row inverse-RMS scalars and a tail path for incomplete groups; compile feasibility is unverified.
- UB/CORE/DMA_IMPACT: A few scalars of extra live state; no extra cores or workspace. Gamma/bias tile transfers per group may fall from one per row to one per group; x/residual and output traffic stay unchanged.
- SYNC_IMPACT: Keep the existing EnQue/DeQue ownership order; no cross-core synchronization.
- PRECISION_RISK: Low if each row keeps its existing reduction order and inverse-RMS value.
- DUPLICATE_CHECK: Distinct from H3, which splits one row across cores, and H4, which overlaps output DMA with vector work. This changes row traversal to reuse read-only parameters.
- MINIMAL_OFAT_DIFF: Reorder only the second pass over a bounded row group and hold one gamma/bias tile across that group; keep TILE, reduction, and arithmetic unchanged.
- EXPECTED_LOCAL_PROBES: Confirm an eligible shape from Route-owned task evidence; compile; run the existing dtype/width correctness cases plus multi-row-per-block and incomplete-group cases; only consider same-binary then paired measurement after Main resolves provenance and issues a device lease.

## H6: Scalar reciprocal-square-root path

- STATUS: NEEDS_MORE_EVIDENCE
- MECHANISM: Replace `Sqrt(rmsInput)` followed by scalar reciprocal with a supported `Rsqrt(rmsInput)` result, retaining the existing sum, width division, and epsilon addition.
- BOTTLENECK: One scalar RMS normalization per row currently uses a square-root operation and a separate reciprocal operation.
- EXPECTED_SHAPES: Any supported width with many rows; relative opportunity is uncertain because the scalar work is small beside the tile loops.
- WHY_IT_MAY_HELP: A reciprocal-square-root instruction could combine the two scalar operations and shorten the per-row dependency chain.
- WHY_IT_MAY_FAIL: The compiler may already lower the scalar reciprocal efficiently; `Rsqrt` may have different latency or approximation error, and its exact CANN 8.5.0.alpha002/DAV_2201 availability was not confirmed from the local API index.
- ASCEND_FEASIBILITY: The local API index names `Rsqrt`, but does not establish this toolkit/architecture combination. Verify the versioned public API declaration and constraints before any implementation.
- UB/CORE/DMA_IMPACT: No intended buffer, core mapping, or DMA change; reuses the existing scalar-sized temporary storage.
- SYNC_IMPACT: The scalar result is still read after its vector operation; no new cross-core synchronization.
- PRECISION_RISK: Medium; reciprocal-square-root approximation may change output error, especially for small positive variance plus epsilon.
- DUPLICATE_CHECK: Distinct from H2's partial-reduction layout and V001's tile-width change; it targets only final scalar normalization.
- MINIMAL_OFAT_DIFF: Replace only the square-root-plus-reciprocal expression after API/version support is confirmed.
- EXPECTED_LOCAL_PROBES: Verify exact API support; compile/link; run all nine existing correctness cases plus small-positive and zero-variance inputs within task semantics; same-binary and paired measurement only after provenance review and an explicit lease.

## H7: Aligned wide-path output copy

- STATUS: NEEDS_MORE_EVIDENCE
- MECHANISM: For complete 4096-element output tiles on the wide path, use a direct aligned GM/UB copy instead of `DataCopyPad`; retain the padded path for any shape or tail that does not meet the verified alignment requirements.
- BOTTLENECK: The current output loop constructs and submits a padded-copy descriptor for every tile, although supported wide widths are multiples of the tile size.
- EXPECTED_SHAPES: FP32, FP16, and BF16 at widths 16384 and 32768; these produce only full wide tiles.
- WHY_IT_MAY_HELP: If the direct-copy path has lower setup cost on this CANN build, it may reduce per-tile output-transfer overhead without changing arithmetic.
- WHY_IT_MAY_FAIL: The compiler/runtime may lower both forms equivalently, making the change neutral; the benefit may be below measurement resolution for wide tiles.
- ASCEND_FEASIBILITY: Route-local wide shapes make each output address offset and transfer length a multiple of 512 bytes. The local API references disagree on the direct-copy alignment threshold and do not establish queued LocalTensor base alignment for CANN 8.5.0.alpha002; verify the installed public restriction before use.
- UB/CORE/DMA_IMPACT: No intended UB, core, or byte-count change.
- SYNC_IMPACT: Keep the existing output queue DeQue/FreeTensor sequence; no new synchronization.
- PRECISION_RISK: None intended because values and transfer lengths are unchanged; correctness still must cover all dtypes.
- DUPLICATE_CHECK: Distinct from H4's double-buffer overlap: this changes only the copy API for already aligned full tiles.
- MINIMAL_OFAT_DIFF: Select the direct-copy API only for verified aligned wide-path tiles; retain `DataCopyPad` for fallback and any unaligned tail.
- EXPECTED_LOCAL_PROBES: Confirm the installed CANN 8.5.0.alpha002 alignment/API rules; compile/link; run the existing nine-case correctness matrix; same-binary then paired measurement only after Main resolves provenance and grants a lease.

## H8: Double-buffer x/residual input queues

- STATUS: INFEASIBLE
- MECHANISM: Double-buffer the x and residual VECIN queues and prefetch the next tile while the current tile is processed.
- BOTTLENECK: Input DMA and vector work are currently serialized at each tile by immediate DeQue in `LoadInput`.
- EXPECTED_SHAPES: Wide path D=16384 or 32768; no shape can justify proceeding until the UB budget fits.
- WHY_IT_MAY_HELP: It could overlap MTE2 input transfers with vector work across adjacent tiles.
- WHY_IT_MAY_FAIL: Under current `InitBuffer` sizes, the nominal FP32 allocation is 176 KiB plus 32 bytes; double-buffering two input queues adds 32 KiB, exceeding the 192 KiB UB budget before alignment or allocator overhead. FP16/BF16 still allocate float-sized raw input queues and also exceed the budget.
- ASCEND_FEASIBILITY: `InitBuffer(..., 2, size)` is a documented double-buffer form, but this isolated change does not fit the Route's current declared UB allocation. Reconsider only if independent Route-local evidence establishes a non-combined way to reclaim sufficient UB.
- UB/CORE/DMA_IMPACT: Adds 32 KiB nominal UB; same core mapping and transfer byte count. Current resource estimate exceeds target UB.
- SYNC_IMPACT: Requires explicit prefetch/compute queue ownership and drain of final in-flight input tiles; no cross-core synchronization.
- PRECISION_RISK: Low if input values and arithmetic order stay unchanged.
- DUPLICATE_CHECK: Distinct from H4's output-side MTE3 buffering; this uses MTE2 input queues. Screened out as a standalone probe due to current UB capacity, not folded into H1 or H4.
- MINIMAL_OFAT_DIFF: Double-buffer only x and residual queues and schedule next-tile prefetch; no other buffer sizing or tile changes belong in that hypothesis.
- EXPECTED_LOCAL_PROBES: No implementation or device probe in the current state. Revisit only with Route-owned resource evidence showing the isolated allocation fits, then compile and run correctness before any lease-qualified measurement.

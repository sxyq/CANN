# WIDE-X-FRESH4 Next Hypotheses

Research only. V001 remains the current Candidate and is unchanged. No later performance revision is opened here. Shape expectations below come from the operator domain and this Route's source; the official scored-shape distribution is not present in Route-local evidence.

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

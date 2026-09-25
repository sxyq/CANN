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
- DUPLICATE_CHECK: V001 changes aligned `DataCopy` versus `Adds` inside the shared helpers. Source dispatch review found no same-type helper-copy call site in the active FP32 paths, so this proposal has no current edit target.
- MINIMAL_OFAT_DIFF: One FP32 call-site family, with a compile-time FP32 path and no arithmetic or scheduling edits.
- EXPECTED_LOCAL_PROBES: Compile/link; CPU/static alias cases; then exact-shape correctness before any measurement.
- READINESS: DUPLICATE.

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
- BOTTLENECK: widths 65 and 127 miss the aligned batch predicate and use the generic row path; width 129 also misses it but enters `ProcessNarrowMidOverlap` because it is greater than `kMidRowMinWidth` (128). All three bypass the aligned contiguous batch path because their widths are not multiples of `kSmallFp32ScalarStride`.
- EXPECTED_SHAPES: Widths 65, 127, and 129 across all three prefixes.
- WHY_IT_MAY_HELP: Grouping independent rows may amortize scalar V/S handoffs without changing the row reduction formula.
- WHY_IT_MAY_FAIL: Padding can add UB traffic, and the task has only 12 rows, limiting the group size on some cores. At width 129, a replacement batch path may lose the existing input/parameter DMA overlap.
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
- READINESS: INFEASIBLE.

## DTYPE-FP32-06: Batch Aligned Small-Row Reductions with Pattern AR

- MECHANISM: In the existing aligned small-row FP32 batch path, replace the per-row Level 2 `ReduceSum` loop with one Pattern AR `ReduceSum` call that reduces the last dimension independently for each row in the already contiguous batch.
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

## Track-B Cross-Route Screen (2026-09-26)

This pass selects four existing studies for the next Main review. Their mechanisms and OFAT boundaries are unchanged. Cross-route labels use only the permitted Main-1 scheduler summaries; no other Route source was inspected. `ADJACENT_DIFFERENT_MECHANISM` means related traffic or arithmetic, with no direct mechanism match established. `UNRESOLVED` means the permitted summary does not expose enough detail to classify overlap.

| ID | Cross-route overlap label | Expected information gain | Likely global upside | Screen |
|---|---|---|---|---|
| DTYPE-FP32-01 | `DUPLICATE`: V001's actual FP32 dispatch has no same-type helper-copy call site, so there is no change to compare with other Routes. R31B details remain `UNRESOLVED` in its permitted summary. | LOW: source-path review resolved the proposed call-site target without a device run. | NONE unless a future FP32 dispatch introduces such a helper call. | DUPLICATE |
| DTYPE-FP32-02 | `NO_DIRECT_MATCH_IN_VISIBLE_SUMMARIES`: this changes only full-tile GM output-store form. MODE-X-R015C's summary says its segment/copy form stays unchanged; R31A and MIX-A concern other mechanisms. R31B is `UNRESOLVED`. | MEDIUM: determine whether the aligned output overload lowers differently and retains correct tail separation. | MEDIUM on aligned widths; none for tails or if both APIs lower identically. | NEEDS_MORE_EVIDENCE |
| DTYPE-FP32-04 | `NO_DIRECT_MATCH_IN_VISIBLE_SUMMARIES`: this fuses only the output affine multiply-add. R31A fence placement, MIX-A synchronization removal, and MODE-X-R015C row/copy handling do not describe the same operation. R31B is `UNRESOLVED`. | HIGH: establish whether CANN emits a fused instruction and quantify the rounding difference against task tolerance. | MEDIUM-HIGH if vector issue is material across several widths; low if fusion is unavailable or arithmetic is hidden by reduction. | NEEDS_MORE_EVIDENCE |
| DTYPE-FP32-06 | `ADJACENT_DIFFERENT_MECHANISM`: parked REDUCE-X covers reduction/V-S handoff redesign; this study changes only the reduction call granularity inside the existing aligned two-row FP32 batch. MODE-X-R015C's row/copy path differs. R31B is `UNRESOLVED`. | MEDIUM-HIGH: resolve Pattern AR API/buffer feasibility and whether one call beats two short row reductions. | LOW-MEDIUM because only two-row groups benefit and the 12-row/8-block split leaves four single-row blocks. | NEEDS_MORE_EVIDENCE |

No candidate, kernel, runner, or revision was changed by this screen.

## Source/API Evidence Triage (2026-09-26)

This pass used the DTYPE-SPECIAL-X V001 source and build outputs, its API evidence, local Ascend C references, and the permitted scheduler summaries. No accelerator program or timing workload was run.

### Removed from the active shortlist

- DTYPE-FP32-01: `DUPLICATE`. The FP32 dispatch in `Process` selects `ProcessWideFp32`, `ProcessFp32FullRowOutputPipelined`, the two `ProcessSmallFp32*` paths, `ProcessNarrowMidOverlap`, or the direct-FP32 generic branches. Those paths operate on `LocalTensor<float>` directly. The `ToFloat`/`FromFloat` uses shown by the source are in low-precision branches. The existing FP32 dispatch therefore has no same-type helper copy at a call site to remove. Minimum falsifier: a source-level call-path change that exposes an FP32 `ToFloat`/`FromFloat` call; absent that, no code experiment is warranted.
- DTYPE-FP32-05: `INFEASIBLE`. The V001 source states that scalar device math is unavailable on this Vector Core target and uses vector `Sqrt` on a one-element tensor. The local API restriction reference disallows C++ standard-library math in kernel code, and its `Sqrt` interface is tensor-based. No supported scalar square-root API for this target is evidenced. Minimum falsifier: a local CANN 8.5 target API declaration that accepts and computes a scalar square root on the Vector Core; without it, drop this direction.
- DTYPE-FP32-02: retain only as low priority. The local DataCopy reference allows aligned `DataCopy`, but says aligned-path performance differences from `DataCopyPad` are negligible and requires 32-byte alignment. The retained V001 build directory contains ELF executables with an `.aicore_binary` section but no disassembly or generated source. Minimum falsifier: target code-generation evidence showing a distinct, lower-cost output-store sequence for the aligned full-tile case. Do not spend a revision without that evidence.

### Ranked remaining directions

The detailed mechanism, feasibility, resource, synchronization, and precision notes remain in DTYPE-FP32-03, DTYPE-FP32-04, DTYPE-FP32-06, and DTYPE-FP32-07 above. The ranking below reflects information value and a cheap first falsifier, not a performance claim.

| Rank | ID and task shapes | Bottleneck and mechanism | UB/core/DMA/sync and precision risk | Cross-Route overlap | Minimum falsifier; information and global upside |
|---|---|---|---|---|---|
| 1 | DTYPE-FP32-06; prefixes `[12]`, `[3,4]`, `[1,2,6]` × widths 64, 128, 1024 | `ProcessSmallFp32ContiguousBatched` forms contiguous row batches but calls Level 2 `ReduceSum` once per row. Replace only the two-row batch reduction with Pattern AR, which reduces the last dimension independently for each row. | Needs Pattern temporary UB and the required aligned row shape; no intended GM traffic or core-map change. Keep the result-ready point before scalar reads. Reduction order can change, so FP32 tolerance must be checked. | `ADJACENT_DIFFERENT_MECHANISM`: scheduler lists REDUCE-X as parked with reduction/V-S redesign; MODE-X-R015C uses one row per block and leaves its copy path unchanged. No exact Pattern-AR overlap appears in the permitted summaries. | First verify the CANN 8.5 `ReduceSum` AR overload, required temporary size, and fit within the current UB budget; missing API or insufficient UB falsifies it before source work. HIGH information gain; MEDIUM upside, limited to two-row groups. A speed claim would still require exact-shape lease-backed measurement. |
| 2 | DTYPE-FP32-07; prefixes `[12]`, `[3,4]`, `[1,2,6]` × widths 64, 128, 1024 | Eight blocks split 12 rows into four two-row and four one-row blocks. Cap only this FP32 case at four blocks so every block receives three rows and can use the existing batch path. | No extra UB or DMA; active blocks halve, which can reduce parallel throughput. No new synchronization. Assignment alone has no expected precision change. | `ADJACENT_DIFFERENT_MECHANISM`: MODE-X-R015C's approved summary names one row per block; this changes the block cap and within-block batch size, not its segment/copy path. | First model row coverage and confirm each width reaches the existing batch with 3 rows and no uncovered row; reject if the batch limit or dispatch prevents that. MEDIUM information gain; MEDIUM upside with a clear parallelism risk. Performance remains untested. |
| 3 | DTYPE-FP32-03; prefixes `[12]`, `[3,4]`, `[1,2,6]` × widths 65, 127, 129 | The aligned batch predicate excludes these widths; 65/127 use the generic row path and 129 uses `ProcessNarrowMidOverlap`. Add one irregular-width row-batch path with padded local row pitch and valid-element tails. | More padded UB and possible input staging; keep block mapping and row arithmetic unchanged. Preserve per-row reduction/output order and current dependence barriers. Tail and padding errors are the main precision/correctness risk. | `ADJACENT_DIFFERENT_MECHANISM`: MODE-X-R015C's summary concerns row ownership and unchanged copy handling; this is a DTYPE-width dispatch change. For 129 it may replace an existing DMA-overlap path, so it is not assumed additive. | First build a host-side layout model for pitches 72/128/136 FP32 elements and prove each source row's valid range and 32-byte UB row starts; reject on cross-row read, misaligned local start, or UB overflow. MEDIUM information gain; LOW-MEDIUM upside because only three widths qualify and 129 may lose overlap. |

DTYPE-FP32-04 remains unranked: the local API index and references provide no vector FMA declaration, while the retained executable has no readable code-generation listing. This absence does not establish that CANN 8.5 lacks such an API. Its first falsifier is a target-specific API/signature and generated-instruction search before any source edit. Cross-Route labels in this section use scheduler summaries only; no other Route source was opened.

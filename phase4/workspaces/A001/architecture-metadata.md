# A001 Architecture Metadata

## candidate

`A001`

## architecture hypothesis

Row-resident fused streaming on DAV_2201. A Vector Core owns complete rows and
streams the last dimension through a bounded UB tile. The RMS statistic is
accumulated in FP32 on the owning core, then the row is reread for the fused
normalization and bias epilogue.

## core mapping

The host selects `blockNum = min(R, vector_core_count)` and assigns contiguous
row ranges. Each block receives `firstRow` and `rowsThisCore`; no block reads or
writes another block's rows.

## blockDim strategy

`blockDim` equals the number of participating Vector Cores. The normal target is
40 on Ascend910B3, reduced to `R` for small batches. The same value is passed in
the tiling record as `blockNum`.

## row ownership

One Vector Core exclusively owns each complete row. Row ownership is contiguous
and the tail row range is assigned to the final participating block.

## D ownership

The owning core processes all `D` elements of its row. D is never divided across
cores. A chunk is only a local UB streaming unit.

## UB allocation estimate

The queues are double buffered. For FP16/BF16 with a 2048-element chunk:

```text
input/output queues = 10 * 2048 * sizeof(T) = 40,960 B
eight FP32 work buffers = 8 * 2048 * 4       = 65,536 B
scalar buffers                                      64 B
fixed planning reserve                            2,048 B
estimated total                               108,608 B
```

For FP32 with a 1024-element chunk:

```text
input/output queues = 10 * 1024 * 4          = 40,960 B
eight FP32 work buffers = 8 * 1024 * 4       = 32,768 B
scalar buffers                                      64 B
fixed planning reserve                            2,048 B
estimated total                                75,840 B
```

Both estimates are below the 184 KiB candidate workspace budget. Every transfer
uses 32B-aware padding, so the estimate includes the fixed alignment reserve.

## TQue allocation

`x`, `residual`, `gamma`, and `bias` each use a VECIN queue with two slots.
`output` uses a VECOUT queue with two slots. FP32 conversion, fused arithmetic,
square accumulation, reduction scratch, and scalar state use reusable VECCALC
buffers.

## parameter residency lifetime

Gamma and bias are loaded for one D chunk during the output pass, consumed by the
fused epilogue, then released before the next chunk. They are reused across all
rows owned by the same core through the same queue allocation.

## reduction topology

Each chunk computes `u = x + residual`, squares `u`, and applies FP32
`ReduceSum`. Chunk scalars are accumulated into one FP32 row scalar, followed by
mean, epsilon addition, and square root. There is no cross-core reduction.

## cross-core synchronization strategy

None. Each core owns independent rows and output addresses.

## cross-core synchronization count

`0`

## expected x rereads

`1` extra row pass: one pass forms the RMS statistic and one pass writes the
normalized output.

## expected residual rereads

`1` extra row pass, for the same two-pass row lifecycle.

## expected gamma reload groups

One group per output-pass D chunk, with `ceil(D / chunkElements)` groups per
row. The hot path uses one group when `D <= 1024`.

## expected bias reload groups

One group per output-pass D chunk, matching gamma.

## workspace usage

The implementation reserves at most 108,608 B for FP16/BF16 and 75,840 B for
FP32, including a 2,048 B fixed planning reserve. The candidate therefore stays
within the 184 KiB Vector workspace after queues, temporaries, reduction state,
and alignment loss.

## hot-path domain

FP16, BF16, or FP32; rank 2D/3D/4D flattened to `R` rows; `64 <= D <= 1024`.
The path also accepts non-32B-aligned D, tail rows, NaN/Inf, and any legal
epsilon because GM transfers use `DataCopyPad` and arithmetic remains FP32.

## hot-path dispatch condition

`tiling.d <= 1024`

## fallback domain

All remaining legal rows and D values, including `1025 <= D <= 32768`,
non-32B-aligned D and the final partial chunk.

## fallback dispatch condition

`tiling.d > 1024`

## A001-V002 focused update

- Evidence: `phase4/online/A001/result.json` testcase 14 took 118093.53 us,
  which is 87.76% of the 15-case total. The update targets the streamed
  large-D path while keeping row ownership and arithmetic unchanged.
- Change: `submission_v002.asc` raises the half/bfloat16 chunk from 2048 to
  3072 elements and the FP32 chunk from 1024 to 2048 elements.
- UB estimate for half/bfloat16: 10 queue slots × 3072 × 2 plus eight FP32
  work buffers × 3072 × 4, scalar state, and the 2048 B reserve = 161856 B.
- UB estimate for FP32: 10 queue slots × 2048 × 4 plus eight FP32 work
  buffers × 2048 × 4, scalar state, and the 2048 B reserve = 149568 B.
- Tail handling remains on `DataCopyPad`; all legal dtypes, rows, D tails,
  and epsilon values retain the existing dispatch and fallback behavior.
- Server compile target: Ascend910B3 / `dav-2201` with CANN
  `8.5.0.alpha002`; target `a001_submission_v002` passed.

## A001-V003 focused update

- Evidence: the V002 result reports 124949.87 us for testcase 14 and
  12187.19 us for testcase 15. Testcase 14 remains the dominant long-D
  latency case.
- Change: `submission_v003.asc` dispatches complete 32B-aligned GM transfers
  through `DataCopy`; partial or unaligned row/chunk transfers retain
  `DataCopyPad`. The alignment predicate covers both byte length and source or
  destination element offset.
- Scope: row ownership, D ownership, FP32 reduction, dtype handling, and the
  3072-element half/bfloat16 or 2048-element FP32 stream chunks are unchanged.
- Server compile target: Ascend910B3 / `dav-2201` with CANN
  `8.5.0.alpha002`; target `a001_submission_v003` passed. Logs:
  `build/configure-v003.log` and `build/compile-v003.log`.

## A001-V004 focused update

- Evidence: V003 measured testcase 14 at `118631.29 us` and testcase 15 at
  `11909.16 us`; testcase 14 was `89.17%` of the measured total.
- Change: the output of the x conversion now goes directly into the existing
  `u` buffer, removing the separate FP32 `xFloat_` buffer and its allocation.
  The freed UB capacity allows streamed chunks of 3584 half/bfloat16 elements
  or 2560 FP32 elements, reducing long-row chunk iterations.
- Scope: the row-resident mapping, complete-row ownership, FP32 reduction,
  aligned `DataCopy`/tail `DataCopyPad` dispatch, and submission ABI remain
  unchanged.
- Server compile target: Ascend910B3 / `dav-2201` with CANN
  `8.5.0.alpha002`; target `a001_submission_v004` passed. Logs:
  `build/configure-v004.log` and `build/compile-v004.log`.

## A001-V005 focused update

- Evidence: the online result stayed at 15/15 and `25.97`; testcase 14 was
  `124365 us` and testcase 15 was `11955 us`. The reciprocal-hoisting change
  did not show a confirmed latency improvement over the prior control.
- Change: `submission_v005.asc` computes one row-level reciprocal after the
  RMS square root and reuses it for all output chunks.
- Scope: chunk sizes, queue allocation, row ownership, D ownership, transfer
  selection, arithmetic precision, and the submission ABI are unchanged.
- Server compile target: Ascend910B3 / `dav-2201` with CANN
  `8.5.0.alpha002`; target `a001_submission_v005` passed. Logs:
  `build/configure-v005.log` and `build/compile-v005.log`.

## A001-V006 focused update

- Hypothesis: the V005 streamed implementation immediately dequeues each
  transfer, so double-buffer slots do not overlap MTE movement with vector
  work. Single-slot queues should recover workspace for fewer long-row tile
  iterations without changing the row or D ownership model.
- Change: `submission_v006.asc` uses one slot for the x, residual, gamma, bias,
  and output queues, and raises the streamed chunk to `4096` half/bfloat16 or
  `3072` FP32 elements.
- UB estimate: approximately `174144 B` for half/bfloat16 and `161856 B` for
  FP32, including scalar state and the existing planning reserve.
- Scope: the hot path, row ownership, FP32 reduction, aligned `DataCopy`/tail
  `DataCopyPad` dispatch, dtype handling, and submission ABI are unchanged.
- Server compile target: Ascend910B3 / `dav-2201` with CANN
  `8.5.0.alpha002`; target `a001_submission_v006` passed. Logs:
  `build/configure-v006.log` and `build/compile-v006.log`.
- Online validation: 15/15, official score `27.30`; testcase 14 was
  `125075.65 us` and testcase 15 was `10677.60 us`.

## A001-V007 focused update

- Hypothesis: the V006 streamed path keeps an extra FP32 `xFloat_` buffer even
  though `u` is consumed immediately by the add. Converting `x` directly into
  `u` should reduce local buffer traffic while preserving the V006 chunk plan.
- Change: `submission_v007.asc` removes `xFloat_` allocation and performs
  `ToFloat(u, xSrc, count)` followed by the in-place `Add(u, u, residualFloat,
  count)`.
- UB estimate: approximately `157760 B` for half/bfloat16 and `149568 B` for
  FP32, including scalar state and the existing planning reserve.
- Scope: the V006 single-slot queues, `4096` half/bfloat16 chunk, `3072` FP32
  chunk, row ownership, reduction order, transfer selection, dtype handling,
  and submission ABI remain unchanged.
- Server compile target: Ascend910B3 / `dav-2201` with CANN
  `8.5.0.alpha002`; target `a001_submission_v007` passed. Logs:
  `build/configure-v007.log` and `build/compile-v007.log`.

- Online validation: 15/15, official score `27.24`; testcase 14 was
  `124964.64 us` and testcase 15 was `10653.88 us`.

## A001-V008 focused update

- Hypothesis: V007 still allocates a separate FP32 residual staging buffer,
  although `squareFloat_` is idle until `LoadU` returns. Reusing that buffer
  should recover workspace for larger streamed tiles without changing the
  reduction order.
- Change: `submission_v008.asc` removes `residualFloat_`, uses
  `squareFloat_` for residual conversion inside `LoadU`, and raises the
  streamed chunk to `5120` half/bfloat16 or `4096` FP32 elements.
- UB estimate: approximately `176192 B` for half/bfloat16 and `182336 B` for
  FP32, including scalar state and the existing planning reserve.
- Scope: the V007 single-slot queues, hot-path dispatch, row ownership, FP32
  arithmetic, transfer selection, dtype handling, and submission ABI remain
  unchanged.
- Server compile target: Ascend910B3 / `dav-2201` with CANN
  `8.5.0.alpha002`; target `a001_submission_v008` passed. Logs:
  `build/configure-v008.log` and `build/compile-v008.log`.

- Online validation: 15/15, official score `28.10`; testcase 14 was
  `118183.70 us` and testcase 15 was `11402.05 us`.

## A001-V009 focused update

- Hypothesis: the wide-D FP32 case spends a full second pass rereading
  `x` and `residual` and rebuilding `u`. Retaining exact FP32 `u` tiles in
  output GM during the first pass should remove those reads and the repeated
  conversion/addition from the second pass.
- Change: `submission_v009.asc` adds a compile-time retained-`u` path selected
  for FP32 inputs with `D >= 16384`. It stores `u` through the existing output
  queue, loads it through the x input queue, then runs the unchanged output
  epilogue. Other dtypes and shapes use the V008 path.
- Scope: FP32 arithmetic order, row ownership, chunk sizes, transfer alignment
  handling, dtype fallback behavior, and submission ABI remain unchanged.
- Server compile target: Ascend910B3 / `dav-2201` with CANN
  `8.5.0.alpha002`; target `a001_submission_v009` passed. Logs:
  `build/configure-v009.log` and `build/compile-v009.log`.

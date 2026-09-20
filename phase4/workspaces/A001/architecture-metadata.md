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


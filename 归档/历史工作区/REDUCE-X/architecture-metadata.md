# REDUCE-X Architecture Metadata

Route: REDUCE-X
Revision: V001
Target: Ascend 910B3 / DAV_2201 / CANN 8.5.0.alpha002 (server3)

## Hypothesis

The shared safe idiom pays 1 V/S handoff (SyncVToS + GetValue) per row.
Batching multi-row ReduceSum and applying the scale entirely on the Vector
side (Rsqrt + Brcb + zero-stride binary Mul) removes that handoff from the
hot path.

## Mechanisms demonstrated

1. Multi-row simultaneous ReduceSum into packed sum[B].
2. Batched RMS scalars via vector Rsqrt, no GetValue.
3. Brcb 8-wide scale block with BinaryRepeatParams src1BlkStride=0.
4. Hierarchical UB reduction for D larger than single-row residency.
5. Reduce + DMA overlap via TQue depth 2.
6. Epilogue fusion: u * inv * gamma + bias.
7. Rsqrt multiplier instead of Sqrt + Div.
8. Generic fallback with 1 GetValue per row (full coverage).

## Paths

| Path | Trigger | V/S per row |
|------|---------|-------------|
| hot-zero-scalar | B>=1 rows fit with work | 0 |
| wide-hierarchical | D needs chunking | 0 |
| generic-fallback | all else | 1 |

## Changed

V001 first reduction-centric kernel. Fresh design. No historical compute reuse.
V002 fix Brcb 256B workspace overflow + ReduceSum 8B alignment; GetValue golden
for tiny rows; zero-VS Brcb for larger hot batches; hierarchical wide path kept.
V003 strip to textbook GetValue golden two-pass for ALL shapes (no Brcb /
hierarchical / zero-VS). Target 15/15 before any reduction experiment.

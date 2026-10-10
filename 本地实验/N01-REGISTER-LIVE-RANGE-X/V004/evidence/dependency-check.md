# N01 V004 Dependency Record

ROUTE=N01 REGISTER-LIVE-RANGE-X
REVISION=V004
FUNCTION=ProcessWideFp32FullCacheRows
PARENT=本地实验/N01-REGISTER-LIVE-RANGE-X/V003/submission.asc
CANDIDATE=本地实验/N01-REGISTER-LIVE-RANGE-X/V004/submission.asc

## Executable Structure

The first pass keeps the existing row-major and tile-major loops. For every tile, the existing Load, input synchronization, Add, vector barrier, square Mul, vector barrier, partial ReduceSum, and release synchronization now live in one inner scope. The scope ends after `SyncVToMTE2()`.

The per-row post-reduction code remains in row order. Its existing sequence is divided into two adjacent scopes:

1. The reduction scope performs `ReduceSum` into `residualLocal`, `SyncVToS`, reads `squareSum`, and computes `meanSquare`.
2. The normalization scope performs `SyncSToV`, `Duplicate`, `Sqrt`, `PipeBarrier`, `SyncVToS`, reads the reciprocal input, and performs the final `SyncSToV`.

The second scope obtains `normalizationValue` from the same `residualBuf_` storage after the old reduction vector has been consumed by `GetValue(0)`. The scope boundary is the intended FP32 temporary lifetime change. No helper function, new buffer, new event, or alternate execution path was added.

## Dependency Preservation

- `valueLocal` still retains every `x + residual` row at the same row and column offset.
- Every GM Load and Store keeps its original address, count, and order.
- Every vector arithmetic operation keeps its original operands and order.
- `SyncMTE2ToV`, `SyncVToMTE2`, `SyncVToS`, `SyncSToV`, and `PipeBarrier<PIPE_V>` remain in the same per-tile/per-row sequence.
- `xBuf_`, `residualBuf_`, `valueFp32Buf_`, and `reduceFp32Buf_` are unchanged; no UB allocation or ownership rule changes.
- The parameter and output pass is unchanged.

PARENT_SOURCE_SHA256=f355ef215f9e717b3207010ade0893fed34c7d6078bd1f2d0e7dbd7c966c1da0
CANDIDATE_SOURCE_SHA256=b7fdfc0aff03fffd52f72c55f2ed3b9e61db1ab795de4c0e9684834782f012c1
FULL_SOURCE_SIMILARITY=NOT_MEASURED; no similarity result is claimed in V004

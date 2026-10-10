# N01 V003 Dependency Record

ROUTE=N01 REGISTER-LIVE-RANGE-X
REVISION=V003
FUNCTION=ProcessWideFp32FullCacheRows
SOURCE=本地实验/N01-REGISTER-LIVE-RANGE-X/V003/submission.asc
PARENT=本地实验/N01-REGISTER-LIVE-RANGE-X/V002/submission.asc

The changed block is the per-row normalization block after pass1 partial reduction and before `SyncVToMTE2()` / the parameter-output pass.

1. Pass1 loads x and residual into `xLocal` and `residualLocal`.
2. `Add(valueTile, xLocal, residualLocal, valid)` consumes both staging vectors and retains y in `valueLocal`.
3. `Mul(xLocal, valueTile, valueTile, valid)` makes the square in `xLocal`.
4. `ReduceSum(residualLocal, reduceLocal[...], xLocal, tileCount)` writes the row reduction into `residualLocal` and consumes `xLocal`.
5. `SyncVToS()` and `residualLocal.GetValue(0)` copy the reduction scalar to `squareSum`.
6. The old vector contents of `residualLocal` are no longer read. After `SyncSToV()`, V003 writes `meanSquare`, takes `Sqrt`, and reads `invRms` from `residualLocal`.

The only source change is the destination and source of the three normalization operations in step 6: `xLocal` becomes `residualLocal`. Add, square, ReduceSum, barriers, synchronization, loads, stores, buffer sizes, and reduction layout are unchanged.

PARENT_SOURCE_SHA256=59ea1096afa7fa72ae7768a8b7fb51389917d6fda28660ee77ca68a9821b3a8d
CANDIDATE_SOURCE_SHA256=f355ef215f9e717b3207010ade0893fed34c7d6078bd1f2d0e7dbd7c966c1da0
V002_FP16_AND_BF16_CHANGES=unchanged

# D003 hypothesis

## Hypothesis

D001 and D002 use the same resolver for all three dtypes. D003 tests one dtype boundary: BF16 always uses the `LARGE_FEW` plan, while FP16 and FP32 retain D002's floor-average row classifier. The BF16 plan uses a 4096-element tile, one row per batch, and reloads gamma/bias per row. This removes multi-row parameter residency from the BF16 mode while retaining FP32 conversion for the BF16 arithmetic path.

The motivation is that the BF16 path has an extra source-to-FP32 conversion for both data and parameters. A mode that reuses parameters across rows can therefore have a different break-even point from FP16 and FP32. The change is limited to the dtype-to-plan boundary; the reduction formula, tile loop, valid-element handling, and output conversion remain unchanged.

## Resolver behavior

| dtype | Resolver rule | Tile | Rows per batch | Parameter policy |
| --- | --- | ---: | ---: | --- |
| FP32 | D002 `avgRows` classifier | D002-selected | D002-selected | D002-selected |
| FP16 | D002 `avgRows` classifier | D002-selected | D002-selected | D002-selected |
| BF16 | unconditional `LARGE_FEW` | 4096 | 1 | load per row |

The BF16 single-row plan still uses `DataCopyPad` with valid byte lengths for unaligned D and uses the Level-2 row reduction path with FP32 temporary values. No testcase index is assigned a hidden shape or dtype.

## Acceptance

The acceptance point for D003 is a successful CANN compile on server3. No local NPU execution, CPU matrix, profiling, benchmark, or platform submission is part of this candidate.

# PARAM-RESIDENCY H1 Source Audit

ROUTE=PARAM-RESIDENCY-CHAMPION-X
HYPOTHESIS=H1-GAMMA-TWO-ROW-LIFETIME
STATUS=RESEARCH_ONLY
KERNEL_CHANGED=NO
REVISION_CREATED=NO

## Parent Identity

- DIRECT_PARENT: R31B V011
- PARENT_SOURCE: `线上结果/R31B/V011/submission.asc`
- PARENT_SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- OFFICIAL_SCORE: `45.16`
- SOURCE_LINES: `3554`

## Exact Execution Site

| Item | Evidence |
| --- | --- |
| Dispatch | `AddRmsNormBiasKernel<T>::Process`, lines 160-168 |
| FP32 wide entry | `ProcessWideFp32`, lines 1847-1853 |
| FP32 wide implementation | `ProcessWideFp32FullCacheRows`, lines 2082-2246 |
| Wide threshold | `rowWidth > kCacheElems`, with `kCacheElems=8192`, lines 60-113 and 1285-1288 |
| FP32 wide tile | `kWideFullYTileElems=4096`, line 1283 |
| FP32 wide UB budget | `kWideFullYBudgetBytes=176*1024`, line 1281 |

## Current Gamma Lifetime

For the FP32 wide full-y path, the current source already has the lifetime
that H1 proposes:

1. `ChooseWideFullYRows` selects the largest complete-y batch that fits the
   fixed UB budget (lines 1293-1329).
2. `ProcessWideFp32FullCacheRows` forms a row batch and retains complete `y`
   rows before the affine pass (lines 2101-2157).
3. For each parameter tile, the kernel loads gamma and bias exactly once
   (lines 2174-2193).
4. The same gamma tile is consumed by every row in that batch (lines
   2194-2207), including both adjacent rows when `batchRows=2`.
5. The x/residual staging pair is reused only after the outstanding output
   stores have been drained (lines 2181-2189 and 2208-2244).

The concrete current lifetime for a two-row FP32 batch is:

- LIFETIME_BEGIN: gamma `Load` at line 2191 for the current parameter tile.
- OWNER: `xBuf_` gamma staging tensor at line 2179.
- CONSUMERS: `valueRow` for each `batchRow` at lines 2194-2206.
- RELEASE_POINT: after the final row store release is waited, before the
  next tile can overwrite `xBuf_` at lines 2183-2189 and 2235-2244.
- LIFETIME_END: the next tile's parameter load, or the final batch cleanup.

This is not a latent reload that H1 can remove. It is already one gamma load
per parameter tile for the complete two-row batch.

## Shape-by-Shape Existence Check

### FP32 D=16384

The fixed budget admits two complete FP32 `y` rows:

- y rows: `16384 * 4 * 2 = 131072` bytes
- x/residual tiles: `2 * 4096 * 4 = 32768` bytes
- reduction metadata: `2 * 16 * 4 = 128` bytes
- total modeled footprint: `163968` bytes, below `176*1024`

Therefore `wideFullYRows_=2`, and the source already loads each gamma tile
once and consumes it across the two rows. H1 is already present.

### FP32 D=32768

Two complete FP32 `y` rows do not fit the fixed budget. The chooser falls
back to `wideFullYRows_=1`; the batch loop therefore has `batchRows=1` for
each iteration. There is no adjacent two-row gamma consumer in this path.

Making D=32768 use two-row gamma lifetime would require changing the batch
footprint, tile width, y retention, or another UB allocation. Those changes
are explicitly outside H1's one-factor boundary and would reproduce the
already rejected UB/tile tradeoff from COEFF-LOCALITY-X.

### D=8192 control

D=8192 is not on the wide path because the threshold is strictly greater
than `kCacheElems`. On the regular path, `cacheParams = cacheRow &&
localRows > 1` (lines 242-247), gamma/bias are preloaded once (lines
249-281), and the row loop reuses them (lines 284 onward). The requested
two-row parameter lifetime is again already present.

## DMA, Synchronization, and UB Boundaries

- DMA: FP32 wide gamma/bias use ordinary per-tile `Load` calls at lines
  2191-2192; there is no second gamma load inside the row-consumer loop.
- MTE2/V: the batch waits for the reduction phase at line 2157, then waits
  for the parameter tile at line 2193 before vector consumption.
- V/MTE3: the two-row stores use the existing `storeReady0/1` and
  `storeRelease0/1` events at lines 2164-2173 and 2208-2244.
- UB: gamma and bias temporarily alias `xBuf_` and `residualBuf_` during the
  affine pass. No additional gamma allocation exists in the current FP32
  wide path.
- No producer overwrite is permitted until the outstanding store release is
  drained; this is already enforced before the next parameter tile load.

## Source-Audit Conclusion

`H1-GAMMA-TWO-ROW-LIFETIME` is not a missing runtime mechanism in exact
R31B V011:

- D=16384: two-row gamma lifetime already exists.
- D=32768: two-row lifetime is unavailable because the fixed UB budget only
  admits one retained row; enabling it would change the declared mechanism.
- D=8192: multi-row gamma reuse already exists on the regular cached path.

SOURCE_GATE=PASS_FOR_AUDIT
EXISTENCE_GATE=FAIL_NO_RUNTIME_MECHANISM_FOR_H1
IMPLEMENTATION_AUTHORIZED=NO

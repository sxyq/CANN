# CASE47-SMALL-CLUSTER-CHAMPION-X V002

DATE: 2026-10-04
ROUTE: CASE47-SMALL-CLUSTER-CHAMPION-X
REVISION: V002
DIRECT_PARENT: `线上结果/R31B/V011/submission.asc`
PARENT_SOURCE_SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
PARENT_SOURCE_COMMIT: `43a1049a1e08e518c88e354a754fdebb85a96f99`
PARENT_OFFICIAL_SCORE: 45.16
MAIN_SELECTED: YES (H2; approved 2026-10-04)

## SINGLE_HYPOTHESIS

For FP32 rows of width 257 only, when `ProcessNarrowMidOverlap` receives exactly two rows for a block, replace its two Level-2 square-sum reductions with one `ReduceSum<float, Pattern::Reduce::AR>` over a padded `{2, 264}` FP32 matrix. The required staging and dependency schedule is part of this same batched-reduction mechanism: stage both rows' `y = x + residual` values and squares in existing UB buffers, zero-pad each row to 264 elements, issue one AR reduction, then run the existing scalar normalization and epilogue for row 0 followed by row 1. This delays only row 0's scalar tail and Store until row 1's square sum has been staged and included in the batch.

Every element retains the Direct Parent arithmetic order: Add, square Mul, row reduction, multiplication by `invRowWidth`, epsilon addition, Sqrt and reciprocal, scale, gamma Mul, and bias Add. Row ownership, GM input/output addresses, parameter loads, and output row order are unchanged. All other dispatches, dtypes, widths, and row counts retain Parent behavior. The existing DMA operations and synchronization events remain; this schedule adds no GM pass and removes no independent DMA or synchronization operation.

The hypothesis is that one two-row AR call reduces the per-row reduction issue overhead. It is refuted if the generated target code does not contain one AR reduction for this branch, if either side fails the declared correctness tolerance, or if later qualified same-binary and paired measurements show no improvement beyond the measured noise. Measurement is not authorized by this declaration.

## DEPENDENCY_SCHEDULE_AND_BUFFER_LIFETIME

- `kTileElems=4096` and `kCacheElems=8192` in the Direct Parent narrow FP32 setup allocate `xBuf_` and `residualBuf_` at 4096 FP32 elements each (16 KiB each), `gammaBuf_` and `biasBuf_` at 8192 FP32 elements each (32 KiB each), `xFp32Buf_` and `residualFp32Buf_` at 4096 FP32 elements each (16 KiB each), `valueFp32Buf_` at 8192 FP32 elements (32 KiB), and `reduceFp32Buf_` at 4096 FP32 elements (16 KiB). The total remains 176 KiB; no TBuf or UB allocation is added.
- Offsets below are in FP32 elements, with byte offsets in parentheses. In `valueFp32Buf_`, row 0 y starts at 0 (0 B) and row 1 y starts at 264 (1056 B). In `xFp32Buf_`, the two square rows use the same starts: 0 (0 B) and 264 (1056 B). Each row has 257 valid values followed by zeroed lanes 257-263; the two rows occupy elements 0-527 (2112 B) in either buffer, within their existing capacities.
- The exact explicit-buffer call is `AscendC::ReduceSum<float, AscendC::Pattern::Reduce::AR, true>(dst, src, sharedTmp, shape, true)`. In CANN `8.5.0.alpha002`, the installed declaration takes `const LocalTensor<float>&` for `dst` and `src`, `const LocalTensor<uint8_t>&` for `sharedTmpBuffer`, `const uint32_t srcShape[]`, and `bool srcInnerPad`; `true` is the `isReuseSource` template argument and the final `true` is `srcInnerPad`. The explicit signature is in `/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/asc/include/adv_api/reduce/reduce.h:212-215`.
- Obtain those views from the existing TBufs with `reduceFp32Buf_.Get<float>()`, `xFp32Buf_.Get<float>()`, and `residualFp32Buf_.Get<uint8_t>()`, respectively. CANN's installed `TBuf::Get<T>()` constructs a `LocalTensor<T>` view over the TBuf address (`.../aarch64-linux/ascendc/include/basic_api/impl/kernel_tbuf_impl.h:66-78`); `TPipe::InitBuffer` rounds allocation lengths to `ONE_BLK_SIZE` (`.../aarch64-linux/ascendc/include/basic_api/impl/kernel_tpipe_impl.h:289-333`). The explicit API implementation reinterprets the uint8 temporary view as T scratch and accepts UB tensors (`.../aarch64-linux/asc/impl/adv_api/detail/reduce/reduce_sum/reduce_sum_v220_impl.h:271-282`).
- For the input view, the API requires at least `2*264=528` FP32 elements; its AR output requires at least 2 FP32 elements. These extents are enforced in `.../aarch64-linux/asc/impl/adv_api/detail/api_check/kernel_check/reduce/reduce_check.h:44-70`. The input starts at `xFp32Buf_` offset 0; row 1 starts at byte 1056, which is 33*32 bytes. Both the source row pitch and the 264-element row width are 32-byte aligned. The dst view starts at `reduceFp32Buf_` offset 0. The installed interface and parameter validation specify the padded inner dimension but no separate byte-alignment predicate for src/dst base addresses; the implementation accepts UB tensor positions (`.../aarch64-linux/asc/impl/adv_api/detail/reduce/reduce_sum/reduce_sum_v220_impl.h:101-115`). `srcInnerPad=true` is required by the installed parameter validation (`.../aarch64-linux/asc/impl/adv_api/detail/api_check/kernel_check/reduce/reduce_check.h:82-87`).
- AR writes one value per outer row through `dst[row]` and validates a destination extent of at least `srcShape[0]`, so the two results are contiguous FP32 values at `reduceFp32Buf_` elements 0 and 1 (byte offsets 0 and 4); an 8-element padded scalar stride is not required for this Pattern API output. See `.../aarch64-linux/asc/impl/adv_api/detail/reduce/reduce_sum/reduce_sum_v220_impl.h:241-259` and `.../aarch64-linux/asc/impl/adv_api/detail/api_check/kernel_check/reduce/reduce_check.h:59-70`.
- The recorded server3 host query used `{2,264}`, `DT_FLOAT`, AR, `srcInnerPad=true`, and `isReuseSource=true`; it returned maximum and minimum temporary sizes of 0 bytes. Passing the existing `residualFp32Buf_.Get<uint8_t>()` view still satisfies the explicit-buffer overload without allocating new UB. The temporary view spans the existing 4096-FP32-element (16384-byte) buffer.
- For each row, `inputReady` is waited before Vector reads the shared `xBuf_`/`residualBuf_` input staging. After that row's Add and square have consumed those inputs and filled its distinct y/square slots, the existing `inputRelease` event is set. Before row 1 overwrites the shared input staging, it waits on `inputRelease`. Both rows' square writes complete before the AR call; the post-AR `SyncVToS` precedes reading its results and reusing `xFp32Buf_` as scalar scratch. Thus no live square input is overwritten.
- Both y rows remain live in their separate `valueFp32Buf_` slots through the AR call. After AR completion, row 0's existing scalar normalization and epilogue update only its own slot, then its existing V-to-MTE3 synchronization and Store run. The existing row-boundary MTE3-to-V wait remains after that delayed row-0 Store and before row 1's scalar/output work; row 1 then uses its separate y slot. The final MTE3-to-V drain remains after row 1's Store. Resident gamma/bias buffers remain unchanged and live through both row outputs.
- This dependency schedule preserves the existing input/output DMA count and event set. It adds no GM reread, pass, or buffer, and drops no independent DMA or synchronization operation.

## PROXY_SCOPE

- The sole target is FP32 `D=257`, `M=2*A`, labeled `PROXY_D257_FP32_M2A`. `A` must be queried at runtime by the exact correctness runner from `ACL_DEV_ATTR_VECTOR_CORE_NUM`; the runner computes `M=2*A`. For Parent's `blockCount=min(A,M)`, this gives `localRows=2` per block.
- This synthetic PROXY is not an Official case4 or case7 input. Their shape, dtype, M, D, dispatch, and runtime core count remain UNKNOWN. Official latency is not used to infer any of them.
- V002 is a sibling of V001 and starts from the Direct Parent source above. It does not include V001's scalar-handoff grouping.

## API_AND_UB_BASIS

- The server3 Toolkit is CANN `8.5.0.alpha002`; its installed `adv_api/reduce/reduce.h` declares the explicit temporary-buffer FP32 `ReduceSum` overload and its `Pattern::Reduce::AR` form. The installed tiling header declares `GetReduceSumMaxMinTmpSize`.
- A host-only query against that Toolkit's `libtiling_api.a`, using shape `{2,264}`, `DT_FLOAT`, `ReducePattern::AR`, `srcInnerPad=true`, and `isReuseSource=true`, returned maximum and minimum temporary sizes of 0 bytes. The installed API header requires the padded inner dimension to be 32-byte aligned; 264 FP32 elements is 1056 bytes. `srcInnerPad=true` is the A2/A3 mode required by this API and the target is Ascend910B3 / DAV_2201.
- Existing narrow FP32 buffers total 176 KiB: `xBuf_` 16 KiB, `residualBuf_` 16 KiB, `gammaBuf_` 32 KiB, `biasBuf_` 32 KiB, `xFp32Buf_` 16 KiB, `residualFp32Buf_` 16 KiB, `valueFp32Buf_` 32 KiB, and `reduceFp32Buf_` 16 KiB. They fit within the target's 192 KiB Vector Core UB. V002 uses `xFp32Buf_` for the 528-value squared matrix, `reduceFp32Buf_` for two contiguous sums, and `residualFp32Buf_` as the explicit `LocalTensor<uint8_t>` API temporary view; it adds no UB allocation.
- Both padded lanes (columns 257-263) are zeroed before the matrix reduction. The API shape is `{2,264}` and `srcInnerPad=true`; no padded value contributes to either row sum. Each row's mean-square still divides by exactly 257.
- FP32 reduction order changes and may alter rounded square sums. Correctness must report matched elements / total elements, maximum absolute error, and the project FP32 tolerance. Parent and Candidate must use the same generated inputs, runtime `A`, and output comparison. If the Parent fails the trusted comparison, stop without calling it a Candidate failure.

## V001_SEPARATION

V001 is closed with `MEASUREMENT_BLOCKED`: its Parent W45 and W60 same-binary attempts did not qualify, and Candidate P/C did not run. V002 does not modify or depend on V001's source result. The prior Parent timing at `[80,257]` does not qualify this V002 PROXY; a future performance stage needs a fresh same-binary qualification for the exact runtime `M=2*A` shape and dtype.

## STATUS

BUILD: NOT_RUN
CORRECTNESS: NOT_RUN
PERFORMANCE: NOT_RUN
OFFICIAL_CASE4_CASE7_MAPPING: UNKNOWN
ONLINE_SUBMISSION: NOT_PERFORMED

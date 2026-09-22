# MID-X V001 Architecture Metadata

## Route

MID-X Mid-Range Multi-Row Specialist. Fresh. Scope T04/T06/T07/T08 class only.

## Hypothesis

Mid-case 2~4× TBest ratios come from per-row scalar handoff, re-loading gamma/bias, and no MTE overlap. V001 attacks those four costs at once.

## Kernel shape

| Item | Value |
|---|---|
| Entry | `extern "C" __global__ __vector__ mid_x_v001_batch` |
| Host ABI | `extern "C" void run_kernel(GM_ADDR, const TensorGroupInfo&, ..., int64_t, aclrtStream, float)` |
| Judge types | not redefined in submission; `local_types.h` for local compile only |
| Data motion | `DataCopyPad` with `rightPadding` element count |
| Reduce | `ReduceSum<float, true>` with dedicated 8 KiB tmp |
| Math domain | FP32 accumulate; `u=x+res`; `rms=sqrt(mean(u*u)+eps)`; `out=u/rms*gamma+bias` |

## Mid hot path (mode 0)

- Multi-row per core: each block owns a contiguous row range (`rows/usedCores` + remainder).
- Row batching: `batchRows` chosen from UB budget (dtype-specific via `sizeof(T)`).
- Parameter residency: gamma/bias loaded once per core into FP32 UB.
- Batch reduction: all rows in a batch `ReduceSum` into `sumSq[B]`, then one vector `Rsqrt` over the batch.
- Scalar handoff: one `GetValue` per row (invRms only). No per-row sum readback.
- MTE overlap: `TQue` depth 2 on x / residual / output queues.
- u residency: full FP32 `u` row(s) kept in UB across reduce and apply (no second x/res load).

## Generic fallback (mode 1)

- Single block, tiled columns (1024), full legal domain: FP16/BF16/FP32, rank 2..4, D 64..32768, unaligned D, tail rows, NaN/Inf, any legal epsilon.
- Correctness only; not a performance path. T14/T15 are not this route's target.

## Dispatch

| Field | Policy |
|---|---|
| usedCores | `min(rows, availableCoreNum, 40)` |
| blockDim | `usedCores` on hot path; `1` on generic |
| batchRows | `PickBatch(colsPad, elemSize, 64)` |
| mode | 0 if one FP32 u row + queues + 8 KiB reduce tmp fit 184 KiB; else 1 |
| dtype | 0=FP32, 1=FP16, 2=BF16 (27 accepted as BF16) |
| workspace | none (no cross-core SyncAll) |
| sync count | 0 (row-owned cores; no cross-core barrier) |

## UB budget (184 KiB vector workspace)

- xQue / resQue / outQue depth 2 × batch × colsPad × sizeof(T)
- uBuf: batch × colsPad × FP32
- gammaF + biasF: 2 × colsPad × FP32 (resident params)
- workBuf: colsPad × FP32
- reduceTmp: 8 KiB
- sumSq: up to 64 × FP32

## Target

Ascend 910B3 / DAV_2201 / `--npu-arch=dav-2201` / CANN 8.5.0.alpha002 on cann-server3.

## Revision log

| Rev | Change | Compile | Notes |
|---|---|---|---|
| V001 | mid multi-row batch + resident params + batch ReduceSum + TQue depth 2 | device/submission/full PASS | first MID-X revision; enough coverage for 15 cases |

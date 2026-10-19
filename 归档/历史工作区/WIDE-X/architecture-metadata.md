# WIDE-X V002 — Hierarchical UB Reduction (RE-hardened)

## Route

Non-D-slice wide-D specialist. Target Ascend 910B3 / DAV_2201.

## Hypothesis

Wide-D AddRmsNormBias is bound by sum-of-squares reduction latency and by
re-reading x/residual for the normalize stage. D-slice across cores does not
pay for T14-class (few-row, wide D).

V002 keeps **hierarchical UB reduction** with single-core full/near-full `u`
residency, and fixes V001 online T02 RE:

1. Row-block ownership only. No D-slice, no SyncAll.
2. `u = x + residual` resident in UB as FP32 when the row fits 184 KiB.
3. `sum(u*u)`: 64-wide leaf `ReduceSum` (8B-aligned `leafBuf`) → running `Add`.
4. `inv_rms = 1/Sqrt(mean+eps)` + one Newton polish.
5. Epilogue streams gamma/bias; FP32 resident path stores the row once.
6. TQue EnQue/DeQue with full FreeTensor; FetchEventID (not shared event 0).
7. Level-2 ops use explicit `int32_t` counts and `kVecChunk=64`.
8. dtype-specific: FP32 native; FP16/BF16 upcast, `CAST_ROUND` on store.

## Difference from R31 D-slice

No D-stripe / more-cores / slice-count sweep. Reduction hierarchy is in one
core's UB, not across cores.

## Files

See HANDOFF_V002.md.


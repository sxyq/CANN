# REDUCE-X V001 Reduction / V-S Handoff Design

## Goal

RMS reduction for AddRmsNormBias on DAV_2201 with minimum V/S handoff and
minimum latency. Portable mechanisms for later high-score routes.

## Math

```text
u      = x + residual
rms    = sqrt(mean(u*u) + eps)
output = u / rms * gamma + bias
```

Reduction is `sum(u*u)` over D, then `mean+eps`, then `rsqrt`, then scale.

## Known safe baseline (shared contracts)

```text
ReduceSum<float,true>(dst, src, tmp, count)  +  GetValue once after SyncVToS
8 KiB dedicated tmp
```

That is **1 V/S handoff per row** (pipeline drain + scalar extract).

## REDUCE-X redesign

### 1. Multi-row simultaneous reduction (batched RMS scalars)

Rows are processed in batches of `B` that fit UB with work buffers.
For each row `i` in the batch:

```text
ReduceSum<float,true>(sum[i], u[i], tmp, D)
```

All `B` reductions stay in the Vector pipe. Scalar side sees only the batch
boundary.

### 2. Zero-scalar scale path (primary V/S handoff design)

After the batch of sums:

```text
mean_eps[i] = sum[i] * (1/D) + eps     // Muls/Adds use S-side constants already in hand
inv[i]      = Rsqrt(mean_eps[i], B)    // vector primitive, no GetValue
Brcb(scale_blk[i], inv[i], 1, ...)     // 8-wide block of inv[i]
Mul/Div with BinaryRepeatParams{src1BlkStride=0, src1RepStride=0}
```

`src1BlkStride=0` re-reads the same 32B block for every output block, so one
Brcb block of 8 identical `inv[i]` values scales a full row.

**V/S handoff count per row = 0** on this path.
No `GetValue`, no `SyncVToS` between reduce and scale.

Scalars that remain on S (`1/D`, `epsilon`, loop bounds) were already S-side
function arguments. They are not V/S handoffs.

### 3. Hierarchical UB reduction (large D)

When one row does not fit with work buffers (D up to 32768):

```text
Level-1: stream D in chunks, Add(u*u) tree into partial[c]
Level-2: ReduceSum(partial) -> sum
Level-3: Rsqrt on the single mean_eps
```

Partial accumulator stays in V. Final handoff (if any) is batched with sibling
rows in the same core tile.

### 4. Reduce + DMA overlap

TQue depth 2 on x/residual: while Vector reduces batch `k`, MTE loads batch
`k+1`. Parameter tensors (gamma/bias) load once per core when they fit.

### 5. Epilogue fusion

```text
out = ((u * inv) * gamma) + bias
```

fused in one Vector pass after reduction. FP16/BF16 upcast to FP32 for `u` and
all RMS math; round-cast only on store.

### 6. sqrt / rsqrt choice

Use `Rsqrt` (one vector instruction) instead of `Sqrt` + `Div`. Multiplier is
`inv_rms` not divisor `rms`, matching the zero-scalar scale path.

### 7. Fallback (full 15-case coverage)

Private generic path: one row at a time,
`ReduceSum` + `GetValue` once after `SyncVToS`, scalar `Muls`. Used when batch
or zero-scalar path is unsafe (odd tail, exotic rank/dtype edge, tiny R).
Fallback is correctness-only, not the performance path.

## V/S handoff accounting

| Path | SyncVToS per row | GetValue per row | Notes |
|------|------------------|------------------|-------|
| Shared safe idiom | 1 | 1 | baseline |
| REDUCE-X zero-scalar batch | 0 | 0 | Brcb + src1BlkStride=0 |
| REDUCE-X fallback | 1 | 1 | known-safe ReduceSum+GetValue |
| REDUCE-X wide hierarchical | 0 (amortized) | 0 | partials stay in V |

**V_S_HANDOFF_COUNT_PER_ROW = 0** on the hot path.

## Coverage

Rank 2/3/4 flattened to R x D, D in 64..32768 (any 32B alignment),
dtype FP32 / FP16 / BF16 (judge codes 0/1/2), tail rows, D tail via DataCopyPad,
NaN/Inf pass-through, deterministic chunk order left-to-right.

## Portable mechanisms

1. Multi-row ReduceSum into a packed `sum[B]` before any scalar use.
2. Brcb 8-wide scale block + BinaryRepeatParams zero-stride broadcast.
3. Rsqrt-as-multiplier epilogue (no Div-by-sqrt).
4. Hierarchical chunk + final ReduceSum for D > UB residency.
5. Batch one SyncVToS for B rows if a scalar fallback is required (amortize).

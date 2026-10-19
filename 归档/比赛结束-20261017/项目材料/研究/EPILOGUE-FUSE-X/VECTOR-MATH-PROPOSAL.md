# VECTOR-MATH-X PROPOSAL — Track-B Hypotheses

ROUTE_PROPOSED=VECTOR-MATH-X (new lane; worktree pending current route record)
PARENT=FROZEN_R31B_V011 (R31B-V011-LP-ROW-PIPELINE_kernel.asc)
PARENT_SOURCE_SHA=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
OFFICIAL_ANCHOR=45.16
CONTEXT_CLASS=FROZEN_STRONG_BASELINE_VECTOR_MATH
SCOPE=invD / mean / epsilon / sqrt / rsqrt / reciprocal / scalar-to-vector application for the RMS denominator. Unified FP32 intermediates. No dtype specialization. No reduction topology. No scheduling / DMA / wide-path change.

---

## 0. THE SCALAR TAIL IN THE FROZEN SEED

After `ReduceSum` produces the square-sum partials, the seed computes the RMS denominator through a scalar bottleneck (same pattern in `Process()` L351–367, `ProcessNarrowMidOverlap` L565–574, `ProcessFp32FullRowOutputPipelined` L1192–1201, and the batched paths):

```text
SyncVToS()                                // V→S handoff #1
squareSum = reduceLocal.GetValue(0)       // scalar pull #1
meanSquare = squareSum * invRowWidth + ε  // scalar math (mul + add)
SyncSToV()                                // S→V handoff #1
Duplicate(xFp32, meanSquare, 1)           // V pass (1 element)
Sqrt(xFp32, xFp32, 1)                     // V pass (1 element)
PipeBarrier
SyncVToS()                                // V→S handoff #2
invRms = 1.0f / xFp32.GetValue(0)         // scalar pull #2 + scalar divide
SyncSToV()                                // S→V handoff #2
```

**Cost per row:** 2 V→S + 2 S→V handoffs, 2 `GetValue` pulls, scalar mul+add+div, 2 one-element V ops. On short kernels (5–12 µs) with many rows per core, this tail is a significant fraction of the row latency. The batched paths group some handoffs but still do per-row `GetValue` + scalar math.

---

## 1. HYPOTHESES

---

### VM-H1 — RSQRT-DENOMINATOR: replace Sqrt + scalar reciprocal with vector Rsqrt

**MECHANISM**
Replace the `Duplicate` + `Sqrt` + scalar `1.0f/` chain with a single vector `Rsqrt` on the mean-square value:

```text
SyncVToS()
squareSum = reduceLocal.GetValue(0)
meanSquare = squareSum * invRowWidth + ε
SyncSToV()
Duplicate(xFp32, meanSquare, 1)
Rsqrt(xFp32, xFp32, 1)          // was: Sqrt + scalar 1.0f/
PipeBarrier
SyncVToS()
invRms = xFp32.GetValue(0)      // no scalar divide
SyncSToV()
```

Eliminates: the scalar `1.0f/` divide and one `PipeBarrier` (Sqrt→Rsqrt is one op). The V→S/S→V handoff count is unchanged (still 2 each), but the scalar math drops from mul+add+div to mul+add.

**EXPECTED_BOTTLENECK**
Scalar divide + extra V pass + extra barrier on the denominator tail.

**FILES/FUNCTIONS TO TOUCH**
All sites with the `Duplicate`+`Sqrt`+`1.0f/` pattern: `Process()` L362–366, `ProcessNarrowMidOverlap` L569–573, `ProcessFp32FullRowOutputPipelined` L1196–1200, `ProcessBf16FullRowOutputPipelined` L949–953, `ProcessFp16FullRowOutputPipelined` L1067–1071, batched paths L694–701, L820–823, L1430–1437, L1527–1534, L1627–1634, L1736–1747. Requires confirming `AscendC::Rsqrt` exists on dav-c220.

**WHY_ORTHOGONAL_TO_MAIN1**
FP32 vector math on the denominator; no dtype split, no conversion change.

**EXPECTED_WIN_SHAPES**
All shapes (the denominator tail is on every path). Est. 2–5% on row-dense shapes.

**EXPECTED_RISK_SHAPES**
If `Rsqrt` is less accurate than `Sqrt`+`1/x`, the correctness tolerance may fail (FP32 `Rsqrt` is typically within 1 ulp of `1/Sqrt`).

**CORRECTNESS_RISK**
Low–medium. `Rsqrt(x)` vs `1/Sqrt(x)` differ by ≤1 FP32 ulp. Must pass the 54-case battery.

**MEASUREMENT_PLAN**
Same-binary + P/C on 8×256 FP32 and 64×8192 FP32. Falsifier: if no signal, the scalar tail is not the bottleneck.

---

### VM-H2 — VECTOR-DENOMINATOR: keep the entire denominator in the V pipe

**MECHANISM**
Move `meanSquare = squareSum·invD + ε` into the vector domain and compute `invRms` as a vector, pulling only the final scalar for the `Muls` broadcast:

```text
SyncVToS()
squareSum = reduceLocal.GetValue(0)     // 1 scalar pull (unavoidable: ReduceSum dest)
SyncSToV()
Duplicate(xFp32, squareSum, 1)
Muls(xFp32, xFp32, invRowWidth, 1)     // vector: sum * invD
Adds(xFp32, xFp32, epsilon, 1)         // vector: + ε
Rsqrt(xFp32, xFp32, 1)                 // vector: 1/sqrt (or Sqrt + Div)
PipeBarrier
SyncVToS()
invRms = xFp32.GetValue(0)             // 1 scalar pull
SyncSToV()
```

V→S handoffs: 2→2 (same). Scalar math: eliminated (mul+add move to vector). Scalar pulls: 2→2. But the scalar unit does **zero arithmetic** — only two `GetValue` transfers. On batched paths, the `Duplicate`+`Muls`+`Adds`+`Rsqrt` chain can process B rows at once (B-element vectors), amortizing the handoffs.

**EXPECTED_BOTTLENECK**
Scalar-unit occupancy from `meanSquare = sum·invD + ε` and `invRms = 1/√·` on every row.

**FILES/FUNCTIONS TO TOUCH**
Same denominator sites as VM-H1. For batched paths: replace per-row `GetValue`+scalar math with a B-element vector chain (`Duplicate` B partials, `Muls invD`, `Adds ε`, `Rsqrt`), then one `SyncVToS` + B `GetValue` pulls (or one `Brcb`-style broadcast).

**WHY_ORTHOGONAL_TO_MAIN1**
FP32 vector math; no dtype split. The scalar pull of the ReduceSum destination is reduction-adjacent but the `GetValue` itself is unchanged — only the post-pull math moves to V.

**EXPECTED_WIN_SHAPES**
- Row-dense shapes (64×8192, 8×256 batched): est. 3–8% from eliminating scalar math on the hot path.
- Batched paths: bigger win (B rows amortize the handoffs).

**EXPECTED_RISK_SHAPES**
Single-row shapes: handoff count unchanged, only scalar math moves — marginal.

**CORRECTNESS_RISK**
Low. FP32 vector `Muls`+`Adds`+`Rsqrt` matches the scalar formula within 1 ulp.

**MEASUREMENT_PLAN**
Same-binary + P/C on 64×8192 FP32 and 8×256 FP32 (batched). Compare with VM-H1 to isolate the vector-math contribution.

---

### VM-H3 — BROADCAST-INVSCALE: keep invRms as a 1-element vector, eliminate the V→S→V round trip

**MECHANISM**
After computing `invRms` as a 1-element vector, do **not** pull it to scalar. Instead, use `Mul` with a broadcast-repeat (or `Brcb` expansion) to apply it to the value tile:

```text
// denominator computed in V (as VM-H2), invRms left in xFp32[0]
// NO SyncVToS / GetValue / SyncSToV
// per tile:
Mul(valueTile, valueTile, xFp32Broadcast, valid)   // broadcast 1-element invRms
```

Eliminates both V→S handoffs and both `GetValue` pulls for the normalize step. The `Muls(valueTile, valueTile, invRms)` (scalar broadcast) becomes `Mul(valueTile, valueTile, invRmsVec)` (vector broadcast). On Ascend, `Mul` with a zero-stride source repeat broadcasts one element across the tile — same hardware as `Muls` but without the scalar round trip.

**EXPECTED_BOTTLENECK**
The `SyncVToS`/`GetValue`/`SyncSToV` round trip per row (4 handoffs, 2 pulls).

**FILES/FUNCTIONS TO TOUCH**
All `Muls(valueTile, valueTile, invRms, valid)` sites — replace with `Mul(valueTile, valueTile, invRmsSlot, valid)` where `invRmsSlot` is a 1-element vector in UB (from VM-H2's denominator chain). The `SyncVToS`/`GetValue`/`SyncSToV` around invRms is deleted.

**WHY_ORTHOGONAL_TO_MAIN1**
Pure FP32 vector application; no dtype split. The denominator arithmetic is unchanged (VM-H1 or VM-H2 prerequisite).

**EXPECTED_WIN_SHAPES**
Row-dense shapes: est. 5–12% from eliminating 4 handoffs + 2 scalar pulls per row. Bigger on short kernels where the handoff latency is a larger fraction.

**EXPECTED_RISK_SHAPES**
If the broadcast-`Mul` is slower than scalar-`Muls` on this hardware, the win could be negative. Must measure.

**CORRECTNESS_RISK**
Low. Broadcasting invRms[0] is identical to passing the scalar. But VM-H1/H2 must land first (denominator in vector form).

**MEASUREMENT_PLAN**
Depends on VM-H1 or VM-H2. Same-binary + P/C on 64×8192 FP32 and 8×256 FP32.

---

### VM-H4 — BATCHED-DENOMINATOR: one vector chain for all rows in a batch

**MECHANISM**
For batched paths (`ProcessSmall*`, `Process*Batched*`), replace the per-row `GetValue` + scalar math + `Duplicate`+`Sqrt` with a single B-element vector chain:

```text
SyncVToS()
for b in 0..B-1: partials[b] = reduceLocal[b].GetValue(0)   // B scalar pulls
SyncSToV()
Duplicate(vecB, partials, B)          // B-element vector
Muls(vecB, vecB, invRowWidth, B)      // vector mul
Adds(vecB, vecB, epsilon, B)          // vector add
Rsqrt(vecB, vecB, B)                  // vector rsqrt (or Sqrt + Div)
PipeBarrier
SyncVToS()
for b in 0..B-1: invRms[b] = vecB.GetValue(b)   // B scalar pulls
SyncSToV()
```

V→S handoffs: 2 per batch (was 2 per row). Scalar math: zero (was mul+add+div per row). For B=8, this cuts handoffs from 16 to 2 and scalar ops from 24 to 0.

**EXPECTED_BOTTLENECK**
Per-row scalar tail on batched paths (B×4 handoffs + B×3 scalar ops per batch).

**FILES/FUNCTIONS TO TOUCH**
`ProcessSmallFp32Batched` L1419–1438, `ProcessSmallFp32FullTileBatched` L1517–1535, `ProcessSmallFp32ContiguousBatched` L1616–1635, `ProcessSmallLowPrecisionContiguousBatched` L1728–1747, `ProcessBf16FullTileBatchedOutputPipelined` L685–703, `ProcessFp16FullTileBatchedOutputPipelined` L810–825.

**WHY_ORTHOGONAL_TO_MAIN1**
FP32 vector math; no dtype split. Reduction topology unchanged (partials are already in `reduceLocal`).

**EXPECTED_WIN_SHAPES**
Batched shapes (8×256, 16×128, 64×64): est. 8–15% from amortizing handoffs across the batch.

**EXPECTED_RISK_SHAPES**
Single-row shapes (localRows=1): B=1, no amortization.

**CORRECTNESS_RISK**
Low. Same formula, vectorized across rows.

**MEASUREMENT_PLAN**
Same-binary + P/C on 8×256 FP32 and 16×128 FP32. Compare with VM-H2 to isolate the batching contribution.

---

## 2. SUMMARY TABLE

| ID | Single variable | est. win | Prerequisite | Readiness |
|---|---|---|---|---|
| VM-H1 RSQRT-DENOMINATOR | Sqrt+scalar-div → vector Rsqrt | 2–5% | `Rsqrt` API check | READY |
| VM-H2 VECTOR-DENOMINATOR | scalar math → vector Muls+Adds+Rsqrt | 3–8% | VM-H1's Rsqrt | READY |
| VM-H3 BROADCAST-INVSCALE | Muls(scalar) → Mul(broadcast), no V→S→V | 5–12% | VM-H1 or VM-H2 | NEEDS_MORE_EVIDENCE |
| VM-H4 BATCHED-DENOMINATOR | per-row scalar tail → one B-element vector chain | 8–15% batched | VM-H2 | NEEDS_MORE_EVIDENCE |

---

## 3. RECOMMENDATION

**VECTOR-MATH first revision = VM-H2 (VECTOR-DENOMINATOR).**

Reasons:
1. Cuts all scalar arithmetic from the denominator tail (mul+add+div → zero).
2. Unifies the denominator into a vector chain — enables VM-H3 and VM-H4 as follow-ups.
3. Broader shape coverage than VM-H4 (works on single-row and batched alike).
4. VM-H1 alone is too small (2–5%); VM-H2 includes VM-H1's Rsqrt as a sub-step.
5. Clean orthogonality: FP32 vector math, no dtype split, no reduction change.

---

## 4. REQUEST_MAIN_ROUTE_RECORD

REQUEST_MAIN_ROUTE_RECORD: **VM-H2 VECTOR-DENOMINATOR** as the first VECTOR-MATH-X revision.

Requested disposition: record VM-H2, create the VECTOR-MATH-X worktree/branch from FROZEN_R31B_V011, and declare the revision. VM-H1 (Rsqrt) is included as a sub-step of VM-H2. VM-H3 and VM-H4 stay in backlog with their stated prerequisites. kernel edit follows the selected hypothesis in the current specification.

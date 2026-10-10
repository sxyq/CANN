# TRACK-B HYPOTHESES V002 — VECTOR-MATH-X

ROUTE=VECTOR-MATH-X
REVISION_TARGET=V002
DIRECT_PARENT=V001 (SOURCE_SHA dbe776f9165a86ede9d136604e806d9f5dc0f641ee8e05a26bca1ebe8dd33424)
PARENT_SOURCE_SHA=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3 (frozen seed)
CONTEXT_CLASS=FROZEN_STRONG_BASELINE_VECTOR_MATH
TRACK=Track-B read-only research; no kernel edit.

---

## 0. REMAINING SCALAR TAIL AFTER V001

V001 moved `meanSquare = sum·invD + ε` into the V pipe. The remaining scalar tail is:

```text
PipeBarrier
SyncVToS()                                // V→S handoff
invRms = 1.0f / xFp32.GetValue(0)         // scalar pull + scalar divide
SyncSToV()                                // S→V handoff
// per tile: Muls(valueTile, valueTile, invRms, valid)   // scalar broadcast
```

**Per row:** 1 V→S + 1 S→V handoff, 1 `GetValue` pull, 1 scalar divide. The `Muls` broadcast is scalar-fed. On batched shapes (B rows), this repeats B times per batch.

**Hardware constraint discovered:** `AscendC::Rsqrt` on dav-c220 is a fast-approximation (measured max_abs 0.007 vs tol 1e-5). `Duplicate` takes a host-side scalar only — no UB-source variant. Level 0 `Mul` stride-0 broadcasts 8-element blocks (FP32), not single elements. So a true zero-pull single-element broadcast is not directly available.

---

## 1. HYPOTHESES

---

### VM-H3a — VEC-RECIPROCAL + BROADCAST-MUL (recommended)

**MECHANISM**
Move the scalar divide into the V pipe (`Div`), then build a broadcast buffer with `Duplicate` (1 unavoidable pull) and use `Mul` (vector source) instead of `Muls` (scalar source):

```text
// V chain (V001):
Muls(xFp32, reduceDest, invD, 1);
Adds(xFp32, xFp32, eps, 1);
Sqrt(xFp32, xFp32, 1);
Div(xFp32, onesSlot, xFp32, 1);        // V: invRms = 1/sqrt (eliminates scalar divide)
PipeBarrier
SyncVToS()
Duplicate(broadcastBuf, xFp32.GetValue(0), valid);  // 1 pull + broadcast
SyncSToV()
// per tile: Mul(valueTile, valueTile, broadcastBuf, valid)   // vector-source Mul
```

`onesSlot` is a pre-built 1-element UB buffer holding `1.0f` (initialized once per core with `Duplicate(onesSlot, 1.0f, 1)` — compile-time constant, no pull). Net: **−1 scalar divide per row**, V→S→V count unchanged (1), `Muls`→`Mul` (same hardware cost). On batched shapes: −B scalar divides per batch.

**EXPECTED_BOTTLENECK**
Scalar divide on the denominator tail, repeated per row.

**FILES/FUNCTIONS TO TOUCH**
All non-wide denominator tails (V001's sites): `Process()` L359–364, `ProcessNarrowMidOverlap` L564–569, `ProcessBf16FullRowOutputPipelined` L984–989, `ProcessFp16FullRowOutputPipelined` L1100–1105, `ProcessFp32FullRowOutputPipelined` L1227–1232, batched paths L681–714, L823–856, L1449–1480, L1559–1590, L1680–1711, L1813–1844. Add `onesSlot` buffer (1 float) in `Init`. Replace `Muls(valueTile, valueTile, invRms, valid)` with `Mul(valueTile, valueTile, broadcastBuf, valid)`.

**WHY_ORTHOGONAL_TO_MAIN1**
FP32 vector math; no dtype split. The denominator arithmetic is unified across T.

**EXPECTED_WIN_SHAPES**
- Batched (8×256, 32×256): −B scalar divides per batch. Est. 3–7% (V001 showed −7% on 32×256; this removes the last scalar op).
- Single-row: −1 scalar divide per row. Est. 1–3%.

**EXPECTED_RISK_SHAPES**
- If `Div` is slower than scalar `1/x` on this hardware, the win could be negative.
- `Duplicate` pull remains — not zero-pull.

**CORRECTNESS_RISK**
Low. `1.0f / x` (scalar) vs `Div(1.0f, x)` (vector) are identical within 1 ulp.

**MEASUREMENT_PLAN**
Same-binary + P/C on 32×256 FP32 and 8×256 FP32 (V001's positive shapes). Falsifier: if no additional signal beyond V001's baseline, the scalar divide is not the bottleneck.

---

### VM-H3b — BATCHED-BROADCAST-MUL: one batched Mul for all rows

**MECHANISM**
For batched paths, build a B×D broadcast matrix (each row = invRms_b broadcast across D) and issue one batched `Mul` instead of B per-row `Muls`:

```text
// V chain per row (V001): compute invRms_b into slot[b]
// Build broadcast matrix:
for b in 0..B-1:
    Duplicate(broadcastBuf[b*D], slot[b].GetValue(0), D)   // B pulls + B V passes
// One batched Mul:
Mul(valueBuf, valueBuf, broadcastBuf, B*D)                 // 1 V pass
```

V→S handoffs: 1 per batch (was 1 per row). Scalar pulls: B per batch (was B). `Muls` calls: 0 per batch (was B). Net: −B scalar-broadcast `Muls` calls + 1 batched `Mul`. The `Duplicate` calls add B V passes (1-element→D-element).

**EXPECTED_BOTTLENECK**
Per-row scalar-broadcast `Muls` on batched paths.

**FILES/FUNCTIONS TO TOUCH**
Batched denominator + normalize sites: `ProcessSmallFp32Batched`, `ProcessSmallFp32FullTileBatched`, `ProcessSmallFp32ContiguousBatched`, `ProcessSmallLowPrecisionContiguousBatched`, `ProcessBf16FullTileBatchedOutputPipelined`, `ProcessFp16FullTileBatchedOutputPipelined`.

**WHY_ORTHOGONAL_TO_MAIN1**
FP32 vector math; no dtype split. Batched normalize is the same arithmetic, just issued at batch granularity.

**EXPECTED_WIN_SHAPES**
Batched (8×256, 32×256, 64×64): est. 5–12% from eliminating B `Muls` calls and B V→S→V round trips.

**EXPECTED_RISK_SHAPES**
B=1 (single-row): no amortization.

**CORRECTNESS_RISK**
Low. Same math, different issue granularity.

**MEASUREMENT_PLAN**
Same-binary + P/C on 8×256 FP32 and 64×64 FP32. Compare with VM-H3a to isolate the batching contribution.

---

### VM-H3c — LEVEL0-STRIDE0-BROADCAST: zero-pull broadcast via block fill

**MECHANISM**
Fill an 8-element block with invRms, then use Level 0 `Mul` with `src1BlkStride = 0, src1RepStride = 0` to broadcast the block to all lanes:

```text
// V chain: compute invRms into xFp32[0]
// Fill 8-element block: need xFp32[0:8] = invRms
//   (requires Duplicate or equivalent — the open question)
// Level 0 Mul with stride-0 source:
Mul(valueTile, valueTile, xFp32, mask, repeat, {1,1,0,1,1,0});
```

If the 8-element block can be filled without a scalar pull (e.g., via `Brcb` on a pre-aligned source), this achieves **zero scalar pulls and zero V→S→V round trips**.

**EXPECTED_BOTTLENECK**
The V→S→V round trip itself (1 per row).

**FILES/FUNCTIONS TO TOUCH**
Same as VM-H3a. Requires Level 0 `Mul` API with `BinaryRepeatParams`.

**WHY_ORTHOGONAL_TO_MAIN1**
FP32 vector math; no dtype split.

**EXPECTED_WIN_SHAPES**
Row-dense: est. 5–12% from eliminating the handoff entirely.

**EXPECTED_RISK_SHAPES**
**Feasibility risk:** filling `xFp32[0:8]` with a single UB value without `Duplicate` is unverified. `Brcb` broadcasts 8-element blocks (not single elements). May be INFEASIBLE.

**CORRECTNESS_RISK**
Medium. Block-fill correctness must be verified before any timing.

**MEASUREMENT_PLAN**
Feasibility probe first: can `xFp32[0:8]` be filled with invRms via `Brcb` or other V op without a scalar pull? If yes, proceed to P/C. If no, reject as INFEASIBLE.

---

## 2. SUMMARY TABLE

| ID | Single variable | est. win | Feasibility | Readiness |
|---|---|---|---|---|
| **VM-H3a VEC-RECIPROCAL + BROADCAST-MUL** | scalar divide → V `Div`; `Muls` → `Mul` | 3–7% batched | confirmed | **READY** |
| VM-H3b BATCHED-BROADCAST-MUL | per-row `Muls` → one batched `Mul` | 5–12% batched | confirmed | NEEDS_MORE_EVIDENCE (depends on H3a) |
| VM-H3c LEVEL0-STRIDE0-BROADCAST | zero-pull broadcast via block fill | 5–12% row-dense | **unverified** | NEEDS_FEASIBILITY |

---

## 3. RECOMMENDATION

**V002 = VM-H3a (VEC-RECIPROCAL + BROADCAST-MUL).**

Reasons:
1. Removes the last scalar arithmetic op from the denominator tail (`1.0f/` → V `Div`).
2. Confirmed feasible (`Div` is a standard V op; `Duplicate` broadcast is proven).
3. Builds on V001's positive signal on batched shapes (32×256 −7.3%).
4. VM-H3b is a follow-up that amplifies H3a on batched paths.
5. VM-H3c needs a feasibility probe first (block-fill without scalar pull).

Minimal OFAT diff (conceptual, per row):

```text
// before (V001 tail):
SyncVToS(); invRms = 1.0f / xFp32.GetValue(0); SyncSToV();
Muls(valueTile, valueTile, invRms, valid);
// after (V002):
Div(xFp32, onesSlot, xFp32, 1);              // V: 1/sqrt
SyncVToS(); Duplicate(broadcastBuf, xFp32.GetValue(0), valid); SyncSToV();
Mul(valueTile, valueTile, broadcastBuf, valid);
```

---

## 4. REQUEST_MAIN_ROUTE_RECORD

REQUEST_MAIN_ROUTE_RECORD: **VM-H3a VEC-RECIPROCAL + BROADCAST-MUL** as the single hypothesis for VECTOR-MATH-X V002.

Requested disposition: record VM-H3a, implement from V001 (PARENT_SOURCE_SHA `dbe776f9…`), run correctness then timing on 32×256 / 8×256 FP32. VM-H3b and VM-H3c stay in backlog. kernel edit follows the selected hypothesis in the current specification.

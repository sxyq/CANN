# NEXT-TRACK-B-OVERNIGHT — VECTOR-MATH-X (C2C research only)

Date: 2026-09-27 (overnight parallel Track-B)
ROUTE=VECTOR-MATH-X
DIRECT_PARENT=FROZEN_R31B_V011 (SOURCE_SHA a8c19a19…)
CURRENT_STATE=V002 (Duplicate broadcast + Mul) implemented; V001 was vector denominator
SCOPE=Research only. No implementation. No worktrees. No MAIN-1/SCHED access.

---

## Source-level bottleneck map (frozen R31B-V011)

Key math sites in the generic path (`Process()` L284–470):

| site | lines | ops | notes |
|---|---|---|---|
| pass-1 tile loop | L297–338 | Add → Mul(square) → ReduceSum | per tile |
| row-level reduction | L355–357 | ReduceSum(tiles) | once per row |
| denominator tail | L358–367 | SyncVToS → GetValue → scalar math → SyncSToV → Duplicate → Sqrt → SyncVToS → GetValue → SyncSToV | 2 V/S round trips |
| epilogue per tile | L427–449 | Muls(invRms) → PB → Mul(gamma) → PB → Add(bias) → PB → Store | 3 vector ops + 3 PipeBarriers |

Same epilogue pattern at: `ProcessNarrowMidOverlap` L579–616, `ProcessBf16FullRowOutputPipelined` L1020–1050, `ProcessFp16FullRowOutputPipelined` L1136–1166, `ProcessFp32FullRowOutputPipelined` L1263–1293.

---

## HYPOTHESES

---

### EPI-FUSE-1 — GAMMA-SCALED FUSION (epilogue arithmetic)

**HYPOTHESIS_ID**: EPI-FUSE-1

**MECHANISM**: Algebraic regrouping of the epilogue. Currently per tile:
```
Muls(valueTile, invRms, valid);          // scalar broadcast multiply
Mul(valueTile, valueTile, gamma, valid); // vector multiply
Add(valueTile, valueTile, bias, valid);  // vector add
```
Rewrite as `y * (gamma * invRms) + bias`. Pre-multiply gamma by invRms once per row into a scratch buffer:
```
Muls(gammaScaled, gamma, invRms, D);     // once per row (or per batch)
// per tile:
Mul(valueTile, valueTile, gammaScaled[col], valid);  // 1 op instead of 2
Add(valueTile, valueTile, bias[col], valid);
```
Net: −1 Muls per tile, +1 Muls per row on gamma buffer. For rows with N tiles, saves N−1 vector dispatches and N−1 PipeBarriers.

**BOTTLENECK_EVIDENCE**: Frozen R31B-V011 L427–433 (generic FP32 path), L579–585 (ProcessNarrowMidOverlap), L1020–1026 (BF16 full-row), L1136–1142 (FP16 full-row), L1263–1269 (FP32 full-row). Each has the 3-op epilogue with 3 PipeBarriers.

**TARGET_SHAPES**: Wide rows with multiple tiles (D > 4096): 8×8192 FP32, 64×4096 FP32, 2×16384 FP32. Also batched paths where epilogue repeats per row in a batch.

**EXPECTED_EFFECT**: 3–8% on multi-tile shapes (saves 1 dispatch + 1 PipeBarrier per tile). Neutral on single-tile shapes (D ≤ 4096): +1 Muls −1 Muls = net 0.

**FILES/FUNCTIONS**: `Init()` (add `gammaScaledBuf_`), `Process()` generic path L427–449, `ProcessNarrowMidOverlap` L579–585, `ProcessBf16FullRowOutputPipelined` L1020–1026, `ProcessFp16FullRowOutputPipelined` L1136–1142, `ProcessFp32FullRowOutputPipelined` L1263–1269. Replace `Muls+Mul` pair with single `Mul` on scaled gamma.

**WHY_ORTHOGONAL_TO_MAIN1**: Epilogue arithmetic. No scheduling, no DMA, no reduction change. MAIN-1 not accessed.

**WHY_NOT_DUPLICATE_CURRENT_MAIN2**: COEFF-LOCALITY-X H1 (param prefetch) and H2 (stripe residency) both failed — they changed *when* gamma is loaded, not *how* the epilogue math is grouped. VECTOR-MATH-X V002 changed `Muls`→`Mul` on the value tile; EPI-FUSE-1 changes the gamma operand itself (pre-scale). Distinct mechanism.

**CORRECTNESS_RISK**: Low. Algebraically identical: `y·invRms·γ + β = y·(γ·invRms) + β`. Floating-point: the two forms may differ in last ULP (non-associativity). Tolerance 1e-5 (FP32) should absorb this. Must verify on 24-case correctness matrix.

**LOCAL_MEASUREMENT_PLAN**: Same-binary + P/C on 8×8192 FP32 (multi-tile), 64×4096 FP32 (multi-tile), 17×256 FP32 (single-tile control, expect ≈0).

**STOP_CONDITION**: If 8×8192 shows <2% delta beyond noise, or if correctness ULP drift exceeds tolerance on any case, stop and revert.

---

### SEQ-FUSE-2 — INLINE-RECIPROCAL DENOMINATOR (scalar/vector sequencing)

**HYPOTHESIS_ID**: SEQ-FUSE-2

**MECHANISM**: Eliminate the second V→S→V round trip in the denominator tail. Currently:
```
SyncVToS(); invRms = 1.0f / xFp32.GetValue(0); SyncSToV();
```
Instead, compute the reciprocal entirely in V using `Div` (vector divide) against a pre-built `onesSlot` (constant 1.0f, initialized once per core):
```
Div(xFp32, onesSlot, xFp32, 1);  // invRms = 1.0 / sqrt(meanSquare), in V
// xFp32[0] now holds invRms; broadcast via Duplicate or Mul as in V002
```
Net: −1 V→S handoff, −1 S→V handoff, −1 GetValue pull, −1 scalar divide per row. The `Div` is 1-element vector op (same cost class as Sqrt).

**BOTTLENECK_EVIDENCE**: Frozen R31B-V011 L365–367 (generic path), L572–574 (ProcessNarrowMidOverlap), L984–989 (BF16 full-row), L1100–1105 (FP16 full-row), L1227–1232 (FP32 full-row), L698–703 (batched paths). Each has `SyncVToS → GetValue → 1.0f/x → SyncSToV`.

**TARGET_SHAPES**: All shapes — the denominator tail is on every non-wide path. Most impactful on batched shapes (B rows per batch → B× savings): 64×64 FP32, 128×128 FP32, 33×100 FP32.

**EXPECTED_EFFECT**: 2–5% on batched shapes (removes 2 handoffs + 1 scalar divide per row). Marginal on single-row shapes.

**FILES/FUNCTIONS**: All denominator tail sites listed above. Add `onesSlot` buffer (1 float) in `Init()`. Replace scalar divide with `Div(xFp32, onesSlot, xFp32, 1)`.

**WHY_ORTHOGONAL_TO_MAIN1**: Scalar/vector sequencing. No mode/DMA/reduction change. MAIN-1 not accessed.

**WHY_NOT_DUPLICATE_CURRENT_MAIN2**: VECTOR-MATH-X V002 replaced `Muls` with `Mul` using a Duplicate-built broadcast buffer. SEQ-FUSE-2 eliminates the *scalar pull* itself (the `GetValue` + scalar divide), which V002 still performs. VM-H3a (proposed but not implemented) moves the divide to V but still does 1 pull for the broadcast; SEQ-FUSE-2 can combine with V002's broadcast to achieve zero pulls if the `Div` result stays in UB.

**CORRECTNESS_RISK**: Medium. `Div` on dav-c220 must be IEEE-accurate (not fast-approx). If `Div` is fast-approx (like `Rsqrt`), accuracy will fail. Must verify `Div` accuracy first (test 1-element `Div` against scalar divide). If fast-approx, this hypothesis is INFEASIBLE.

**LOCAL_MEASUREMENT_PLAN**: First: 1-element `Div` accuracy probe (host vs device, max_abs ≤ 1e-7). If PASS: same-binary + P/C on 64×64 FP32 (batched), 128×128 FP32 (batched), 17×256 FP32 (single-row control).

**STOP_CONDITION**: If `Div` is fast-approx (max_abs > 1e-6), STOP as INFEASIBLE. If correctness fails on any case, stop and revert.

---

### EPI-PIPE-3 — EPILOGUE/PROLOGUE OVERLAP (epilogue dataflow)

**HYPOTHESIS_ID**: EPI-PIPE-3

**MECHANISM**: Overlap the current row's epilogue with the next row's pass-1 prologue. Currently rows are strictly sequential: pass-1 → reduction → epilogue → next pass-1. Instead, after computing invRms for row N, immediately start row N+1's first-tile load (MTE2) while row N's epilogue (Muls+Mul+Add+Store) executes. The epilogue uses `valueFp32Buf_` and `outputBuf_`; the prologue uses `xBuf_` and `residualBuf_` — no buffer aliasing.

This is a *dataflow* change (pipelining), not a sync removal. The existing PipeBarriers within each phase are kept; only the inter-row scheduling changes.

**BOTTLENECK_EVIDENCE**: Frozen R31B-V011 generic path L284–470. The `for (row = beginRow; row < beginRow + localRows; ++row)` loop is strictly sequential. Pass-1 (L297–338) and epilogue (L427–449) for the same row are serial. The `hasPrefetchedFirstTile` mechanism (L288) already prefetches x/residual for the *next* row's first tile during the current row's pass-1, but the epilogue still blocks the next row's pass-1 from starting.

**TARGET_SHAPES**: Multi-row shapes where `localRows > 1`: 64×256 FP32, 33×100 FP32, 8×8192 FP32 (multi-row wide). Most impactful when epilogue is a significant fraction of per-row time (narrow rows).

**EXPECTED_EFFECT**: 5–15% on multi-row narrow shapes (hides epilogue latency behind next row's DMA+compute). Requires at least 2 rows per core to show benefit.

**FILES/FUNCTIONS**: `Process()` generic path L284–470 (restructure row loop into software pipeline). Possibly `ProcessNarrowMidOverlap` L527–641 (same pattern). NOT the batched paths (they already process multiple rows).

**WHY_ORTHOGONAL_TO_MAIN1**: Epilogue dataflow / pipelining. No math change, no sync removal, no DMA change. MAIN-1 not accessed.

**WHY_NOT_DUPLICATE_CURRENT_MAIN2**: VECTOR-MATH-X V001/V002 changed the math ops inside the epilogue. EPI-PIPE-3 changes *when* the epilogue executes relative to the next row's prologue. ASYNC-TRIPLE-X (pipeline) is a different route but its mechanism is triple-buffering DMA, not epilogue/prologue overlap. COEFF-LOCALITY-X changed param loading, not row pipelining.

**CORRECTNESS_RISK**: Medium. Buffer aliasing must be verified: epilogue uses `valueFp32Buf_`/`outputBuf_`; prologue uses `xBuf_`/`residualBuf_`. If `cacheRow=true`, the prologue also writes `valueFp32Buf_` — potential conflict. Must only enable overlap when `!cacheRow` or use separate buffers. MTE2/MTE3/V pipe ordering must be preserved.

**LOCAL_MEASUREMENT_PLAN**: Same-binary + P/C on 64×256 FP32 (multi-row, single-tile), 33×100 FP32 (multi-row, single-tile), 8×8192 FP32 (multi-row, multi-tile). Control: 17×256 FP32 (single-row per core, expect ≈0).

**STOP_CONDITION**: If any correctness case fails (buffer aliasing), stop and revert. If 64×256 shows <3% delta beyond noise, the overlap gain is not worth the complexity.

---

## Recommendation

**Priority 1: SEQ-FUSE-2** (scalar/vector sequencing). Lowest complexity, broadest applicability (all shapes), and directly attacks the 2×V/S handoff overhead that remains after V001/V002. Requires a quick `Div` accuracy probe first.

**Priority 2: EPI-FUSE-1** (epilogue arithmetic). Clean algebraic identity, targeted at multi-tile shapes where the epilogue dispatch overhead is measurable. Lower risk than EPI-PIPE-3.

**Priority 3: EPI-PIPE-3** (epilogue dataflow). Highest expected effect but highest complexity and buffer-aliasing risk. Only if SEQ-FUSE-2 and EPI-FUSE-1 underperform.

**Not proposed**: COEFF-LOCALITY variants (H1/H2 already failed), reduction variants (3 already failed), mode/wide/dtype changes (out of scope).

---

REQUEST_MAIN_APPROVAL: SEQ-FUSE-2 (INLINE-RECIPROCAL DENOMINATOR) as first V003 hypothesis for VECTOR-MATH-X.

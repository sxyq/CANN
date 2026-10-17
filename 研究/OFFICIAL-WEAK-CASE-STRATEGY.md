# OFFICIAL-WEAK-CASE-STRATEGY — R31B-V011 weak cases → mechanism mapping

Date: 2026-09-27 (C2C overnight, analysis only — no implementation)
Source: `phase4/online/R31B/V011/result.json` (15/15 Pass, Official 45.16)
Frozen seed: `R31B-V011-LP-ROW-PIPELINE_kernel.asc` (SOURCE_SHA a8c19a19…)

---

## Weak case table (sorted by score ascending)

| case | timeUs | bestTimeUs | ratio | score | shape class (inferred) | confidence |
|---|---:|---:|---:|---:|---|---|
| 14 | 16486.82 | 3750.12 | 4.40x | 21.5 | Very wide D (>16384) or many rows × wide | MEDIUM |
| 7 | 52.34 | 13.99 | 3.74x | 23.5 | Medium rows × short D (64–256) | MEDIUM |
| 1 | 5.44 | 1.70 | 3.20x | 25.8 | Tiny (few rows × short D) | HIGH |
| 6 | 28.46 | 10.86 | 2.62x | 29.6 | Small–medium rows × short D | MEDIUM |
| 4 | 16.55 | 6.66 | 2.48x | 30.8 | Small rows × short–mid D | MEDIUM |
| 8 | 69.21 | 30.16 | 2.29x | 32.8 | Medium rows × mid D | MEDIUM |
| 3 | 5.16 | 2.47 | 2.09x | 35.5 | Tiny (similar to case 1) | HIGH |

Shape class inference basis: absolute time magnitude + ratio pattern. Tiny kernels (< 10 µs) are launch-overhead dominated. Cases 14 (16 ms) is clearly wide-D or large-M. Cases 6/7 (28–52 µs) sit in the per-row overhead regime. Exact Judge shape mapping is unknown; ranges are estimates.

---

## Per-case kernel region analysis

### Case 14 — ratio 4.4x, score 21.5 (HIGHEST gap)

**Time scale**: 16.5 ms — extremely long. Either very wide D (D > 16384, many tiles per row) or many rows with wide D.

**Dominant kernel region** (from frozen source structure):
- Wide path (`ProcessWideFp32FullCacheRows` L2082+ or `ProcessWideLowPrecision` L3076+): per-row loop with per-tile Load → compute → Store. For D=32768 with tileElems=7680, that's 5 tiles per row × (Load+Add+Mul+ReduceSum+Muls+Mul+Add+Store) = 8 ops × 5 tiles = 40 ops per row.
- The wide reduction tail (per-tile ReduceSum → row-level ReduceSum → denominator) adds 2 V/S handoffs per row.
- The wide epilogue stores per-tile (no merged writeback).

**Dominant region estimate**: ~40% epilogue stores, ~30% pass-1 compute+DMA, ~20% reduction tail, ~10% overhead.

**Open MAIN-2 axes**:
- STORE-H2B (writeback merge): reduces store descriptor count. **OPEN** (spec ready, not implemented).
- EPI-FUSE-1 (gamma-scaled fusion): reduces epilogue dispatch. **OPEN**.
- EPI-PIPE-3 (epilogue/prologue overlap): **OPEN** but high complexity.
- Wide-path changes: **BLOCKED** (mode/wide is deferred in current lanes).
- Reduction: **BLOCKED** (3 variants already failed).

---

### Case 7 — ratio 3.7x, score 23.5 (SECOND highest gap)

**Time scale**: 52 µs — medium. Many rows × short D, or moderate rows × mid D.

**Dominant kernel region**:
- Per-row overhead: 2 V/S handoffs + GetValue + scalar divide (denominator tail) × rows
- Per-tile epilogue: 3 PipeBarriers × tiles × rows
- If D ≤ 256 (1 tile): epilogue is 4 ops + 3 barriers per row; reduction is 4 ops + 2 handoffs per row. Handoffs + barriers ≈ 50% of non-DMA time.

**Dominant region estimate**: ~45% reduction tail + handoffs, ~35% epilogue dispatch, ~20% pass-1.

**Open MAIN-2 axes**:
- SEQ-FUSE-2 (inline reciprocal): eliminates 2 V/S handoffs + scalar divide per row. **OPEN**.
- EPI-FUSE-1 (gamma-scaled fusion): neutral on single-tile shapes. **LOW ROI** here.
- EPI-PIPE-3 (overlap): hides epilogue behind next row's prologue. **OPEN**.
- SCHED (ownership/launch): **PARK recommendation** (structural ceiling).

---

### Case 1 — ratio 3.2x, score 25.8

**Time scale**: 5.4 µs — tiny. Launch + Init overhead dominates.

**Dominant kernel region**:
- Block launch + TPipe Init (buffer allocation) is fixed cost per block.
- If few rows (e.g., 2–8 rows × 64–128 D), the actual compute is < 1 µs; the remaining 4+ µs is launch/Init/sync.

**Dominant region estimate**: ~70% fixed overhead (launch, Init, sync), ~30% compute.

**Open MAIN-2 axes**:
- SCHED (ownership/launch width): **PARK** (structural ceiling documented).
- Init optimization (fewer/smaller buffers): **BLOCKED** (UB lifetime is deferred).
- Everything else: marginal — fixed cost is not reducible within current constraints.

**Note**: Case 1 and case 3 are the hardest to improve without changing launch/Init. They are the floor for this kernel architecture.

---

### Case 6 — ratio 2.6x, score 29.6

**Time scale**: 28 µs — small–medium. Per-row overhead visible but compute is significant.

**Dominant kernel region**: Similar to case 7 but less extreme. ~35% reduction tail, ~35% epilogue, ~30% pass-1.

**Open MAIN-2 axes**: SEQ-FUSE-2, EPI-FUSE-1 (if multi-tile), EPI-PIPE-3.

---

### Case 4 — ratio 2.5x, score 30.8

**Time scale**: 16.5 µs — small. Similar to case 6.

**Dominant region**: ~40% reduction tail + handoffs, ~30% epilogue, ~30% pass-1.

**Open MAIN-2 axes**: SEQ-FUSE-2 (primary), EPI-PIPE-3.

---

### Case 8 — ratio 2.3x, score 32.8

**Time scale**: 69 µs — medium. Balanced compute/overhead.

**Dominant region**: ~30% reduction tail, ~35% epilogue, ~35% pass-1.

**Open MAIN-2 axes**: EPI-PIPE-3 (best fit — enough rows for overlap), EPI-FUSE-1.

---

### Case 3 — ratio 2.1x, score 35.5

**Time scale**: 5.2 µs — tiny. Same class as case 1.

**Dominant region**: ~70% fixed overhead. **Near floor** for this architecture.

---

## Axis status summary

| axis | status | weak cases helped | notes |
|---|---|---|---|
| SEQ-FUSE-2 (inline reciprocal) | **OPEN** | 7, 6, 4, 8 | Broadest applicability; needs Div accuracy probe |
| EPI-FUSE-1 (gamma-scaled fusion) | **OPEN** | 14, 6, 8 | Multi-tile only; neutral on single-tile |
| EPI-PIPE-3 (epilogue/prologue overlap) | **OPEN** | 7, 8, 6 | High complexity; buffer aliasing risk |
| STORE-H2B (writeback merge) | **OPEN** (spec ready) | 14, 8 | Reduces store descriptors; 待主线讨论 |
| COEFF-LOCALITY (param prefetch/residency) | **BLOCKED** | — | H1 +6.6% regression, H2 failed |
| Reduction variants | **BLOCKED** | — | 3 variants failed on this parent |
| Mode/wide/dtype changes | **MAIN-1 / out of scope** | — | Deferred in current lanes |
| SCHED (ownership/launch) | **PARK** | 1, 3 (marginal) | Structural ceiling documented |
| ALIGN-TAIL (copy primitive) | MAIN-2, separate lane | 14 (wide stores) | Different route |
| UB-LIVENESS (buffer layout) | MAIN-2, separate lane | all (marginal) | Different route |

---

## Highest-ROI remaining MAIN-2 mechanism after STORE-H2B

**Recommendation: SEQ-FUSE-2 (INLINE-RECIPROCAL DENOMINATOR)**

Rationale:
1. **Broadest case coverage**: Helps cases 7, 6, 4, 8 (4 of 7 weak cases), which are the per-row overhead regime where the denominator tail's 2 V/S handoffs + scalar divide are a measurable fraction.
2. **Lowest complexity**: Single `Div` op replacing scalar divide + 2 handoffs. One buffer addition (`onesSlot`).
3. **Complements STORE-H2B**: STORE-H2B targets case 14 (wide stores); SEQ-FUSE-2 targets cases 7/6/4/8 (short rows). Together they cover the top 2 weak-case classes.
4. **Prerequisite check is cheap**: 1-element `Div` accuracy probe (minutes) determines feasibility.

If SEQ-FUSE-2's `Div` is fast-approx (INFEASIBLE), fallback is **EPI-FUSE-1** (gamma-scaled fusion), which helps cases 14/6/8 and has zero correctness risk (algebraic identity).

---

**TOP RECOMMENDATION: SEQ-FUSE-2 (INLINE-RECIPROCAL DENOMINATOR)** — targets 4/7 weak cases (7, 6, 4, 8) by eliminating 2 V/S handoffs + scalar divide per row; complements STORE-H2B's wide-store focus; low complexity; cheap feasibility probe first.
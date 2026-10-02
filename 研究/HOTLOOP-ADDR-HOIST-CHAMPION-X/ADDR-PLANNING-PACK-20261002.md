# HOTLOOP-ADDR-HOIST-CHAMPION-X — ADDR planning pack

## 0. Scope, identity, and boundary

```text
DATE_UTC=2026-10-02
ROUTE=HOTLOOP-ADDR-HOIST-CHAMPION-X
TRACK=TRACK-B-ONLY
MAIN=MAIN-2
WORKTREE=/home/data4t2/lelinfeng/cann/worktrees/w2/m2/hotloop-addr
BRANCH=w2/m2/hotloop-addr
DIRECT_PARENT=R31B V011
PARENT_SOURCE=线上结果/R31B/V011/submission.asc
PARENT_SOURCE_SHA=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
PARENT_OFFICIAL=45.16
REVISION=NONE
IMPLEMENTATION=NONE
BUILD=NOT_RUN
CORRECTNESS=NOT_RUN
TIMING=NOT_RUN
PROFILING=NOT_RUN
ONLINE=NOT_RUN
MAIN_SELECTED=NONE
ADDR_PLANNING_CANDIDATE_1=ADDR-H3
ADDR_PLANNING_CANDIDATE_2=ADDR-H1
PLANNING_ORDER_ONLY=YES
```

This is a bounded, baseline-only code-site audit. It does not select a Main
candidate, open a Revision, edit a kernel, create a Candidate, compile a new
source, run server3, run NPU correctness, perform timing/profiling, or submit
Online. The two planning entries are ordered for Planning/Review only.

The parent SHA was checked against `线上结果/R31B/V011/submission.sha256`,
`线上结果/R31B/V011/result.json`, and the retained archived/staging copies.
The parent source is the exact `ProcessWideLowPrecision` implementation at
source lines 3076-3365. The retained V011 result is 15/15, but its testcase
shape map is not retained; the dimensions below are kernel-path probes, not
Official testcase identities.

## 1. Evidence read and evidence limits

Read-only inputs for this pack:

- `研究/HOTLOOP-ADDR-HOIST-CHAMPION-X/TRACK-B-HANDOFF.md` — current route
  handoff, H1-H3 pool, frozen parent, and `MAIN_SELECTED=NONE`.
- `研究/HOTLOOP-ADDR-HOIST-CHAMPION-X/TRACK-B-HYPOTHESES-V004-20261001.md`
  and `TRACK-B-HYPOTHESES-V005-20261001.md` — H4-H8 surfaces, prior duplicate
  audit, path qualification, and the H7/H8 negative-control concerns.
- `研究/HOTLOOP-ADDR-HOIST-CHAMPION-X/TRACK-B-CROSS-BRANCH-CODEGEN-AUDIT-20261001.md`
  — cross-route boundary audit.
- `线上结果/R31B/V011/submission.asc`, `diff.patch`, `source-meta.json`,
  `result.json`, and `submission.sha256` — exact parent source and result
  identity.
- `归档/历史工作区/R31B/configure-v011-server3.log` and
  `compile-v011-server3.log`, plus the retained V011 review logs — parent
  Release compiler/build evidence only.

The retained compiler evidence says CANN `8.5.0.alpha002`, Release, and
successful compile/link. It contains no V011 device IR, scalar instruction
listing, register/spill report, or assembly. No safe V011 parent-only local
profile is retained: the current handoff explicitly records no local parent or
Candidate timing. Online `timeUs` values are not used as a parent-only profile
or as evidence for an address change.

Therefore `EXPECTED_INSTRUCTION_REDUCTION` below is a source-level upper
bound/conditional estimate. It is not an emitted-instruction count and not a
performance claim. Any implementation would still require exact-source
codegen inspection before a Revision declaration.

## 2. Exact parent code-site model

The audited function is:

```text
ProcessWideLowPrecision(rowCount, rowWidth, blockCount, invRowWidth, epsilon)
  source: 线上结果/R31B/V011/submission.asc:3076-3365
```

For one outer `batchBegin` iteration, define:

```text
B = batchRows
T = tileCount
U = B * T
Q = U + (U - 1) = 2U - 1       # current unit + next-prefetch unit
```

The exact pass-1 sites are:

| Site | Parent expression | Source lines | Static execution count |
|---|---|---:|---:|
| Current index decode | `batchRow = u / tileCount`; `tile = u - batchRow * tileCount` | 3125-3127 | once per current unit: `U` |
| Current column | `col = static_cast<uint64_t>(tile) * tileWidth` | 3128 | `U` |
| Current row base | `(batchBegin + static_cast<uint64_t>(batchRow)) * rowWidth` | 3129-3130 | `U` |
| Current input GM offsets | `rowOffset + col` for x and residual | 3140-3143 | two uses per current unit: `2U` |
| Next index decode | `(u + 1) / tileCount`; `(u + 1) - nRow * tileCount` | 3146-3148 | once per nonterminal unit: `U-1` |
| Next column | `nCol = static_cast<uint64_t>(nTile) * tileWidth` | 3149 | `U-1` |
| Next row base | `(batchBegin + static_cast<uint64_t>(nRow)) * rowWidth` | 3150-3151 | `U-1` |
| Next input GM offsets | `nRowOffset + nCol` for x and residual | 3172-3174 | two uses per nonterminal unit: `2(U-1)` |

Thus the parent source contains `Q` divisions, `Q` decode
`batchRow*tileCount` multiplies, `Q` decode subtractions, `Q` column
multiplies, and `Q` row-base multiplies per batch. The two Load operands make
`2Q` source-level offset uses; an optimizer may materialize each identical
offset once, so this is not counted as `2Q` guaranteed additions.

The exact pass-2 sites are independent:

| Site | Parent expression | Source lines | Static execution count |
|---|---|---:|---:|
| Current parameter/output column | `col = static_cast<uint64_t>(tile) * tileWidth` | 3267-3269 | `T` |
| Next parameter column | `nCol = col + static_cast<uint64_t>(tileWidth)` | 3278-3296 | `T-1` |
| Output row base | `(batchBegin + static_cast<uint64_t>(batchRow)) * rowWidth` | 3313-3343 | `B*T = U` stores |
| Output store | `Store(outputGm_, rowBase + col, outputLocal, valid)` | 3341-3344 | `U` |

The `Load` helper consumes an element offset as `src[offset]` at lines
3367-3375. No DMA count, byte count, event, barrier, local-tensor lifetime,
math order, or store order is part of this address-only baseline model.

## 3. Path and static cost matrix

The exact V011 path is low-precision wide-LP only (`rowWidth > 8192`; FP32
uses another function). The retained route model gives the following controls:

| dtype | D / `rowWidth` | `tileWidth` | `T` | `Bmax` | `U` at `Bmax` | `Q` at `Bmax` | final tile |
|---|---:|---:|---:|---:|---:|---:|---:|
| FP16 | 12288 | 4096 | 3 | 3 | 9 | 17 | 4096 |
| FP16 | 16384 | 4096 | 4 | 2 | 8 | 15 | 4096 |
| FP16 | 24576 | 4096 | 6 | 1 | 6 | 11 | 4096 |
| FP16 | 32768 | 4096 | 8 | 1 | 8 | 15 | 4096 |
| BF16 | 12288 | 4096 | 3 | 2 | 6 | 11 | 4096 |
| BF16 | 16384 | 4096 | 4 | 1 | 4 | 7 | 4096 |
| BF16 | 24576 | 4096 | 6 | 1 | 6 | 11 | 4096 |
| BF16 | 32768 | 2560 | 13 | 1 | 13 | 25 | 2048 |
| BF16 | 40960 | 2048 | 20 | 1 | 20 | 39 | 2048 |

All counts scale with the actual `B`, not necessarily `Bmax`; every future
address proof must include `B=1..Bmax`, a partial final batch, and a nonzero
`batchBegin`. The reduced-tile BF16 rows are required because a fixed 4096
assumption would invalidate the audit.

## 4. Ordered planning candidates

### ADDR_PLANNING_CANDIDATE_1 — ADDR-H3: stateful pass-1 index counters

**Exact function/code site.** `ProcessWideLowPrecision`, flattened pass-1
loop `for (int32_t u = 0; u < units; ++u)` at lines 3122-3211. The exact
candidate sites are current decode lines 3126-3127 and next-unit decode lines
3147-3148. Their values feed `col`, `rowOffset`, `valid`, retained-y/value
indices, and reduction slots at lines 3128-3133, 3178-3203.

**Current expression.**

```cpp
const int32_t batchRow = u / tileCount;
const int32_t tile = u - batchRow * tileCount;
// next unit:
const int32_t nRow = (u + 1) / tileCount;
const int32_t nTile = (u + 1) - nRow * tileCount;
```

**One-factor proposed diff.** Replace only this logical `(batchRow,tile)`
decode with two scalar counters and a rollover update. Keep `u`, `units`,
`tileCount`, `tileWidth`, `rowOffset`, `col`, `valid`, `nValid`, all current
and next GM offsets, local indices, reduction indices, DMA calls, event IDs,
waits, flags, barriers, and dispatch predicates unchanged. This is a
planning description only; no source was edited.

**Loop level and executions.** One current counter state is consumed for every
pass-1 tile (`U=B*T` per batch). The next state is needed for every nonterminal
unit (`U-1`). The parent therefore exposes `Q=2U-1` decode groups. The
counter candidate must perform exactly one tile rollover per row boundary and
must cover `B=1..Bmax`, partial final batches, and nonzero `batchBegin`.

**Target shapes/dtypes.** FP16 and BF16 wide-LP probes in the matrix in §3,
especially FP16 D=12288/16384, FP16 D=32768, BF16 D=12288, and reduced-tile
BF16 D=32768/40960. FP32 and D=8192 are excluded from this function. These
are path probes, not recovered Official cases.

**Why the cost matters.** In the current flattened stream, index decoding is
performed for the current unit and again for the prefetched unit. If the
compiler emits a real runtime divide plus multiply/subtract sequence, H3
removes the repeated variable-divisor decode from the address/index hot loop
without changing any data movement or vector work. The static opportunity is
largest at `Q=39` for BF16 D=40960 and `Q=25` for BF16 D=32768, but the
multi-row FP16 D=12288 case tests rollover frequency.

**Why the compiler may already optimize it.** `u` is sequential and
`tileCount` is loop-invariant, so CCEC may strength-reduce the division to a
reciprocal multiply/shift, derive the remainder without the explicit
subtraction, or partially recognize the quotient/remainder pair. Conversely,
`tileCount` and `tileWidth` are runtime values, so source-level strength
reduction is not guaranteed. The retained V011 artifacts contain no IR,
assembly, instruction count, or register report; this uncertainty is a hard
codegen gate.

**Related old routes and non-duplicate proof.** R005/FULL-R005 changes tile
geometry; R012/FULL-R012 changes alignment/row ownership; R028/FULL-R028
changes scalar reduction/synchronization; R029/FULL-R029 changes dispatch or
cached-row policy; R31A V024 hoists `invRms` arithmetic; MIX-A changes modes
or synchronization. None replaces the V011 flattened quotient/remainder
decoder. The branch-hoist audit's final-valid-length item preserves `u`,
`batchRow`, and `tile`; it is adjacent at the `valid` expression, not the
same mechanism. H3 leaves every address formula intact, so it is distinct
from H1/H2/H4/H5 address-state changes.

**Expected instruction reduction.** Conditional static upper bound: remove up
to `Q` divide-like operations, `Q` decode multiplies, and `Q` decode
subtractions, i.e. up to `3Q` scalar decode primitives per batch before
counting counter increment/compare/rollover instructions. For the §3 matrix,
`Q` is 7, 11, 15, 17, 25, or 39 at the listed maximum batches. The net
emitted reduction can be zero if the parent is already strength-reduced; a
positive result requires the counter update/branch sequence to be cheaper
than the parent decode. No timing gain is asserted.

**Precision and safety risk.** Numeric precision risk is LOW: no vector
operation, cast, reduction order, or scalar arithmetic changes. Address/index
risk is MEDIUM: a rollover error can select a valid but wrong row/tile and
corrupt the retained-y or reduction slot while still issuing legal DMA. A
future proof must assert exact equality of current/next `(batchRow,tile)`, all
input offsets, `valid`/`nValid`, and reduction indices.

**Compiler elimination/profile evidence.**

```text
PARENT_COMPILE_EVIDENCE=PASS, Release, CANN 8.5.0.alpha002
PARENT_CODEGEN_ELIMINATION=NOT_AVAILABLE (no retained V011 IR/assembly)
PARENT_PROFILE=NOT_AVAILABLE (no safe V011 parent-only local profile)
ONLINE_TIMES_USED_AS_PROFILE=NO
CODEGEN_GATE=REQUIRED_BEFORE_REVISION
```

The candidate survives because its source-level cost is materially distinct
and potentially large, not because compiler elimination or performance has
been demonstrated.

### ADDR_PLANNING_CANDIDATE_2 — ADDR-H1: pass-1 row-base state

**Exact function/code site.** `ProcessWideLowPrecision`, pass-1 flattened
loop lines 3125-3211. Current row-base formation is lines 3129-3130 and the
current x/residual GM operands are lines 3140-3143. The prefetched row base is
formed at lines 3150-3151 and consumed by the next x/residual operands at
lines 3172-3173.

**Current expression.**

```cpp
const uint64_t rowOffset =
    (batchBegin + static_cast<uint64_t>(batchRow)) * rowWidth;
Load(xLocal, xGm_, rowOffset + col, valid);
Load(residualLocal, residualGm_, rowOffset + col, valid);
// next unit:
const uint64_t nRowOffset =
    (batchBegin + static_cast<uint64_t>(nRow)) * rowWidth;
Load(xNext, xGm_, nRowOffset + nCol, nValid);
Load(rNext, residualGm_, nRowOffset + nCol, nValid);
```

**One-factor proposed diff.** Retain the flattened decode, `col`/`nCol`,
`valid`/`nValid`, and all logical indices. Replace only repeated pass-1 row
base multiplication with a `uint64_t` row-base state initialized for the
batch and advanced by `rowWidth` at an exact row rollover; use the current
base or the next-row base for the two prefetch loads. Pass 2 is untouched.
This is a planning description only; no source was edited.

**Loop level and executions.** There are `U` current row-base evaluations and
`U-1` next-row-base evaluations per batch, for `Q` parent row-base multiply
sites. The proposed state is reused for all `T` tiles in a logical row; the
conservative implementation-neutral comparison leaves at most `B` row-base
initializations and replaces the other `Q-B` multiplications with row-base
increments/rollover bookkeeping. The two x/residual loads still execute at
every current and prefetched unit, so DMA count and bytes are unchanged.

**Target shapes/dtypes.** FP16/BF16 wide-LP probes in §3, with emphasis on
FP16 D=12288/16384 and BF16 D=12288 for multi-row batches, plus one-row
controls and reduced-tile BF16 D=32768/40960 for row-base correctness. Use
`B=1..Bmax` and a partial final batch; do not assume a fixed tile width.

**Why the cost matters.** The row base is invariant over all tiles in one
logical row, while the parent recomputes a row-width multiplication for both
the current and lookahead unit. The static upper-bound savings are `Q-B`
row-base multiplications per batch: 14 for FP16 D=12288 at `B=3`, 13 for
FP16 D=16384 at `B=2`, 9 for BF16 D=12288 at `B=2`, and 24 for the one-row
BF16 D=32768 reduced-tile path. The one-row controls remain useful because
they separate row-base savings from multi-row pipeline effects.

**Why the compiler may already optimize it.** The current and next row bases
are derived from quotient values and a loop-invariant `rowWidth`; CCEC may
reuse a row-base value, strength-reduce the quotient/row multiplication, or
fold `rowOffset + col` into the load address. The parent source does not
prove such elimination, and the retained Release build has no IR/assembly
for confirmation. The proposed state can also add a rollover predicate,
live-range pressure, or a scalar dependency chain, so source operation
counts alone cannot establish a win.

**Related old routes and non-duplicate proof.** R005/FULL-R005 changes tile
size; R012/FULL-R012 changes alignment and row grouping; R028/FULL-R028
changes `GetValue`/event and scalar reduction work; R029/FULL-R029 changes
dispatch/cache policy; R31A V024 hoists math rather than a row address; MIX-A
and R31B history retain explicit row offsets. H1 changes only pass-1 row-base
formation. It does not change H3's logical decoder, H2/H4's rolling
current/next offset policy, H5's column state, pass-2 H6/H8 surfaces, valid
lengths, or event predicates. The cross-branch audit treats row-boundary
bookkeeping as adjacent to a branch guard, not duplicate with it.

**Expected instruction reduction.** Conditional static upper bound: up to
`Q-B` row-base multiply eliminations per batch, with one or more row-width
increments and a rollover/select sequence added. For the §3 maximum-batch
rows, `Q-B` is 14, 13, 10, 14, 9, 6, 10, 24, and 38 respectively in table
order. Net emitted reduction is unknown and can be zero or negative if the
compiler already hoists the parent or the state spills. No timing gain is
asserted.

**Precision and safety risk.** Numeric precision risk is LOW: no numeric
operation or order changes. Address risk is MEDIUM/HIGH at row rollover and
the BF16 reduced-tile tail; a wrong correction can alias the next row without
causing an invalid DMA. A future proof must compare every current and next
input offset against the parent, including `batchBegin != 0`, partial final
batch, `T=13`, and `T=20`.

**Compiler elimination/profile evidence.**

```text
PARENT_COMPILE_EVIDENCE=PASS, Release, CANN 8.5.0.alpha002
PARENT_CODEGEN_ELIMINATION=NOT_AVAILABLE (no retained V011 IR/assembly)
PARENT_PROFILE=NOT_AVAILABLE (no safe V011 parent-only local profile)
ONLINE_TIMES_USED_AS_PROFILE=NO
CODEGEN_GATE=REQUIRED_BEFORE_REVISION
```

The candidate survives because the row-base invariant is explicit in the
source and its static opportunity is independent of H3; it is not a claim
that the parent emitted repeated multiplies.

## 5. Rejected hypotheses

Every item below is explicitly removed from this planning pack as
`DUPLICATE_OR_NO_EFFECT_REJECTED`. No rejected item creates a Revision.

| Hypothesis and exact site | Current expression / one-factor idea | Rejection evidence |
|---|---|---|
| `ADDR-H2`, pass-1 lines 3141-3173 | Replace current/next x and residual GM offsets with one rolling cursor | `NO_EFFECT_REJECTED`: `rowOffset`, `col`, `nRowOffset`, and `nCol` remain needed for `valid`, retained-y/value indexing, and reduction. H2 can remove at most the same logical offset composition that H4 subsumes, while adding row-boundary state; no unique positive static count remains after H3/H1. It is not a third planning slot. |
| `ADDR-H4`, next prefetch lines 3146-3174 | Replace `nRowOffset+nCol` with current-offset plus a row-boundary delta | `DUPLICATE_OR_NO_EFFECT_REJECTED`: this is a strict next-prefetch subset of H2's rolling input-offset surface. It also adds a terminal-row select and has no retained codegen/profile evidence showing that the two parent multiplies survive. |
| `ADDR-H5`, pass-1 `col` at line 3128 | Replace `tile * tileWidth` with a column cursor | `NO_EFFECT_REJECTED`: `col` feeds `valid`, retained-y/value offsets, and reduction-adjacent indexing, so the cursor does not remove the surrounding logical work. It adds rollover state for only one multiply per unit; H3/H1 expose larger independent decode/row-base costs. |
| `ADDR-H6`, pass-2 lines 3267-3296 | Replace pass-2 `tile * tileWidth` with a column cursor | `NO_EFFECT_REJECTED`: this is a canonical affine `tile++` loop and the parent already forms `nCol = col + tileWidth`. CCEC is especially likely to use an induction value here; no retained assembly proves otherwise. |
| `ADDR-H7`, pass-1 lines 3141-3142 | Name one `currentInputOffset = rowOffset + col` for both Loads | `NO_EFFECT_REJECTED`: the two operands are identical and adjacent, so one semantic address value is already sufficient; a Release optimizer may CSE them. The source change has no guaranteed arithmetic removal and no retained codegen/profile evidence. |
| `ADDR-H8`, pass-2 store lines 3313-3343 | Precompute a `storeRowBase[batchRow]` table | `NO_EFFECT_REJECTED`: it trades `B*T` row-base multiplies for table setup plus `B*T` scalar table accesses and live state. Most large-D controls have `B=1`, where it cannot save more than the parent initialization and can add overhead. No spill/register/codegen evidence supports the table. |
| `Q9`, pass-2 lines 3291-3296 | Add a next-parameter cursor | `DUPLICATE_OR_NO_EFFECT_REJECTED`: V011 already uses the affine `nCol = col + tileWidth`; there is no independent multiply to remove. |
| `Q10`, dispatch/shape qualification | Treat all D/dtype rows as one body | `DUPLICATE_OR_NO_EFFECT_REJECTED`: this is qualification evidence, not an address hypothesis. FP32 and D=8192 do not enter this function, and reduced BF16 tile widths must remain runtime-qualified. |

The rejections do not claim that a compiler comparison could never revive an
item. They record that, with the retained parent-only evidence and the
one-factor/no-duplicate constraint, those items have no separate planning
priority in this pack.

## 6. Planning handoff state

```text
SURVIVING_INDEPENDENT_CANDIDATES=2 (ADDR-H3, ADDR-H1)
ADDR_PLANNING_CANDIDATE_1=ADDR-H3
ADDR_PLANNING_CANDIDATE_2=ADDR-H1
REJECTED_AS_DUPLICATE_OR_NO_EFFECT=H2,H4,H5,H6,H7,H8,Q9,Q10
CODEGEN_EVIDENCE=NOT_RETAINED
PARENT_PROFILE_EVIDENCE=NOT_AVAILABLE
BASELINE_ONLY=YES
REVISION_CREATED=NO
IMPLEMENTATION=NO
MAIN_SELECTED=NONE
ONLINE=NO
```

Planning/Review may choose at most one item for a later exact-source codegen
gate. This route agent makes no `KEEP`, `PARK`, `CLOSE`, `MERGE`, `REPLACE`,
Revision, or Online decision.

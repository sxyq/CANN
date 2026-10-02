# HOTLOOP-ADDR-HOIST-CHAMPION-X V001 — Revision Declaration

```text
ROUTE=HOTLOOP-ADDR-HOIST-CHAMPION-X
REVISION=V001
DIRECT_PARENT=R31B V011
PARENT_SOURCE=线上结果/R31B/V011/submission.asc
PARENT_SOURCE_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
PARENT_OFFICIAL=45.16
SINGLE_HYPOTHESIS=ADDR-H3 stateful pass-1 index counters
CONTEXT_CLASS=MAIN-2 WAVE-2 ADDR TRACK-A
PLANNING_DECISION=ADDR-H3 only
CODEGEN_GATE=REQUIRED_BEFORE_PERFORMANCE_CLAIM
```

## EXACT_FUNCTION

`ProcessWideLowPrecision`, flattened pass-1 loop `for (int32_t u = 0; u < units; ++u)` at lines 3122-3211.

## EXACT_CODE_SITE

The exact candidate sites are current decode lines 3126-3127 and next-unit decode lines 3147-3148. Their values feed `col`, `rowOffset`, `valid`, retained-y/value indices, and reduction slots at lines 3128-3133, 3178-3203.

## CURRENT_EXPRESSION

```cpp
const int32_t batchRow = u / tileCount;
const int32_t tile = u - batchRow * tileCount;
// next unit:
const int32_t nRow = (u + 1) / tileCount;
const int32_t nTile = (u + 1) - nRow * tileCount;
```

## PROPOSED_ONE_FACTOR_DIFF

Replace only this logical `(batchRow,tile)` decode with two scalar counters and a rollover update. Keep `u`, `units`, `tileCount`, `tileWidth`, `rowOffset`, `col`, `valid`, `nValid`, all current and next GM offsets, local indices, reduction indices, DMA calls, event IDs, waits, flags, barriers, and dispatch predicates unchanged. This is a planning description only; no source was edited.

## TARGET_SHAPES

FP16 and BF16 wide-LP probes in the matrix in §3, especially FP16 D=12288/16384, FP16 D=32768, BF16 D=12288, and reduced-tile BF16 D=32768/40960. FP32 and D=8192 are excluded from this function. These are path probes, not Official testcase identities.

## TARGET_DTYPES

FP16 and BF16.

## WHY_NOT_DUPLICATE

R005/FULL-R005 changes tile geometry; R012/FULL-R012 changes alignment/row ownership; R028/FULL-R028 changes scalar reduction/synchronization; R029/FULL-R029 changes dispatch or cached-row policy; R31A V024 hoists `invRms` arithmetic; MIX-A changes modes or synchronization. None replaces the V011 flattened quotient/remainder decoder. The branch-hoist audit's final-valid-length item preserves `u`, `batchRow`, and `tile`; it is adjacent at the `valid` expression, not the same mechanism. H3 leaves every address formula intact, so it is distinct from H1/H2/H4/H5 address-state changes.

## EXPECTED_INSTRUCTION_REDUCTION

Conditional static upper bound: remove up to `Q` divide-like operations, `Q` decode multiplies, and `Q` decode subtractions, i.e. up to `3Q` scalar decode primitives per batch before counting counter increment/compare/rollover instructions. For the §3 matrix, `Q` is 7, 11, 15, 17, 25, or 39 at the listed maximum batches. The net emitted reduction can be zero if the parent is already strength-reduced; a positive result requires the counter update/branch sequence to be cheaper than the parent decode. No timing gain is asserted.

## RISKS

Numeric precision risk is LOW: no vector operation, cast, reduction order, or scalar arithmetic changes. Address/index risk is MEDIUM: a rollover error can select a valid but wrong row/tile and corrupt the retained-y or reduction slot while still issuing legal DMA. A future proof must assert exact equality of current/next `(batchRow,tile)`, all input offsets, `valid`/`nValid`, and reduction indices.

## Required pre-performance evidence

```text
PARENT_COMPILE_EVIDENCE=PASS, Release, CANN 8.5.0.alpha002
PARENT_CODEGEN_ELIMINATION=NOT_AVAILABLE (no retained V011 IR/assembly)
PARENT_PROFILE=NOT_AVAILABLE (no safe V011 parent-only local profile)
ONLINE_TIMES_USED_AS_PROFILE=NO
```

The selected hypothesis is conditional on exact-source codegen inspection. No H1, H2, or other performance optimization is included in V001.

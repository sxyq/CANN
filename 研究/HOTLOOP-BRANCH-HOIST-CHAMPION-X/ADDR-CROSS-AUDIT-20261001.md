# HOTLOOP-BRANCH-HOIST-CHAMPION-X — ADDR cross duplicate/codegen audit

## Audit boundary

- DATE: 2026-10-01
- ROUTE: HOTLOOP-BRANCH-HOIST-CHAMPION-X
- TRACK: B / research-only
- AUDIT_TARGETS: HBH-01, HBH-02, HBH-03
- DIRECT_PARENT: R31B V011
- PARENT_SOURCE_SHA: a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
- BRANCH_HANDOFF_HEAD: 01641fed8a1096d000de3a0bc1b3db13cab53f84
- ADDR_HANDOFF_COMMIT: 3c65f7e053f79bf8b4d87a1b2d8de43fba4d7889
- MAIN_SELECTED: NONE

This is a source/control-flow and constant-propagation audit only. No compiler
output was collected because this task forbids build/codegen execution. The
results below are feasibility gates, not claims that any instruction was
removed.

Read-only evidence:

- 线上结果/R31B/V011/submission.asc, especially Process() at lines 159-493,
  the adaptive wide setup at lines 56-113 and 1293-1330, and
  ProcessWideLowPrecision() at lines 3076-3365.
- The committed ADDR handoff at commit 3c65f7e0.
- The current branch handoff and the committed route/official records.

## Dispatch and target corrections

1. V011 sets widePath_ only when rowWidth is strictly greater than
   kCacheElems=8192 (source lines 60-61 and 1288). Therefore:

   - D=8192 does not enter ProcessWideLowPrecision().
   - For D=8192, a generic path is possible only when localRows=1, because
     the dtype-specific full-row paths return first for localRows>1.
   - HBH-01 at D=8192/localRows=1 is a streamed-parameter control:
     cacheParams is false, so it does not exercise the cached-vs-streamed
     mode split.
   - HBH-02 at D=8192/localRows=1 cannot exercise the next-row prefetch
     guard, because localRows=1 makes row + 1 < beginRow + localRows false.

   D=8192 must therefore not be counted as an active HBH-02 guard probe or
   as an HBH-03 wide-path control.

2. ProcessWideLowPrecision() receives wideFullYTileElems_ from
   ChooseWideFullYRows(), which starts at 4096 and may decrement it by 512
   until the UB budget fits (source lines 1293-1329). D and dtype are not
   sufficient to identify tileWidth at source level. In particular, the
   source-level budget calculation for BF16 D=32768 (sizeof(T)=2) selects
   one row and tileElems=2560; it is not a 4096-element full-tile control.
   Any HBH-03 or ADDR-HOIST probe must record the runtime-selected
   tileWidth/tileCount rather than assume a D bucket.

3. This adaptive setup is a codegen boundary, not a new candidate. A
   dtype-by-D generated body, a fixed tile-count body, or a dispatch change
   would enter the historical R022/static-unroll/R029 ownership areas and is
   rejected for this Route.

## Cross-route result matrix

| Candidate | ADDR-H1/H2/H3 duplicate result | Codegen feasibility | Required boundary |
|---|---|---|---|
| HBH-01 | OVERLAP=NO | CONDITIONALLY_FEASIBLE; generic-loop CFG split is expressible, but code-size/register growth is likely | Keep runtime cacheParams/cacheParamFp32 selection; no dtype-by-D generated bodies |
| HBH-02 | OVERLAP=NO | CONDITIONALLY_FEASIBLE; terminal-row split needs body duplication or a helper whose branch may remain | Keep the exact prefetch location and first-tile state; no first-tile peel |
| HBH-03 | CODE-SURFACE OVERLAP=YES, MECHANISM DUPLICATE=NO | CONDITIONALLY_FEASIBLE but weak until emitted code proves arithmetic removal | Preserve u, batchRow, tile, rowOffset, col, nCol, all GM offsets, loop shape, and valid byte values |

All three remain NEEDS_MORE_EVIDENCE. No candidate is selected.

## HBH-01 — generic coefficient-mode guard

The target is the non-wide generic Process() body, not the
ProcessWideLowPrecision() body used by ADDR-H1/H2/H3. For D=6144, the
dispatch falls through to the generic loop for FP32/FP16/BF16; cacheRow is
true and cacheParams differs by localRows. The mode is computed once before
the row loop at source lines 242-247, but the output tile loop tests it again
at lines 383, 389, 416, 430, 442 and related source selections.

The dtype branches around these tests are if constexpr branches. FP32 and
FP16 do not instantiate the BF16 cacheParamFp32 subcase; BF16 does. Thus a
valid HBH-01 implementation may hoist the existing runtime mode decision,
but must not describe that as new dtype code generation. An outer
cacheParams split can produce two runtime-counted loop bodies. It may remove
repeated source-selection branches, but it also duplicates a large output
body. A helper taking a runtime mode flag is not sufficient evidence: the
branch can remain inside the helper.

Duplicate result:

- ADDR-H1/H2/H3: NO. Those candidates are confined to the wide
  low-precision address/index loop; HBH-01 is in the generic coefficient
  selection loop.
- R022/codegen: NO only for a source-level runtime mode split. New
  dtype-by-D generated functions or host dispatch buckets are
  DUPLICATE_REJECTED.

Codegen gate: inspect the generated control flow for the generic FP32,
FP16 and BF16 instantiations. If the repeated mode tests remain, classify
HBH-01 as a source-level no-op. The D=8192/localRows=1 control is useful
only for the streamed body and cannot prove the cached-mode split.

## HBH-02 — generic next-row prefetch terminal guard

The guard at source line 342 is in the generic cached-row path, after the
first-pass tile loop and before scalar normalization. It is separate from
the first-tile load guard at line 303 and from hasPrefetchedFirstTile at line
288. A valid terminal-row split can remove the line-342 branch only by
splitting the dynamic row loop or by generating separate non-terminal and
terminal row-body variants. Passing a runtime hasNextRow flag to one shared
body does not establish branch removal.

Duplicate result:

- ADDR-H1/H2/H3: NO. The ADDR handoff targets the wide path and preserves
  this generic row-prefetch operation; it does not own this generic
  row-count terminal guard.
- TILECOUNT-STATIC-UNROLL: NO for a genuinely dynamic row-count split.
  Peeling the first tile, propagating col=0, or changing
  hasPrefetchedFirstTile would be a different static-offset/control
  mechanism and remains DUPLICATE_REJECTED.
- R013/ASYNC overlap: the source-level guard move is not an overlap
  schedule change, but any changed prefetch position or event order would
  leave this Route's scope.

Codegen gate: prove that the non-terminal body contains the same
next-row Load calls at the same program point and that the terminal body
does not issue them. Also report the code-size effect. D=8192/localRows=1
cannot provide this proof because the guard is false by construction.

## HBH-03 — per-row runtime tail-length hoist

The active low-precision wide path computes the same min/subtraction form at
four relevant sites:

- pass-1 current unit valid at lines 3131-3133;
- pass-1 lookahead nValid at lines 3152-3154;
- pass-2 current tile valid at lines 3269-3271;
- pass-2 lookahead nValid at lines 3292-3294.

The source also computes the current and next address/index state around
lines 3125-3151 and uses it in the Load/Store/event sequence at lines
3140-3175 and 3277-3344. The mathematically equivalent runtime form is:

    tailLen = rowWidth - (tileCount - 1) * tileWidth
    valid = (tile == tileCount - 1) ? tailLen : tileWidth

with the analogous expression for nTile. This can remove repeated
rowWidth-col subtraction if emitted as such, but it leaves a per-unit
terminal test and all address/index, parity, DMA, event, and vector work.
The compiler may already lower the existing min expression to equivalent
code; no gain is established without emitted-code evidence.

Duplicate result:

- ADDR-H1/H2/H3: NO at the mechanism level, with a real code-surface
  overlap. The ADDR handoff freezes valid lengths; HBH-03 changes only how
  the identical valid value is derived. Any HBH-03 implementation that
  changes rowOffset/col/nCol, replaces the u decode, introduces a rolling
  address cursor, or changes the flattened loop is a combined ADDR
  candidate and must be rejected here.
- R009/R010/ALIGN-TAIL: NO if byte counts and padding remain identical.
  Changing tail copy semantics would be out of scope.
- R022/static-unroll: DUPLICATE_REJECTED if the implementation uses
  compile-time D buckets, fixed tile counts, loop peeling, or propagated
  constant offsets.

Codegen gate: compare current and candidate emitted control flow and scalar
address arithmetic. Accept the hypothesis as informative only if repeated
length arithmetic disappears while the old index/address expressions and
ordered DMA/event operations remain. A tail-length precompute with no
instruction change is a no-op.

## Final disposition

- HBH-01: ADDR duplicate NO; source-level runtime CFG split is feasible in
  principle; keep NEEDS_MORE_EVIDENCE and use D=6144 for the mode-bearing
  generic probe.
- HBH-02: ADDR duplicate NO; dynamic row split is feasible in principle but
  has material body/code-size risk; use D=6144/localRows>=2, not
  D=8192/localRows=1.
- HBH-03: ADDR mechanism duplicate NO, code-surface overlap YES; runtime
  tail-length hoist is feasible in principle but has weak expected codegen
  leverage and requires adaptive tileWidth qualification. Do not use
  D=8192 as a wide control and do not assume BF16 D=32768 is full-tile.

ROUTE_HYPOTHESIS_POOL_EXHAUSTED=NO
MAIN_SELECTED=NONE
REVISION_CREATED=NO
BUILD_OR_CODEGEN_EXECUTED=NO
CORRECTNESS_OR_TIMING_EXECUTED=NO
ONLINE_EXECUTED=NO

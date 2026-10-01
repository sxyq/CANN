# HOTLOOP-ADDR-HOIST-CHAMPION-X — Track-B Handoff

## 0. Scope and frozen parent

```text
ROUTE=HOTLOOP-ADDR-HOIST-CHAMPION-X
TRACK=TRACK-B-ONLY
MAIN=MAIN-2
MAIN_WAVE=M2-2 research-only
MAIN_SELECTED=NONE
DIRECT_PARENT=R31B V011
PARENT_OFFICIAL=45.16
PARENT_SOURCE_SHA=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
OFFICIAL_CASE_SHAPE_MAP=MISSING
FOCUS_AXIS=address-expression type
REMOTE_FRESHNESS=FETCH_SUCCEEDED (origin fetched once in bounded 20s attempt)
REVISION=NONE
IMPLEMENTATION=NONE
TESTS=NONE
```

The parent source is the recovered exact online blob at
`线上结果/R31B/V011/submission.asc`; its sidecar SHA matches the file SHA. The
official record is 15/15, score 45.16. No local parent/candidate timing is
claimed for V011.

The inspected V011 hot path is `ProcessWideLowPrecision` (source lines
3076–3365). Pass 1 flattens `(batchRow,tile)` into `u`, then repeatedly derives
`batchRow = u / tileCount`, `tile = u - batchRow * tileCount`,
`rowOffset = (batchBegin + batchRow) * rowWidth`, and `col = tile * tileWidth`.
Pass 2 repeats the tile `col`/`nCol` address expressions. The proposed changes
are scalar address/index expression changes only. Every candidate below must
preserve `tileWidth`, `tileCount`, valid lengths, DMA call count and byte
counts, UB allocation, event order, barriers, arithmetic order, stores, and
dispatch predicates.

The 12288/16384/32768 dimensions below are proposed probe points, not recovered
Official testcase identities: the V011 Official shape-to-case map is missing
from retained evidence. Actual local probes must use the exact V011 dispatch
and record the concrete shape/dtype; do not infer a testcase mapping.

## 1. Candidate pool

### ADDR-H1 — row-base hoist in the pass-1 flattened stream

- **Mechanism:** Keep the flattened `u` loop and its current `batchRow`/`tile`
  decode. Form the first `rowBase` from
  `(batchBegin + batchRow) * rowWidth`; reuse it for interior tiles and advance
  it by `rowWidth` only when the decoded tile rolls to zero. For the prefetched
  next unit, use the current base or current base plus `rowWidth` according to
  whether the next unit crosses a row boundary. Pass 2 remains untouched.
- **Bottleneck:** scalar integer multiply/add expression issue and address
  operand construction in the V011 row/tile loop, especially when
  `batchRows * tileCount` is large.
- **Direct parent/source SHA:** R31B V011 / `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- **Target shapes/dtypes:** FP16 and BF16 wide-LP branch, D=12288, 16384 and
  32768 (the retained R31B task evidence lists these eligible widths; V011
  enters this path only for D>8192). Probe M=1 and a multi-row M such as 8,
  leaving V011's full-y batch selection unchanged.
- **Why it may help:** the row base is invariant over all tiles of one row;
  exposing that invariant should shorten scalar address formation and reduce
  compiler-generated repeated wide-integer arithmetic.
- **Why it may fail:** the compiler may already common-subexpression-eliminate
  the multiplication; the row-rollover condition and serial address state may
  cost more than the saved multiply, while the V/MTE pipeline or event waits
  dominate.
- **UB/DMA/sync/precision impacts:** no UB allocation or view lifetime change;
  same `Load`/`Store` calls, counts, valid lengths and bytes; no event/barrier
  change; no numeric operation or precision change. Address overflow safety
  remains `uint64_t`.
- **Duplicate check:** NEW relative to V011: address-expression placement
  only. R31A V024 hoists `invRms` arithmetic, not a row address. R005 changes
  tile size, R012 changes alignment/row grouping, R028 changes scalar sync,
  and R029 changes dispatch taxonomy; none is this row-base transform.
- **Related old routes:** R005/FULL-R005, R012/FULL-R012, R028/FULL-R028,
  R029/FULL-R029, R31A V024, R31B V011, and MIX-A were audited as adjacent
  evidence, not as this mechanism.
- **Minimal experiment:** one OFAT change limited to pass 1 of V011 wide-LP;
  retain the existing expressions in all other paths and in pass 2. Compare exact
  V011 source against the child on D=12288/16384/32768 FP16/BF16 with 1-row and
  4-row cases; inspect compiler output or instruction count for the removed
  row multiply before any NPU timing.
- **Focus axis:** address-expression type — row-base invariant hoist.
- **Expected evidence:** source diff shows one row-base computation per row,
  unchanged DMA descriptors and valid counts, and codegen evidence of fewer
  scalar address operations; correctness must be bitwise/common-mode before
  timing.
- **Disposition:** `READY_FOR_MAIN_REVIEW`.

### ADDR-H2 — one rolling input-address cursor across `(row,tile)`

- **Mechanism:** Keep `u`, `batchRow`, `tile`, `col`, and the local-tensor tile
  indices unchanged. Replace only the pass-1 GM element offset passed to the
  x/residual `Load` calls with one monotonic `uint64_t` cursor. Advance by
  `tileWidth` after an interior tile and by the exact row-tail correction at a
  row boundary. Pass 2 and parameter loads remain unchanged. No tile or DMA
  geometry changes.
- **Bottleneck:** repeated row multiplication plus tile multiplication and
  addition in each pass-1 current/prefetch GM address.
- **Direct parent/source SHA:** R31B V011 /
  `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- **Target shapes/dtypes:** FP16/BF16 wide low-precision D=12288, 16384 and
  32768, especially multi-row batches where the flattened unit loop is long;
  these widths are full-tile controls for tileWidth=4096. Add a non-multiple
  tail only if already supported by V011's current dispatch.
- **Why it may help:** turns x/residual GM address formation into one increment
  on the common interior path, potentially reducing scalar issue pressure and
  register dependencies on the load address.
- **Why it may fail:** the row-boundary correction is
  `rowWidth - (tileCount - 1) * tileWidth`; its calculation/selection may cost
  more than the multiply saved, and a live cursor may increase register
  pressure. A wrong correction would silently alias adjacent rows.
- **UB/DMA/sync/precision impacts:** no UB, DMA count/bytes, event, barrier,
  math, or store-policy change. The cursor is `uint64_t`; tail count remains
  the original `valid` calculation and the final cursor must equal the old
  offset sequence exactly.
- **Duplicate check:** distinct from H1: H1 hoists the row base but retains
  `rowBase + col` for GM addresses; H2 removes that GM address composition via
  a single input cursor while retaining all logical indices. No audited
  R005/R012/R028/R029, FULL, R31A/R31B or MIX evidence implements this cursor
  in the V011 wide-LP loop.
- **Related old routes:** R005 is only tile sizing, R012 is 32B alignment,
  R028 is GetValue/event reduction, R029 is mode selection; R31B V011 is the
  direct pipeline parent, while MIX-A/R31A retain explicit row offsets.
- **Minimal experiment:** keep the original `batchRow`, `tile`, `valid`, and
  unit count for control, but replace only x/residual input GM offsets with the
  cursor. Static-check the generated sequence for D=12288/16384/32768 and,
  only if currently supported, a non-multiple tail; verify all addresses against a host model
  before any correctness run.
- **Focus axis:** address-expression type — rolling element-offset cursor.
- **Expected evidence:** an address trace/model proves exact equality for all
  x/residual offsets and confirms output offsets are unchanged; codegen shows
  multiply elimination on the interior path; tail handling and register count
  are reported separately.
- **Disposition:** `READY_FOR_MAIN_REVIEW`.

### ADDR-H3 — stateful `(batchRow,tile)` index counters, no address cursor

- **Mechanism:** Replace `u / tileCount` and `u - batchRow * tileCount` with
  two scalar counters (`batchRow`, `tile`) updated by compare/increment. Keep
  the existing address formula `rowOffset + col` and all DMA arguments. This
  isolates index decode strength reduction from address-base or pointer
  changes.
- **Bottleneck:** integer divide/remainder-like index decode in every unit of
  the V011 flattened loop; this is separate from the address arithmetic itself.
- **Direct parent/source SHA:** R31B V011 /
  `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- **Target shapes/dtypes:** FP16/BF16 wide low-precision D=12288/16384/32768,
  with `tileCount` 3/4/8 and batchRows 1/4/8; include the final batch with fewer
  rows than `batchLimit`.
- **Why it may help:** a counter update can be cheaper and more predictable
  than repeated division and multiply/subtract, while preserving exact
  `(batchRow,tile)` values consumed by the old address expressions.
- **Why it may fail:** the compiler may strength-reduce the division already;
  compare/branch overhead and extra live state can erase the gain. Any counter
  rollover bug can select a valid but wrong row/tile address.
- **UB/DMA/sync/precision impacts:** no UB or tensor view change; no DMA
  count/bytes, synchronization, math, store, or dispatch change. Only scalar
  index state changes; `valid` remains derived from the same logical tile.
- **Duplicate check:** distinct from H1 (row-base hoist) and H2 (address
  cursor). R029 mode dispatch and R005 tile selection do not
  alter this index decoder; R028's scalar-sync work concerns `GetValue` and
  events, not loop index arithmetic. No R31A/R31B/MIX audit item replaces
  this flattened-loop decoder in the V011 wide-LP path.
- **Related old routes:** R005/R012/R028/R029 and FULL equivalents are
  adjacent historical scalar/address evidence; R31A V024 is arithmetic hoist
  evidence only; R31B V011 supplies the direct hot loop.
- **Minimal experiment:** preserve the old expressions in a reference helper
  and assert counter-derived `(batchRow,tile)` equality for all unit counts,
  including partial final batches. Then inspect generated code for the removal
  of integer division before requesting any implementation experiment.
- **Focus axis:** address-expression type — loop-index decode.
- **Expected evidence:** exhaustive host-side index table equality, no changed
  DMA arguments, and codegen showing whether division remains. If the compiler
  already lowers division to a multiply/shift, reject as no information gain.
- **Disposition:** `READY_FOR_MAIN_REVIEW` conditional on codegen showing a
  real index-decode cost.

## 2. Duplicate and history audit

| Audited family | OLD_MECHANISM | NEW_DIFFERENCE here | Decision |
|---|---|---|---|
| R005 / FULL-R005 | larger tile to reduce tile iterations; FULL-R005 PASS 15/15, 25.71 | no tileElems/tileCount/DMA change; only address expression form | not duplicate |
| R012 / FULL-R012 | 32B row-group/alignment and ownership; FULL-R012 PASS 15/15, 22.96 with recorded source-SHA binding discrepancy | no alignment, row grouping, or ownership change | not duplicate |
| R028 / FULL-R028 | reduce scalar `GetValue`/sync and batched RMS scalar work; FULL-R028 PASS 15/15, 25.06, with no local paired timing | no GetValue, event, barrier, or reduction change; scalar work is address/index only | not duplicate |
| R029 / FULL-R029 | legacy R029 is five-mode tiling/dispatch taxonomy; actual FULL-R029 V001 is wide cached-row and ended 4/15 Runtime Error | no dispatch, mode selection, cache residency, or tile policy change | not duplicate; keep the two R029 meanings separate |
| FULL-R022 | generated dtype/D buckets; PASS 15/15, 24.11 | no generator or dtype specialization; candidates are local source-level address forms | not duplicate |
| R023 / FULL-R023 | engineering/static checks include 64-bit addressing; not an address-performance optimization | no API, address-width, or safety-contract change | audit/reference only |
| R011 / R019 / Main-2 VECTOR-MATH-X | vector reduction/GetValue, scalar reciprocal, denominator/vector sequencing | no reduction slot indexing, scalar math, conversion, or V/S handoff change | adjacent scalar history; not duplicate |
| R31A V024 | hoist `invRms` arithmetic out of an affine loop | H1–H3 hoist/strength-reduce row/tile addresses and indices; math remains fixed | not duplicate |
| R31B V011 | 2-deep MTE2 pipeline and full-y low-precision wide path | direct frozen parent; candidates expose scalar address/index cost inside its existing loops | parent, not duplicate |
| MIX-A / R31A/R31B address history | explicit `rowOffset + col` forms and pipeline variants | no prior exact rolling cursor or counter decoder found in audited evidence | not duplicate |

The distinction is therefore `OLD_MECHANISM` versus `NEW_DIFFERENCE`, not a
new parent label. All three candidates keep V011's algorithm and dispatch.

## 2A. Cross-route duplicate audit: branch-hoist hypotheses

Compared against `HOTLOOP-BRANCH-HOIST-CHAMPION-X` hypotheses in committed
branch-hand-off commit
`01641fed8a1096d000de3a0bc1b3db13cab53f84`
(`研究/HOTLOOP-BRANCH-HOIST-CHAMPION-X/TRACK-B-HANDOFF.md`). Both routes name
the same frozen parent, R31B V011 at source SHA
`a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
Here `(a)` means address/index expressions, `(b)` row-tail or prefetch branch,
and `(c)` per-tile valid/tail length. “Adjacent” means a shared loop boundary
or possible integration surface, not the same changed expression/predicate.

| ADDR candidate | HBH-01 coefficient-mode guard | HBH-02 terminal-row prefetch guard | HBH-03 final-tile valid-length hoist |
|---|---|---|---|
| ADDR-H1 row-base hoist | (a) none; (b) none; (c) none | (a) none; (b) adjacent row-boundary bookkeeping only—the generic final-row prefetch predicate is different; (c) none | (a) none; (b) none; (c) adjacent valid-length code surface only—H1 preserves `valid` |
| ADDR-H2 rolling input cursor | (a) none; (b) none; (c) none | (a) none; (b) adjacent row-boundary correction only—not the generic prefetch predicate; (c) none (cursor correction changes an element offset, not DMA valid length) | (a) none; (b) none; (c) adjacent valid-length code surface only—H2 preserves `valid` |
| ADDR-H3 stateful row/tile counters | (a) none; (b) none; (c) none | (a) none; (b) adjacent row-counter wrap only—not the generic prefetch predicate; (c) none | (a) none; (b) none; (c) adjacent valid-length code surface only—H3 preserves `valid`; flattened traversal overlap is gone |

**Decision:** no ADDR candidate duplicates HBH-01/02/03's exact mechanism on
any of the three audited axes. HBH-02 and ADDR row-rollover state touch a
conceptual row boundary but use different predicates in different V011 paths;
HBH-03 now hoists only final-tile valid-length arithmetic while retaining the
flattened loop, `u`, `batchRow`, `tile`, every offset, and the per-unit tail
predicate. Thus the earlier ADDR-H3 × HBH-03 traversal/index overlap does not
remain: it is now adjacency only at the shared per-unit valid-length code
surface. H3 still changes index decoding while HBH-03 preserves it and hoists
only length arithmetic. No ADDR candidate is removed or selected.

MAIN-2 Wave-1 provides a useful negative-control context rather than a prior
address candidate: REDUCE-HIER-X's reduction variants did not establish a
reduction bottleneck; VECTOR-MATH-X measured V/S handoff cost at about
0.049 µs/row and its pure-vector sequence was not a win; COEFF-LOCALITY-X
found parameter MTE2 timing non-dominant at large D; ASYNC-OVERLAP-X changed
event/pipeline scheduling; MULTIROW-DMA-X changed DMA shape and is explicitly
outside this route's allowed axis. The Wave-1 campaign remains
`MAIN_SELECTED=NONE` for this route. None supplies evidence that address
integer issue is on the critical path, so codegen inspection is a precondition
for selecting a scalar-address hypothesis, not a presumed performance result.

## 3. Evidence and execution boundary

- Read-only evidence audited: AGENTS/project rules, current official ledgers,
  V011 exact source/result, R001–R029/FULL route summaries, R005/R012/R028/R029
  legacy records, R31A/R31B/MIX source/evidence, MAIN-2 wave-1 records, and
  the fetched MAIN-2 Wave-2 campaign status. The Wave-1/previous Main-2
  records report `MAIN_SELECTED=NONE` and no permission to implement here.
- No kernel/source/build file was edited. No Revision directory was created.
- No server, build, correctness, timing, profiling, or Online command was run.
- This handoff does not choose a candidate. Planning/Review must select at
  most one before any Revision declaration.

## 4. Pool status

```text
ROUTE_HYPOTHESIS_POOL_EXHAUSTED=NO
SURVIVING_INDEPENDENT_CANDIDATES=3
READY_FOR_MAIN_REVIEW=2 (H1, H2)
NEEDS_MORE_EVIDENCE=1 (H3 conditional on codegen)
MAIN_SELECTED=NONE
```

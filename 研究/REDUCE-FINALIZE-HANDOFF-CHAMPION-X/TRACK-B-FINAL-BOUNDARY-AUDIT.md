# TRACK-B FINAL BOUNDARY AUDIT

```text
ROUTE=REDUCE-FINALIZE-HANDOFF-CHAMPION-X
WORKTREE=/home/data4t2/lelinfeng/cann/worktrees/w2/m2/reduce-finalize
BRANCH=w2/m2/reduce-finalize
DATE=2026-10-01
TRACK=TRACK-B
MODE=RESEARCH_ONLY
DIRECT_PARENT=R31B V011
PARENT_OFFICIAL_SCORE=45.16
PARENT_SOURCE_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
MAIN_SELECTED=NONE
IMPLEMENTATION_APPROVED=NO
REVISION=NONE
ROUTE_HYPOTHESIS_POOL_STATE=ROUTE_HYPOTHESIS_POOL_EXHAUSTED
INDEPENDENT_SURVIVOR_COUNT=1
DUPLICATE_OR_OUT_OF_SCOPE_COUNT=4
```

## Result

The handoff-edge audit does not produce 3-5 independent candidates. It
produces one remaining research lead and four screened classes that are either
already owned by a named route or outside this route's frozen boundary.
Therefore the stronger route fact is:

```text
ROUTE_HYPOTHESIS_POOL_EXHAUSTED=YES
INDEPENDENT_SURVIVOR=RFH-1 DEFER-TERMINAL-INPUT-RELEASE-WAITS
MAIN_SELECTED=NONE
IMPLEMENTATION_APPROVED=NO
REVISION=NONE
```

This is a research classification, not a route lifecycle decision. Planning /
Review retains authority over whether RFH-1 is ever selected for an approved
experiment.

## Frozen implementation boundary

The parent source's active low-precision wide path is
`ProcessWideLowPrecision()` at `线上结果/R31B/V011/submission.asc:3076`.
The relevant source surfaces are:

| Surface | Parent fact | Route disposition |
|---|---|---|
| Per-unit reduction | FP16/BF16 `ReduceSum` emits one partial at `:3188-3203`; the `V_MTE2` release flag is set at `:3205-3210` | Frozen; no primitive, width, partial count, or reduction topology change |
| Input-slot reuse | In-loop `rel0`/`rel1` waits protect the next `xBuf_`/`residualBuf_` load at `:3146-3174` | Frozen; no in-loop wait relocation |
| Terminal input-release edge | Conditional `rel0`/`rel1` waits at `:3212-3217`, followed by ID release at `:3218-3221` | The only in-scope research variable: defer these existing waits/releases |
| Finalizer | Per-row collapse, `GetValue`, mean-square, `Sqrt`, second `GetValue`, and reciprocal at `:3224-3241` | Frozen; no scalar/vector math or `GetValue` change |
| Pass boundary | Existing `SyncVToMTE2()` at `:3243`; pass-2 parameter events and `xBuf_`/`residualBuf_` loads begin at `:3245-3265` | Deferred waits, if ever tested, must finish before this handoff |
| First consumer | FP16/BF16 scalar `Muls` at `:3320` and `:3330` | Frozen; no broadcast or consumer rewrite |

RFH-1 is conditionally feasible only as a static event-edge question: keep
the event IDs live through the finalizer, wait on the same pending input
release events, release those IDs before the existing pass-2 handoff, and do
not acquire or alias them for pass 2 first. This is not a safety proof and is
not an implementation authorization.

## Boundary audit

### R007 and R011: reduction primitive and tree

R007 is the native `ReduceSum` family (`技术路线/技术路线总表.md`, R007
row; `技术路线/路线成绩表.tsv:R007`). R011 is the manual vector-reduction
family, with the FULL-R011 result recorded as the historical vector-tree
implementation (`技术路线/技术路线总表.md`, R011 row). The later
REDUCE-HIER-X evidence makes the boundary concrete: V002 reproduces the
R011-style manual tree and is parent-favored on the primary 1x32768 probe;
V004 and V005 also replace or reorganize the reduction and are strongly
parent-favored (`技术路线/全版本记录.tsv:69,81,85`).

RFH-1 leaves every per-tile `ReduceSum`, the end-of-row `ReduceSum`, partial
bank, `tileCount`, accumulator precision, and `GetValue` sequence unchanged.
Any candidate phrased as "reduce earlier", "fold partials", "replace
ReduceSum", "use a manual tree", or "change the collapse width" is therefore
`DUPLICATE_REJECTED` for this route and belongs to the reduction route.

### R028 and VECTOR-MATH-X: scalar/vector finalizer

R028 owns scalar synchronization reduction (`技术路线/技术路线总表.md`, R028
row; `技术路线/路线成绩表.tsv:R028`). VECTOR-MATH-X V001 moves the
denominator calculation toward a vector sequence, V002 tests a vector
broadcast consumer but remains measurement-blocked, and V003's SEQ-FUSE-2
probe is mixed with the mechanism classified as closed in the route ledger
(`技术路线/全版本记录.tsv:75-76,91`; `调度/当前任务.tsv:38`).

RFH-1 deletes no `SyncVToS`, `SyncSToV`, `GetValue`, `Duplicate`, `Sqrt`,
reciprocal, or scalar-to-vector consumer operation. A scalar-only finalizer,
vector denominator, broadcast `invRms`, or changed first-consumer form is
`DUPLICATE_REJECTED`; it cannot be relabeled as a wait-placement candidate.

### REDUCE-HIER-X current boundary

REDUCE-HIER-X is explicitly restricted to RMS square-sum reduction topology.
Its recorded V002/V004/V005 outcomes are parent-favored, and its current route
state says the reduction axis is exhausted pending new bottleneck evidence
(`调度/当前任务.tsv:35`; `技术路线/路线成绩表.tsv:51`). That is independent
evidence against reopening the same reduction surfaces inside this route. RFH-1
does not reopen them; it only asks whether a pending input-release wait can be
overlapped with the already-existing finalizer.

### R31A boundary

R31A V024 hoists the `invRms` row-scale `Muls` out of the CachedRows pass-2
affine tile loop (`线上结果/R31A/V024/diff.patch`; `source-meta.json`). V025
and V026 change pass-1/pass-2 staging liveness, while V027 studies a pass
boundary staging overlap; V028 removes two batch affine barriers
(`技术路线/全版本记录.tsv:92-94,96,98`). These are pass-2 affine/staging
surfaces or an MTE3/store-side wait, not the pass-1 `V_MTE2` input-release
terminal edge in RFH-1. They remain duplicate boundaries for any proposal
that changes staging, parameter lifetime, affine barriers, or row-scale
placement.

R31A V021 is the closest event-shaped comparison: it delays an `MTE3 -> V`
output-store wait in a CachedRows loop. Its event direction, protected
storage, and pass-2 output purpose differ from RFH-1's `V -> MTE2`
pass-1 input-slot release. It is related evidence, not a duplicate
implementation of RFH-1.

### R31B boundary

R31B V017 defers an `MTE3_V` store-completion wait until output staging is
reused (`线上结果/R31B/V017/diff.patch`; `source-meta.json`). V020 tests
three-deep pass-1 MTE2 staging and is recorded infeasible because of UB
overflow (`技术路线/全版本记录.tsv:97,100`). Earlier V006/V009 queue-depth
changes are also outside RFH-1: queue depth, buffer count, and event count
are frozen here. RFH-1 adds no event, changes no queue depth, and does not
move a store drain; it only screens relocation of the two already-existing
terminal `V_MTE2` waits.

The parent and nearby route evidence also preserves the separation between
local and Official results. R31B V017 and R31A V028 were locally accepted but
Officially rejected below the 45.16 champion (`线上结果/R31B/V017/source-meta.json`,
`线上结果/R31A/V028/source-meta.json`; `技术路线/路线成绩表.tsv:49-50`).
No local or Official score is inferred for RFH-1.

## Screened candidate classes

| ID | Proposed difference | Classification | Reason |
|---|---|---|---|
| RFH-1 | Move only terminal pass-1 `rel0`/`rel1` waits and matching releases after the finalizer, before pass-2 synchronization and loads | `INDEPENDENT_RESEARCH_LEAD` | Different event direction/stage/storage edge; all reduction and scalar math remains frozen |
| RFH-2 | Remove or reorder scalar/vector finalizer work and derive `invRms` directly from scalar state | `DUPLICATE_REJECTED` | R028 and VECTOR-MATH-X boundary; changes `GetValue`/V-S/denominator math |
| RFH-3 | Keep `invRms` vectorized and broadcast into the first consumer | `DUPLICATE_REJECTED` | VECTOR-MATH-X V002/V003 and R028 boundary; changes consumer materialization |
| RFH-4 | Replace per-tile partial reduction or collapse with an eager/manual/whole-row reduction | `DUPLICATE_REJECTED` | R007/R011 and REDUCE-HIER-X boundary; changes reduction topology or primitive |
| RFH-5 | Add queue depth, pass-2 store drain, staging lifetime, parameter prefetch, or affine barrier movement | `OUT_OF_SCOPE_OR_DUPLICATE` | R31A/R31B/ASYNC boundaries; changes storage, queue, pass-2 schedule, or barrier surface |

The five rows are screening classes, not five independent candidates. Only
RFH-1 survives the duplicate and scope audit. This is why the route reports
pool exhaustion rather than inventing independent labels for already-owned
mechanisms.

## Route handoff state

```text
IN_SCOPE=static research of the active ProcessWideLowPrecision finalizer edge
ALLOWED_RESEARCH_VARIABLE=terminal rel0/rel1 V_MTE2 wait placement only
FROZEN=reduction topology; scalar/vector finalizer; precision; tile geometry;
       UB allocation; DMA shape/count/bytes; row mapping; pass count;
       first-consumer arithmetic; queue depth; staging and parameter policy
FORBIDDEN=kernel/source/build edits; Revision; build; correctness; timing;
          profiling; server3; NPU; Online; shared-control edits;
          lifecycle decisions; candidate selection
DECISION_OWNER=Planning/Review
MAIN_SELECTED=NONE
IMPLEMENTATION_APPROVED=NO
REVISION=NONE
ROUTE_HYPOTHESIS_POOL_EXHAUSTED=YES
```

The audit uses this route's committed handoffs and canonical
route/revision/score evidence as its basis. No shared scheduling or score
file is changed by this research artifact.

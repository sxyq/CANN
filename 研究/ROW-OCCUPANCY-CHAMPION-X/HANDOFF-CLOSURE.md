# ROW-OCCUPANCY-CHAMPION-X Handoff Closure

ROUTE=ROW-OCCUPANCY-CHAMPION-X
TRACK=TRACK-B ONLY
WORKTREE=/home/data4t2/lelinfeng/cann-w2-m2-occupancy
BRANCH=w2/m2/row-occupancy
DIRECT_PARENT=R31B V011
PARENT_SOURCE_SHA=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
PARENT_SCORE=45.16
OFFICIAL_ANCHOR=45.16
MAIN_SELECTED=NONE
REVISION_CREATED=NO

## Closure

The Track-B package is closed with exactly three hypotheses in
`TRACK-B-HYPOTHESES.md`:

1. `ROW-OCC-H1-BOUNDED-LOGICAL-BLOCKS`
2. `ROW-OCC-H2-NEAR-DIVISOR-CORE-COUNT`
3. `ROW-OCC-H3-FEW-ROW-ACTIVE-CORE-CAP`

They are separate OFAT concepts. H1 tests bounded logical-block
oversubscription above physical core count. H2 tests a divisor-based active
core choice while retaining the parent row-unit split. H3 tests a small-row
under-subscription cap. None uses 32B group ownership, a tile, reduction,
DMA, store, arithmetic, parameter residency, or a synchronization mechanism.

No Revision was created. No `.asc`, `.cpp`, `.h`, `.hpp`, CMake, runner, host
wrapper, canonical shared ledger, scheduler file, calibration file, or route
status file was changed. No build, server3, correctness, timing, profiling,
Online, or Judge operation was run. No lifecycle decision was made.

## Parent evidence lock

| Item | Locked fact | Evidence |
|---|---|---|
| Parent | R31B V011 | `线上结果/R31B/V011/source-meta.json` |
| Parent source SHA | `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3` | `线上结果/R31B/V011/source-meta.json`, `result.json` |
| Parent source commit | `43a1049a1e08e518c88e354a754fdebb85a96f99` | `线上结果/R31B/V011/source-meta.json` |
| Parent Official | 45.16, 15/15 | `线上结果/R31B/V011/result.json`, `技术路线/冠军/champions.tsv` |
| Parent launch rule | `min(availableCoreNum, rowCount)` | frozen source `run_kernel`, lines 3532-3540 |
| Parent narrow ownership | contiguous `baseRows/extraRows` | frozen source `Process`, lines 171-176 |
| Parent wide ownership | repeated base/extra split in wide helpers | frozen source lines 1860+, 2088+, 2254+, 2387+, 3076+ |

## Concrete duplicate audit

### R001-R029

| Route | Audited mechanism | Result for this route |
|---|---|---|
| R001 | Two-pass scan | NOT_DUPLICATE; scan count, not ownership or active-core policy |
| R002 | Retained y | NOT_DUPLICATE; residency, forbidden here |
| R003 | Pure Ascend C direct path | NOT_DUPLICATE; entry/build architecture |
| R004 | Low-precision middle | NOT_DUPLICATE; dtype/arithmetic |
| R005 | Large tile | NOT_DUPLICATE; tile, forbidden here |
| R006 | Partitioned reduction | NOT_DUPLICATE; reduction |
| R007 | Native ReduceSum | NOT_DUPLICATE; reduction |
| R008 | Tile across cores / D-slice | NOT_DUPLICATE; tile ownership, not row block count |
| R009 | DataCopyPad tail | NOT_DUPLICATE; DMA tail |
| R010 | Manual tail | NOT_DUPLICATE; store/copy tail |
| R011 | Manual vector reduction | NOT_DUPLICATE; reduction |
| R012 | 32B row-group alignment | DUPLICATE_REJECTED for any row-group ownership transplant; deliberately excluded from H1-H3 |
| R013 | Double-buffer pipeline | NOT_DUPLICATE; MTE2/V/MTE3 overlap |
| R014 | Gamma/bias residency | NOT_DUPLICATE; parameter locality |
| R015 | Multi-row DMA | NOT_DUPLICATE; DMA mechanism, explicitly forbidden |
| R016 | Row-to-AI-Core scheduling | CLOSEST_NEIGHBOR, not a direct duplicate after source audit; its recorded parent policy is D-band granularity, not H1 oversubscription, H2 divisors, or H3 small-row cap |
| R017 | FP32 middle | NOT_DUPLICATE; dtype/precision |
| R018 | CAST_RINT output | NOT_DUPLICATE; output conversion |
| R019 | Reciprocal normalization | NOT_DUPLICATE; arithmetic |
| R020 | Sqrt normalization | NOT_DUPLICATE; arithmetic |
| R021 | CANN9 build chain | NOT_DUPLICATE; build infrastructure |
| R022 | Auto code generation | NOT_DUPLICATE; code generation |
| R023 | Engineering convention | NOT_DUPLICATE; process |
| R024 | CPU validation matrix | NOT_DUPLICATE; validation |
| R025 | Submission integrity | NOT_DUPLICATE; submission channel |
| R026 | Performance measurement | NOT_DUPLICATE; measurement method |
| R027 | GPU migration reference | NOT_DUPLICATE; external design reference |
| R028 | Scalar synchronization reduction | NOT_DUPLICATE; V/S synchronization and arithmetic |
| R029 | Five-mode / wide cached-row dispatch | NOT_DUPLICATE for H1-H3; mode and wide-path edits are forbidden |

The same result applies to the corresponding `FULL-R001` through
`FULL-R029` records. `FULL-R012` and `FULL-R016` are duplicate neighbors for
group alignment and scheduling context, respectively; no FULL route records
the three exact policies proposed here.

### SCHED and champion history

| Existing item | Duplicate status | Boundary decision |
|---|---|---|
| SCHED-ROWGROUP-X V001 | DUPLICATE_REJECTED for 32B-safe group ownership, fixed task bands, cyclic/contiguous mapping, and group-floor scheduling | donor/reference only; no transplant |
| SCHED-CHAMPION-X V001 | DUPLICATE_REJECTED for group-aligned balanced ownership | H1-H3 keep row-unit parent ownership |
| SCHED-CHAMPION-X V002 | DUPLICATE_REJECTED for active-core preservation gate and group-conditioned launch policy | H1 is bounded oversubscription, H2 divisor steering, H3 small-row cap |
| SCHED-CHAMPION-X V003 | DUPLICATE_REJECTED for tail-group folding | no tail-group policy in H1-H3 |
| SCHED Track-B group-quantized launch width | DUPLICATE_REJECTED | H1 does the opposite width direction and keeps row-unit ownership; H2/H3 use different row-count rules |
| SCHED Track-B even-split task extents | DUPLICATE_REJECTED | no task extent redistribution proposed |
| SCHED Track-B cyclic group assignment | DUPLICATE_REJECTED | no cyclic group mapping proposed |
| SCHED Track-B dynamic task-pull queue | DUPLICATE_REJECTED | no atomic or device queue proposed |
| SCHED Track-B wide-cap / threshold sweep | DUPLICATE_REJECTED | no D threshold, tile, or wide-path policy is changed |

### R31A, R31B, MIX, Wave-1, and related M2 routes

| Evidence family | Audit result |
|---|---|
| R31A V008/V009 | NOT_DUPLICATE; tiny-path setup and dispatch changes, not active-core policy |
| R31A V010/V013-V016/V021-V028 | NOT_DUPLICATE; MTE pipeline, dtype, D-slice, cached-row, store, or epilogue mechanisms |
| R31B V001-V010, V012-V017 | NOT_DUPLICATE; wide residency, batch, MTE3/MTE2 pipeline, and epilogue evolution |
| R31B V011 | DIRECT_PARENT only; frozen source is the audit target |
| MIX-A V003-V007 | NOT_DUPLICATE; FastKernel dispatch and synchronization changes |
| REDUCE-HIER-X | NOT_DUPLICATE; reduction topology, and its reduction axis is closed |
| VECTOR-MATH-X | NOT_DUPLICATE; scalar/vector arithmetic sequence, including the closed SEQ-FUSE-2 axis |
| COEFF-LOCALITY-X | NOT_DUPLICATE; gamma/bias locality and UB residency |
| ASYNC-OVERLAP-CHAMPION-X | NOT_DUPLICATE; MTE2/V/MTE3 overlap and sync placement |
| MULTIROW-DMA-CHAMPION-X | NOT_DUPLICATE; stride/multi-row DMA |
| STORE-EPILOGUE-X | NOT_DUPLICATE; output-store organization |
| EPILOGUE-ARITH-CHAMPION-X | NOT_DUPLICATE; epilogue arithmetic |
| UB-LIVENESS-X | NOT_DUPLICATE; UB alias/lifetime and correctness gap |
| DTYPE-SPECIAL-X | NOT_DUPLICATE; dtype-specific copy path; dtype specialization remains forbidden here |
| INTEGRATION-X | DUPLICATE_REJECTED as a route-level stack; its rejected SCHED+VECTOR combination is not revived |

The Wave-1 and M2 audit sources are `研究/主代理/MAIN-2/CAMPAIGN-STATUS.md`,
`技术路线/路线成绩表.tsv`, `技术路线/技术路线图.md`, and the route-specific
research/evidence paths listed there. The audit is closed at this scope; no
additional repository search is required for this handoff.

## Handoff decision state

| Field | Value |
|---|---|
| Hypothesis count | 3 |
| Main selection | `MAIN_SELECTED=NONE` |
| Implementation | None |
| Revision | None created |
| Route lifecycle | No PARK/CLOSE/ABANDON/MERGE/REPLACE decision made |
| Required next owner | Planning / Review Layer selects at most one hypothesis |
| Online/Judge | Not requested and not performed |

The child recommends review priority H3, then H1, then H2 only as a review
ordering suggestion. This is not an implementation selection. Any future
approval must create a separate Revision declaration with the exact direct
parent and one-factor audit before source work begins.

## Evidence paths

- `线上结果/R31B/V011/source-meta.json`
- `线上结果/R31B/V011/result.json`
- `技术路线/冠军/champions.tsv`
- `归档/历史整理记录/review/staging/R31B-V011/src/submission.asc`
- `研究/SCHED-CHAMPION-X/TRACK-B-HYPOTHESES.md`
- `研究/SCHED-CHAMPION-X/TRACK-B-HYPOTHESES-V003.md`
- `研究/SCHED-ROWGROUP-X/next-hypotheses.md`
- `研究/主代理/MAIN-2/CAMPAIGN-STATUS.md`
- `技术路线/路线成绩表.tsv`
- `技术路线/技术路线图.md`

## Final closure statement

Track-B is complete for this handoff. The only files created are this closure
and `TRACK-B-HYPOTHESES.md` under the route-owned research directory. The
worktree contains no Candidate source or Revision artifact. `MAIN_SELECTED`
is `NONE`; the route agent has made no lifecycle or Online decision.

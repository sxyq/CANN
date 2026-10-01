# UB-LIFETIME-SAFE-CHAMPION-X — MAIN-2 Wave-2 Track-B Handoff

ROUTE: `UB-LIFETIME-SAFE-CHAMPION-X`
WORKTREE: `/home/data4t2/lelinfeng/cann-w2-m2-ub`
BRANCH: `w2/m2/ub-lifetime-safe`
TRACK: `TRACK-B ONLY`
STATUS: `RESEARCH_HANDOFF_CLOSED`
MAIN_SELECTED: `NONE`
DIRECT_PARENT: `R31B V011`
PARENT_SOURCE_SHA: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
PARENT_SCORE: `45.16`
OFFICIAL_ANCHOR: `R31B V011 / 45.16`

## Scope and closure

This file is the current Wave-2 M2-1 research handoff. It records only
statically auditable UB buffer lifetime/reuse candidates. It does not select a
candidate, create a Revision, or authorize any implementation or experiment.
The five hypotheses are mutually exclusive OFAT candidates; Planning/Review
may select at most one in a later task.

The branch already contains pre-existing V001 source/build evidence from an
earlier worker. That evidence is read-only input for this handoff:
`本地实验/UB-LIFETIME-SAFE-CHAMPION-X/V001/` records source identity,
`BUILD=PASS`, `CORRECTNESS=NOT_RUN`, `PERFORMANCE=NOT_ELIGIBLE`, and no Online
result. This task does not extend, repair, measure, or promote that artifact.

No `.asc`, `.cpp`, `.h`, `.hpp`, CMake, runner, or host-wrapper file was
changed here. No Revision was created. No build, server3, correctness, timing,
profiling, Online, Judge, shared-ledger, route-lifecycle, or `MAIN_SELECTED`
state change was performed.

## Parent facts used by every hypothesis

- The parent source is `线上结果/R31B/V011/submission.asc`; its SHA-256 is the
  `PARENT_SOURCE_SHA` above and its recorded Official score is `45.16`.
- The relevant wide low-precision path is
  `ProcessWideLowPrecision`; the parent allocates separate input, FP32 work,
  retained-y, output, and reduction buffers at `:86-97`.
- A lifetime candidate is admissible only if its named `LAST_READ` is before
  its named `REUSE_POINT`/`FIRST_WRITE`, the typed capacity is proved, and all
  outstanding MTE2/MTE3 users are drained by the parent's existing ordering.
- A static proof is not a correctness result and does not imply a performance
  result. Any unresolved API, allocator, or path-predicate question blocks a
  future candidate.

## H1 — FP16 wide FP32 scratch as output tile

HYPOTHESIS_ID: `UBX-H1-FP16-WIDE-XFP32-SCRATCH-OUTPUT`

MECHANISM: In the parent FP16 `ProcessWideLowPrecision` path only, reuse the
  `xFp32Buf_` backing as the half output tile after the pass-1 square/reduction
  work has ended. Remove only the separate wide-FP16 `outputBuf_` reservation;
  do not alias the retained half `gammaBuf_` y rows.

BOTTLENECK: A persistent wide-FP16 output tile reservation remains live in UB
  even though the FP32 scratch tile is dead before the output pass.

DIRECT_PARENT: `R31B V011`

PARENT_SOURCE_SHA: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`

TARGET_SHAPES: `[1,16384]`, `[1,32768]`, `[2,32768]`, and a non-aligned tail
  control such as `[1,16385]`, only when the exact parent path is reached.

TARGET_DTYPES: `FP16` only; wide `rowWidth > 8192`.

WHY_IT_MAY_HELP: It can remove the separate `tileElems * sizeof(half)` output
  reservation (8 KiB when `tileElems=4096`) without changing the retained-y
  representation, arithmetic, or GM transaction shape. The only claimed
  benefit at Track-B level is lower UB reservation/allocator pressure.

WHY_IT_MAY_FAIL: A float TBuf may not legally support the required aligned half
  view, the compiler may require a distinct typed allocation, or an overlooked
  FP16-path use may still read `xFp32Buf_`. Any such finding rejects the item;
  it is not permission for an unproved alias.

UB_IMPACT: Potential removal of `outputBuf_` at parent `:88`, about
  `4096 * sizeof(half) = 8192` bytes at the default tile; no new allocation.

DMA_IMPACT: GM output offsets, valid element counts, `DataCopyPad`, store count,
  and store order remain unchanged. Only the LocalTensor source backing would
  differ.

SYNC_IMPACT: Preserve the parent `SyncVToMTE2` boundary after pass 1 (`:3243`),
  the output-side `SyncVToMTE3` (`:3340`), and every `SyncMTE3ToV` drain
  (`:3344`). No event or barrier is removed or moved.

PRECISION_RISK: No arithmetic or cast mode change is intended. Risk is limited
  to same-byte alignment/capacity and compiler typed-view legality; the
  existing `CAST_ROUND` conversion must remain unchanged.

DUPLICATE_CHECK: `DISTINCT_FROM_EXISTING_V001`; the prior retained-y alias uses
  `gammaBuf_` y storage, while this item uses the separate `xFp32Buf_` backing.
  It is also distinct from the Wave-1 x-slot/phase-role alias family, which is
  `DUPLICATE_REJECTED` below. This item is research-only and not selected.

RELATED_OLD_ROUTES: `R002/FULL-R002` y/input residency (adjacent, not copied),
  `R005/FULL-R005` tile sizing, `R013/FULL-R013` pipeline, `R31B V011` full-y
  retention, and `UB-LIVENESS-X V001-V003` phase-role aliasing.

MINIMAL_EXPERIMENT: First perform a read-only source/lifetime table and typed
  capacity/alignment audit on the SHA-bound parent. No source edit or runtime
  probe is allowed in this task. A later selected child must prove that the
  FP16 specialization has no `xFp32Buf_` read after `:3231` before considering
  an exact-source build/correctness gate.

ROUTE_BOUNDARY: Only this statically bounded FP16 scratch-to-output lifetime
  transition; no live-y alias, unsafe cross-phase overlap, or timing premise.

ALLOWED_CHANGES: Research notes, source line/lifetime tables, API feasibility
  notes, and—only after external selection—a single named allocation/view OFAT
  diff in the FP16 wide path.

FORBIDDEN_CHANGES: Any current implementation; unsafe alias; arithmetic,
  reduction topology, tile size, row/core schedule, DMA geometry/order,
  pipeline overlap, synchronization removal, `.asc/.cpp/.h/.hpp`, CMake,
  runner/host wrapper, Revision, build/server3/correctness/timing/profiling,
  Online/Judge, shared ledger, route lifecycle, or `MAIN_SELECTED` change.

EVIDENCE_PATHS: `线上结果/R31B/V011/submission.asc`,
  `线上结果/R31B/V011/source-meta.json`,
  `线上结果/R31B/V011/result.json`,
  `本地实验/UB-LIFETIME-SAFE-CHAMPION-X/V001/source-meta.json`,
  `本地实验/UB-LIFETIME-SAFE-CHAMPION-X/V001/diff.patch`,
  `研究/UB-LIFETIME-SAFE-CHAMPION-X/BUFFER-LIFETIME-PROOF-V001.md`.

BUFFER: `A=xFp32Buf_` FP32 tile; `B` is the same aligned backing viewed as the
  half output tile. The parent `outputBuf_` is the separate reservation being
  considered for omission.

ALLOC: `xFp32Buf_` at parent `:94`; separate `outputBuf_` at `:88`.

FIRST_WRITE: `xFp32Buf_` is first written by the pass-1 square at `:3186`.

LAST_READ: The final parent read of the FP16-path `xFp32Buf_` contents is the
  row-level reduction input at `:3231`; the later scalar `Duplicate` at `:3236`
  writes the scratch and the FP16 pass-2 branch does not consume it.

REUSE_POINT: Bind the half output view immediately before the existing
  `FromFloat` at `:3322`; the output store remains at `:3341-3344`.

LIFETIME_PROOF: `LAST_READ(A)=:3231 < FIRST_WRITE(B)=:3322`. The parent drains
  the pass-1 dependency at `:3243`, and the next pass-1 generation of the same
  slot is after the prior output's `SyncMTE3ToV :3344`.

PROPOSED_ONE_FACTOR_DIFF: Replace only the wide-FP16 output allocation/use with
  the named `xFp32Buf_` half view; do not combine this with any other buffer
  reuse or synchronization edit.

EXPECTED_LOCAL_PROBES: Not run. A future selected child would record exact
  source identity, UB allocation/peak, typed-view compile result, and targeted
  FP16 wide correctness before any approved local timing protocol; no timing
  outcome is assumed here.

CHILD_RECOMMENDED_HYPOTHESIS: `NONE_SELECTED`

OPEN_QUESTIONS: Does the target toolchain permit this aligned float-to-half view
  with the required LocalTensor capacity? Does the compiler prove the
  compile-time FP16 branch has no later `xFp32Buf_` read? Is the allocator's
  byte accounting reduced exactly as predicted?

## H2 — FP16 wide output backing as reduction scratch, then output

HYPOTHESIS_ID: `UBX-H2-FP16-WIDE-OUTPUT-TO-REDUCE-SLOT`

MECHANISM: In the FP16 wide path, use the otherwise-unused `outputBuf_` backing
  as the FP32 per-tile reduction partial/scalar storage during pass 1 and the
  row reduction, then reuse the same bytes as the existing half output tile in
  pass 2. Omit only the independent `reduceFp32Buf_` allocation for this path.

BOTTLENECK: A small but persistent FP32 reduction buffer survives through the
  output phase although the output backing is dead until after the reduction.

DIRECT_PARENT: `R31B V011`

PARENT_SOURCE_SHA: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`

TARGET_SHAPES: FP16 wide `[1,16384]`, `[1,32768]`, and multi-row batches that
  exercise more than one `batchBegin`.

TARGET_DTYPES: `FP16` only; wide `rowWidth > 8192`.

WHY_IT_MAY_HELP: The parent `reduceFp32Buf_` is sized for
  `wideFullYRows_ * kWideFullYReduceStride` floats (at most 512 bytes for the
  current constants). Reusing the already reserved output backing may lower
  the UB live set without changing reduction operations.

WHY_IT_MAY_FAIL: The `ReduceSum` destination/workspace contract may require a
  distinct FP32 allocation or stronger alignment than a half-typed TBuf view.
  The reduction layout may also have a hidden capacity requirement. Failure of
  either proof rejects H2; no unproved type punning is allowed.

UB_IMPACT: Potential removal of `reduceFp32Buf_` at `:96-97`, up to
  `8 * 16 * sizeof(float) = 512` bytes under the parent constants; no extra
  bytes are allocated.

DMA_IMPACT: No GM address, byte count, valid count, or store operation changes.

SYNC_IMPACT: Preserve pass-1 `SyncVToMTE2`/scalar handoffs (`:3211-3243`) and
  every output MTE3 drain (`:3340-3344`). The next batch may write reduction
  scratch only after the prior batch's output store has drained.

PRECISION_RISK: Reduction primitive, operand order, scalar read, and arithmetic
  remain unchanged. Risk is limited to storage type/capacity and stale-byte
  visibility; reject if the API cannot prove the view.

DUPLICATE_CHECK: `DISTINCT_FROM_H1`; H1 removes the output reservation and
  keeps reduction storage, while H2 keeps output storage and removes only the
  reduction reservation. `R006/R007/R011/REDUCE-HIER-X` reduction topology or
  primitive changes are `DUPLICATE_REJECTED`; H2 must not change the topology.

RELATED_OLD_ROUTES: `R006/R007/R011`, `FULL-R006/FULL-R011`,
  `REDUCE-HIER-X`, `R028` scalar synchronization, and `R31B V011` reduction
  path. Only storage lifetime is in scope.

MINIMAL_EXPERIMENT: Read-only count the maximum partial/scalar elements and
  verify the output backing's byte capacity/alignment against the exact
  `ReduceSum` contract. Then prove the per-batch order from `:3188-3231` to
  `:3322-3344`; do not implement or run this probe now.

ROUTE_BOUNDARY: Only sequential reuse of the output backing between pass-1
  reduction and pass-2 output; no reduction rewrite, event change, or overlap.

ALLOWED_CHANGES: Research/API-contract notes and one future allocation/view
  OFAT at the named FP16-wide sites after Planning selection.

FORBIDDEN_CHANGES: Any current implementation; reduction primitive/tree/order
  change; unsafe typed alias; DMA, arithmetic, tile, schedule, pipeline, or
  synchronization change; `.asc/.cpp/.h/.hpp`, CMake, runner/host wrapper,
  Revision, build/server3/correctness/timing/profiling, Online/Judge, shared
  ledger, route lifecycle, or `MAIN_SELECTED` change.

EVIDENCE_PATHS: Parent source and metadata listed above;
  `研究/UB-LIFETIME-SAFE-CHAMPION-X/BUFFER-LIFETIME-PROOF-V001.md`;
  `研究/REDUCE-HIER-X/`; `本地实验/REDUCE-HIER-X/V004/`;
  `本地实验/REDUCE-HIER-X/V005/`.

BUFFER: `A=outputBuf_` viewed as FP32 reduction storage during pass 1;
  `B=the same backing` viewed as the existing half output tile in pass 2.

ALLOC: `outputBuf_` at `:88`; independent `reduceFp32Buf_` at `:96-97`.

FIRST_WRITE: Reduction partial destination at `:3188-3190` (and final scalar
  destination at `:3228-3231`) after the output backing has not yet been used.

LAST_READ: The final reduction read of the reused partials is at `:3230-3231`.

REUSE_POINT: The existing half output `FromFloat` first writes the same backing
  at `:3322`, after the scalar handoff and `SyncVToMTE2 :3243`.

LIFETIME_PROOF: `LAST_READ(A)=:3231 < FIRST_WRITE(B)=:3322`; for the next
  `batchBegin`, the preceding output role drains at `:3344` before pass 1
  reaches the next reduction destination. Capacity and typed API legality are
  explicit gates, not assumptions.

PROPOSED_ONE_FACTOR_DIFF: Replace only `reduceFp32Buf_`'s FP16-wide storage
  view with the existing output backing; preserve all `ReduceSum` calls and
  all output/store code paths.

EXPECTED_LOCAL_PROBES: Not run. A future child would record the static capacity
  proof, exact source identity, UB allocation/peak, and targeted FP16 correctness
  for one- and multi-batch wide shapes before any separate performance work.

CHILD_RECOMMENDED_HYPOTHESIS: `NONE_SELECTED`

OPEN_QUESTIONS: What exact byte/alignment and alias restrictions apply to the
  `ReduceSum` destination/workspace? Does the compiler accept the FP32 view of
  the half output TBuf? Is every next-batch partial write ordered after the
  prior MTE3 read?

## H3 — Omit dead BF16 narrow-mid FP32 parameter caches

HYPOTHESIS_ID: `UBX-H3-BF16-NARROW-MID-DEAD-PARAM-CACHE-OMISSION`

MECHANISM: On the BF16 path that dispatches to `ProcessNarrowMidOverlap`, do
  not reserve `gammaFp32Buf_` and `biasFp32Buf_`. That function loads native
  gamma/bias and never reads the full FP32 parameter caches; this is an
  allocation omission, not an alias.

BOTTLENECK: Two full 8192-element FP32 parameter-cache reservations are dead
  on the narrow-mid call path and unnecessarily raise peak UB reservation.

DIRECT_PARENT: `R31B V011`

PARENT_SOURCE_SHA: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`

TARGET_SHAPES: BF16 shapes that actually reach `ProcessNarrowMidOverlap`, for
  example `[1,256]`, `[1,1024]`, `[1,2048]`, `[1,4095]`, and an unaligned
  multi-row case such as `[2,257]`. For aligned `129<=D<=2048`, require
  `rowCount==1` (otherwise the earlier contiguous-batched path reads these
  caches); exclude `D==4096` when a core may take the full-tile batched path.

TARGET_DTYPES: `BF16` only.

WHY_IT_MAY_HELP: It can remove two `8192 * sizeof(float)` reservations, up to
  64 KiB, from a path that uses native parameter buffers and the existing
  `valueFp32Buf_`/work buffers instead.

WHY_IT_MAY_FAIL: `Init` may not be able to express the exact safe predicate
  without changing template layout, or a hidden instantiation may require the
  members. The aligned small contiguous-batched function explicitly gets both
  caches at `:1682-1683`, and the BF16 full-tile batched function gets them at
  `:631-632`; if either dispatcher can reach those functions, omit nothing and
  reject H3 for that shape.

UB_IMPACT: Potential reduction of `2 * 8192 * sizeof(float) = 65536` bytes on
  the proven narrow-mid BF16 path; no replacement buffer.

DMA_IMPACT: None. Native gamma/bias loads at `:515-516` or `:539-540` and their
  valid counts remain unchanged.

SYNC_IMPACT: Preserve `inputReady`, `paramReady`, `inputRelease`, and all
  existing V/S/MTE2/MTE3 waits in `:504-620`; no event is removed.

PRECISION_RISK: None if the call-graph predicate proves no read/write of either
  omitted cache. The guard must not alter native-to-FP32 conversion or math.

DUPLICATE_CHECK: `DISTINCT_FROM_R014/FULL-R014`, `BATCH-RESIDENT-X`, and
  `COEFF-LOCALITY-X`: those routes change parameter residency/locality, while
  H3 removes storage that is never touched on one path. It is also distinct
  from `DTYPE-SPECIAL-X`, which changes arithmetic/conversion behavior.

RELATED_OLD_ROUTES: `R014/FULL-R014`, `BATCH-RESIDENT-X`,
  `COEFF-LOCALITY-X`, `DTYPE-SPECIAL-X`, and parent BF16 generic cache logic.

MINIMAL_EXPERIMENT: Read-only dispatcher/call-graph audit from `:212-240` and
  `:499-620`, including the competing cache users at `:622-639` and
  `:1665-1688`; prove the proposed Init guard implies the narrow-mid dispatch
  for every block. In particular, aligned `129<=D<=2048` is allowed only when
  `rowCount==1`, while `2048<D<4096` cannot take the small contiguous path.
  No source edit or runtime action is allowed now.

ROUTE_BOUNDARY: Allocation omission only on the exact BF16 narrow-mid path;
  no parameter residency, conversion, DMA, arithmetic, or synchronization
  change.

ALLOWED_CHANGES: Research call-graph/lifetime tables and one future guarded
  `InitBuffer` omission after external selection and predicate proof.

FORBIDDEN_CHANGES: Any alias of the omitted buffers; path/dispatch rewrite;
  parameter prefetch/residency change; arithmetic or dtype conversion change;
  DMA/sync/schedule/tile changes; `.asc/.cpp/.h/.hpp`, CMake, runner/host
  wrapper, Revision, build/server3/correctness/timing/profiling, Online/Judge,
  shared ledger, route lifecycle, or `MAIN_SELECTED` change.

EVIDENCE_PATHS: Parent source/metadata;
  `研究/UB-LIFETIME-SAFE-CHAMPION-X/BUFFER-LIFETIME-PROOF-V001.md`;
  `研究/COEFF-LOCALITY-X/`; `本地实验/COEFF-LOCALITY-X/V003/`;
  `本地实验/COEFF-LOCALITY-X/V004/`; `调度/当前任务.tsv`.

BUFFER: `A=gammaFp32Buf_` and `biasFp32Buf_` on the BF16 narrow-mid path;
  `B` is the set of existing live native/FP32 work buffers, unchanged.

ALLOC: `gammaFp32Buf_`/`biasFp32Buf_` at `:153-155`.

FIRST_WRITE: `NONE` on the selected path; the first cache conversion writes at
  `:276-277` only in the preceding generic cache branch, which the dispatcher
  returns before entering narrow-mid.

LAST_READ: `NONE` on the selected path; `:631-639` and later cache consumers
  are not reachable after the narrow-mid return at `:239`.

REUSE_POINT: No alias or reuse is proposed. The safe reuse point is the freed
  UB reservation itself: do not allocate A for this path, leaving B unchanged.

LIFETIME_PROOF: `LAST_READ(A)=NONE` on the exact proven path. For aligned
  `129<=D<=2048`, `rowCount==1` prevents the multi-row contiguous-batched
  return; unaligned widths cannot satisfy its `%16==0` gate; and
  `2048<D<4096` cannot satisfy its width gate or the `D==4096` special path.
  The dispatcher therefore reaches `ProcessNarrowMidOverlap` before any cache
  consumer. If `Init` cannot encode a predicate with these implications, H3 is
  infeasible; no cache may be omitted.

PROPOSED_ONE_FACTOR_DIFF: Guard only the two cache `InitBuffer` calls with the
  exact proven path predicate; do not add a replacement cache or alter the
  narrow-mid function.

EXPECTED_LOCAL_PROBES: Not run. A future child would record dispatcher-path
  identity, UB allocation/peak, and BF16 narrow-mid correctness on aligned,
  unaligned, single-row, and multi-row paths before any local performance
  qualification.

CHILD_RECOMMENDED_HYPOTHESIS: `NONE_SELECTED`

OPEN_QUESTIONS: Can `Init` obtain the same local-row/path predicate without
  changing ABI or template layout? Does the allocator reserve members even when
  a guarded call is skipped? Are all special/contiguous BF16 returns excluded
  exactly?

## H4 — FP16 full-tile batched input slots as output ring

HYPOTHESIS_ID: `UBX-H4-FP16-FULL-TILE-INPUT-SLOTS-OUTPUT-RING`

MECHANISM: In the exact `rowWidth == kTileElems` and `localRows > 1` path,
  after the complete pass-1 input sweep, reuse the already-dead one-tile
  `xBuf_` and `residualBuf_` slots as the two existing output-ring slots.
  Omit the separate two-tile `outputBuf_` allocation for this path only.

BOTTLENECK: The special FP16 full-tile batched path keeps an 8192-half output
  reservation even though both input slots are dead before its output loop.

DIRECT_PARENT: `R31B V011`

PARENT_SOURCE_SHA: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`

TARGET_SHAPES: `[2,4096]`, `[4,4096]`, and other FP16 `[rows,4096]` cases that
  dispatch to `ProcessFp16FullTileBatchedOutputPipelined`.

TARGET_DTYPES: `FP16` only.

WHY_IT_MAY_HELP: It can remove the separate `kCacheElems * sizeof(half)`
  output reservation (16 KiB) while retaining two same-typed tile slots from
  the input buffers. It does not propose deeper buffering or event changes.

WHY_IT_MAY_FAIL: A hidden input use may remain after the first pass, or the
  path predicate used by `Init` may admit a generic/full-row caller that still
  needs `outputBuf_`. The two slot mappings must also match existing store
  release flags exactly.

UB_IMPACT: Potential removal of `8192 * sizeof(half) = 16384` bytes for the
  exact full-tile batched path; no additional storage.

DMA_IMPACT: Same output GM row offsets, tile counts, valid counts, and
  `DataCopyPad` calls; only the two local output sources change.

SYNC_IMPACT: Preserve `outputReadyFirst/Second`, `outputReleaseFirst/Second`,
  `WaitFlag` before slot reuse, and the final drains at `:864-873`. Preserve
  the pass-1 input release at `:804`; no event depth or wait is removed.

PRECISION_RISK: Same FP16 `FromFloat`, `Mul`, and `Add` sequence and same typed
  LocalTensor kind. Risk is limited to slot lifetime, capacity, and event-order
  proof.

DUPLICATE_CHECK: `DISTINCT_FROM_H1/H2`; it is a same-dtype special-path
  input-slot reuse, not a wide retained-y alias or reduction-storage reuse.
  `ASYNC-TRIPLE-X`/`R013` store-ring changes are `DUPLICATE_REJECTED`; H4
  preserves the existing ring rather than changing overlap.

RELATED_OLD_ROUTES: `R013/FULL-R013`, `ASYNC-TRIPLE-X`,
  `ASYNC-OVERLAP-CHAMPION-X`, `STORE-EPILOGUE-X`, and parent FP16 full-tile
  output pipeline.

MINIMAL_EXPERIMENT: Read-only prove that the full first pass has no future
  `xBuf_`/`residualBuf_` read after `:804`, that no next-row prefetch exists in
  this function, and that the two slot capacities equal one output tile. Do
  not alter the event code or run the path now.

ROUTE_BOUNDARY: Existing full-tile output ring only; no new ring, overlap,
  store batching, sync removal, or pipeline redesign.

ALLOWED_CHANGES: Research function/path/lifetime tables and one future
  same-typed slot-binding/allocation OFAT after Planning selection.

FORBIDDEN_CHANGES: Event-ring redesign; wait/barrier removal or movement;
  multi-row DMA/store merge; arithmetic, tile, schedule, or dtype changes;
  unsafe alias; `.asc/.cpp/.h/.hpp`, CMake, runner/host wrapper, Revision,
  build/server3/correctness/timing/profiling, Online/Judge, shared ledger,
  route lifecycle, or `MAIN_SELECTED` change.

EVIDENCE_PATHS: Parent source/metadata;
  `研究/UB-LIFETIME-SAFE-CHAMPION-X/BUFFER-LIFETIME-PROOF-V001.md`;
  `研究/ASYNC-TRIPLE-X/`; `研究/STORE-EPILOGUE-X/`;
  `本地实验/ASYNC-OVERLAP-CHAMPION-X/V001/`.

BUFFER: `A=xBuf_` and `B=residualBuf_`, each one FP16 tile during pass 1;
  after pass 1 they become the first/second output-ring slots. `outputBuf_` is
  the separate two-tile reservation considered for omission.

ALLOC: `xBuf_` and `residualBuf_` at `:116-117`; `outputBuf_` at `:132`.

FIRST_WRITE: Input DMA writes A/B at `:793-794` for each batch row.

LAST_READ: The final vector read of A/B in this function is the native add at
  `:796`; the conversion at `:798` reads xLocal, and `SyncVToMTE2 :804`
  completes the input-buffer release. There is no successor input prefetch in
  this function.

REUSE_POINT: The first output `FromFloat` writes the selected x/residual output
  slot at `:845`; existing store issue/drain is unchanged at `:851-855`.

LIFETIME_PROOF: `LAST_READ(A/B)=:798 < FIRST_WRITE(output-slot)=:845`, with
  `SyncVToMTE2 :804` before the output loop. Each later slot reuse remains
  after its existing `MTE3_V` release wait.

PROPOSED_ONE_FACTOR_DIFF: For only this dispatcher-selected path, map the two
  existing output-ring roles to x/residual slots and omit `outputBuf_`; do not
  change event IDs, flags, waits, or store calls.

EXPECTED_LOCAL_PROBES: Not run. A future child would record exact path
  identity, slot-capacity/lifetime proof, UB allocation/peak, and targeted
  FP16 correctness for two and four rows before any separate timing work.

CHILD_RECOMMENDED_HYPOTHESIS: `NONE_SELECTED`

OPEN_QUESTIONS: Can `Init` derive the exact `rowWidth==4096 && localRows>1`
  predicate without ABI changes? Does the compiler keep the input slots
  available after `:804`? Do the existing first/second event flags exactly
  cover every output slot reuse for odd batch-row counts?

## H5 — Generic FP16 single-row input slot as output

HYPOTHESIS_ID: `UBX-H5-FP16-GENERIC-SINGLE-ROW-INPUT-OUTPUT`

MECHANISM: In the generic FP16 path with exactly one local row and no successor
  input prefetch, reuse the single `xBuf_` tile as `outputLocal` after its
  pass-1 input lifetime ends. Omit the generic-path `outputBuf_` reservation
  only if the dispatcher/Init predicate is proven exact.

BOTTLENECK: A generic single-row FP16 output tile remains allocated although
  the cached y row means the original x input slot is dead before output
  conversion.

DIRECT_PARENT: `R31B V011`

PARENT_SOURCE_SHA: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`

TARGET_SHAPES: Single-row generic-path FP16 shapes such as `[1,4097]`,
  `[1,6144]`, `[1,8192]`, plus a small generic control `rowWidth <= 128`;
  exclude narrow-mid, contiguous-batched, and full-row/full-tile special
  returns.

TARGET_DTYPES: `FP16` only.

WHY_IT_MAY_HELP: It may remove the generic FP16 tile reservation, up to 8 KiB
  for the parent `kTileElems` allocation, using an existing same-typed input
  slot after its last read. The hypothesis makes no latency claim.

WHY_IT_MAY_FAIL: The generic dispatcher may share Init state with a path that
  needs `outputBuf_`, or a hidden prefetch/consumer may retain xBuf_ beyond the
  source lines audited. A path predicate that cannot distinguish these cases
  makes H5 infeasible rather than permitting a broad alias.

UB_IMPACT: Potential removal of `fp16OutputElems * sizeof(half)` at parent
  `:132` for the exact single-row generic path: 8192 bytes when
  `fp16OutputElems=4096`, or 16384 bytes for the `rowWidth==8192` allocation
  branch. This is an allocation amount, not a predicted latency gain.

DMA_IMPACT: Output GM offsets, valid counts, `DataCopyPad`, store count, and
  order stay identical.

SYNC_IMPACT: Preserve the input release at `:336`, the parameter wait and
  output handoff at `:477-481`, and all existing MTE3 drains. No wait is
  deleted or used as a performance assumption.

PRECISION_RISK: Same FP16 output conversion and affine operations; same-typed
  xBuf output avoids a cross-dtype view. Risk is only path/lifetime proof and
  delayed DMA reuse.

DUPLICATE_CHECK: `DISTINCT_FROM_H4` because H4 is the exact multi-row
  full-tile function with a two-slot ring; H5 is generic single-row with one
  slot. It does not copy `STORE-EPILOGUE-X` writeback merging or
  `ASYNC-OVERLAP-CHAMPION-X` scheduling.

RELATED_OLD_ROUTES: `R002` y caching, `R013` pipeline, `STORE-EPILOGUE-X`,
  `ASYNC-OVERLAP-CHAMPION-X`, and the parent generic FP16 path.

MINIMAL_EXPERIMENT: Read-only prove the exact generic dispatcher predicate,
  absence of the next-row prefetch for `localRows==1`, and the xBuf last-read
  at `:321/:336` before the output write at `:440`. Do not widen the predicate
  or edit source in this task.

ROUTE_BOUNDARY: Single-row generic FP16 slot reuse only; no special-path
  dispatch change, no prefetch, DMA, arithmetic, schedule, or sync change.

ALLOWED_CHANGES: Research path/lifetime notes and one future same-typed
  allocation/view OFAT after external selection and predicate proof.

FORBIDDEN_CHANGES: Broad output-buffer removal; alias on multi-row/prefetch
  paths; unsafe alias; dispatch, DMA, event, arithmetic, tile, or schedule
  changes; `.asc/.cpp/.h/.hpp`, CMake, runner/host wrapper, Revision,
  build/server3/correctness/timing/profiling, Online/Judge, shared ledger,
  route lifecycle, or `MAIN_SELECTED` change.

EVIDENCE_PATHS: Parent source/metadata;
  `研究/UB-LIFETIME-SAFE-CHAMPION-X/BUFFER-LIFETIME-PROOF-V001.md`;
  `本地实验/UB-LIFETIME-SAFE-CHAMPION-X/V001/diff.patch`;
  `研究/STORE-EPILOGUE-X/`; `研究/ASYNC-OVERLAP-CHAMPION-X/`.

BUFFER: `A=xBuf_` one FP16 input tile; `B` is the same one-tile backing as
  `outputLocal` for the exact generic single-row path.

ALLOC: `xBuf_` at `:116`; separate FP16 `outputBuf_` at `:132`.

FIRST_WRITE: Input loads at `:304-305`.

LAST_READ: For the no-prefetch single-row generic path, the last xBuf/residual
  consumer is the native add and y conversion at `:319-321`; the input release
  completes at `:336`.

REUSE_POINT: The FP16 `FromFloat` first writes the xBuf-backed output view at
  `:440`, before the existing affine operations and store.

LIFETIME_PROOF: `LAST_READ(A)=:321 < FIRST_WRITE(B)=:440`, with the existing
  input release at `:336` and parameter/output handoff before `:477`. The proof
  is invalid for any path that prefetches a successor row, so such paths are
  explicitly excluded.

PROPOSED_ONE_FACTOR_DIFF: Guard only the generic single-row FP16 output view
  and its matching allocation omission; preserve all special dispatches and
  all existing synchronization.

EXPECTED_LOCAL_PROBES: Not run. A future child would record exact dispatcher
  identity, no-prefetch proof, UB allocation/peak, and targeted FP16 correctness
  for one small and one medium generic shape before any timing qualification.

CHILD_RECOMMENDED_HYPOTHESIS: `NONE_SELECTED`

OPEN_QUESTIONS: Can Init and Process share an exact single-row generic-path
  predicate? Is `localRows` available without changing ABI? Are there any
  compiler-generated uses of xBuf_ between `:336` and `:440`?

## Concrete duplicate audit

DUPLICATE_AUDIT_STATUS: `PASS_WITH_DUPLICATES_REJECTED`

The following mechanisms are explicitly rejected and must not be revived as
new hypotheses:

- `DUPLICATE_REJECTED — UB-LIVENESS-X V001-V003`: phase-role/x-slot aliasing
  and live-y alias architecture. The current H1/H4/H5 name exact dead ranges
  and preserve the parent event order; none reuses the old alias pool.
- `DUPLICATE_REJECTED — pre-existing UB-LIFETIME-SAFE-CHAMPION-X V001`: the
  retained FP16 y in `gammaBuf_` is already the historical V001 concept. This
  handoff does not reselect or extend it; H1 uses `xFp32Buf_`, H2 uses the
  output backing as reduction storage, H3 is an allocation omission, and H4/H5
  are separate generic/special input-slot paths.
- `DUPLICATE_REJECTED — prior unselected value-tile-to-reduction proposal`:
  aliasing `valueFp32Buf_` with `residualFp32Buf_` depends on an unresolved
  `ReduceSum` workspace contract and is not carried as a child hypothesis.
- `DUPLICATE_REJECTED — R002/FULL-R002`: full-y/input residency or reread
  elimination. The candidates do not add y residency or change GM reads.
- `DUPLICATE_REJECTED — R005/FULL-R005 and R029`: tile-size or five-mode
  tiling changes. Tile widths and dispatch remain fixed.
- `DUPLICATE_REJECTED — R006/R007/R011/FULL-R006/FULL-R011 and
  REDUCE-HIER-X`: reduction tree, primitive, or topology changes. H2 keeps
  the exact calls and order.
- `DUPLICATE_REJECTED — R013/FULL-R013, R028, ASYNC-TRIPLE-X, and
  ASYNC-OVERLAP-CHAMPION-X`: pipeline, event-ring, or scalar-sync changes.
  Every candidate preserves existing waits and flags.
- `DUPLICATE_REJECTED — R014/FULL-R014, BATCH-RESIDENT-X, and
  COEFF-LOCALITY-X`: parameter residency, locality, or multi-row DMA. H3
  removes caches only where the call graph proves they are never accessed.
- `DUPLICATE_REJECTED — R015/FULL-R015 and MULTIROW-DMA-CHAMPION-X`: stride,
  cross-row, or descriptor-batching DMA. No candidate changes GM geometry.
- `DUPLICATE_REJECTED — R016, SCHED-ROWGROUP-X, and SCHED-CHAMPION-X`: row
  ownership or core scheduling. Row mapping is fixed.
- `DUPLICATE_REJECTED — STORE-EPILOGUE-X`: intra-row writeback merging and
  store-call reduction. H4/H5 keep one Store call per existing output tile.
- `DUPLICATE_REJECTED — EPILOGUE-FUSE-X and EPILOGUE-ARITH-CHAMPION-X`:
  epilogue arithmetic/fusion. No arithmetic instruction or order changes.
- `DUPLICATE_REJECTED — DTYPE-SPECIAL-X`: dtype-specific arithmetic or cast
  specialization. H3 only removes unreachable BF16 cache allocation.
- `DUPLICATE_REJECTED — MIX-A, R009/R010, ALIGN-TAIL-X, and all external
  schedule/DMA compositions`: mixed mechanisms, tail geometry, and alignment
  changes are outside this route boundary.

## Common route contract and child handoff

ROUTE_BOUNDARY: Only statically provable buffer lifetime/reuse or dead
  allocation omission. Every future item must record `BUFFER`, `ALLOC`,
  `FIRST_WRITE`, `LAST_READ`, `REUSE_POINT`, capacity, and the ordering proof.
  No unsafe alias, timing assumption, implementation, or route decision.

ALLOWED_CHANGES: Append/update research files in this route, static source and
  call-graph audits, API feasibility notes, and a future single selected OFAT
  storage-lifetime diff only after Planning/Review approval.

FORBIDDEN_CHANGES: Any `.asc/.cpp/.h/.hpp`, CMake, runner, host wrapper,
  kernel, build, server3, correctness, timing, profiling, Online, Judge,
  Revision creation, shared canonical ledger, `MAIN_SELECTED`, or route
  lifecycle state.

EVIDENCE_PATHS: `AGENTS.md`; `.agents/skills/cann-mainline/SKILL.md`;
  `项目规则/实验总则.md`; `项目规则/执行约定.md`;
  `项目规则/本地性能测试规范.md`; `项目规则/服务器实验规范.md`;
  `项目规则/线上提交规范.md`; `项目规则/Git工作流程.md`;
  `技术路线/技术路线总表.md`; `技术路线/技术路线图.md`;
  `技术路线/路线成绩表.tsv`; `技术路线/全版本记录.tsv`;
  `调度/当前任务.tsv`; `线上结果/R31B/V011/`;
  `本地实验/UB-LIFETIME-SAFE-CHAMPION-X/V001/`;
  `研究/UB-LIFETIME-SAFE-CHAMPION-X/` and the related-route paths listed in
  the duplicate audit.

PROPOSED_ONE_FACTOR_DIFF: Planning/Review selects at most one of H1-H5; the
  future diff may touch only that hypothesis's named allocation/view site. No
  child selection is made here.

EXPECTED_LOCAL_PROBES: `NOT_RUN_IN_TRACK_B`. If externally selected later,
  first verify source identity, typed capacity, allocation/lifetime proof, and
  targeted correctness. Only after those gates may the approved same-binary
  and Parent/Candidate timing protocol be considered; no local or Official
  score is inferred by this handoff.

CHILD_RECOMMENDED_HYPOTHESIS: `NONE_SELECTED`

OPEN_QUESTIONS: Which typed LocalTensor views and `ReduceSum` workspace roles
  are legal on the target compiler? Can exact dispatcher/local-row predicates
  be shared between `Init` and `Process` without ABI changes? What byte-level
  UB reservation does the allocator report after a proven omission? Are all
  asynchronous MTE2/MTE3 users covered by the existing waits?

HANDOFF_DECISION: `RETURN_TO_PLANNING_REVIEW`

MAIN_SELECTED: `NONE`

CURRENT_CANDIDATE: `NONE_SELECTED_FOR_THIS_TASK`

ONLINE: `FORBIDDEN`

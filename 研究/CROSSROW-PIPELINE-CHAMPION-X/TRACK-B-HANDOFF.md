# CROSSROW-PIPELINE-CHAMPION-X - Wave-2 Track-B Handoff

ROUTE=CROSSROW-PIPELINE-CHAMPION-X
WAVE=WAVE-2
TRACK=TRACK-B-ONLY
DATE=2026-09-30
BRANCH=w2/m2/crossrow-pipeline
WORKTREE=/home/data4t2/lelinfeng/cann-w2-m2-crossrow
MAIN_SELECTED=NONE
ROUTE_LIFECYCLE=UNCHANGED
CURRENT_STATUS=RESEARCH_ONLY
ROUTE_BOUNDARY=SEE_SECTION
ALLOWED_CHANGES=SEE_SECTION
FORBIDDEN_CHANGES=SEE_SECTION
EVIDENCE_PATHS=SEE_SECTION
PROPOSED_ONE_FACTOR_DIFF=SEE_SECTION
EXPECTED_LOCAL_PROBES=SEE_SECTION
OPEN_QUESTIONS=SEE_SECTION

DIRECT_PARENT=R31B V011
PARENT_SOURCE_SHA=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
PARENT_OFFICIAL_SCORE=45.16
PARENT_SOURCE=线上结果/R31B/V011/submission.asc

NO_REVISION_CREATED=YES
KERNEL_MODIFIED=NO
BUILD_LINK=NO
CORRECTNESS=NO
TIMING=NO
PROFILING=NO
SERVER3=NO
ONLINE=NO
CANONICAL_SHARED_LEDGER_MODIFIED=NO

## ROUTE_BOUNDARY

Only study stage overlap between row N and row N+1 on the same executing core. The
allowed stage edges are Store(N)/Load(N+1), Affine(N)/ReducePrep(N+1), a fixed
two-row ping-pong, depth=2 stage credits, and one stage-boundary relaxation.

This route does not change which core owns a row. A proposed overlap is useful only
when the existing dispatch already gives one core `localRows >= 2`; `localRows == 1`
is an explicit inert control. The proposal must preserve each row's arithmetic
order and its per-row output address order.

## ALLOWED_CHANGES

- Research diagrams, pseudocode, dependency tables, duplicate audits, and a future
  one-factor implementation sketch.
- In a future Main-approved Revision, reorder or gate existing row-N/row-N+1 stage
  issue and wait points within one of the five boundary forms above.
- Existing two-row storage can be assigned as alternating row contexts when the
  lifetime proof fits the parent UB plan.
- Per-row copies remain per-row copies; the only proposed cross-row effect is
  issue-time overlap.

## FORBIDDEN_CHANGES

- Any `.asc`, `.cpp`, `.h`, `.hpp`, CMake file, runner, host wrapper, or kernel edit
  in this Track-B turn.
- Any Revision directory, build, link, correctness run, timing run, profiling,
  server3 activity, Online/Judge activity, or shared-ledger edit.
- Multi-row DMA aggregation, strided multi-row DataCopy, or changing copy count by
  merging rows.
- Row-to-core ownership, block count, scheduling policy, row grouping, or occupancy
  changes.
- Tile size, tile count policy, arithmetic fusion, affine algebra changes, precision
  policy, reduction topology, or store/writeback policy.
- `MAIN_SELECTED=YES`, route lifecycle decisions, PARK/CLOSE/ABANDON/MERGE/REPLACE,
  or a claim that this route is selected for implementation.

## BASELINE_STAGE_MAP

The frozen parent has two different relevant behaviors:

1. The generic/narrow row loops are row-serial. In the parent generic path, the
   row loop and first-pass/output-pass boundary are visible in
   `线上结果/R31B/V011/submission.asc` around the `Process()` body. In
   `ProcessNarrowMidOverlap`, the next row starts only after the previous input
   release and `SyncMTE3ToV`; its input load, RMS preparation, affine chain, and
   Store are therefore not a cross-row pipeline.
2. R31B V011 already has a two-deep MTE2 pipeline over the `(row, tile)` stream in
   its wide low-precision path. A proposal that only renames that existing wide
   pipeline is not new. The candidate set below targets the serial generic/narrow
   paths or changes the row-boundary lifetime relation itself.

The key bottleneck question is therefore not whether an intra-row pipeline exists,
but whether a second row can safely occupy the next stage without changing row
ownership, tile geometry, arithmetic, or Store policy.

## HYPOTHESES

The eligible set is H2, H4, and H5. H1 and H3 are included as explicit audit
records because they are natural starting points, but are rejected before Main
selection when the committed evidence shows a mechanism collision.

### H1 - Store(N) overlap Load(N+1)

HYPOTHESIS_ID=CROSSROW-H1-STORE-LOAD
STATUS=DUPLICATE_REJECTED
MECHANISM=Defer the row-boundary MTE3 completion wait so row N+1 x/residual Load can be issued while row N Store is outstanding; keep the same per-row copies and output addresses.
BOTTLENECK=The row-boundary Store drain blocks the next row's input prologue.
DIRECT_PARENT=R31B V011
PARENT_SOURCE_SHA=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
TARGET_SHAPES=Existing localRows>=2 narrow/mid shapes such as 64x256 FP32, 33x100 FP32, 128x2048 FP16; only if the selected path uses separate input/output storage.
TARGET_DTYPES=FP32 and FP16 are the safe first candidates; BF16 is excluded where output staging aliases xBuf_ until a separate lifetime proof exists.
WHY_IT_MAY_HELP=It could hide the MTE3 tail behind the next row's MTE2 input issue.
WHY_IT_MAY_FAIL=The parent may already issue the next-row prologue before the Store boundary, or the Store source may alias the next-row input buffer. A short Store/load window can also be below noise.
UB_IMPACT=No intended capacity change, but aliasing must be proven per dtype.
DMA_IMPACT=Same per-row byte count and same copy primitives; no aggregation.
SYNC_IMPACT=High; only the wait location changes, so an early slot reuse would be a correctness failure.
PRECISION_RISK=Low arithmetic risk; high race risk if MTE3 completion is released too early.
DUPLICATE_CHECK=DUPLICATE_REJECTED: Wave-1 ASYNC-OVERLAP-CHAMPION-X V002 is explicitly `W2 NarrowMid inter-row issue reorder (hoist next-row x/res load before invRms tail; defer SyncMTE3ToV)`. That is the same Store/load boundary experiment on the same Champion parent. The older EPI-PIPE-3 note is the broader version of the same row-epilogue/prologue idea. There is no independent experiment value left for this route.
RELATED_OLD_ROUTES=R013/FULL-R013 and ASYNC-TRIPLE-X are adjacent intra-row pipelines; R31A V021 is same-row MTE3 wait deferral. None makes H1 new after ASYNC-OVERLAP V002.
MINIMAL_EXPERIMENT=Do not create a Revision. The prior V002 evidence is the only admissible experiment for this exact mechanism; any re-run would need Planning/Main ownership approval and would not be a new hypothesis.

### H2 - Affine(N) overlap ReducePrep(N+1)

HYPOTHESIS_ID=CROSSROW-H2-AFFINE-REDUCEPREP
STATUS=READY_FOR_MAIN_REVIEW
MECHANISM=After row N has committed its RMS partial state, begin row N+1 input preparation (x/residual Load plus the existing y/square/partial-reduction preparation) while row N executes its existing Affine chain `Muls -> Mul -> Add`. Keep the two rows' reduction state separate and do not overlap the Store boundary.
BOTTLENECK=The current row's affine work leaves the next row's MTE2 and reduction-prep prologue cold; the exposed cost is a per-row stage transition rather than a copy-count problem.
DIRECT_PARENT=R31B V011
PARENT_SOURCE_SHA=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
TARGET_SHAPES=Rows greater than the existing block count with localRows>=2: 64x256 FP32, 33x100 FP32, 128x2048 FP16, and one 128x4096 FP32 confirmation if the parent dispatch reaches the same path.
TARGET_DTYPES=FP32 first; FP16 second. BF16 is a follow-up only after an output/input UB alias audit.
WHY_IT_MAY_HELP=MTE2 input work and the next row's early preparation can progress during current-row affine issue. It attacks a boundary that H1 does not touch and can remove a cold prologue without changing Store behavior.
WHY_IT_MAY_FAIL=Affine and ReducePrep both use the Vector pipeline, so there may be no physical overlap after issue arbitration. The scalar RMS tail or one shared reduction slot may serialize the rows, and event/setup cost may exceed the hidden MTE2 time.
UB_IMPACT=Medium. Two row-specific reduction/validity states are needed, but no larger tile or multi-row payload is allowed. If the parent buffers cannot provide two lifetimes without a broad UB redesign, classify this hypothesis infeasible rather than changing UB policy.
DMA_IMPACT=Same per-row x/residual bytes and command count; no strided or aggregated DMA.
SYNC_IMPACT=High. The current row must finish its reduction read and affine dependencies before its output is committed; the next row must wait on its own input-ready and reduction-ready events. No cross-row event may be reused before its row slot is released.
PRECISION_RISK=Low if each row preserves the parent's operation order. Interleaving rows must not change reduction order or convert a correctness race into a numerical claim.
DUPLICATE_CHECK=ADJACENT_DISTINCT: EPI-PIPE-3 proposed generic epilogue/prologue overlap and Wave-1 ASYNC-OVERLAP V002 tested Store/load plus invRms-tail ordering. H2 starts the next row's ReducePrep and overlaps it with the current row's Affine; it does not defer the MTE3 Store wait and does not merely hoist a first Load. R013/ASYNC-TRIPLE overlap MTE2/V/MTE3 within one row/tile stream. The independent experiment value is separating input/reduction-prep overlap from the already-tested Store/load boundary.
RELATED_OLD_ROUTES=R013/FULL-R013, ASYNC-TRIPLE-X, ASYNC-OVERLAP-CHAMPION-X V002/V004, R31A V020, and the parent R31B V011 wide `(row,tile)` pipeline. R015 and R016 are explicitly excluded because they aggregate DMA or change ownership.
MINIMAL_EXPERIMENT=From V011, change only the row-loop issue order so one next-row ReducePrep prefix is admitted during current-row Affine; keep tile geometry, row mapping, arithmetic calls, Store calls, and all non-boundary paths byte-identical. This is a future Main-approved OFAT sketch, not an implementation in this handoff.

### H3 - Explicit two-row ping-pong

HYPOTHESIS_ID=CROSSROW-H3-TWO-ROW-PINGPONG
STATUS=DUPLICATE_REJECTED
MECHANISM=Keep row N and row N+1 as two explicit row contexts and alternate their ReducePrep/Affine stage tokens through a fixed two-row state machine, with row-local reduction metadata and output lifetime tracked independently.
BOTTLENECK=The serial row loop pays the prologue, affine, and boundary bookkeeping cost before admitting the next row.
DIRECT_PARENT=R31B V011
PARENT_SOURCE_SHA=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
TARGET_SHAPES=Any existing path with localRows>=2, with 64x256 FP32 and 33x100 FP32 as the cleanest two-row-per-core stress cases; 128x2048 FP16 as a dtype transfer check.
TARGET_DTYPES=FP32 first, FP16 only after slot lifetime is proved.
WHY_IT_MAY_HELP=Persistent two-row state can keep one row in Affine while the other row occupies the next stage, avoiding repeated open/close costs at each row boundary.
WHY_IT_MAY_FAIL=The two-row state machine can require more live row values than the parent UB plan, and a shared scalar/reduction slot can force the same serialization. Vector issue contention may make the state machine slower.
UB_IMPACT=Potentially high even with only two logical rows; no new tile or row aggregation is allowed. A capacity increase beyond two row slots violates this route boundary.
DMA_IMPACT=Per-row copies unchanged; no multi-row descriptor or stride aggregation.
SYNC_IMPACT=Very high. Every row context needs independent input-ready, reduction-ready, and output-release ownership. Incorrect alternation can corrupt a neighboring row even when arithmetic is unchanged.
PRECISION_RISK=Low arithmetic risk, high lifetime/race risk. The per-row reduction and affine order must remain identical to V011.
DUPLICATE_CHECK=DUPLICATE_REJECTED: The committed EPI-PIPE-3 research note already proposes restructuring the row loop into a software pipeline where row N epilogue overlaps row N+1 pass-1 prologue. H3 is a more explicit implementation form of that same cross-row pipeline, not a new performance mechanism. The required independent value is therefore already represented by the EPI-PIPE-3 audit and must not consume a new Revision slot.
RELATED_OLD_ROUTES=EPI-PIPE-3 in 研究/NEXT-TRACK-B-OVERNIGHT.md; R013/FULL-R013 and ASYNC-TRIPLE-X are narrower intra-row two-stage/triple-stage donors; R015 is forbidden multi-row DMA and R016 is forbidden row scheduling.
MINIMAL_EXPERIMENT=Do not create a Revision. Keep H3 as a design explanation for Main's duplicate audit; do not repackage EPI-PIPE-3 under a new route name.

### H4 - Depth=2 cross-row stage credits

HYPOTHESIS_ID=CROSSROW-H4-DEPTH2-CREDITS
STATUS=READY_FOR_MAIN_REVIEW
MECHANISM=Give the serial generic/narrow row path two logical credits for the ReducePrep stage: credit A remains owned by row N while its Affine is issued, and credit B admits only row N+1's next preparation. Preserve the existing stage order and do not create a full two-row software pipeline or move the MTE3 Store boundary.
BOTTLENECK=One outstanding row-prep lifetime may make the next-row MTE2/V work wait conservatively even when the current row has stopped consuming its input buffers.
DIRECT_PARENT=R31B V011
PARENT_SOURCE_SHA=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
TARGET_SHAPES=localRows>=2 generic/narrow cases: 64x256 FP32, 33x100 FP32, and 128x2048 FP16. Use a localRows==1 shape from the same path as the inert control.
TARGET_DTYPES=FP32 first; FP16 second; BF16 only after confirming that its output staging does not consume the alternate input credit.
WHY_IT_MAY_HELP=It tests whether queue/lifetime depth, rather than arithmetic or DMA volume, is the limiting boundary. A second credit lets the next row's prep be in flight while the current row's affine finishes.
WHY_IT_MAY_FAIL=R31B V006/V009 and ASYNC-TRIPLE already show that generic queue-depth increases can be flat, and the narrow path may not have enough independent work to use a second credit. Extra event operations can cost more than the hidden work.
UB_IMPACT=Small-to-medium metadata and one alternate stage slot; hard cap is depth=2. No larger tile, row cache, or third buffer is permitted.
DMA_IMPACT=No change to per-row bytes, copy count, or address calculation. Only the time at which the next row's existing copy is issued changes.
SYNC_IMPACT=High but narrower than H3: add/retain per-credit ready/release ownership, while the row's arithmetic and Store waits remain unchanged. The final two credits must be drained before return.
PRECISION_RISK=Low if credit reuse is correct; no arithmetic order or dtype conversion changes are part of the hypothesis.
DUPLICATE_CHECK=ADJACENT_DISTINCT: R31B V006/V009 changed queue depth or queue placement for intra-row/tile paths, and ASYNC-TRIPLE uses depth-2 rings for MTE2/V/MTE3 within a row. H4 applies depth=2 to the cross-row ReducePrep credit in the serial generic/narrow path; it does not add MTE3 overlap, aggregate DMA, or change row ownership. H3 is a full row-state machine; H4 is a bounded credit/lifetime test. The independent experiment value is to distinguish depth pressure from control-loop restructuring.
RELATED_OLD_ROUTES=R013/FULL-R013, ASYNC-TRIPLE-X, R31B V006/V009, R31B V011 wide pipeline, and UB-LIVENESS-X. These are donors or adjacent evidence, not proof of this cross-row credit mechanism.
MINIMAL_EXPERIMENT=Only change the number and release condition of ReducePrep stage credits from one to two in the selected generic/narrow path; do not edit tile constants, row mapping, Store policy, arithmetic, or the wide V011 path. Main must first approve the exact buffer/event footprint.

### H5 - One Affine/ReducePrep boundary relaxation

HYPOTHESIS_ID=CROSSROW-H5-AFFINE-REDUCEPREP-RELAX
STATUS=READY_FOR_MAIN_REVIEW
MECHANISM=Keep one row context and one existing stage credit, but move exactly one `WaitFlag<MTE2_V>`/input-ready boundary so row N+1's first ReducePrep input issue can begin before row N's Affine completion; the actual next-row vector reduction still waits for its own input-ready event.
BOTTLENECK=A conservative wait placement exposes the first next-row MTE2 latency even though row N's Affine has independent value/output storage.
DIRECT_PARENT=R31B V011
PARENT_SOURCE_SHA=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
TARGET_SHAPES=localRows>=2 short-to-mid rows: 64x256 FP32, 33x100 FP32, and 128x2048 FP16. A localRows==1 case must be a no-effect control.
TARGET_DTYPES=FP32 first; FP16 second; exclude BF16 until buffer aliasing is proven.
WHY_IT_MAY_HELP=It isolates the value of a single stage-boundary relaxation without the UB/event complexity of H3 or H4. The next row's input transfer can overlap current-row Affine while preserving the next row's data dependency.
WHY_IT_MAY_FAIL=The moved wait may already be hidden by the parent issue stream, the next-row input may alias a live output source, or the exposed window may be too short. It can also regress if an earlier MTE2 command starves current Affine.
UB_IMPACT=None in the intended form; it must use the existing single stage slot and must not create a second row payload.
DMA_IMPACT=Unchanged per-row copy bytes and count; no aggregation.
SYNC_IMPACT=Medium-high. Exactly one wait moves; all producer/consumer waits and final drains remain. If a second credit is needed, this hypothesis has become H4 and is invalid as an OFAT.
PRECISION_RISK=Low arithmetic risk; synchronization/lifetime risk remains the primary correctness risk.
DUPLICATE_CHECK=ADJACENT_DISTINCT: H1 is rejected because it relaxes the Store(N)->Load(N+1) MTE3 boundary, already tested by ASYNC-OVERLAP V002. H5 relaxes only the Affine(N)->ReducePrep(N+1) MTE2/V readiness boundary and leaves MTE3 untouched. H2 performs actual next-row ReducePrep work; H5 only tests the single wait placement, so it has independent value as the smallest causal probe. It must not be combined with H2 or H4.
RELATED_OLD_ROUTES=ASYNC-OVERLAP V002/V004, SYNC-TOPOLOGY-CHAMPION-X H2/H3, R31B V019, and MIX-A V007 are adjacent wait-placement evidence. Their same-row or Store-boundary changes do not establish H5.
MINIMAL_EXPERIMENT=Move one existing cross-row input-ready wait in one selected generic/narrow function; no second buffer, no new queue depth, no Store wait change, no arithmetic change. First prove the parent path has two rows per core, then run only this future single-factor Revision if Main selects it.

## PROPOSED_ONE_FACTOR_DIFF

There is no source diff in this handoff. If Main later selects one item, the direct
parent remains V011 and only the corresponding schedule axis may change:

- H2: admit one next-row ReducePrep prefix during current-row Affine.
- H4: change only ReducePrep stage credit/lifetime from one to depth=2.
- H5: move only one Affine/ReducePrep readiness wait.

H2, H4, and H5 are mutually exclusive OFAT candidates. H1 and H3 are audit-only
and must not be revived under a new identifier.

## EXPECTED_LOCAL_PROBES

No probe has run in this Track-B turn. A future Main-approved Revision must use the
current local protocol, with the exact V011 parent binary first:

1. Establish `localRows>=2` from the existing dispatch for 64x256 FP32 and 33x100
   FP32; use 128x2048 FP16 as a transfer probe. Do not force block count or row
   ownership to manufacture a two-row case.
2. Use one `localRows==1` shape from the same path as an inert control. A zero delta
   there is required evidence that the mechanism is cross-row, not proof of failure.
3. Run correctness and source/executable identity checks before any timing. Then use
   current warmup, same-binary qualification, and interleaved device-event Parent /
   Candidate pairs only if the exact shape qualifies.
4. Preserve raw samples and classify noise/measurement blockage separately from the
   architecture. Do not use the Wave-1 short-kernel or wide-FP32 nondeterministic
   paths as the sole acceptance basis.

## EVIDENCE_PATHS

- `线上结果/R31B/V011/source-meta.json`
- `线上结果/R31B/V011/submission.asc`
- `线上结果/R31B/V011/diff.patch`
- `线上结果/R31B/V011/result.json`
- `技术路线/路线成绩表.tsv`
- `技术路线/全版本记录.tsv`
- `技术路线/技术路线总表.md`
- `研究/NEXT-TRACK-B-OVERNIGHT.md` (EPI-PIPE-3 audit source)
- `研究/ASYNC-TRIPLE-X/ROUTE-BRIEF.md`
- `研究/ASYNC-TRIPLE-X/next-hypotheses.md`
- `本地实验/ASYNC-TRIPLE-X/V001/MAIN-REVIEW.md`
- `本地实验/ASYNC-TRIPLE-X/V001/HANDOFF-TRACK-A-20260925.md`
- `本地实验/ASYNC-OVERLAP-CHAMPION-X/V001/`
- `本地实验/ASYNC-OVERLAP-CHAMPION-X/V002/`
- `本地实验/ASYNC-OVERLAP-CHAMPION-X/V003/`
- `本地实验/ASYNC-OVERLAP-CHAMPION-X/V004/`
- `本地实验/MULTIROW-DMA-CHAMPION-X/V001/`
- `本地实验/MULTIROW-DMA-CHAMPION-X/V002/`
- `本地实验/R31A/V020/source-meta.json`
- `本地实验/R31A/V021/source-meta.json`
- `研究/主代理/MAIN-2/CAMPAIGN-STATUS.md`
- `研究/主代理/MAIN-2/COMPLIANCE-REVIEW.md`
- `研究/主代理/MAIN-2/ONLINE-GAP-AUDIT.md`
- committed Wave-2 sibling handoffs on `origin/w2/m1/*` were used only as
  duplicate-boundary evidence; no sibling worktree or private writable directory
  was read or modified.

## DUPLICATE_AUDIT_R001_R029

The following is an explicit mechanism audit. `DISTINCT-OUTSIDE-SCOPE` means the
historical mechanism is not being claimed by this route. `DUPLICATE_REJECTED` is
reserved for a real collision or a forbidden repackaging; it is not a claim that
the item was never tested.

| old route | historical mechanism | audit result | real difference / independent value |
|---|---|---|---|
| R001 | two-pass scan | DISTINCT-OUTSIDE-SCOPE | Changes pass count, not row-N/N+1 stage issue. No value for this route. |
| R002 | retained y / one-read residency | DISTINCT-OUTSIDE-SCOPE | Changes data residency and read count, not stage boundary. |
| R003 | pure direct-call/API path | DISTINCT-OUTSIDE-SCOPE | Build/entry mechanism only. |
| R004 | low-precision middle | DISTINCT-OUTSIDE-SCOPE | Arithmetic/dtype mechanism; forbidden here. |
| R005 | larger tile | DISTINCT-OUTSIDE-SCOPE | Tile geometry; explicitly forbidden. |
| R006 | partial reduction architecture | DISTINCT-OUTSIDE-SCOPE | Reduction topology; H2 only changes when existing prep is admitted. |
| R007 | native ReduceSum | DISTINCT-OUTSIDE-SCOPE | Reduction primitive, not row overlap. |
| R008 | tile across cores | DISTINCT-OUTSIDE-SCOPE | Cross-core ownership/scheduling, not same-core row N/N+1. |
| R009 | DataCopyPad/copy-centric tail | DISTINCT-OUTSIDE-SCOPE | Tail copy policy, not stage overlap. |
| R010 | manual tail | DISTINCT-OUTSIDE-SCOPE | Tail arithmetic/copy policy, not stage overlap. |
| R011 | manual vector reduction | DISTINCT-OUTSIDE-SCOPE | Reduction instruction topology, not stage overlap. |
| R012 | 32B row-group alignment | DISTINCT-OUTSIDE-SCOPE | Row grouping/ownership and alignment; not a row pipeline. |
| R013 | MTE2/V/MTE3 double buffer | ADJACENT-DISTINCT | Its proven donor overlaps tiles within one row; H2/H4 target a cross-row ReducePrep credit. Reusing R013 without this boundary proof is a duplicate. |
| R014 | Gamma/Bias residency | DISTINCT-OUTSIDE-SCOPE | Parameter residency, not row stage overlap. |
| R015 | multi-row DMA aggregation | DUPLICATE_REJECTED | Forbidden collision: R015 changes DMA transaction shape by aggregating rows. This route keeps per-row DMA and measures only issue overlap. |
| R016 | row-to-core scheduling | DUPLICATE_REJECTED | Forbidden collision: R016 changes ownership/load balance. This route assumes existing ownership and requires localRows>=2. |
| R017 | FP32 all-middle | DISTINCT-OUTSIDE-SCOPE | Precision policy only. |
| R018 | CAST_RINT output | DISTINCT-OUTSIDE-SCOPE | Output rounding only. |
| R019 | reciprocal/invRms normalization | DISTINCT-OUTSIDE-SCOPE | Arithmetic sequence only. |
| R020 | sqrt normalization | DISTINCT-OUTSIDE-SCOPE | Arithmetic primitive only. |
| R021 | CANN9 build chain | DISTINCT-OUTSIDE-SCOPE | Build infrastructure only. |
| R022 | code generation | DISTINCT-OUTSIDE-SCOPE | Generation mechanism only. |
| R023 | engineering convention | DISTINCT-OUTSIDE-SCOPE | Process convention only. |
| R024 | CPU validation matrix | DISTINCT-OUTSIDE-SCOPE | QA evidence only. |
| R025 | submission integrity | DISTINCT-OUTSIDE-SCOPE | Submission identity only. |
| R026 | performance measurement | DISTINCT-OUTSIDE-SCOPE | Measurement protocol only. |
| R027 | GPU migration reference | DISTINCT-OUTSIDE-SCOPE | External design reference, no same mechanism evidence. |
| R028 | scalar synchronization reduction | ADJACENT-DISTINCT | Changes cross-row scalar/reduction synchronization. H2/H5 retain the parent reduction and only change stage admission; no GetValue batching. |
| R029 | five-mode tiling/dispatch | DISTINCT-OUTSIDE-SCOPE | Mode and tile dispatch; forbidden here. |

FULL-R001..FULL-R029 are the one-shot implementations of the same R001..R029
axes. In particular, FULL-R013 is the intra-row double-buffer donor, FULL-R015 is
the forbidden multi-row DMA axis, and FULL-R016 is the forbidden scheduling axis;
none is a new cross-row stage experiment.

## DUPLICATE_AUDIT_CHAMPION_MIX_AND_WAVE1

- **R31A**: V020 pipelines MTE2/V for the next tile inside a wide FP32 row;
  V021 defers MTE3-to-V completion within a row. They are adjacent same-row
  evidence, not cross-row ownership. H1 must not be recreated as V021 with a row
  index.
- **R31B**: V006/V009 are queue-depth/placement experiments on intra-row or wide
  paths; V011 already has a two-deep `(row,tile)` MTE2 pipeline in its wide LP
  path; V018/V019 remove or retain same-row V-side synchronization. H4 is only
  new if it targets the serial generic/narrow ReducePrep credit and proves that
  the wide V011 path is not the subject.
- **MIX-A**: V003/V007 adjust V-to-MTE2 release and `localRows==1` behavior in
  narrow/mid paths. They do not establish a two-row Affine/ReducePrep pipeline.
  Any proposal that changes `localRows` eligibility or row ownership is
  `DUPLICATE_REJECTED` under this route boundary.
- **EPI-PIPE-3** in `研究/NEXT-TRACK-B-OVERNIGHT.md`: exact broad
  epilogue/prologue row-pipeline idea; H3 is `DUPLICATE_REJECTED`. H2 remains
  only because it narrows the experimental question to Affine(N) versus actual
  ReducePrep(N+1), while H5 isolates one readiness wait.
- **ASYNC-TRIPLE-X V001**: MTE2/V/MTE3 triple overlap within a row/tile stream;
  adjacent-distinct, not a cross-row proposal. Its timing was blocked and cannot
  be used as positive performance evidence.
- **ASYNC-OVERLAP-CHAMPION-X V001**: same-row inter-pass parameter prefetch;
  outside this route. **V002 is the exact H1 collision** and is
  `DUPLICATE_REJECTED`; V003 is V/S handoff sequencing; V004 is FullCache
  parameter prefetch with an xBuf scratch hazard. They do not make H2/H4/H5
  proven.
- **MULTIROW-DMA-CHAMPION-X V001/V002**: V001 aggregates multi-row stride DMA;
  V002 changes aligned copy form. Both are `DUPLICATE_REJECTED` or
  `OUT_OF_BOUNDARY_REJECTED` for this route. The real difference is that every
  proposed H2/H4/H5 keeps row copies separate and changes only stage timing.
- **SCHED-ROWGROUP-X / SCHED-CHAMPION-X**: row-group ownership and 32B safe row
  assignment; `DUPLICATE_REJECTED` for any cross-core or row-to-core variant.
- **SYNC-TOPOLOGY-CHAMPION-X**: committed Wave-2 H1/H2/H3 handoff changes
  same-row MTE3 wait, parameter MTE2 wait, or Muls placement. It is adjacent
  sync evidence, not proof of cross-row overlap. H5 must retain a cross-row
  label and cannot silently become a same-row sync revision.
- **STORE-EPILOGUE-W2-X**: output Store issue/chunk policy; `DUPLICATE_REJECTED`
  and forbidden here. H1 does not survive its ASYNC V002 collision either.
- **EPILOGUE-ARITH-CHAMPION-X**: affine arithmetic hoisting/fusion; forbidden
  arithmetic axis. H2/H4/H5 preserve the existing affine operations and order.
- **SHAPE-TILING-CHAMPION-X**: tile/UB shape selection; forbidden tile axis.
- **w2/m2/interpass-pipeline**: no committed handoff for this sibling was
  present in the current worktree audit, so no `NOT_DUPLICATE` claim is made.
  Main must resolve ownership before selecting any cross-row inter-pass item.
- **w2/m2/row-occupancy**: any row-to-core or occupancy change is
  `DUPLICATE_REJECTED` by this route boundary. **w2/m2/param-residency** is
  adjacent parameter locality, not a row-stage overlap. **w2/m2/ub-lifetime-safe**
  is an ownership dependency for H3/H4, not a reason to add a third buffer.

## CHILD_RECOMMENDED_HYPOTHESIS

CHILD_RECOMMENDED_HYPOTHESIS=CROSSROW-H5-AFFINE-REDUCEPREP-RELAX
RECOMMENDATION_STATUS=RECOMMENDATION_ONLY
REASON=H5 gives the cleanest causal test of the still-unsettled boundary without
repeating Wave-1 Store/load overlap, changing queue depth, changing row state
ownership, or adding a broad two-row software pipeline. H2 is the higher-upside
follow-up if H5 proves that the single wait is load-bearing. H4 should be selected
only if Main confirms a real one-credit lifetime bottleneck.
MAIN_SELECTION=NONE

## OPEN_QUESTIONS

1. Which existing V011 generic/narrow path and block count produce `localRows>=2`
   for the official shape set without changing row ownership?
2. For each dtype, which of xBuf_, residualBuf_, value/output buffers, and the
   reduction partial area remain live across the N/N+1 boundary?
3. Can the required row-local reduction state fit a depth=2 design without changing
   tile size, row residency policy, or the parent UB allocation plan?
4. Does the 910B event model permit the proposed cross-row MTE2/V admission while
   preserving the existing final drains, or does it serialize at the same boundary?
5. Can Main confirm that `w2/m2/interpass-pipeline` has no overlapping ownership
   claim before any H2/H4/H5 implementation is approved?
6. How will wide-FP32 parent nondeterminism be excluded from this route's probe
   acceptance so a common-mode artifact is not attributed to cross-row overlap?

## HANDOFF_CHECK

HANDOFF_SCOPE=OWN_ROUTE_RESEARCH_ONLY
SHARED_LEDGER_EDITED=NO
REVISION_CREATED=NO
KERNEL_EDITED=NO
SERVER3_CONTACTED=NO
ONLINE_ATTEMPTED=NO
MAIN_SELECTED=NONE

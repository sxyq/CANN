# PARAM-RESIDENCY-CHAMPION-X Track-B Handoff

ROUTE=PARAM-RESIDENCY-CHAMPION-X
BRANCH=w2/m2/param-residency
WORKTREE=/home/data4t2/lelinfeng/cann-w2-m2-param
TRACK=TRACK-B-ONLY
BASELINE=R31B-V011
BASELINE_OFFICIAL=45.16
PARENT_SCORE=45.16
PARENT_SOURCE_SHA=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
CONTEXT_CLASS=FROZEN_STRONG_BASELINE_PARAM_RESIDENCY_RESEARCH
WHY_NOT_DUPLICATE=Only bounded gamma/bias consumer lifetime is studied; broad residency, prefetch timing, multi-row DMA, scheduling, arithmetic, store merge, and tile search are explicitly rejected by the audit.
MAIN_SELECTED=NONE
IMPLEMENTATION_SELECTED=NO
REVISION_CREATED=NO
KERNEL_CHANGED=NO
SERVER_RUN=NO

## ROUTE_BOUNDARY

Study only gamma/bias lifetime across rows or bounded row groups, with the goal of reducing repeated global-to-UB parameter traffic. The row-to-core mapping, arithmetic sequence, tile search, and output-store behavior are fixed. A gamma+bias proposal is gated on prior evidence from a gamma-only or bias-only probe; it is not a first implementation.

## ALLOWED_CHANGES

- Research notes, pseudocode, footprint accounting, duplicate audit, and handoff metadata under this Route only.
- Future single-mechanism proposals for gamma-only, bias-only, gamma+bias after evidence, two-row reuse, bounded small-row-group parameter load, or a minimal parameter-lifetime interaction with the existing row pipeline.
- Existing V011 tile width, row ownership, dtype path, arithmetic, and runner remain the reference in every proposal.

## FORBIDDEN_CHANGES

- Any `.asc`, `.cpp`, `.h`, `.hpp`, CMake, runner, host wrapper, or generated kernel change in this Track-B handoff.
- Creating a Revision, build/link, correctness, timing, profiling, server3 activity, Online/Judge activity, or score change.
- Store merge, row scheduling, rows-per-block or mode selection, arithmetic fusion, tile search, new DMA stride/multi-row input scheme, or new synchronization topology.
- Modifying `技术路线/`, `调度/`, another Route, another worktree, or another child context.
- Setting `MAIN_SELECTED=YES`, selecting a child implementation, or changing Route lifecycle.

## BASELINE_FACTS

- The formal baseline is R31B V011, 15/15, Official 45.16. `技术路线/冠军/champions.tsv` and `技术路线/冠军/R31B-V011/RESULT_REF.txt` bind the source and submission SHA to `PARENT_SOURCE_SHA` above.
- The V011 wide low-precision path already has a two-deep input pipeline and a two-deep gamma/bias parameter MTE2 pipeline. It retains complete y for a bounded batch and applies one parameter tile to all retained rows in that batch.
- The V011 FP32 wide path is the useful unexplained surface for this Route: parameter staging is per output tile in `ProcessWideFp32FullCacheRows`; it does not inherit the low-precision two-deep parameter path.
- The committed Wave-1 evidence rejects parameter-load timing alone at constant `tileElems=4096`: COEFF-LOCALITY-X V003 was parent-favored on the primary shapes, while V001 lost tile budget and V002 reported about 16 KiB remaining UB with a 32 KiB gamma+bias tile not fitting.
- The same evidence says that a generic full-row preload or unrestricted stripe residency is exhausted. Any surviving idea must therefore reuse a bounded row lifetime or an already-live buffer and must state its UB cost before implementation.
- `kCacheElems=8192`, `kWideFullYTileElems=4096`, and the V011 wide FP16 batch output tile is 4096 elements. These values are evidence anchors, not permission to change tile sizing.

## HYPOTHESES

### H1

HYPOTHESIS_ID=H1-GAMMA-TWO-ROW-LIFETIME
MECHANISM=Keep one gamma parameter tile live while two adjacent rows consume the same tile; bias remains a per-row load. Use the existing row order and parameter tile boundaries.
BOTTLENECK=Repeated gamma global-to-UB traffic in a multi-row FP32 wide output path when the parent reloads the same gamma tile for each row.
DIRECT_PARENT=R31B-V011
PARENT_SOURCE_SHA=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
TARGET_SHAPES=FP32 wide D=16384 and 32768 with localRows>=2; D=8192/localRows>=2 as a control; rowCount 2-8 only for the reuse probe.
TARGET_DTYPES=FP32 primary; FP16/BF16 are controls only because V011 already shares one parameter tile across its retained low-precision batch.
WHY_IT_MAY_HELP=It can remove up to one of two gamma loads per two-row group without changing arithmetic, tile width, row ownership, or output stores. The benefit is proportional to gamma load latency rather than total parameter bytes.
WHY_IT_MAY_FAIL=V011 may already retain gamma for the relevant FP32 path, localRows may be one after block partitioning, or parameter MTE2 may be hidden by input/compute work. A new gamma buffer can shrink the tile and erase the saving.
UB_IMPACT=One FP32 gamma tile is P*4 bytes; at P=4096 this is 16 KiB, or P*2 bytes for FP16/BF16 if ever applicable. If no existing buffer can be safely aliased, tile-shrink risk is HIGH; COEFF V001 demonstrated 4096->2560 after UB staging pressure. Extra-sync risk is MEDIUM: one reuse handoff per two-row group is required, but no new event family should be added.
DMA_IMPACT=Gamma bytes and gamma load transactions fall by at most 2x for the reused rows; bias traffic is unchanged. No stride DataCopy or multi-row DMA is allowed.
SYNC_IMPACT=Prefer the existing parameter-ready and parameter-release events with one lifetime extension across the second row. A per-tile or per-row new barrier would likely consume the gain; extra-sync risk MEDIUM.
PRECISION_RISK=No arithmetic change. The main risk is stale gamma data at a row boundary or a race with reuse of the staging buffer; FP32 values must be byte-identical to the parent when the same tile is consumed.
DUPLICATE_CHECK=PASS_WITH_SCOPE. The unrestricted gamma/bias cache in R014/FULL-R014/R030 is DUPLICATE_REJECTED; this is gamma-only, exactly two rows, fixed tiles, and no wide-row cache redesign. COEFF generic preload and R31A parameter staging-liveness are adjacent but change staging lifetime/prefetch, not this bounded gamma-only consumer lifetime.
RELATED_OLD_ROUTES=R014; FULL-R014; R030/FULL-R030; COEFF-LOCALITY-X V001/V004; R31A V025/V026; R31B V011.
MINIMAL_EXPERIMENT=After Planning approval, one FP32-only source change that retains one gamma tile for exactly two existing adjacent rows. Use one D=16384 and one D=32768 shape with localRows>=2, plus the D=8192 control. First verify UB footprint and exact output bytes, then use the prescribed same-binary and interleaved P/C protocol. Do not combine bias reuse or pipeline reordering.
ROUTE_BOUNDARY=Gamma only, exactly two existing adjacent row consumers; no row scheduling, row-group ownership, tile search, arithmetic fusion, store merge, or DMA-shape change.
ALLOWED_CHANGES=One gamma consumer-lifetime extension using existing V011 parameter tile/event machinery after Planning approval.
FORBIDDEN_CHANGES=Bias reuse, P/tile shrink, row mapping, arithmetic, store merge, stride/multi-row DMA, new event family, or new barrier.
ASCEND_FEASIBILITY=MEDIUM. Existing single-tile parameter loads and existing ready/release events are reusable; feasibility depends on proving a two-row lifetime without allocating beside live V011 buffers.
CORE_IMPACT=NONE. Block count, row ownership, row order, and per-core work are unchanged.
EVIDENCE_PATHS=技术路线/冠军/champions.tsv; 技术路线/冠军/R31B-V011/RESULT_REF.txt; 本地实验/COEFF-LOCALITY-X/V001/; 本地实验/COEFF-LOCALITY-X/V003/; 技术路线/全版本记录.tsv
PROPOSED_ONE_FACTOR_DIFF=Gamma-only lifetime extension across exactly two existing adjacent rows; P, bias lifetime, DMA form, arithmetic, and event topology unchanged.
EXPECTED_LOCAL_PROBES=Exact source/UB/lifetime audit; deterministic FP32 correctness with distinct gamma/bias; same-binary qualified FP32 D=16384, D=32768, D=8192 control; >=4 interleaved P/C pairs only after qualification.
CHILD_RECOMMENDED_HYPOTHESIS=NONE
OPEN_QUESTIONS=Does the exact FP32 wide path reload gamma per row, and can an existing slot retain it through the second consumer without a tile shrink or additional fence?

## LATEST MAIN-2 GATE CLOSURE

AUDIT_DATE=2026-10-05
MAIN_SELECTED=PARAM-RESIDENCY/H1-GAMMA-TWO-ROW-LIFETIME
SOURCE_AUDIT=研究/PARAM-RESIDENCY-CHAMPION-X/PARAM-H1-SOURCE-AUDIT.md
DUPLICATE_AUDIT=研究/PARAM-RESIDENCY-CHAMPION-X/PARAM-H1-DUPLICATE-AUDIT.md
PARAM_H1_GATE=FAIL_DUPLICATE
PARAM_H1_EXISTENCE_GATE=FAIL_NO_RUNTIME_MECHANISM
IMPLEMENTATION_AUTHORIZED=NO
REVISION_CREATED=NO
BUILD=NOT_RUN
CORRECTNESS=NOT_RUN
PERFORMANCE=NOT_RUN
ONLINE=FORBIDDEN
NEXT_ACTION=RETURN_TO_PLANNING_FOR_A_DIFFERENT_NON-DUPLICATE_HYPOTHESIS

The exact V011 FP32 wide path already loads one gamma tile and consumes it
across the two-row full-y batch at D=16384. D=32768 cannot retain two rows
under the fixed parent UB budget without changing the batch/tile/UB
mechanism. D=8192 already reuses preloaded parameters on the regular cached
path. No Param-Residency V001 was created.

### H2

HYPOTHESIS_ID=H2-BIAS-TWO-ROW-LIFETIME
MECHANISM=Keep one bias parameter tile live while two adjacent rows consume the same tile; gamma remains a per-row load. The lifetime boundary is deliberately the opposite of H1.
BOTTLENECK=Repeated bias global-to-UB traffic in a multi-row FP32 wide output path, independent of gamma load behavior.
DIRECT_PARENT=R31B-V011
PARENT_SOURCE_SHA=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
TARGET_SHAPES=FP32 wide D=16384 and 32768 with localRows>=2; D=8192/localRows>=2 control; rowCount 2-8 only.
TARGET_DTYPES=FP32 primary; FP16/BF16 controls only.
WHY_IT_MAY_HELP=It isolates the second parameter stream and can remove repeated bias loads without changing gamma timing or arithmetic. It provides information even if gamma has a different cache or consumer lifetime.
WHY_IT_MAY_FAIL=Bias may be consumed or aliased differently from gamma, the parent may already retain bias, or the two-row population may not occur on the official partition. A dedicated bias tile has the same tile-budget danger as H1.
UB_IMPACT=One FP32 bias tile is P*4 bytes; at P=4096 it is 16 KiB, or P*2 bytes for FP16/BF16. Without safe aliasing, tile-shrink risk is HIGH for the same reason as COEFF V001. Extra-sync risk is MEDIUM and must stay within existing parameter-ready/release events.
DMA_IMPACT=Bias bytes and bias load transactions fall by at most 2x for the reused rows; gamma traffic and all x/residual traffic are unchanged. No stride or multi-row DMA.
SYNC_IMPACT=One bias lifetime handoff across the second row; no new pipeline stage or event family. Extra-sync risk is MEDIUM because a conservative wait can dominate a single tile.
PRECISION_RISK=No arithmetic change. A stale bias is an additive error that can pass some symmetric tests, so exact output comparison must include nonzero and varied bias values.
DUPLICATE_CHECK=PASS_WITH_SCOPE. Broad gamma+bias residency in R014/FULL-R014/R030 is DUPLICATE_REJECTED; this is bias-only and two-row bounded. COEFF V003 split-phase emission is DUPLICATE_REJECTED as a loading-timing change, not reused bias lifetime. H2 remains distinct from H1 by the sole resident parameter.
RELATED_OLD_ROUTES=R014; FULL-R014; R030/FULL-R030; COEFF-LOCALITY-X V003/V004; BATCH-RESIDENT-X; R31B V011.
MINIMAL_EXPERIMENT=After Planning approval, one FP32-only source change that retains only bias for exactly two existing adjacent rows. Use the same shape/control matrix as H1 and verify that gamma remains byte-identical to the parent path. Do not change gamma, row mapping, tile width, or arithmetic.
ROUTE_BOUNDARY=Bias only, exactly two existing adjacent row consumers; no gamma lifetime change, row scheduling, row-group ownership, tile search, arithmetic fusion, store merge, or DMA-shape change.
ALLOWED_CHANGES=One bias consumer-lifetime extension using existing V011 parameter tile/event machinery after Planning approval.
FORBIDDEN_CHANGES=Gamma reuse, P/tile shrink, row mapping, arithmetic, store merge, stride/multi-row DMA, new event family, or new barrier.
ASCEND_FEASIBILITY=MEDIUM. The operation is a single existing bias load with a bounded lifetime extension; exact feasibility depends on bias-buffer alias/liveness and existing event ordering.
CORE_IMPACT=NONE. Block count, row ownership, row order, and per-core work are unchanged.
EVIDENCE_PATHS=技术路线/冠军/champions.tsv; 技术路线/冠军/R31B-V011/RESULT_REF.txt; 本地实验/COEFF-LOCALITY-X/V003/; 本地实验/COEFF-LOCALITY-X/V004/; 技术路线/全版本记录.tsv
PROPOSED_ONE_FACTOR_DIFF=Bias-only lifetime extension across exactly two existing adjacent rows; P, gamma lifetime, DMA form, arithmetic, and event topology unchanged.
EXPECTED_LOCAL_PROBES=Exact source/UB/lifetime audit with distinct nonzero bias; deterministic FP32 correctness; same-binary qualified FP32 D=16384, D=32768, D=8192 control; >=4 interleaved P/C pairs only after qualification.
CHILD_RECOMMENDED_HYPOTHESIS=NONE
OPEN_QUESTIONS=Does the exact FP32 wide path reload bias per row, and is its staging lifetime independently extendable without stale additive coefficients or an extra wait?

### H3

HYPOTHESIS_ID=H3-GAMMA-BIAS-TWO-ROW-PAIR
MECHANISM=Keep the gamma+bias pair live for exactly a two-row group and consume both rows before releasing the pair. This is a gated pair hypothesis, not a first change.
BOTTLENECK=Residual repeated loads of both coefficient streams after a positive gamma-only or bias-only result establishes that the parameter lifetime, rather than arithmetic or DMA command shape, matters.
DIRECT_PARENT=R31B-V011
PARENT_SOURCE_SHA=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
TARGET_SHAPES=Only shapes that first show a positive, source-verified H1 or H2 signal: FP32 D=16384/32768 with localRows>=2; D=8192 control.
TARGET_DTYPES=FP32 first; FP16/BF16 only if a preceding single-parameter probe proves repeated parameter traffic on that exact parent path.
WHY_IT_MAY_HELP=It can remove both coefficient reloads for the second row while preserving the existing tile and row ownership. It is the maximum traffic reduction inside the two-row scope.
WHY_IT_MAY_FAIL=The pair costs twice the staging capacity, may force a smaller tile, may require a long parameter lifetime across output stores, and can add waits that exceed the saved loads. COEFF V002 already shows the raw UB ceiling for a 32 KiB FP32 pair at P=4096.
UB_IMPACT=Two FP32 tiles are 2*P*4 bytes; at P=4096 this is 32 KiB, or 2*P*2 bytes for FP16/BF16. Tile-shrink risk is VERY HIGH and the proposal is INFEASIBLE unless both tiles alias already-dead storage with no live-value conflict. Extra-sync risk is HIGH; a pair release must not race either row's output.
DMA_IMPACT=Both gamma and bias load bytes/transactions can fall by at most 2x over the two rows. No multi-row DataCopy, stride descriptor, or input/output DMA change.
SYNC_IMPACT=Requires pair-ready and pair-release lifetime across two row consumers. No new event topology is allowed; if existing events cannot express this without an extra wait, mark the experiment infeasible.
PRECISION_RISK=No arithmetic change, but stale or cross-row pair reuse corrupts both multiplicative and additive terms. Require exact output identity on both rows and an input matrix with distinct gamma/bias values.
DUPLICATE_CHECK=PASS_ONLY_IF_GATED. Generic pair staging, full-row caching, stripe residency, or prefetch timing is DUPLICATE_REJECTED against R014/FULL-R014/R030, COEFF V001-V004, BATCH-RESIDENT-X, and R31B V011. The only non-duplicate claim is the bounded two-row pair with a prior single-parameter signal and no tile change; without that gate this item is DUPLICATE_REJECTED.
RELATED_OLD_ROUTES=R014; FULL-R014; R030/FULL-R030; BATCH-RESIDENT-X; COEFF-LOCALITY-X V001-V004; R31B V011/V020.
MINIMAL_EXPERIMENT=Do not implement first. If H1 or H2 is later LOCAL_ACCEPTED by Main, run one pair-only follow-up on the exact winning shape and one control. Keep P fixed, account for 32 KiB at P=4096, and abort at research review if aliasing or tile preservation is not proven.
ROUTE_BOUNDARY=Gamma+bias pair only after a positive single-parameter gate, exactly two existing rows; no independent first implementation or broader row group.
ALLOWED_CHANGES=One gated pair-lifetime extension using existing V011 tiles/events with P fixed.
FORBIDDEN_CHANGES=Ungated pair staging, tile search/shrink, scheduling, arithmetic fusion, store merge, stride/multi-row DMA, new event topology, or extra stage.
ASCEND_FEASIBILITY=LOW-to-MEDIUM and gated. The pair is expressible with existing operations only if two coefficient tiles fit or safely alias dead storage; otherwise it is infeasible before code work.
CORE_IMPACT=NONE. No block, ownership, scheduling, or cross-core behavior may change.
EVIDENCE_PATHS=技术路线/冠军/champions.tsv; 本地实验/COEFF-LOCALITY-X/V002/; 本地实验/COEFF-LOCALITY-X/V003/; 技术路线/全版本记录.tsv; 研究/COEFF-LOCALITY-X/TRACK-B-HYPOTHESES.md
PROPOSED_ONE_FACTOR_DIFF=After a positive H1 or H2 gate, extend both coefficient lifetimes across exactly two rows; P and existing event topology remain fixed.
EXPECTED_LOCAL_PROBES=Review gate only; 32 KiB FP32 footprint proof at P=4096; exact deterministic output on the winning shape and one control; >=4 interleaved P/C pairs only if tile capacity and identity remain unchanged.
CHILD_RECOMMENDED_HYPOTHESIS=NONE
OPEN_QUESTIONS=Can both tiles coexist with every live V011 buffer at the winning shape, or does the pair necessarily reproduce COEFF V002's no-UB-budget result?

### H4

HYPOTHESIS_ID=H4-BOUNDED-ROW-GROUP-PARAM-STRIPE
MECHANISM=For a small fixed row group K, load one bounded contiguous parameter stripe S once at group entry and let K existing row consumers use it before advancing to the next stripe. K and S are compile-time bounded; row-to-core ownership and global input/output order are unchanged.
BOTTLENECK=Parameter setup and repeated global-to-UB traffic for short or medium rows where a complete coefficient tile is too expensive to retain, but a small stripe can stay live.
DIRECT_PARENT=R31B-V011
PARENT_SOURCE_SHA=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
TARGET_SHAPES=FP32 D=4096, 8192, 16384 with K=2 or 4 and S<=1024 elements; include one D=32768 control only if the stripe loop preserves the parent tile count. Require localRows>=K.
TARGET_DTYPES=FP32 primary; FP16/BF16 only as controls after confirming that V011 does not already share the stripe across the low-precision retained batch.
WHY_IT_MAY_HELP=The bounded stripe keeps the footprint below a full pair tile while amortizing parameter load setup across K rows. It explores a smaller residency unit than H1-H3 and can target shapes that cannot afford full-tile retention.
WHY_IT_MAY_FAIL=The stripe loop can multiply parameter loads across S segments, destroy contiguous output progress, or introduce a loop/barrier cost larger than the saved row reload. If it changes row order or block assignment it leaves this Route boundary and must be rejected.
UB_IMPACT=Two coefficient stripes cost 2*S*bytes; at S=1024 this is 8 KiB FP32 or 4 KiB FP16/BF16, before alignment. Tile-shrink risk is LOW to MEDIUM if the stripe aliases dead staging, but HIGH if it is added beside the existing 4096-element buffers. Extra-sync risk is MEDIUM because each stripe needs one bounded ready/release boundary.
DMA_IMPACT=For each stripe, gamma+bias bytes are loaded once per K-row group instead of once per row; total bytes are unchanged relative to one full parameter pass, while transactions may fall by up to Kx. No stride or multi-row DMA command is allowed.
SYNC_IMPACT=One group/stripe handoff is permitted; no row scheduling, cross-core barrier, or third event pipeline. Extra-sync risk is MEDIUM-HIGH if K or S causes per-stripe waits.
PRECISION_RISK=Parameter values must be copied exactly; stripe boundaries must preserve the parent tail masking and alignment. No arithmetic or cast change is permitted.
DUPLICATE_CHECK=PASS_WITH_STRICT_BOUNDARY. Full stripe residency and gamma+bias tile staging are DUPLICATE_REJECTED against COEFF V002 and BATCH-RESIDENT-X. H4 is only a bounded, small-stripe lifetime with fixed K/S, no multi-row DMA, no row scheduling, and no tile search; any implementation that changes tile width or block rows is DUPLICATE_REJECTED against R015/R016/SCHED routes.
RELATED_OLD_ROUTES=R014; FULL-R014; R015/FULL-R015; R016/FULL-R016; R030/FULL-R030; BATCH-RESIDENT-X; COEFF-LOCALITY-X V002/V004; SCHED-ROWGROUP-X; SCHED-CHAMPION-X.
MINIMAL_EXPERIMENT=After Planning approval, choose one fixed K and S before source edit, use one FP32 D=8192 and one D=16384 shape with localRows>=K, and compare against the unchanged V011 tile count. Reject if the footprint changes tile capacity, row ownership, or parameter load order outside the bounded stripe.
ROUTE_BOUNDARY=Fixed bounded K-row consumer group and fixed S-element parameter stripe; no ownership or row-order change.
ALLOWED_CHANGES=One predeclared K/S stripe lifetime with ordinary single-row/tile parameter loads and unchanged P.
FORBIDDEN_CHANGES=Tile search/shrink, changed row order/ownership, stride/multi-row DMA, store merge, arithmetic fusion, mode selection, or route-wide pipeline.
ASCEND_FEASIBILITY=MEDIUM-LOW. Ordinary single-row/tile copies are available, but fixed K/S stripe traversal must preserve parent tails and cannot pay a per-stripe synchronization cost that exceeds the bounded reuse.
CORE_IMPACT=NONE. The K rows are existing consumers only; block ownership, row order, and mode dispatch remain unchanged.
EVIDENCE_PATHS=技术路线/冠军/champions.tsv; 本地实验/COEFF-LOCALITY-X/V002/; 本地实验/COEFF-LOCALITY-X/V004/; 研究/BATCH-RESIDENT-X/next-hypotheses.md; 技术路线/技术路线图.md
PROPOSED_ONE_FACTOR_DIFF=One fixed K/S coefficient-stripe lifetime across existing row consumers; no tile search, row-group ownership change, stride DMA, arithmetic, or store change.
EXPECTED_LOCAL_PROBES=Pre-edit footprint and tile-count proof; exact FP32 correctness at D=8192/16384 with localRows>=K; same-binary qualification and >=4 interleaved P/C pairs; D=32768 is control only if P/tile count is unchanged.
CHILD_RECOMMENDED_HYPOTHESIS=NONE
OPEN_QUESTIONS=Can a fixed S<=1024 preserve P=4096 and parent tail behavior, or does the extra stripe loop introduce enough DMA/sync overhead to erase row reuse?

### H5

HYPOTHESIS_ID=H5-PARAM-LIFETIME-ROW-PIPELINE-MINIMAL-SHARE
MECHANISM=Use the free half of the existing V011 row-pipeline storage after the current row's input consumer is finished to carry one gamma-only or bias-only tile into the next row. Reuse existing event IDs and lifetime boundaries; do not add a third stage or prefetch above invRms.
BOTTLENECK=Repeated coefficient load on a row transition when the V011 input row pipeline has a storage slot whose lifetime is otherwise idle.
DIRECT_PARENT=R31B-V011
PARENT_SOURCE_SHA=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
TARGET_SHAPES=FP16/BF16 wide D=16384/32768 and FP32 wide D=16384/32768 only where an existing two-deep row slot is demonstrably free; N=2-8 controls; no shape with an unresolved parent alias race.
TARGET_DTYPES=The dtype whose parent path has a provably free row-pipeline half; FP16/BF16 are primary candidates because V011 exposes a two-deep row pipeline, FP32 is a separate feasibility probe.
WHY_IT_MAY_HELP=It can reduce repeated parameter traffic with approximately zero new UB by extending an already allocated buffer lifetime. It differs from a load-timing change because the load remains at the existing row transition and only the consumer lifetime/alias is changed.
WHY_IT_MAY_FAIL=The supposedly free half may still be needed by a pending input or output operation, and the required wait may serialize the row pipeline. An alias race can produce nondeterministic output or the same 507035 class seen by unsafe buffer reuse.
UB_IMPACT=Target +0 bytes by aliasing an existing two-deep slot; fallback allocation of P*bytes is forbidden for this minimal version. Tile-shrink risk is LOW by construction, but feasibility is binary. Extra-sync risk is HIGH: one incorrect lifetime fence invalidates correctness, while one conservative fence can remove all performance benefit.
DMA_IMPACT=Reduce parameter global-to-UB reloads at the row boundary without changing DataCopy shape, byte count per unique parameter tile, or input/output DMA. No stride, batch, or store merge.
SYNC_IMPACT=Reuse only existing MTE2/V and V/MTE2 event pairs and add no event IDs. The required row-boundary wait must be measured as part of the hypothesis; if it adds a new barrier or fence placement, classify as DUPLICATE_REJECTED against the Wave-1 sync lanes.
PRECISION_RISK=High correctness sensitivity from aliasing/lifetime only; arithmetic is unchanged. Exact per-row parameter identity, output hash, and repeated-run determinism are mandatory before any timing.
DUPLICATE_CHECK=PASS_WITH_BOUNDARY. R31B V011 already has a two-deep parameter MTE2 stream, so redoing that exact tile prefetch is DUPLICATE_REJECTED. R31A V025/V026 and COEFF V001-V004 cover generic staging liveness or parameter-load timing and are DUPLICATE_REJECTED. H5 is narrower: existing row-pipeline slot alias at the row boundary, no prefetch reorder, no third stage, and no new sync topology. Any extra fence or reorder makes it a duplicate.
RELATED_OLD_ROUTES=R013/FULL-R013; R014/FULL-R014; R030/FULL-R030; R31B V011/V020; R31A V025/V026; COEFF-LOCALITY-X; ASYNC-OVERLAP-CHAMPION-X Wave-1 V001-V004; ASYNC-TRIPLE-X; MIX-A.
MINIMAL_EXPERIMENT=After Planning approval, choose one parameter (gamma or bias) and one existing pipeline slot from an exact source audit. Make one alias/lifetime change only, with no new buffer, event, or prefetch location. Validate a two-row FP16/BF16 wide shape first, then one FP32 control if the slot exists; stop on any output nondeterminism or new synchronization.
ROUTE_BOUNDARY=One existing V011 row-pipeline slot alias at a row boundary for one parameter stream; no prefetch relocation.
ALLOWED_CHANGES=One gamma-only or bias-only zero-new-UB alias/lifetime change using existing event IDs, after exact source audit and Planning approval.
FORBIDDEN_CHANGES=New buffer, third stage, prefetch reorder, extra fence/event topology, scheduling, arithmetic fusion, store merge, stride/multi-row DMA, or tile search.
ASCEND_FEASIBILITY=MEDIUM but binary. It is feasible only if a source-level lifetime proof shows an existing slot is dead at the alias point and all existing event IDs already express the handoff; otherwise classify INFEASIBLE.
CORE_IMPACT=NONE. No block, ownership, row order, mode, or cross-core behavior changes.
EVIDENCE_PATHS=技术路线/冠军/champions.tsv; 技术路线/冠军/R31B-V011/RESULT_REF.txt; 本地实验/R31B/V016/; 本地实验/ASYNC-OVERLAP-CHAMPION-X/V001/; 本地实验/ASYNC-OVERLAP-CHAMPION-X/V004/; 技术路线/全版本记录.tsv
PROPOSED_ONE_FACTOR_DIFF=Alias one existing V011 row-pipeline slot for one parameter stream at the row boundary; zero new UB, no prefetch movement, no new event/fence, and no arithmetic change.
EXPECTED_LOCAL_PROBES=Exact source lifetime and event audit; repeated deterministic output with distinct gamma/bias; FP16/BF16 two-row wide correctness and same-binary qualification; one FP32 control; >=4 interleaved P/C pairs only after all gates pass.
CHILD_RECOMMENDED_HYPOTHESIS=NONE
OPEN_QUESTIONS=Which exact dtype path has a genuinely dead row-pipeline half after input consumption, and can the alias avoid the known V011/FullCache lifetime races without a new fence?

## REQUIRED_FIELD_AUDIT

The five hypotheses above are complete research records. The following matrix makes the
fields that are shared by the route handoff explicit for every hypothesis, so a child or
Main review does not infer scope from prose. `P` means the existing parent tile width;
the audit uses `P=4096` as the V011 wide-path anchor and does not authorize changing it.

| HYPOTHESIS_ID | ROUTE_BOUNDARY | ALLOWED_CHANGES | FORBIDDEN_CHANGES | EVIDENCE_PATHS | PROPOSED_ONE_FACTOR_DIFF | EXPECTED_LOCAL_PROBES | CHILD_RECOMMENDED_HYPOTHESIS |
|---|---|---|---|---|---|---|---|
| H1-GAMMA-TWO-ROW-LIFETIME | gamma lifetime across exactly two existing adjacent row consumers; no row-group scheduling | one gamma lifetime extension using existing tile/event machinery, after Planning approval | bias reuse, tile search/shrink, row mapping, arithmetic, store merge, stride/multi-row DMA, new sync topology | V011 champion/evidence paths listed below; COEFF V001-V004; R31A V025/V026 | gamma-only two-row lifetime extension; P fixed | source/UB audit, exact-output deterministic check, then FP32 D=16384/32768 plus D=8192 control with interleaved P/C | NONE; Planning selects |
| H2-BIAS-TWO-ROW-LIFETIME | bias lifetime across exactly two existing adjacent row consumers; no gamma lifetime change | one bias lifetime extension using existing tile/event machinery, after Planning approval | gamma reuse, tile search/shrink, row mapping, arithmetic, store merge, stride/multi-row DMA, new sync topology | V011 champion/evidence paths listed below; COEFF V003/V004; BATCH-RESIDENT-X | bias-only two-row lifetime extension; P fixed | source/UB audit with distinct nonzero bias, exact-output check, then same FP32 matrix as H1 | NONE; Planning selects |
| H3-GAMMA-BIAS-TWO-ROW-PAIR | paired coefficient lifetime across exactly two rows, gated by a prior H1 or H2 signal | pair lifetime only after gate; existing tile/events only | first implementation without gate, tile search/shrink, scheduling, arithmetic, store merge, stride DMA, new event topology | V011 champion/evidence paths listed below; R014/FULL-R014/R030; COEFF V002 | gamma+bias pair lifetime; P fixed | gate review, 32 KiB footprint proof, exact-output check on winning shape and one control, then interleaved P/C | NONE; Planning selects |
| H4-BOUNDED-ROW-GROUP-PARAM-STRIPE | fixed small K-row consumer group and fixed S-element coefficient stripe; no ownership change | one predeclared K/S stripe lifetime with ordinary single-row/tile loads | tile search, changed row order/ownership, stride/multi-row DMA, store merge, arithmetic, new route-wide pipeline | V011 champion/evidence paths listed below; COEFF V002/V004; R015/R016/SCHED evidence | one fixed bounded stripe lifetime; K and S chosen before edit | prove unchanged tile capacity and parent load order, then FP32 D=8192/16384 with localRows>=K plus unchanged control | NONE; Planning selects |
| H5-PARAM-LIFETIME-ROW-PIPELINE-MINIMAL-SHARE | one existing V011 row-pipeline slot alias at a row boundary; no prefetch relocation | one gamma-only or bias-only alias/lifetime change with existing event IDs | new buffer, third stage, prefetch reorder, extra fence/event topology, scheduling, arithmetic, store merge, stride DMA | V011 champion/evidence paths listed below; R013/R014/R030; ASYNC/COEFF/UB-LIVENESS evidence | zero-new-UB alias of one parameter tile into a proven-free slot | exact source lifetime audit, repeated deterministic output, then FP16/BF16 two-row wide shape and FP32 control | NONE; Planning selects |

All five rows have explicit `MECHANISM`, `BOTTLENECK`, `DIRECT_PARENT`,
`PARENT_SOURCE_SHA`, `TARGET_SHAPES`, `TARGET_DTYPES`, `WHY_IT_MAY_HELP`,
`WHY_IT_MAY_FAIL`, `UB_IMPACT`, `DMA_IMPACT`, `SYNC_IMPACT`, `PRECISION_RISK`,
`DUPLICATE_CHECK`, `RELATED_OLD_ROUTES`, and `MINIMAL_EXPERIMENT` fields immediately
under their hypothesis heading. `CHILD_RECOMMENDED_HYPOTHESIS=NONE` is intentional:
this Track-B agent does not select an implementation.

## UB_FOOTPRINT_AND_RISK_CALCULATION

The relevant coefficient footprint is `gamma + bias = 2 * P * sizeof(dtype)` per
resident stripe/tile. At the frozen wide-path anchor `P=4096`:

| dtype | one gamma-or-bias tile | H1 or H2 added resident tile | H3 gamma+bias pair | H4 at S=1024 pair | H5 new UB |
|---|---:|---:|---:|---:|---:|
| FP32 | 16 KiB | 16 KiB | 32 KiB | 8 KiB pair | 0 KiB (alias only) |
| FP16/BF16 | 8 KiB | 8 KiB | 16 KiB | 4 KiB pair | 0 KiB (alias only) |

For the V011 wide FP32 budget, `176 KiB` total and `D=32768` y storage alone is
`32768*4 = 128 KiB`; therefore H1/H2 consume 16 KiB of remaining budget before
any live x/residual/reduction storage, H3 consumes 32 KiB, and H4 consumes 8 KiB
at `S=1024`. These are feasibility bounds, not permission to resize the parent:
if the exact live-buffer audit cannot fit the proposed footprint while preserving
`P=4096`, classify the hypothesis `INFEASIBLE`/`DUPLICATE_REJECTED` and stop.

`TILE_SHRINK_RISK`: H1=HIGH, H2=HIGH, H3=VERY_HIGH, H4=LOW-to-MEDIUM only if
aliased (otherwise HIGH), H5=LOW by construction. `EXTRA_SYNC_RISK`: H1=MEDIUM,
H2=MEDIUM, H3=HIGH, H4=MEDIUM-HIGH, H5=HIGH. Any implementation that pays these
risks through a new barrier, new event family, or changed tile count is outside
this route and must not be created.

## DUPLICATE_AUDIT

The audit below is a mechanism audit, not a route-lifecycle decision. Exact repeats are marked `DUPLICATE_REJECTED`; scoped proposals remain eligible only with the boundary stated above.

| History | Concrete comparison |
|---|---|
| R001 | Two-pass scan; no gamma/bias row residency. No duplicate. |
| R002 | y retention; not parameter residency. No duplicate. |
| R003 | Direct-invocation/build structure; no duplicate. |
| R004 | Low-precision middle computation; no duplicate, and no dtype cast change is allowed here. |
| R005 | Large tile; H4 may not change tile size. Any tile-search variant is `DUPLICATE_REJECTED`. |
| R006 | Blocked reduction; no duplicate. |
| R007 | ReduceSum topology; no duplicate. |
| R008 | Cross-core tile distribution; no duplicate, and cross-core sharing is forbidden. |
| R009 | DataCopyPad tail handling; no duplicate. |
| R010 | Manual tail handling; no duplicate. |
| R011 | Manual vector reduction; no duplicate. |
| R012 | 32B row-block alignment; no duplicate, and H4 cannot become alignment or row-group ownership. |
| R013 | Double-buffer pipeline; adjacent only to H5. A new double/triple-buffer design is `DUPLICATE_REJECTED`. |
| R014 | Gamma/bias residency is the exact ancestor concept. Unbounded full-row/full-tile gamma+bias cache is `DUPLICATE_REJECTED`; only H1/H2 bounded single-parameter lifetime, gated H3, bounded H4, or alias-only H5 remain distinct. |
| R015 | Multi-row/stride DMA; `DUPLICATE_REJECTED` for any H4 or other proposal that uses a multi-row DataCopy. |
| R016 | Row-to-core scheduling; `DUPLICATE_REJECTED` for changed block ownership, rows-per-block, or mode dispatch. |
| R017 | FP32-all-middle precision path; no duplicate. |
| R018 | CAST_RINT output; no duplicate. |
| R019 | Reciprocal/normalization math; no duplicate. |
| R020 | Sqrt normalization; no duplicate. |
| R021 | Build chain; no duplicate. |
| R022 | Code generation; no duplicate. |
| R023 | Engineering convention; no duplicate. |
| R024 | CPU validation; no duplicate. |
| R025 | Submission integrity; no duplicate. |
| R026 | Performance measurement; no duplicate. |
| R027 | GPU migration reference; no direct duplicate. |
| R028 | Scalar synchronization reduction; H5 must not alter scalar handoff. Any scalar-sync change is `DUPLICATE_REJECTED`. |
| R029 | Five-mode tiling/dispatch; no duplicate. Mode selection is forbidden. |
| FULL-R001..FULL-R029 | Same mapping as R001-R029. `FULL-R014` broad parameter residency, `FULL-R015` multi-row DMA, `FULL-R013` double buffering, and `FULL-R016/FULL-R029` scheduling/tiling are the relevant repeats and are `DUPLICATE_REJECTED` outside the stated bounded scopes. |
| R31A | V025/V026 pass-1/pass-2 staging liveness release+prefetch is an adjacent parameter-lifetime result. Generic staging release/prefetch is `DUPLICATE_REJECTED`; H5 must use an existing V011 row-pipeline slot at the row boundary and must not import R31A path changes. |
| R31B | V011 already has 2-deep parameter MTE2 in low precision; repeating that per-tile prefetch is `DUPLICATE_REJECTED`. V020 2->3-deep staging is infeasible from UB overflow; H3/H4 cannot add a third stage. V011 full-row y retention is not a new parameter hypothesis. |
| MIX-A | Hybrid/synchronization-removal variants, including SyncVToMTE2 changes, are `DUPLICATE_REJECTED`. No hybrid path, fence removal, or arithmetic mixing is proposed here. |
| Wave-1 COEFF-LOCALITY-X | V001 full-cache prefetch, V002 stripe residency/NO_UB_BUDGET, V003 split-phase parameter emission at constant tileElems, and V004 generic full-row preload exhaust load timing/locality. Those exact forms are `DUPLICATE_REJECTED`; H1/H2 are bounded row-consumer lifetime, H4 is a strictly smaller fixed stripe only if it preserves the parent tile, and H5 is alias-only row-pipeline lifetime. |
| Wave-1 ASYNC-OVERLAP-CHAMPION-X / ASYNC-TRIPLE-X | Inter-pass prefetch, inter-row issue reorder, GetValue handoff, FullCache prologue, and MTE2/V/MTE3 triple overlap are `DUPLICATE_REJECTED`. H5 cannot move load issue, add a stage, or change fence topology. |
| Wave-1 MULTIROW-DMA-CHAMPION-X / BATCH-RESIDENT-X | Stride multi-row DMA, aligned-copy form, parameter residency plus batch DMA, and A001-derived full-row batching are `DUPLICATE_REJECTED`. H4 must use ordinary single-row/tile loads; no batch DMA. |
| Wave-1 SCHED-ROWGROUP-X / SCHED-CHAMPION-X | Row-group ownership/core assignment is unrelated and `DUPLICATE_REJECTED` if introduced. H4's row group is only a local parameter-lifetime bound; it does not change ownership or scheduling. |
| Wave-1 UB-LIVENESS-X | Generic UB alias/liveness is a neighboring feasibility concern, not the performance mechanism. H5 may use an already-free V011 slot only; a general alias redesign is `DUPLICATE_REJECTED`. |
| Wave-1 STORE-EPILOGUE-X / EPILOGUE-FUSE-X / VECTOR-MATH-X / DTYPE-SPECIAL-X / SHAPE-TILING-CHAMPION-X | Store merge, post-RMS arithmetic, vector math, dtype specialization, and tile selection are outside this boundary. Combining any of them is `DUPLICATE_REJECTED`. |

## PROPOSED_ONE_FACTOR_DIFF

| Hypothesis | One conceptual change | Explicitly unchanged |
|---|---|---|
| H1 | Extend gamma tile lifetime to two existing adjacent rows. | Bias lifetime, tile width, row mapping, arithmetic, DMA form, sync topology. |
| H2 | Extend bias tile lifetime to two existing adjacent rows. | Gamma lifetime, tile width, row mapping, arithmetic, DMA form, sync topology. |
| H3 | After H1/H2 evidence only, extend the gamma+bias pair lifetime to two rows. | Tile width, row mapping, arithmetic, DMA form, event topology. |
| H4 | Add one fixed small parameter stripe lifetime for K existing rows. | K/S fixed before edit; no tile search, ownership change, stride DMA, or arithmetic change. |
| H5 | Alias one parameter tile into an already-free V011 row-pipeline slot at the row boundary. | No new buffer, event ID, prefetch location, fence topology, tile width, or arithmetic change. |

## EXPECTED_LOCAL_PROBES

No probes were run in this Track-B handoff. If Planning later approves one hypothesis, the first evidence must be a source/UB audit against exact R31B-V011, then target correctness with distinct gamma/bias values and deterministic repeated output. Only after correctness and same-binary qualification should the prescribed interleaved Parent/Candidate timing be considered. Minimum shape coverage is one primary wide shape, one second wide shape, and one control; parent and candidate must use the same runner, dtype, device, and timing boundary. A local win on one shape is not an Official claim.

## CHILD_RECOMMENDED_HYPOTHESIS

CHILD_RECOMMENDED_HYPOTHESIS=NONE
REASON=This Route Agent does not select an implementation. Planning/Review must choose one hypothesis; H3 additionally requires prior gamma-only or bias-only evidence. All five items remain research candidates with explicit feasibility gates.

## OPEN_QUESTIONS

1. Does `ProcessWideFp32FullCacheRows` actually reload gamma and bias per row for the target localRows, or is one stream already retained by a path not visible in the summary?
2. For each target shape, what localRows survives the existing blockCount partition? A two-row hypothesis has no traffic benefit when localRows is one.
3. Can one gamma or bias tile alias an existing V011 buffer after its last input/output consumer without adding a new event ID or fence?
4. Can H4 preserve the V011 tile capacity and contiguous tail behavior with a fixed S, or does stripe staging recreate the COEFF V002 16 KiB ceiling?
5. Which exact dtype path has a genuinely free row-pipeline half? This must be answered from a source/compiled-identity audit before H5 is considered feasible.
6. Does the target official-case distribution contain enough multi-row wide shapes for row residency to affect the score, given that 1x32768 cannot reuse across rows?

## EVIDENCE_PATHS

- `技术路线/冠军/champions.tsv`
- `技术路线/冠军/R31B-V011/RESULT_REF.txt`
- `线上结果/R31B/V011/submission.asc`
- `线上结果/R31B/V011/diff.patch`
- `技术路线/全版本记录.tsv`
- `技术路线/路线成绩表.tsv`
- `技术路线/技术路线总表.md`
- `技术路线/技术路线图.md`
- `调度/当前任务.tsv`
- `归档/历史控制文件/main2-r2-route-registry.md`
- `研究/主代理/MAIN-2/CAMPAIGN-STATUS.md`

This file is the only new research/handoff artifact for `PARAM-RESIDENCY-CHAMPION-X` in this turn. It contains no source, Revision, build, correctness, timing, profiling, server, Online, score, or lifecycle change.

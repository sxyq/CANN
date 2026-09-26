# SCHED-ROWGROUP-X — Next-Hypotheses (TRACK-B, long-horizon)

> **MAIN-2 APPROVALS 2026-09-25** — APPROVED NEXT backlog: H1 CORE-FILL ROW-GROUP SCHEDULING (quantified: 17×256 currently ≈2 blocks vs ~17 potential); H2 EVEN-SPLIT TASK EXTENTS second in queue; H3 dynamic task-pull stays NEEDS_MORE_EVIDENCE. H1 implementation is FORBIDDEN while V001 is undecided. Scope arbitration: row-group ownership / rows-task / core-fill / shape scheduling fixed to this route (ALIGN-H2 geometry part deferred here).

CURRENT_CANDIDATE: V001 (SHA 0fae0a42e3942356fe3477cd518b17180b6104d57350cee705cba37a5895e65c) — R016 shape-aware rows/task scheduling parent (official 17.14) + R012 32-byte-safe row-group ownership; direct parent R016-V001-COMPILEFIX-A (parent SHA 62de32df…, score 17.14). Local status **ONLINE_CANDIDATE** as of 2026-09-25 LH3 lease (see CYCLE section below): same-binary exact-shape PASS on 33×100 and 17×256, interleaved P/C 4/4 favor V001 at median −51.38% on 33×100 (order-robust), 17×256 aligned control ≈ −0.2% (in noise), bad=0 all runs, SHA unchanged. Local % ≠ Official Score; Main disposes. No kernel edits, no new revision.

CURRENT_BLOCKER: None for Track-B research. Track-A measurement resolved this cycle (was: drift 0.1999/0.1036 and reverse pair). Waiting on Main disposition of V001 before ANY implementation (H1 and H9 remain FORBIDDEN until then). No Main decision is required to continue Track-B research.

BOTTLENECK_MODEL: V001's host scheduler maps D to fixed task granularity (D≤256: 16 rows/task; ≤1024: 8; ≤4096: 4; ≤16384: 2; else 1), then taskCount = ceil(rowCount/rowsPerTask), blockDim = min(availableCoreNum, taskCount), plus an 8-core cap for D>16384 (submission.asc:433-506). Two consequences on the probe shapes: (a) core underfill — 17×256 gives taskCount=2 (2 blocks of 16+1 rows), 33×100 gives taskCount=3 (16+16+1), so 15+ of ~40 cores idle; (b) intra-launch imbalance — the partial tail task is 1 row against a 16-row head task, so wall time ≈ the longest serial row loop. The V001 experiment itself isolates core count as the dominant lever: parent forces single-core for non-32B rows (D=100 → 1 core), V001 lifts that rule and reaches 3 cores → −31.81% on 33×100; on 17×256 (rowBytes 1024, already 32B-aligned) both binaries already launch 2 blocks → −0.76% ≈ zero effect. Read together: moving 1→3 cores bought ~32%; going 2–3 → 17–33 cores via core-fill scheduling is the next untested magnitude, while further row-group tweaks on already-multicore shapes are expected near-zero. Secondary lever: the D>16384 8-core cap underfills wide multi-row shapes; scheduleMode cyclic/contiguous alternation by band has no recorded evidence.

HYPOTHESIS-1: CORE-FILL ROW-GROUP SCHEDULING (shape-aware core count + blockDim utilization)
- MECHANISM: Replace the fixed band table (16/8/4/2/1 rows per task) with a core-count-derived formula computed on host: rowsPerTask = max(rowGroup, ceil(rowCount / min(availableCoreNum, rowCount/rowGroup rounded up))) with an oversubscription cap (target taskCount ≈ cores×2, never thousands of blocks); task boundaries stay multiples of rowGroup (R012 invariant kept). Device loop, ProcessRow, UB layout, precision untouched. Equivalent view: blockDim = min(availableCoreNum, tasks-that-fit), each task still a contiguous row-group-aligned stripe.
- BOTTLENECK: core underfill + blockDim utilization. Current scheduler spends 16 rows serially on one core while 15+ cores idle on small-R shapes; kernel time is max-task-length, not bandwidth.
- EXPECTED_SHAPES: largest gain where rowCount ≤ availableCores and band rowsPerTask is large: 17×256 (2→~17 blocks), 33×100 (3→~9 blocks, rowGroup=4 floor), 7×65, 17×257 (future shapes). Neutral where taskCount already ≥ cores (large outer rows) or rows=1 (1×6144, wide).
- WHY_IT_MAY_HELP: V001 evidence shows core count is the dominant sensitivity (1→3 cores ≈ −32% on 33×100); the bands were never derived from availableCoreNum and there is no recorded experiment where fewer-than-available cores won. Per-row work (two-pass, tile 4096) is identical per row, so total work is conserved while the critical path shrinks toward one row-group.
- WHY_IT_MAY_FAIL: block launch/init overhead (TPipe buffer init per block) may dominate once each task is 1–4 tiny rows; many queued blocks may add scheduler latency; if hidden test shapes have large rowCount the bands already fill cores and gain ≈0 (loss should also ≈0 because per-row work is unchanged); per-block fixed cost on dav-2201 is unmeasured locally.
- ASCEND_FEASIBILITY: host already computes rowsPerTask/taskCount/blockDim (submission.asc:433-506) — pure host arithmetic change + the same launch path; no new API, no kernel-ABI change. Kernel task loop (GetBlockIdx/GetBlockNum) already generic.
- UB/CORE/DMA_IMPACT: UB unchanged (per-row tile buffers fixed). Core: more blocks active, each with fewer rows; DMA per row unchanged (DataCopyPad per tile), so total GM bytes unchanged; HBM concurrency across more cores likely saturates rather than contends at these sizes.
- SYNC_IMPACT: none — kernel has no cross-block sync; no workspace, no SyncAll.
- PRECISION_RISK: none — rows are computed independently with identical arithmetic; task extent does not change per-row accumulation order.
- DUPLICATE_CHECK: vs R016 (parent): parent picks granularity from D only, never from core count — this replaces that rule, not restates it. Vs R012/V001: rowGroup math kept as constraint, untouched mechanism. Vs EXT-ASCEND-X (rowFactor/ubFactor tiling, next-round-plan.md:64): that route varies UB work size / factor-driven tile ownership with per-core consecutive rows under a different provenance class; this changes only how many rows one task claims relative to core count. Vs F001 (blockDim=min(cores,rows), core c → rows c,c+blockDim,…): F001 is a static reference architecture with no band table and no rowGroup rounding, never scored; mechanism overlaps conceptually — flagged as the closest historical neighbor (SOURCE_ROUTE F001) but F001 never became a scored Candidate on this parent, and H1 keeps R016 band caps + R012 group invariants, so not a rerun of F001. Vs BATCH-RESIDENT (R014+R015): no multi-row DMA, no param cache. Vs ALIGN/REDUCE/ASYNC/UB routes (briefs re-read this turn): none owns task-granularity scheduling.
- MINIMAL_OFAT_DIFF: host only — replace lines 436-452 band block with core-count-derived rowsPerTask (keep rowGroup rounding at 454-469, keep 8-core wide cap untouched for this OFAT or explicitly fold cap policy into a later hypothesis); no kernel .Process/.ProcessRow edit.
- EXPECTED_LOCAL_PROBES: same-binary noise floor already PASS on 17×256 and 33×100 → interleaved P/C ×≥4 pairs, device events, warmup 10, 31 samples. Success signature: 17×256 median delta clearly beyond its noise floor (floor ≈ −0.76% observed; same-binary MAD/med 0.023) and 33×100 delta at least as strong as V001's −31.81% with low within-block MAD. 7×65 and 17×257 await shape requalification before any probe.
- CLASSIFICATION: READY_FOR_MAIN_REVIEW (as the first post-V001 architecture hypothesis; do not start until Main disposes Track-A).

HYPOTHESIS-2: EVEN-SPLIT TASK EXTENTS (tail-row ownership rebalance, granularity kept)
- MECHANISM: Keep the band table, taskCount, scheduleMode, and blockDim exactly as in V001; change only how rows are divided among the taskCount tasks: instead of fixed rowsPerTask with one short tail task (16/16/1), give task i the range [floor(rows*i/taskCount)-aligned, floor(rows*(i+1)/taskCount)-aligned) rounded to rowGroup boundaries, so every task size differs by at most rowGroup rows.
- BOTTLENECK: tail-row ownership imbalance within the already-active blocks. On 33×100: active cores run 16, 16, 1 rows → wall time = 16-row serial loop; balanced would be 11/11/11 (rowGroup=4 → e.g. 12/12/9 aligned extents). On 17×256: 16+1 → 9+8.
- EXPECTED_SHAPES: any shape where taskCount is small and rowCount % rowsPerTask is large: 33×100 (remainder 1 vs 16), 17×256 (1 vs 16). Little effect when taskCount ≫ cores (remainder spread amortized) or rows divisible by rowsPerTask.
- WHY_IT_MAY_HELP: with fixed 3 blocks on 33×100 the critical path is the 16-row task; halving max task extent is a direct ~2× potential cut of the serial portion without launching more blocks. Cheap and independent of H1's block-count question.
- WHY_IT_MAY_FAIL: benefit caps at (old_max/avg) ≈ 16/11 ≈ 1.5× on 33×100 and 16/9 ≈ 1.8× on 17×256 before launch overhead — likely smaller than H1; on 17×256 both binaries launch only 2 blocks, so measured delta may sit near the noise floor again; if V001's real advantage came from enabling cores (not balance), balance alone underdelivers.
- ASCEND_FEASIBILITY: host-only arithmetic (integer extents); kernel already takes firstRow/endRow per task (submission.asc:95-99, 129-133) and does not assume uniform rowsPerTask — no kernel edit required if extents are derived host-side; if rowsPerTask is passed as uniform, the minimal variant passes a taskCount + uses in-kernel even division (small kernel edit allowed at revision time).
- UB/CORE/DMA_IMPACT: UB unchanged; same block count (no new cores); DMA bytes unchanged; slightly more even concurrent GM reads across active cores.
- SYNC_IMPACT: none.
- PRECISION_RISK: none — per-row arithmetic identical; only task boundaries move (rowGroup alignment preserved, so R012 32B invariant holds).
- DUPLICATE_CHECK: vs H1 — H1 changes how many tasks/cores; H2 keeps them and only redistributes rows. H1 subsumes H2's effect when it picks small rowsPerTask, but H2 remains the fallback if many-block launch overhead kills H1; distinct OFAT diffs (blockCount changes in H1, blockCount identical in H2). vs R016 parent: parent derives task extents solely from the band — never an even-split. vs F001 static round-robin: F001 has no tasks at all (row-strided ownership); different mechanism. vs ALIGN-TAIL-X: that route changes copy path for tail bytes, not task extents.
- MINIMAL_OFAT_DIFF: host lines 471-477 only (taskCount unchanged, extent formula changed) or equivalent in-kernel division; no change to bands, rowGroup, scheduleMode, wide cap, ProcessRow.
- EXPECTED_LOCAL_PROBES: 33×100 and 17×256 interleaved pairs under the reference protocol (both shapes PASS). Signature: reduced max-task critical path should show as a stable median drop on 33×100 with low MAD; if delta ≤ noise floor, imbalance was not the binding constraint.
- CLASSIFICATION: READY_FOR_MAIN_REVIEW (ranked after H1; strongest as fallback if H1 shows launch-overhead regression).

HYPOTHESIS-3: DYNAMIC TASK-PULL QUEUE (device-side row ownership granularity)
- MECHANISM: Launch a fixed blockDim = min(availableCoreNum, taskCount) but let each block pull the next task index from a shared GM counter (atomic increment via AscendC SetAtomicAdd-style GM atomic) instead of computing taskBegin/taskEnd statically. Host granularity (band + rowGroup) stays unchanged; only who-owns-which-task moves from host-partition to device-pull.
- BOTTLENECK: tail-row ownership and any cost heterogeneity between tasks — static partition assigns the 1-row remainder task to one specific core while another core may still be draining a 16-row task; dynamic pull retires tasks as cores free up, so the makespan approaches total-work/cores for any task-size distribution.
- EXPECTED_SHAPES: helps when taskCount ≫ cores (large rowCount small D: hundreds of tasks) or task costs are heterogeneous (mixed aligned/unaligned tails, future residency changes); on probe shapes (taskCount = 2–3) the queue adds overhead with nothing to rebalance — not expected to win there.
- WHY_IT_MAY_HELP: removes worst-case static imbalance by construction and stays correct for unknown hidden shapes without retuning host bands; a common robustness pattern in GPU normalization (scheduler-owned work lists) though never recorded in this repo.
- WHY_IT_MAY_FAIL: rows in this kernel are uniform-cost two-pass loops (same D, same tile), so static even split is already near-optimal and the atomic round-trip per task is pure added latency; GM atomic semantics on dav-2201 AICore need compile/runtime verification; a misused atomic could race and corrupt task assignment (correctness risk unlike H1/H2).
- ASCEND_FEASIBILITY: unverified — AscendC offers atomic-add-to-GM primitives (SetAtomicAdd on vector outputs); a scalar counter pattern must be confirmed against CANN 8.5 API and AICore cross-block semantics before any revision. No kernel edits until verified.
- UB/CORE/DMA_IMPACT: UB unchanged; same block count as current; one extra tiny GM read+atomic per task; DMA per row unchanged.
- SYNC_IMPACT: introduces cross-block coupling through the counter (no barriers), so architecture-principle 7 (blockDim = participating cores, uniform execution) must be re-audited: blocks with no work still must behave identically (empty pull → return).
- PRECISION_RISK: none if task assignment is correct; arithmetic per row unchanged. Risk is correctness under counter races, not numerical drift.
- DUPLICATE_CHECK: no historical route in phase4/, 归档, or idea-pool-29-routes.md uses a device task queue (searched scheduling/row-group/core keywords this turn); F001/MID-X/D001/E001 are all static host mappings. Not R016 (host bands), not R012 (group math), not R008/D-slice (no D splitting), not ASYNC-TRIPLE (pipeline, not ownership), not BATCH (no DMA change). No sibling next6 route brief mentions dynamic scheduling.
- MINIMAL_OFAT_DIFF: add counter fetch around submission.asc:91-144 task loop (one pattern, both scheduleMode paths collapse to pull-loop); host launch width may stay min(available, taskCount). Single conceptual change: static→dynamic ownership.
- EXPECTED_LOCAL_PROBES: not probeable on current PASS shapes (taskCount ≤ 3 gives nothing to pull); would need a qualified large-rowCount shape (e.g. thousands × small D) first — same-binary qualification for that shape is currently absent, so this cannot be the next revision.
- CLASSIFICATION: NEEDS_MORE_EVIDENCE (mechanism distinct and unhunted, but unverifiable atomic API + no local probe shape today; keep in backlog).

OPTIONAL-HYPOTHESIS-4: MINIMAL-GROUP TASK GRANULARITY (smallest legal task = one rowGroup; scheduler threshold removal)
- MECHANISM: Delete the D-band table entirely (submission.asc:436-452) and set rowsPerTask = rowGroup (the R012 invariant becomes the only granularity rule): taskCount = ceil(rowCount/rowGroup), blockDim = min(available, taskCount). One row per core when rows are 32B-aligned (rowGroup=1); one group per core otherwise.
- BOTTLENECK: scheduler thresholds themselves — the 16/8/4/2/1 band constants are unevidenced (R016 COMPILEFIX 17.14 was a neutral/negative official run, −0.38 vs b0) and actively force core underfill (7×65: rowsPerTask=16 > rowCount=7 → taskCount=1, still single-core even after V001).
- EXPECTED_SHAPES: every shape with rowCount ≥ 2 benefits vs current bands when cores are available: 17×256 (17 tasks vs 2), 33×100 (9 tasks of 4 rows vs 3 of 16), 7×65 (7 tasks vs 1 — except rowGroup=8 > 7 rows keeps it at 1 task, see failure). Large rowCount small D becomes one-block-per-row (up to available cores × … capped only by rowCount).
- WHY_IT_MAY_HELP: matches what both official same-op CUDA (grid = num_tokens, one block per row) and Triton (grid = (M,)) normalization kernels do — per-row ownership is the ecosystem default; V001's own numbers show core count dominates (1→3 cores ≈ −32%).
- WHY_IT_MAY_FAIL: on the two PASS probe shapes this collapses into H1 (17×256: both give 1 row/task; 33×100: both give 4-row groups) so local evidence cannot separate H4 from H1 — DUPLICATE risk for the first probe window; when rowCount < rowGroup (7×65: 7 rows vs group of 8) the group invariant still forces taskCount=1, so underfill survives unless the group-splitting rule itself is relaxed, and the correctness rationale for that rule (parent forced single core for unaligned rows; reason never documented beyond "ownership safety") is unverified — relaxing it carries correctness risk.
- ASCEND_FEASIBILITY: host-only change; kernel loop already generic.
- UB/CORE/DMA_IMPACT: UB unchanged; DMA bytes unchanged; max possible block width (blockDim up to rowCount/available) — note per-block fixed entry cost (Init/GetBlockIdx) grows with task count; unknown magnitude locally.
- SYNC_IMPACT: none (no cross-block sync).
- PRECISION_RISK: none per row; arithmetic untouched.
- DUPLICATE_CHECK: vs H1 — H1 derives rowsPerTask from core count with band caps retained; H4 removes bands and uses group-only granularity; identical outcomes on current PASS shapes, different on large-R (H1 packs rows, H4 uses per-row blocks). First revision should pick ONE (H1 recommended) — do not run both back to back on the same shapes. Vs R012: keeps group invariant; vs parent: parent's band + single-core rules both removed. Vs EXT-ASCEND-X rowFactor/ubFactor (unimplemented, next-round-plan.md:64): that varies UB work size and gamma/bias residency per core — H4 touches neither. Vs official five-mode taxonomy (R029/MIX-A owner): no mode dispatch here.
- MINIMAL_OFAT_DIFF: replace band block (lines 436-452) with rowsPerTask = rowGroup; keep rowGroup math, mapping modes, wide cap (or explicitly decide cap interaction — cap binds only D>16384 where taskCount=rowCount; leaving cap gives blockDim ≤8 there: acceptable for this OFAT).
- EXPECTED_LOCAL_PROBES: same as H1 (17×256, 33×100 interleaved pairs) but results would be attributed to H1's family; to distinguish, probe a large-rowCount small-D shape (requires new same-binary qualification) where H1 packs tasks and H4 does not.
- CLASSIFICATION: NEEDS_MORE_EVIDENCE (valid design, but unseparable from H1 on today's qualified shapes — run H1 first, revisit H4 only if a large-rowCount shape becomes qualified or H1 fails on launch overhead).

OPTIONAL-HYPOTHESIS-5: WIDE-CAP AND THRESHOLD SWEEP (scheduler thresholds on wide rows)
- MECHANISM: single-line OFAT first — remove or raise the `rowWidth > 16384 && requestedBlocks > 8 → 8` cap (submission.asc:490-493), letting wide multi-row shapes use min(available, taskCount) cores like all other bands; a later separate revision may re-derive the 16384/4096 band boundaries from measured crossover instead of constants.
- BOTTLENECK: blockDim utilization on wide multi-row shapes: current policy hard-limits D>16384 rows to 8 active cores while official same-op evidence (vllm-ascend SPLIT_D mode) uses the full AIV core count and splits D instead; wide rows are the most expensive per row (official case times up to 125000µs family), so idle cores there cost the most.
- EXPECTED_SHAPES: only rowCount ≥ 9 with D > 16384 (e.g. several rows × 20000-wide). Single-row wide (1×6144 probe) is unaffected (taskCount=1 regardless of cap) — the cap is a no-op on that probe shape.
- WHY_IT_MAY_HELP: per-core work at fixed total bytes shrinks ~cores/8 on capped shapes; wide rows stream large GM volumes where more concurrent readers typically increase HBM occupancy up to bandwidth saturation.
- WHY_IT_MAY_FAIL: the cap may exist because wide-row tasks are already bandwidth-saturated at 8 readers (more cores → no gain, only launch cost), or because ReduceSum/tmp pressure differs — the R016 author left no rationale; wide-D reduction was MIXED in the architecture evidence map (V016 win / V017 loss on official runs).
- ASCEND_FEASIBILITY: trivial host edit; nothing else changes.
- UB/CORE/DMA_IMPACT: UB unchanged; DMA total unchanged, more concurrent streams; core count up to 40.
- SYNC_IMPACT: none.
- PRECISION_RISK: none (row math unchanged; no cross-core reduction).
- DUPLICATE_CHECK: vs R016 parent (cap origin) — this reverses the parent's own threshold rather than restating it; vs WIDE-X-FRESH4 (wide-D reduction/layout, sqrtf build issues) — that route owns reduction/layout, not launch width; vs MIX-A wide full-y residency — different mechanism (data residency vs core count); no sibling next6 route touches host launch width.
- MINIMAL_OFAT_DIFF: delete/guard 3 lines (490–493). Everything else byte-identical.
- EXPECTED_LOCAL_PROBES: none available today — requires a D>16384 multi-row shape to pass same-binary qualification first (wide 1×6144 currently MAD/med 0.490 = NEEDS_VALIDATION/FAIL, and it would not exercise the cap anyway). Until such a shape qualifies, this revision cannot be screened locally.
- CLASSIFICATION: NEEDS_MORE_EVIDENCE (cleanest possible OFAT, but no qualified probe shape exercises it; queue behind H1/H2).

DONOR-RECORDS (historical candidates inspected this turn):
- SOURCE_ROUTE=R016 | MECHANISM=host D-band rows/task (16/8/4/2/1) + cyclic/contiguous mode alternation + wide 8-core cap | OLD_CONTEXT=phase3 FULL-R016-COMPILEFIX official 17.14, neutral/negative vs b0; original V001 compile-failed on all 15 cases | CURRENT_CONTEXT=direct parent of V001; bands retained unchanged inside V001 | WHY_ORTHOGONAL=routine record — this route builds ON it, does not re-donate it | WHY_NOT_DUPLICATE=H1/H4/H5 each replace exactly one of its three rules (granularity, group rounding, cap).
- SOURCE_ROUTE=R012 | MECHANISM=rowGroup = 32/gcd(rowBytes,32) consecutive-row ownership, tasks rounded to group boundaries | OLD_CONTEXT=phase3 PURE-R012 official 21.37 (+3.85 vs b0, positive checkpoint) | CURRENT_CONTEXT=V001's single added donor; group math is the preserved invariant in H1/H2/H4 | WHY_ORTHOGONAL=alignment-ownership, not parallelism policy | WHY_NOT_DUPLICATE=every hypothesis keeps rowGroup untouched; H4 changes only the band table beside it.
- SOURCE_ROUTE=F001 | MECHANISM=blockDim=min(vector_cores, rows), core c → rows c, c+blockDim,… static strided ownership | OLD_CONTEXT=phase4 clean-slate workspace, official 0 pass / Wrong Answer, PARKED; architecture doc only | CURRENT_CONTEXT=closest historical neighbor of H1's per-row fill idea | WHY_ORTHOGONAL=F001 also replaced math/epilogue (hence WA) — never isolates scheduling | WHY_NOT_DUPLICATE=H1 changes only host rowsPerTask derivation on a proven-correct parent; F001's mapping was never a scored scheduling isolation, so no result is being re-run.
- SOURCE_ROUTE=MID-X | MECHANISM=usedCores = min(rows, availableCoreNum, 40) in metadata | OLD_CONTEXT=workspace architecture; route died on runtime errors (V001–V003), slot replaced from idea pool, never scored | CURRENT_CONTEXT=design-note precedent for row-count-aware core fill | WHY_ORTHOGONAL=died in correctness class, no perf evidence | WHY_NOT_DUPLICATE=MID-X ownership was multi-row batch + ReduceSum lane (R015/R007/R011 family), not host scheduling on the R016 parent; replacement-queue lane is BATCH's donor.
- SOURCE_ROUTE=C001 | MECHANISM=row-group of 4 lanes cooperatively D-slices one row (SyncAll + workspace) | OLD_CONTEXT=D-slice cooperative PROVEN_LOSS online (C001 TLE, evidence map) | CURRENT_CONTEXT=not used | WHY_ORTHOGONAL=splits D across cores — opposite direction (adds sync) | WHY_NOT_DUPLICATE=H1/H2/H4/H5 never split a row; no SyncAll introduced (SYNC_IMPACT=none for all).
- SOURCE_ROUTE=MODE-X | MECHANISM=min(R, cores) row-parallel with per-mode row slice (SINGLE_N inner) | OLD_CONTEXT=host-dispatch probes pass, no online score; retired worktree retained | CURRENT_CONTEXT=design echo of core-fill under a mode taxonomy | WHY_ORTHOGONAL=MODE-X dispatches modes on host; our hypotheses keep one kernel path | WHY_NOT_DUPLICATE=absorbing the five-mode taxonomy would duplicate R029 (owner MIX-A/R31B per idea-pool); explicitly excluded from H1–H5.
- SOURCE_ROUTE=EXT-ASCEND-X (not yet started) | MECHANISM=tiling-driven rowFactor/ubFactor + per-core consecutive rows + gamma/bias loaded once per row group | OLD_CONTEXT=first-round fixed slot 6, ACTIVE/NOT_STARTED, no worktree exists; plan text only (next-round-plan.md:64) | CURRENT_CONTEXT=future overlap risk with H1's core-count-derived granularity | WHY_ORTHOGONAL=EXT varies UB work size and parameter residency; H1 varies only rows-per-task vs core count | WHY_NOT_DUPLICATE=recorded proactively: if EXT-ASCEND-X later starts, Main should read this file before assigning H1 to avoid lane collision — flag, not a conflict today.

EXTERNAL-IDEA RECORDS (architecture/tiling/scheduling ideas only; no code copied):
- EXTERNAL_IDEA=EXT-VLLM-ASCEND-BLOCKFACTOR | PROVENANCE_CLASS=B (official vllm-project/vllm-ascend repo, same op family add_rms_norm_bias) | URL/PATH=github.com/vllm-project/vllm-ascend csrc/moe/add_rms_norm_bias/op_host/add_rms_norm_bias_tiling.cpp, CalculateBlockParameters | IDEA=blockFactor = ceil(numRow/numCore); useCoreNum = ceil(numRow/blockFactor) = min(numCore, numRow); latsBlockFactor = numRow − blockFactor·(useCoreNum−1) hands the remainder rows to the last core; rowFactor/ubFactor derived from measured UB size for small-D row grouping | WHERE_USED=H1 (core-fill derivation), H2 (explicit tail remainder), H4 (small-D grouping via rowGroup instead of UB-size merge) | NOTE=its MERGE_N mode merges multiple small rows per UB pass — that half is batch-compute territory owned by BATCH-RESIDENT-X (R015) and by R029 taxonomy (MIX-A); deliberately NOT taken.
- EXTERNAL_IDEA=EXT-VLLM-CUDA-PERROW | PROVENANCE_CLASS=B (official vllm-project/vllm repo, CUDA) | URL/PATH=csrc/libtorch_stable/layernorm_kernels.cu, dim3 grid(num_tokens) with one block per token, vectorized loop over D inside the block | IDEA=row ownership granularity = exactly 1 row per block; hardware scheduler fills cores; no host rows-per-task band | WHERE_USED=H3 motivation and H4's granularity floor | NOTE=supports per-row default; does not address Ascend DataCopyPad tails (I001-proven pad path stays).
- EXTERNAL_IDEA=EXT-TRITON-ROWGRID | PROVENANCE_CLASS=B (official triton-lang/triton tutorial 05-layer-norm.py) | URL/PATH=python/tutorials/05-layer-norm.py forward launches `_layer_norm_fwd_fused[(M,)]` — grid = (M,), one program per row, column loop in BLOCK_SIZE chunks; BLOCK_SIZE = min(64KB/elem, next_pow2(N)); num_warps heuristics from BLOCK_SIZE | IDEA=per-row grid + column-tile loop sized by register/block budget — granularity chosen so every row is independent, no task grouping | WHERE_USED=H1/H4 rationale for small-D row grouping (group only what alignment requires, nothing more) | NOTE=tutorial, same-op family (RMS/LayerNorm forward).
- EXTERNAL_IDEA=EXT-VLLM-ASCEND-FIVEMODE | PROVENANCE_CLASS=B | PATH=same tiling file: MODE_NORMAL/SPLIT_D/MERGE_N/SINGLE_N/MULTI_N dispatch | IDEA=official five-mode taxonomy thresholds (numCol>ubFactor→SPLIT_D; blockFactor==1→SINGLE_N; small numCol→MERGE_N; aligned FP16→MULTI_N) | WHERE_USED=recorded for completeness; EXCLUDED from hypotheses — idea-pool assigns R029 five-mode dispatch to MIX-A/R31B, so taking it would be a cross-lane duplicate.

THEME-COVERAGE (required exploration themes → where handled): shape-aware core count=H1; row ownership granularity=H1/H3/H4 (+EXT-VLLM-CUDA-PERROW); small-D row grouping=H4 (group floor) + EXT-VLLM-ASCEND rowFactor note; tail-row ownership=H2 (+EXT-VLLM-ASCEND latsBlockFactor); core underfill=H1/H4; blockDim utilization=H1/H5; per-core row continuity=analyzed inside H2 (contiguous extents) and recorded as evidence gap: scheduleMode cyclic/contiguous alternation by band has no measured evidence in any retained run — candidate follow-up only when taskCount > coreNum shapes (large rowCount) are qualified, since at current PASS shapes taskCount ≤ cores and both mappings coincide; 32B alignment interaction=H1 keeps rowGroup rounding (its rowsPerTask ≥ rowGroup by construction; worst case FP16/BF16 odd-D forces rowGroup=16, i.e. granularity 16 regardless of band — quantified in H4); scheduler thresholds=H4 (band constants removed), H5 (wide cap + boundary sweep).

RECOMMENDED_NEXT: (1) No action on Track-A — V001 SHA stays unchanged; wait for Main's timing instruction on 17×256/33×100 under the reference protocol (interleaved pairs, device events, warmup≥10). (2) First new revision after Main disposes V001 → HYPOTHESIS-1 (core-fill rowsPerTask): single host-side OFAT, probes already exist on both PASS shapes, expected effect (2–3 → 9–17 cores) far above the 0.023 same-binary noise floor. (3) If H1 shows launch-overhead regression → HYPOTHESIS-2 (even-split extents, same block count). (4) H3/H4/H5 stay in backlog pending (dynamic-queue API verification / a qualified large-rowCount shape / a qualified wide multi-row shape respectively). (5) Duplicate watch: before Main assigns any core-count scheduling idea to the queued replacement lane CORE-SCHED-X (next-round-plan.md:93), route it here first — this file owns that mechanism family; likewise H1 must be re-read against EXT-ASCEND-X if that slot ever starts.


---

## CYCLE 2026-09-25 (LH3-SCHED-33x100) — Track-A disposition + H1/H2 re-verification + new screening

> This section supersedes the CURRENT_CANDIDATE / CURRENT_BLOCKER status lines above (kept for provenance).

TRACK-A-OUTCOME: **ONLINE_CANDIDATE** (local status; local % ≠ Official Score; Main disposes).
Lease LH3-SCHED-33x100, device d4, unified DEVICE_EVENT protocol, one clean attempt each stage,
no source edits, V001 SHA unchanged `0fae0a42…`.
- Same-binary exact-shape PASS: 33×100 (ALL MAD/med 0.0536, drift 0.0990) and 17×256
  (0.0262, 0.0247); both ≤0.10 on first attempt — prior blockers (drift 0.1999/0.1036,
  reverse pair, high C MAD) resolved.
- Interleaved P/C PC/CP/PC/CP same window: 33×100 deltas −51.68/−51.07/−52.04/−50.99 % →
  **median −51.38%, 4/4 favor V001, no reverse pair, order-robust**; 17×256 aligned control
  −25.80(outlier-tailed P)/−4.16/+2.03/−0.20 → median −2.18%, ex-p1 ≈ −0.2% (in noise floor).
- bad=0 on all 18 runs; load AICore 0% + VLLMEngineCor HBM-resident on d4 (documented).
- Evidence: `cann-next6/SCHED-ROWGROUP-X/phase4/local/SCHED-ROWGROUP-X/V001/support/results-lh3-{sb33,pc}/`,
  handoff `…/V001/HANDOFF-ONLINE-CANDIDATE.md`.

BOTTLENECK-MODEL UPDATE: V001's core-count lever is now measured twice under the reference
protocol: 1→3 cores on 33×100 bought **−51.4%** (this cycle; earlier −31.81% run had a reverse
pair and inflated C MAD — direction consistent, current number is the clean one). Sublinear:
3× cores → 1.95× speedup (~65% parallel efficiency at k=3). 17×256 (2 blocks both binaries,
16+1 rows) stayed in noise (−0.76% prior, ≈−0.2% ex-p1 this cycle) — control confirms the win
comes from lifting the single-core fallback on non-32B rows, not from host noise. H1's premise
(core count is the dominant lever; bands never derived from availableCoreNum) is strengthened;
its gain assumption should now be calibrated against ~65% scaling efficiency at k=3 rather than
assumed linear (see H9).

H1/H2 RE-VERIFICATION vs submission.asc (sha 0fae0a42…, 531 lines — re-read this cycle):
- Band block: lines 436–451 (D≤256: rowsPerTask=16, scheduleMode=0 cyclic; ≤1024: 8/1;
  ≤4096: 4/0; ≤16384: 2/1; else 1/1) — H1's 436-452 ref OK (±1).
- rowGroup = 32/gcd(rowBytes,32): lines 454–463 (`rowGroup` at 463); rounding to group
  boundaries: 465–469; taskCount: 471–477; available/requestedBlocks: 479–486; wide 8-core cap:
  490–493 — H2's 471-477 and H5's 490-493 refs OK.
- Kernel task loop: lines 91–144 (cyclic stride 94–108; contiguous base/extra 114–124;
  firstRow/endRow 96–101, 130–135) — H2's 95-99/129-133 refs OK. Kernel computes
  firstRow = task×rowsPerTask in-kernel → H2's note (host-derived extents need a small kernel
  edit OR in-kernel even division) remains correct.
- availableCoreNum source: runner_ref.inc:212 `aclrtGetDeviceInfo(ACL_DEV_ATTR_VECTOR_CORE_NUM)`
  → real device vector-core count, no hidden hard cap. H1's core-count target is feasible up to
  full AIV count; wide cap (490–493) is the only launch-width restriction and only at D>16384.
- **CORRECTION to H1 EXPECTED_SHAPES**: 33×100 FP32 rowBytes=400 → gcd(400,32)=16 →
  **rowGroup=2 (not 4)**. H1's "~9 blocks" (16/4·…) is wrong; with rowGroup=2 the H1 formula
  gives rowsPerTask=2 → taskCount≈16 on 33×100. Direction of the expectation (≫3 blocks) is
  unchanged, magnitude is better.
- Probe-shape mapping under current V001 (verified): 33×100 → rowsPerTask=16, mode0 cyclic,
  taskCount=3=blockDim ⇒ 16/16/1 rows across 3 cores; 17×256 → taskCount=2=blockDim ⇒ 16+1
  rows, 2 cores. Confirms core-underfill quantification (2–3 blocks vs ~16–17 possible).

PUBLIC RESEARCH RE-VERIFIED (architecture only, no code copied):
- EXT-VLLM-ASCEND-BLOCKFACTOR: live re-read of
  `github.com/vllm-project/vllm-ascend csrc/moe/add_rms_norm_bias/op_host/add_rms_norm_bias_tiling.cpp`
  `CalculateBlockParameters` — confirmed: blockFactor = ceil(numRow/numCore) (via tileNum loop),
  useCoreNum = ceil(numRow/blockFactor), latsBlockFactor = remainder → last core,
  `context->SetBlockDim(use_core_num)` (full core fill is the official default). New details
  recorded: alignment units BLOCK_ALIGN_NUM=16 (B16) / FLOAT_BLOCK_ALIGN_NUM=8 (FP32 ⇒ 32 B,
  matches our rowGroup=32/gcd derivation); MERGE_N threshold SMALL_REDUCE_NUM=2000 columns;
  UB_FACTOR_B32=10240 / B16=12288 elements; numCore from `GetCoreNumAiv()`.
- EXT-TRITON-ROWGRID: live re-read of `triton-lang/triton python/tutorials/05-layer-norm.py` —
  confirmed `_layer_norm_fwd_fused[(M,)]`, `row = tl.program_id(0)`, one program per row,
  BLOCK_SIZE = min(65536//elem_size, next_pow2(N)), num_warps = min(max(BLOCK_SIZE//256,1),8).
- EXT-VLLM-CUDA-PERROW (`dim3 grid(num_tokens)`): record from prior cycle unchanged; not
  re-fetched this cycle (low churn, architecture claim already source-backed).

### H6: SCHEDULEMODE UNIFORMITY ABLATION (cyclic/contiguous by-band alternation removal)
- MECHANISM: force scheduleMode to one value for all five bands (either always 0 = cyclic stride
  `task=coreIdx; task+=coreNum`, or always 1 = contiguous base/extra split); rowsPerTask,
  rowGroup rounding, taskCount, requestedBlocks, cap all unchanged. Pure OFAT on lines 438/444/
  447/450 mode assignments.
- BOTTLENECK: per-core row-continuity / stripe locality — currently D≤256 and D≤4096 bands use
  cyclic, the other bands contiguous, with zero recorded evidence for either choice.
- EXPECTED_SHAPES: **provable zero effect on all currently qualified shapes**: blockDim =
  min(available, taskCount), so whenever taskCount ≤ cores (both PASS shapes: 3 and 2 tasks),
  each block receives exactly one task under both loops → identical ownership. Effect exists only
  when taskCount > cores (large rowCount): cyclic interleaves consecutive tasks across cores,
  contiguous gives each core a packed row range.
- WHY_HELP: on large-R shapes contiguous gives each core a contiguous GM row range (better burst
  locality for DataCopyPad); cyclic guarantees equal task counts with heterogeneous tails.
- WHY_FAIL: on today's probe shapes the two mappings are mathematically identical, so a nonzero
  local delta would falsify the harness, not the hypothesis; no qualified large-R shape exists.
- ASCEND_FEASIBILITY: trivial (constant assignment to an existing uint32); kernel paths already
  both implemented.
- UB/DMA/SYNC/PRECISION: UB unchanged / total bytes unchanged (distribution only) / none / none
  (rows independent).
- DUPLICATE_CHECK: no prior hypothesis owns mode (THEME-COVERAGE only flagged it as an evidence
  gap); distinct from H1 (granularity), H2 (extents), H4 (band removal), H5 (cap).
- MINIMAL_OFAT_DIFF: scheduleMode constant across band block 436–451; nothing else.
- FALSIFICATION: on a qualified large-R shape, interleaved same-window cyclic vs contiguous P/P
  comparison beyond the same-binary floor → mode matters (then pick winner); on PASS shapes
  delta must be ≡0 (else harness fault). 
- CLASSIFICATION: NEEDS_MORE_EVIDENCE (provably inert on today's PASS shapes; needs a qualified
  taskCount>cores shape — same blocker family as H3/H5).

### H7: PER-TASK OWNERSHIP WITHOUT ROW-GROUP FLOOR (relax the R012 rounding invariant)
- MECHANISM: drop the ceil-to-rowGroup rounding (lines 465–469) and let rowsPerTask take any
  band value down to 1 even when rowGroup>1; each task then starts at a byte offset that may not
  be 32B-aligned, relying on the per-row DataCopyPad path the parent already uses for unaligned
  rows. Ownership safety shifts from "complete 32B groups per core" to "disjoint rows per core +
  pad path".
- BOTTLENECK: the granularity floor rowGroup itself. Worst case: FP16/BF16 odd-width rows
  (rowBytes=2w with gcd(2w,32)=2 → rowGroup=16) force ≥16-row tasks regardless of band → core
  underfill that H1/H4 cannot remove (both keep the invariant). On FP32 probes rowGroup≤2 so
  H7 ≈ H1 there (no local separation possible).
- EXPECTED_SHAPES: FP16/BF16 odd-D multi-row (e.g. 17×257 FP16: rowBytes=514 → rowGroup=16 →
  V001 taskCount=2; H7 allows 17 tasks). Neutral on FP32 33×100/17×256 (floor 2/1).
- WHY_HELP: removes the last structural cause of the parent's single-core behavior on unaligned
  shapes; the parent rule's rationale was never documented beyond "ownership safety"; rows are
  disjoint outputs so cross-core pad writes cannot alias if each task owns whole rows.
- WHY_FAIL: the parent rule may exist for a real GM-burst/coalescing cost of unaligned per-core
  access, in which case more cores × slower copies could lose; pad-tail correctness across the
  15-case suite (FP16/BF16 unaligned) must be re-proven; this is the only screened hypothesis
  that questions a V001 donor invariant — higher risk class.
- ASCEND_FEASIBILITY: host-only (remove rounding); kernel untouched — ProcessRow is already
  row-granular and takes row index.
- UB/DMA/SYNC/PRECISION: UB unchanged / same total GM bytes, more concurrent streams (pad path
  per row unchanged) / none / none (per-row arithmetic identical).
- DUPLICATE_CHECK: explicitly questioning R012 (V001's donor) — flagged as risk, not duplicate;
  H1/H2/H4 all preserve the group floor; parent is stricter (single-core fallback), so this is
  not a parent restatement; not ALIGN (copy path unchanged), not C001 (no D split, no sync).
- MINIMAL_OFAT_DIFF: lines 465–469 → use band rowsPerTask directly (no group ceil); everything
  else byte-identical.
- FALSIFICATION: (1) correctness suite must stay PASS-identical vs V001 on all 15 cases — any
  new mismatch kills it; (2) on a qualified FP16 odd-D shape, expect taskCount 2→~rowCount and a
  delta beyond floor; (3) if unaligned-shape times regress at equal core count vs V001 group
  ownership → unaligned per-core GM access cost is real, abandon.
- CLASSIFICATION: NEEDS_MORE_EVIDENCE (correctness-rationale audit + FP16 shape qualification
  required first; queue strictly after H1 — H1 subsumes the FP32 win where rowGroup≤2).

### H9: CORE-SCALING CALIBRATION SWEEP (measurement experiment, pre-falsifies H1's ceiling)
- MECHANISM: experimental host variant ONLY (never a scoring candidate): rowsPerTask = rowGroup
  (H4 floor, so taskCount ≈ rowCount/rowGroup) plus requestedBlocks override k ∈ {1,2,3,4,6,8,
  16,min(available,taskCount)}; run k values interleaved across processes in one window on 33×100
  (17×256 secondary), same-binary DEVICE_EVENT protocol → speedup-vs-core-count curve.
- BOTTLENECK: unknown core-scaling shape of this kernel. V001 gives one measured point (k=3 →
  1.95× vs parent k=1, ~65% efficiency); H1's expected win (k≈9–16) assumes more.
- EXPECTED_SHAPES: 33×100 primary (taskCount≈16 with floor 2 ⇒ k≤16 meaningful), 17×256
  secondary (k≤17).
- WHY_HELP: turns H1's expected gain into a measured projection before any V002 exists; curve
  plateau tells us whether to cap H1's oversubscription target with data; also isolates launch
  overhead (per-block fixed cost) directly — H1/H2's main stated failure risk.
- WHY_FAIL: still a source change (host) → FORBIDDEN until Main disposes V001, same gate as H1;
  costs one lease window (~minutes, cheap under the reference protocol); results are
  shape-specific (33×100 FP32 class).
- ASCEND_FEASIBILITY: runner/CLI unchanged; experimental build only; no kernel edit.
- UB/DMA/SYNC/PRECISION: launch width only — UB/DMA totals/SYNC/precision all unchanged.
- DUPLICATE_CHECK: not a revision of the scoring line — recorded as an experiment so Main does
  not count it as a competing Candidate; composes H4 floor × width sweep; H1 stays the single
  scoring change afterwards. No other route owns a core-scaling experiment.
- MINIMAL_OFAT_DIFF (experiment build): rowsPerTask = rowGroup (465–469 bypass) + requestedBlocks
  = min(k, taskCount) with k from CLI/env.
- FALSIFICATION: curve plateau at k≤4–6 → H1's ~16-core expectation corrected DOWN before
  implementation (and H2 priority rises); near-linear to 16 → H1 proceeds as written; sharply
  falling median with k (launch overhead) → H1's oversubscription cap set below the knee.
- CLASSIFICATION: READY_FOR_MAIN_REVIEW (recommended as a measurement-layer step BEFORE the H1
  revision if Main wants evidence; still gated on V001 disposition).

SCREENED-OUT THIS CYCLE (short form, no full field sets):
| candidate | verdict | reason |
|---|---|---|
| constant rowsPerTask for all bands (D-agnostic granularity) | DUPLICATE_OF H4 | on PASS shapes rowGroup floor makes constant≡H4 (rowGroup 2 / 1) |
| kernel-side row-strided static map (F001 revival, mode0) | DUPLICATE_OF H4 | identical ownership on PASS shapes; F001 donor record already covers WA history |
| always launch full core count + empty-block early return | SCREENED_FAIL | adds launch overhead; blocks beyond taskCount have no work (taskCount≤cores already saturates) |
| merge tail task into penultimate (merge-tail balance) | DUPLICATE_OF H2 inverted | H2 even-split strictly dominates merge-tail on max-task length |
| band D-boundary crossover sweep (re-derive 256/1024/4096/16384) | DUPLICATE_OF H5 stage 2 | same constants family; keep in H5 |
| device task-pull queue | EXISTING H3 | unchanged, still NEEDS_MORE_EVIDENCE |

RECOMMENDED-NEXT (cycle 2026-09-25): (1) Track-A handed off as ONLINE_CANDIDATE — Main
disposes V001 (promote or judge-own); local % ≠ Official Score. (2) H1 remains FORBIDDEN until
that disposition. (3) If Main wants the H1 bet calibrated first → H9 sweep (one lease window).
(4) Then H1 (corrected expectation: 33×100 ≈16 tasks, not 9; rowGroup=2), H2 fallback.
(5) H6/H7/H3/H4/H5 stay NEEDS_MORE_EVIDENCE with their stated qualification blockers.
(6) Lane guard unchanged: CORE-SCHED-X and EXT-ASCEND-X must read this file first.

## POST-ONLINE_CANDIDATE NOTE (2026-09-25, short — no new hypothesis)

- V001 status stays **local ONLINE_CANDIDATE** (SHA `0fae0a42e3942356fe3477cd518b17180b6104d57350cee705cba37a5895e65c` unchanged); local % ≠ Official Score; Main disposes.
- **H1 (core-fill rowsPerTask) and H9 (core-scaling sweep) remain blocked — do not implement.**
  Both require source edits (H1: host band block; H9: experimental host build). They stay
  FORBIDDEN until Main either sends V001 to Judge or issues NEXT_HYPOTHESIS. H9 in particular
  must not be slipped in as a "measurement-only" run: it is a source build and carries the
  same hold.
- Same rule for anything else promoted from this file later (H2/H3/H6/H7/H4/H5): Track-B
  research may continue read-only; no `.asc`/host edits, no new Revision, no device runs from
  this route until Main's explicit instruction.
- This cycle: no hypothesis added, no source edit, no device lease; file append only.

# ALIGN-TAIL-X — Next Hypotheses (TRACK-B research, append-only)

> **MAIN-2 APPROVALS 2026-09-25** — APPROVED NEXT backlog: H1 threshold-controlled bulk+tail split (bulk via direct DataCopy; small remainder via minimal tail / DataCopyPad chosen by threshold). H1 implementation FORBIDDEN while current Candidate is undecided. Scope: ALIGN keeps DataCopy / DataCopyPad / aligned bulk / tail / copy-direction asymmetry only; H2 row-group geometry DEFER_TO_SCHED. H3 copy-direction asymmetry remains in research.

## CURRENT_CANDIDATE

- REVISION: V001 (SHA `f573d16d39fb54a2a75f75f90943168b96dd97d83fb6bd094f11fd73c61df840`)
- DIRECT_PARENT: PURE-R009-V001-ALIGNED-DATACOPY (parent SHA `c8d0f010…`, historical online 17.64)
- SINGLE_HYPOTHESIS: aligned bulk → direct DataCopy; non-aligned remainder → minimal DataCopyPad tail; fully aligned → parent DataCopy; unaligned start → parent full pad. Compute/reduction/scheduling/param policy untouched; no R012 row-group in V001.
- Where the split actually fires: per-tile (4096-elem) Load/Store in `submission.asc:94-195`. For probe shape 2×100 FP32 a row is one 400-byte tile: row 0 aligned-start → bulk 384B DataCopy + 16B pad tail; row 1 unaligned-start (400B offset) → parent full 400B pad. Gamma/bias loads share the same path.
- Host policy: non-32B rowBytes → single core (`submission.asc:438-466`), so every non-aligned-D shape runs on ONE core by design until a later row-group revision.
- Decision state: `NEEDS_ONE_MORE_LOCAL + MEASUREMENT_BLOCKED`; all prior probe rounds LOAD_CONTAMINATED or noise-dominated (L002 median +19.22% vs parent jitter floor 25µs). TRACK-A: source frozen, no edits, no device runs this turn.

## CURRENT_BLOCKER

Exact-shape same-binary qualification (probe shape 2×100 FP32) under the unified protocol has not passed yet (Q-ALIGN 2/2 UNQUALIFIED earlier; protocol now requires per-shape same-binary noise floor with this Route's Direct Parent + device events + warmup≥10). Until that PASS, no P/C pair is interpretable, so no V001 result and no data-driven choice among the hypotheses below. This is a measurement-layer blocker, not a kernel blocker; Main owns the device window.

## BOTTLENECK_MODEL

Per-tile copy-path cost model for the ALIGN parent architecture (R009 copy-centric, two-pass, per-row):

1. **DMA descriptor / branch count.** Each tile does 2 loads per pass (x, residual) × 2 passes + gamma + bias loads (pass 2) + 1 store = 7 copy ops per row-tile at D≤4096. V001 turns every non-aligned tile into TWO ops (bulk DataCopy + pad tail) plus a 3-way branch. If DataCopyPad ≈ DataCopy in raw MTE cost (official best-practice doc: "对齐场景下性能差异可忽略", `api-datacopy.md:53`), the split's saving is at most the pad overhead on the remainder — while paying one extra descriptor + branch. For a 400B tile with a 16B tail the arithmetic can go negative. → "DataCopyPad cost" and "small remainder path" are first-order questions.
2. **One-core collapse on non-aligned D.** Host runs non-32B rows on a single core (`submission.asc:441-454`). The tail mechanism therefore couples to core occupancy: tail shapes (D%8≠0 FP32) lose N-1 cores regardless of copy efficiency.
3. **Unaligned-start rows bypass the candidate entirely.** For D%8≠0, row starts alternate 32B-aligned/unaligned; unaligned-start rows always take the parent full-pad path, so the candidate only ever touches ~half the tiles on such shapes.
4. **Compute-side counts are `valid`, not aligned.** ReduceSum/Add/Mul/Div run on counts like 100 (not multiple of 8 for ReduceSum fast paths); no compute-tail mechanism exists yet in this route.
5. **32B boundary facts (local docs).** DataCopy requires count×sizeof(T) % 32 == 0 (common-traps §原因2); DataCopyPad UB start address must be 32B aligned while blockLen may be unaligned (`api-datacopy.md:123`); DataCopyPad blockCount max 4095 (`api-datacopy.md:152`); GM stride in bytes, UB stride in 32B units (`api-datacopy.md:283`); multi-row blockCount pad lands rows on rLengthAlign (32B) UB slots (scenario 4, `api-datacopy.md:217-274`). CopyIn pad fill rules differ from CopyOut auto-dummy-discard (`api-datacopy.md:108-119`) — real GM→UB vs UB→-GM asymmetry at the semantics level; cost asymmetry not yet evidenced.

## HYPOTHESIS-1

- TITLE: Threshold-controlled split (small-remainder path) — pad whole tile instead of bulk+tail when the remainder is small.
- MECHANISM: In `Load`/`Store`, issue bulk-DataCopy + pad-tail only when the non-aligned remainder ≥ R_min AND the aligned bulk ≥ B_min (both tuned constants, e.g. remainder ≥ 64B and bulk ≥ 512B); otherwise issue the parent's single DataCopyPad over the whole tile. V001 = (R_min=0, B_min=32B); parent = split never. One conceptual change: the split condition.
- BOTTLENECK: per-tile MTE descriptor count + pad-instruction overhead on small tails; at probe D=100 the tail is 4–16 elems (16–64B) and the tile is 400B — exactly the regime where two ops may cost more than one pad op.
- EXPECTED_SHAPES: non-aligned D (D%8≠0 FP32), tiles whose remainder < threshold; probe 2×100 FP32 (every row-tile); wide rows only on the last tile of each row.
- WHY_IT_MAY_HELP: removes one DMA descriptor and one branch from the hot path for small tails; matches official guidance that pad ≈ DataCopy cost, so the split only pays when the remainder is large enough for the pad's byte-level handling to matter.
- WHY_IT_MAY_FAIL: if pad cost is per-byte rather than per-call, splitting large remainders always wins and thresholds help nothing; if descriptor cost is pipelined away, the change is neutral; thresholds add shape-dependence that may regress wide-D last tiles.
- ASCEND_FEASIBILITY: high — pure API-usage change on already-legal ops; no new instruction, no sync change beyond existing `PipeBarrier` placement (`submission.asc:260,327`).
- UB/CORE/DMA_IMPACT: UB identical; core occupancy identical (host one-core policy untouched); DMA op count per non-aligned tile drops from 2 to 1 below threshold.
- SYNC_IMPACT: none (same barrier structure; fewer DMA completions to wait on).
- PRECISION_RISK: none — pad fill dummy never enters compute (counts stay `valid`); CopyOut dummy auto-discarded.
- DUPLICATE_CHECK: not the candidate (V001 has no threshold); not parent (parent never splits); no other active route owns a split policy (SCHED=core ownership, BATCH=batched multi-row DMA, REDUCE=invscale, ASYNC=MTE3 overlap, UB-LIVENESS=buffer aliasing). R010 donor is a per-row independent implementation with different tiling — this is a policy on the ALIGN parent path only.
- MINIMAL_OFAT_DIFF: add two constants + split condition in `Load`/`Store`; nothing else.
- EXPECTED_LOCAL_PROBES: exact-shape 2×100 FP32 same-binary first (V001 vs parent), then threshold sweep {never-split, ≥64B remainder, ≥512B bulk, always-split=V001} as separate binaries under interleaved pairs; device-event primary per protocol.
- CLASSIFICATION: READY_FOR_MAIN_REVIEW (execution contingent on V001 exact-shape result and a clean device window; run as V002 only after Main issues NEXT_HYPOTHESIS).

## HYPOTHESIS-2

- TITLE: Row-stride alignment via padded UB stride (rLengthAlign) + single multi-row DataCopyPad per tile-group on the non-aligned-D path.
- MECHANISM: Lay out UB rows at 32B-padded stride (`rLengthAlign`, official scenario 4, `api-datacopy.md:217-274`) and issue ONE multi-row DataCopyPad (blockCount≤4095, blockLen=validBytes, stride=0) covering a group of contiguous rows instead of per-row/per-tile pad calls with the 3-way branch. Hardware lands each block on a 32B-aligned UB slot; compute indexes at rLengthAlign stride with count=valid; store mirrors with srcStride in 32B units. This is layout-level tail elimination (the tail disappears at the row boundary instead of being handled per transfer).
- BOTTLENECK: (a) DMA descriptor count per row on non-aligned shapes — currently 7 ops/row-tile, collapse to one multi-row op per field per row-group; (b) the one-core collapse: descriptor savings matter most when a single core must stream many rows.
- EXPECTED_SHAPES: non-aligned D with row counts ≥ group size (e.g. 2×100 has only 2 rows — group=2 is one call replacing two); mid/wide R with D%8≠0; unchanged behavior for aligned D (parent's direct DataCopy multi-row path can stay).
- WHY_IT_MAY_HELP: removes per-row branches and pad setups entirely; official docs already describe this as the recommended Softmax/LayerNorm pattern; pad ≈ DataCopy cost per doc, so winning = fewer ops, not faster ops.
- WHY_IT_MAY_FAIL: padded stride wastes UB (rowWidth→rLengthAlign) and this route's 7 buffers are already 112KiB-ish at kTileElems=4096; blockCount pad across rows with different tail sizes needs uniform blockLen (per-tile, not per-row) — rows shorter than the group tile still need per-row handling; and if descriptor cost is hidden by in-flight MTE queues, gain ≈ 0.
- ASCEND_FEASIBILITY: high for the pad/blockCount path (documented, blockCount≤4095 clip required); direct multi-row DataCopy with stride also exists for the aligned case.
- UB/CORE/DMA_IMPACT: UB grows by up to (rLengthAlign−rowWidth)×rows×sizeof(T) per buffer — must re-check the 192KiB budget with 7 buffers; DMA descriptors drop roughly rowCount×(2 passes×2 + 2 params + 1 store) → ~(passes+params+store) per row-group; core occupancy unchanged (host policy is a separate revision).
- SYNC_IMPACT: fewer EnQue/DeQue events possible, but V001 uses raw PipeBarrier — barrier count can drop proportionally to op count; no new sync primitives.
- PRECISION_RISK: padded UB slots are excluded by count=valid in every op; pad dummy in GM→UB must not be read — safe if counts stay valid. I001 V006 "padded uKeep slack fix" (WA 13/15, results.tsv) is the historical warning: padding only fails when padded elements leak into compute; keep counts valid.
- DUPLICATE_CHECK: BATCH-RESIDENT-X owns R015 contiguous multi-row batch DMA *after parameter residency* — overlap risk on the "multi-row single DMA" element; scoping difference: this hypothesis is the padded-stride layout (rLengthAlign) that makes multi-row pad *legal for non-aligned D* on the ALIGN parent path without param-residency changes. SCHED-ROWGROUP-X owns R012 32B row-group **core ownership** — different layer (who owns rows vs how rows are laid out/copied). Must be reviewed against both before promotion; if Main judges it inside BATCH's scope, mark DUPLICATE and drop.
- MINIMAL_OFAT_DIFF: change UB addressing stride + replace per-row pad calls with one blockCount pad per row-group; no compute/schedule/host change.
- EXPECTED_LOCAL_PROBES: 2×100 FP32 (group=2, 7 ops/row → ~4 calls/row-group), then a mid non-aligned shape (e.g. 4×1025 FP32) for descriptor scaling; correctness golden on NaN/Inf rows and row-group boundaries.
- CLASSIFICATION: DEFER_TO_SCHED — row-group ownership / rows-per-task / core-fill / shape scheduling 固定归 SCHED-ROWGROUP-X（MAIN-2 仲裁 2026-09-25）；ALIGN 只保留 DataCopy / DataCopyPad / aligned bulk / tail / copy-direction asymmetry。本条涉及行组几何的部分不属 ALIGN；纯 UB padded-stride 布局部分如需保留须重新表述为不涉 row-group ownership 的版本并再审。UB budget recompute 仍然需要。

## HYPOTHESIS-3

- TITLE: GM→UB vs UB→GM asymmetric split — apply the bulk+tail split to one direction only.
- MECHANISM: The candidate treats Load and Store identically, but the two directions differ in documented semantics: CopyIn unaligned pad has fill/dummy rules (dummy = first element when isPad=false, `api-datacopy.md:108-115`) and carries no rightPadding in the tail path; CopyOut automatically discards dummy (`api-datacopy.md:117-119`). Split the two paths: variant L = split Load only, uniform pad Store; variant S = split Store only, uniform pad Load. Each variant is one conceptual change.
- BOTTLENECK: direction-specific MTE2 vs MTE3 handling of unaligned blockLen; which direction actually pays more pad overhead on the two-pass design (pass 1: 2 loads, pass 2: 4 loads + 1 store — loads outnumber stores 6:1, so load-side savings are structurally larger).
- EXPECTED_SHAPES: non-aligned D; probe 2×100 FP32 hits both directions every row; shapes where gamma/bias loads (col-offset, unaligned for D%8≠0) amplify load-side ops.
- WHY_IT_MAY_HELP: concentrates the split where op count is highest (MTE2) and lets the less frequent direction keep the simpler single-op path; if pad cost is direction-asymmetric, only one side may be worth splitting.
- WHY_IT_MAY_FAIL: no cost evidence yet that directions differ — the documented asymmetry is semantic (fill rules), not performance; if costs are symmetric, both variants regress vs V001 by exactly the removed split on that direction.
- ASCEND_FEASIBILITY: high — both variants use only legal documented call forms.
- UB/CORE/DMA_IMPACT: no UB change; no core change; DMA op count changes by ±1 per non-aligned tile per direction.
- SYNC_IMPACT: none.
- PRECISION_RISK: none (counts unchanged).
- DUPLICATE_CHECK: not V001 (V001 splits both directions); no other route owns direction-specific copy policy. Distinct from HYPOTHESIS-1 (which keys on size, this keys on direction).
- MINIMAL_OFAT_DIFF: delete the split in one of `Load`/`Store`, keep it in the other; two binaries, one mechanism each.
- EXPECTED_LOCAL_PROBES: exact-shape 2×100 pairs for L-variant then S-variant vs the same parent; only after V001's own exact-shape result exists (otherwise the comparison reference is missing).
- CLASSIFICATION: NEEDS_MORE_EVIDENCE (semantic asymmetry documented; performance asymmetry unknown — needs one same-binary measurement of V001 first, then L/S variants).

## OPTIONAL-HYPOTHESIS-4

- TITLE: dtype-conditioned split policy — narrow dtypes (FP16/BF16) use one uniform DataCopyPad per tile, FP32 keeps the bulk+tail split.
- MECHANISM: compile-time (`if constexpr`) per-dtype copy policy. The byte-split granularity is dtype-dependent: FP32 tails are multiples of 4B (bulk granularity 8 elems), FP16/BF16 tails are multiples of 2B (bulk granularity 16 elems), so narrow dtypes produce a tail on every D%16≠0 row while transferring half the bytes per element — descriptor overhead per byte is roughly 2× worse. Uniform single-pad for narrow dtypes removes the split (and its branch) exactly where the byte/descriptor ratio is worst; FP32 keeps V001's split.
- BOTTLENECK: per-byte DMA descriptor overhead for narrow dtypes; judge dtypes include FP16 and BF16 (PROBLEM.md: dtype FP16/BF16/FP32, D 64..32768, D may be non-32B).
- EXPECTED_SHAPES: FP16/BF16 with D%16≠0 (e.g. 4×65 BF16, 2×101 FP16); FP32 behavior unchanged (this hypothesis is a no-op for the current probe).
- WHY_IT_MAY_HELP: matches the split to the regime where it can pay (FP32, larger element bytes) and drops branch+second descriptor where it likely cannot (2B-element tails).
- WHY_IT_MAY_FAIL: if descriptor cost is shape-independent rather than byte-proportional, narrow dtypes benefit from the split just as much; if pad cost scales with blockLen (not per-call), uniform pad loses on wide narrow-dtype rows.
- ASCEND_FEASIBILITY: high — both call forms legal for T=half/bfloat16_t; candidate already instantiates `AddRmsNormBiasKernel<half>/<bfloat16_t>` (`submission.asc:469-481`).
- UB/CORE/DMA_IMPACT: no UB or core change; DMA op count per non-aligned narrow tile 2→1.
- SYNC_IMPACT: none.
- PRECISION_RISK: none — counts stay `valid`; pad dummy excluded from compute.
- DUPLICATE_CHECK: HYPOTHESIS-1 is size-thresholded across dtypes; this is a dtype-keyed all-or-nothing policy with no size constants — different OFAT unit. No active route handles narrow-dtype copy tails (BATCH's batch DMA is dtype-agnostic; REDUCE owns math, not copy). R010's symmetric per-row branch is dtype-generic too.
- MINIMAL_OFAT_DIFF: one `if constexpr` branch selecting the non-aligned path per dtype; FP32 path textually identical to V001.
- EXPECTED_LOCAL_PROBES: FP16/BF16 correctness golden first (dtype matrix), then paired timing on one narrow non-aligned shape (e.g. 4×65 BF16) + re-run one FP32 shape to confirm no FP32 drift. Requires judge_types/runner dtype support already present in support/.
- CLASSIFICATION: NEEDS_MORE_EVIDENCE (no local narrow-dtype timing exists for this route; needs one dtype-capable clean window after the FP32 exact-shape result).

## OPTIONAL-HYPOTHESIS-5

- TITLE: Zero-fill to aligned counts — eliminate the compute-side tail (and the ReduceSum<8 edge) by padding loads to 32B-multiple counts and storing only `valid`.
- MECHANISM: every non-aligned tile load becomes one DataCopyPad with `isPad=true, paddingValue=0` extending blockLen to the next 32B multiple; all compute ops (Cast/Add/Mul/Div/ReduceSum) then run at `alignedCount = round_up(valid, elemsPer32B(dtype))` instead of `valid`; Store keeps `valid` (V001 store path). The tail branch collapses to the load site; padded zeros never leave UB.
- BOTTLENECK: vector/reduce operations on counts that are not 32B multiples — including a latent correctness edge: D in [64..32768] with D ≡ 1..7 (mod 4096) gives a last tile with valid < 8, and Level-2 ReduceSum documents a minimum of 8 elements (common-traps, `api-reduce.md`). The current two-pass parent and V001 both pass such `valid` straight into ReduceSum (`submission.asc:271`).
- EXPECTED_SHAPES: any D with a non-32B-multiple final tile (most non-aligned D); specifically D ≡ 1..7 (mod 4096) for the ReduceSum<8 edge; probe 2×100 (valid=100 → aligned 104).
- WHY_IT_MAY_HELP: one legal load call covers alignment; compute always sits on documented aligned widths; removes the ReduceSum<8 hazard for free; fewer distinct counts per tile loop.
- WHY_IT_MAY_FAIL: pad adds up to 7 elements of wasted DMA per load (negligible bytes); ReduceSum tree grouping on padded counts may reorder real-element summation → tiny numeric drift vs golden; Div/Mul on padded slots is harmless only because slots are +0.0 — any isPad=false fallback (dummy = first element) would poison the sum (CopyIn fill rules, `api-datacopy.md:108-115`).
- ASCEND_FEASIBILITY: high for load-side zero pad (documented); compute at aligned counts is plain API usage; no mask primitives needed (contrast with GPU masked-tail designs).
- UB/CORE/DMA_IMPACT: no buffer growth (buffers are 4096-elem aligned already); DMA bytes +≤28B per load; core policy unchanged; may even allow Store to switch to direct DataCopy later (separate revision).
- SYNC_IMPACT: none (same barriers).
- PRECISION_RISK: medium — summation order over padded counts must be validated against golden (ops-precision-standard tolerances); negative precedent I001 V006 "padded uKeep slack fix" WA 13/15 shows padded-slack leakage fails when padded values reach output or un-padded garbage enters the sum; here both leaks are excluded by construction (pad value 0, store valid) but must be proven by golden + NaN/Inf rows.
- DUPLICATE_CHECK: V001/parent compute with `valid` counts everywhere — no route in this worktree runs aligned-count compute; REDUCE-INVSCALE-X changes reduction math (invscale), not count alignment; this hypothesis keeps math identical and changes only counts+pad. Distinct from HYPOTHESIS-1 (copy-side op count) and HYPOTHESIS-2 (row layout).
- MINIMAL_OFAT_DIFF: round load count up + set pad params to zero-fill; compute and store call sites change only the count expression; one conceptual change ("compute on aligned counts").
- EXPECTED_LOCAL_PROBES: exact-shape 2×100 correctness (sum path) + targeted golden for D ≡ 1..7 (mod 4096) (e.g. D=4103) where the current parent would call ReduceSum(7); timing pairs only after correctness passes.
- CLASSIFICATION: NEEDS_MORE_EVIDENCE (correctness edge makes it partly a hardening candidate; needs golden on the ReduceSum<8 shape and one tolerance run before any timing claim).

## DONOR-RECORDS (candidates inspected this cycle)

1. SOURCE_ROUTE: R009 (FULL-R009-COPY-CENTRIC; phase3 archive `independent__full-r009-copy-centric-i001` + `fast__lane-c` L001-L003; current ALIGN direct parent PURE-R009-V001).
   MECHANISM: copy-centric two-pass RMS; tileCapacity rounded to 32B; direct DataCopy when size fully 32B-aligned, single full DataCopyPad otherwise; double-buffered queues (R009 I001 `kernel.txt`).
   OLD_CONTEXT: phase3 online 17.64, 15/15.
   CURRENT_CONTEXT: ALIGN V001 parent (SHA c8d0f010…), host one-core policy for non-aligned rows.
   WHY_ORTHOGONAL: H1-H5 all modify the non-aligned transfer policy this parent lacks (it has exactly one pad path and no threshold/direction/dtype/count distinctions).
   WHY_NOT_DUPLICATE: parent never splits non-aligned sizes and never keys a policy on remainder size, direction, dtype, or padded counts.
2. SOURCE_ROUTE: R010 (FULL-R010-TAIL-CENTRIC I001, archive independent branch; compile-only run).
   MECHANISM: per-row outer loop; full 32B blocks via DataCopy, last block via DataCopyPad, output uses the same symmetric branch to avoid tail writes into the next row (I001 README).
   OLD_CONTEXT: CANN9 compile + static preflight only; no runtime/perf evidence.
   CURRENT_CONTEXT: flavor donor behind V001's tail specialization; its tiling (per-row, no 4096 tile, no two-pass FP32 residency) differs from ALIGN's architecture.
   WHY_ORTHOGONAL: H3 (direction asymmetry) and H4 (dtype policy) split what R010 kept symmetric; H5 removes the compute tail R010 also ignores.
   WHY_NOT_DUPLICATE: different tiling/scheduling; R010 has no threshold, no dtype branch, no padded-count compute; it is not an active route.
3. SOURCE_ROUTE: R012 (FULL-R012-ALIGNMENT-ROWGROUP I001 archive; idea-pool R012; active SCHED-ROWGROUP-X donor).
   MECHANISM: alignedDim-padded buffers, rowsPerBlock/tailRows tiling, 32B-safe row-group ownership across cores (I001 `tiling.h` fields; SCHED brief V001).
   OLD_CONTEXT: archived i001, marked "covered/widespread" in idea pool; SCHED now owns the ownership layer.
   CURRENT_CONTEXT: excluded from ALIGN V001 by brief ("later revision only"); host comment reserves row-group idea (`submission.asc:444-446`).
   WHY_ORTHOGONAL: H2 applies alignment at UB row-stride/copy layout only — no change to which core owns which rows.
   WHY_NOT_DUPLICATE: SCHED-ROWGROUP-X V001 = shape-aware rows/task + row-group core ownership on a scheduling parent; H2 = padded-stride layout + single multi-row pad on the ALIGN copy parent. Overlap is real at the "row group" wording level — Main must arbitrate scope before promotion (recorded as the open duplicate risk).
4. SOURCE_ROUTE: A001 V003 (results.tsv f027ee8 "aligned DataCopy for full 32B transfers", 25.50→25.74).
   MECHANISM: prefer direct DataCopy for full 32B transfers instead of unconditional pad.
   OLD_CONTEXT: phase4 online win (+0.24), case14/15 improved.
   CURRENT_CONTEXT: already inherited as the parent's aligned path inside V001.
   WHY_ORTHOGONAL: H1 asks the complementary question — when NOT to split / when a single pad beats two ops.
   WHY_NOT_DUPLICATE: the aligned-direct-DataCopy mechanism itself is parent behavior, not a new revision.
5. SOURCE_ROUTE: I001 V004 / V006 (results.tsv 78c6542 pad+ReduceSum fix → PASS 15.53; ee849da padded uKeep slack → WA 13/15).
   MECHANISM: pad enables non-aligned correctness; padded-slack variants leak padded data when padding enters compute/output.
   OLD_CONTEXT: V004 = proven online pad correctness ("UB layout … I001 DataCopyPad tail proven" in architecture-evidence-map); V006 = negative precedent.
   CURRENT_CONTEXT: cited as feasibility evidence for H2/H5 and as the precision warning they must beat.
   WHY_ORTHOGONAL: mechanisms are evidence, not designs being reused.
   WHY_NOT_DUPLICATE: no new mechanism proposed from these; they bound the risk model.

## EXTERNAL-IDEAS

- EXTERNAL_IDEA: EXT-IDEA-ATEN-REMAINDER-LOOP
  SOURCE: PyTorch ATen `aten/src/ATen/native/cpu/layer_norm_kernel.cpp` (github.com/pytorch/pytorch @ main, fetched 2026-09-25); `aten/src/ATen/native/layer_norm.cpp` dispatch layer also fetched.
  MECHANISM: full-width vector main loop up to `N - (N % vec::size())` followed by a separate remainder loop (scalar tail, or partial-width `loadu(ptr, count)`); FP16/BF16 first upcast to float, then the same full-width+remainder split runs in the wider domain.
  WHY_DIFFERENT_FROM_EXISTING_ROUTES: no next6 route isolates the tail at compute level — ALIGN parents use count=`valid` everywhere, and GPU/CPU unaligned `loadu` has no Ascend analog because UB vector start addresses must be 32B aligned (official "非对齐场景" practice doc: vector start address not 32B = error). The transferable idea is the *structure* (one wide fast path + one isolated remainder path), which on Ascend must be rebuilt as pad+aligned-count (H5) rather than copied.
  EXPECTED_BOTTLENECK: vector-unit/reduce handling of counts that are not 32B multiples, plus per-tile branch cost.
  APPLICABLE_ROUTE: ALIGN-TAIL-X (H5); secondary note for REDUCE-INVSCALE-X (compute tail) — no action there from this route.
  PROVENANCE_CLASS: PUBLIC_WEB_PRIMARY_SOURCE (architecture pattern only, no code copied).
- EXTERNAL_IDEA: EXT-IDEA-CANN-LOOP-MODE
  SOURCE: CANN 9.2.0-beta.2 official practice docs "非连续搬运场景减少搬运次数" and "非对齐场景减少无效数据的搬运" (hiascend.com document center, search excerpts 2026-09-25).
  MECHANISM: Loop mode (SetLoopModePara/ResetLoopModePara) copies blocks of differing sizes in ONE transfer instruction, replacing repeated DataCopyPad calls when per-block padding differs; Compact mode (950PR/950DT only) minimizes transferred dummy bytes on unaligned multi-block copies.
  WHY_DIFFERENT_FROM_EXISTING_ROUTES: V001 issues one instruction per tile plus a second for the tail; loop-mode would fold a whole row's full tiles + tail tile into one instruction. No active route uses loop mode or any multi-block single instruction.
  EXPECTED_BOTTLENECK: transfer instruction count per row (7 ops/row-tile at D≤4096 today).
  APPLICABLE_ROUTE: ALIGN-TAIL-X — feasibility first: Loop-mode API availability on dav-2201 under CANN 8.5 toolchain unverified; Compact mode documented for 950-series only, likely INFEASIBLE here.
  PROVENANCE_CLASS: OFFICIAL_CANN_DOC (platform caveats recorded).

## THEME-COVERAGE (required research themes → where handled)

| theme | where |
|---|---|
| aligned bulk copy | CURRENT_CANDIDATE + donor R009/A001-V003 |
| manual tail handling | CURRENT_CANDIDATE + donor R010; Reg asc_storeunalign tail APIs found but RegBase/DAV_3510-only → INFEASIBLE on dav-2201 |
| DataCopyPad cost | BOTTLENECK_MODEL §1 + HYPOTHESIS-1 (official doc: aligned-case pad≈DataCopy) |
| small remainder path | HYPOTHESIS-1 |
| 32B boundary behavior | BOTTLENECK_MODEL §3/§5 (GM start vs size predicates; probe row pattern) |
| dtype-specific tail | OPTIONAL-HYPOTHESIS-4 (+ H5 alignment counts per dtype) |
| row-stride alignment | HYPOTHESIS-2 (rLengthAlign layout) |
| GM-to-UB vs UB→GM asymmetry | HYPOTHESIS-3 (fill rules vs dummy discard; 6:1 load:store ratio) |

## RECOMMENDED_NEXT

1. Track-A unchanged: run exact-shape same-binary (2×100 FP32, Direct Parent, device events, warmup≥10) in the next Main-assigned clean window; no source edits until Main issues NEXT_HYPOTHESIS.
2. HYPOTHESIS-1 (threshold-controlled split) is the leading next revision: READY_FOR_MAIN_REVIEW, execution contingent on the V001 exact-shape result — V001 neutral/negative ⇒ H1 directly; V001 positive ⇒ H1 remains the natural refinement (sweep thresholds on top of the winning split).
3. Main decision needed: HYPOTHESIS-2 scope vs BATCH-RESIDENT-X (multi-row DMA) and SCHED-ROWGROUP-X (row-group ownership) before it can be promoted past NEEDS_MORE_EVIDENCE.
4. OPTIONAL-4/5: gather narrow-dtype probes and the D≡1..7 (mod 4096) ReduceSum-count golden before any revision; H5 doubles as a latent correctness hardening worth a standalone correctness run even without timing.
5. Data still missing: measured DataCopyPad vs DataCopy cost on dav-2201 (no local msprof evidence exists; official docs claim negligible difference in aligned cases only). One msprof comparison of parent vs V001 on the exact shape would resolve H1's core uncertainty — collect it opportunistically with the qualification run if load permits.

## TRACK-A PREP 2026-09-25

- Binaries READY: unified reference harness ported verbatim from SCHED `runner_ref.inc` (md5 `93e6441937e9cd8394b734e124d18469`; DEVICE_EVENT primary + wall secondary, one aclInit/malloc/H2D/stream per process, warmup≥10, ≥21 samples/block, batch N=1, argc 10/11) and built on cann-server3 (cmake -j1): `atx_ref_parent_probe` SHA256 `a8bd66a6b6da5bdf1acf350edd8e6f55c8044a8db48d9421a650637285d3187c`, `atx_ref_v001_probe` SHA256 `bfb2988a482337a21e1ef2d2f5da3c81aaf7695956d1604f7e8c0b2e59a04955`. Local copy: `cann-next6/ALIGN-TAIL-X/phase4/workspaces/ALIGN-TAIL-X/support/build/`; server: `~/phase4-workspaces/ALIGN-TAIL-X/support/build/`.
- Identity verified local + server: Direct Parent PURE-R009-V001-ALIGNED-DATACOPY `c8d0f010…`, Candidate V001 `f573d16d…` (full values in runbook §1). No source edits; no new Revision; kernel SHA unchanged.
- Start-proof only: one untimed launch on d6 (rc=0, `bad=0`, 2×100 FP32) to confirm the binary starts with the documented `LD_LIBRARY_PATH`. No timing runs and no lease lines appended this turn (SCHED held the d4 window).
- Runbook: `cann/phase4/local/ALIGN-TAIL-X/runbook-2x100-qualification.md` — (a) same-binary 2×100 FP32 parent-vs-parent, PASS iff MAD/med≤0.10 AND block drift≤0.10 (max 2 attempts); (b) on PASS, interleaved P/C `P C C P ×2`, 31 samples/process, warmup 10; `ONLINE_CANDIDATE` only if both blocks favor Candidate beyond the same-binary floor. Output dir `support/results-qual-2x100/d<DEV>/`; lease line format in runbook §7.
- Remains: device window — Main opens it (lease id `SV-ALIGN-2x100`), then run (a); PASS → (b) in the same window.

## STATUS UPDATE 2026-09-25 (supersedes CURRENT_BLOCKER and the last line of TRACK-A PREP)

Track-A ran and finished this cycle. Exact-shape same-binary on 2×100 FP32 (Direct Parent vs itself) **failed 2/2**, so the blocker is no longer "window not opened" — it is a measurement-layer failure on this shape.

- Lease `SV-ALIGN-2x100` on **d4**, 2026-09-25T14:21:27Z → 14:24:34Z (released). Method: unified reference harness, DEVICE_EVENT primary, warmup 10, 2 in-process blocks × 31 samples, gap 2 s, batch N=1, `bad=0` on both attempts (correctness OK).
- sb1: ALL_DEVICE median 8.54 µs, MAD 1.88 → **MAD/med 0.2201**; B1 10.18 vs B2 7.00 → **drift 0.3724**.
- sb2: ALL_DEVICE median 6.86 µs, MAD 0.80 → **MAD/med 0.1166**; B1 9.24 vs B2 6.32 → **drift 0.4257**.
- Decision per runbook §4: both attempts FAIL and drift > 0.25 → **MEASUREMENT_PROTOCOL_BLOCKED_FOR_SHAPE** (harness/host issue; never a Candidate issue). No interleaved P/C was run. Handoff value: **MEASUREMENT_BLOCKED**.
- Systematic pattern, not random: B1 median > B2 median in both attempts (~+45 %, +46 %), i.e. block 2 is consistently faster after 10 warmups + 2 s gap; plus sparse device-event outliers (max 174–183 µs vs median 6.9–8.5 µs, p90 ≈ 123 µs). Warmup=10 (validated on SCHED's 17×256) does not make this shape stationary.
- Raw evidence (permanent): `cann-next6/ALIGN-TAIL-X/phase4/local/ALIGN-TAIL-X/V001/support/results-qual-2x100/d4/{sb1,sb2}-parent-w10-s31-raw.tsv`, `-stats.txt`, `summary`, npu-smi start/end, timestamps.
- SHA unchanged: V001 `f573d16d39fb54a2a75f75f90943168b96dd97d83fb6bd094f11fd73c61df840`; parent `c8d0f010…`; probe binaries `a8bd66a6…` / `bfb2988a…` re-verified local + server before the run. No kernel edit, no new revision.
- Device ecology during the window (shared `server3-device-leases.tsv`, all MAIN-2): ALIGN d4 → BATCH-RESIDENT-X d5 → REDUCE-INVSCALE-X d6 leased in sequence, then ASYNC-TRIPLE-X took d4 at 14:27:51Z after my release. One malformed line exists (BATCH's first claim: device field = `2026-09-25T00:00:00Z`, later corrected to d5) — Main should be aware the lease file has an unparseable row.
- Consequence for the hypotheses below: no V001-vs-parent number exists, so the decision rule "V001 neutral/negative ⇒ H1 directly; V001 positive ⇒ H1 as refinement" (RECOMMENDED_NEXT §2) cannot be applied. All three Track-B entries remain research-only; none may be promoted to V002 until Main issues NEXT_HYPOTHESIS.

## HYPOTHESIS-1 — DEEPENING 2026-09-25 (OFAT / falsification / precision-ABI)

Fields follow MAIN-2's required field order; this block is the reviewable version of HYPOTHESIS-1.

- MECHANISM: in `Load`/`Store` (`kernel.asc:94-195`), choose between (i) parent single `DataCopyPad` over the whole tile and (ii) V001 aligned-bulk `DataCopy` + minimal `DataCopyPad` tail, by two scalar thresholds on the tile: `remainderBytes ≥ R_min` AND `bulkBytes ≥ B_min`. Parent = never split; V001 = `R_min=0, B_min=32` (always split when start-aligned and bulk>0); H1 = any interior constant pair.
- BOTTLENECK: per-tile DMA descriptor + scalar predicate cost. Per tile there are 7 copy ops (pass1: 2 loads; pass2: 4 loads + 1 store; plus gamma/bias); V001 turns each non-aligned tile into 2 ops + a 3-way branch. Official doc states pad ≈ DataCopy in aligned cases and that `DataCopyParams`/`DataCopyExtParams` hit the same hardware instruction (`api-datacopy.md:53,55-67`), so the only measurable saving is the descriptor/branch overhead on small tails — exactly what a threshold isolates.
- EXPECTED_SHAPES: D%8≠0 FP32 with tiles whose remainder < threshold; probe 2×100 FP32 (one 400 B tile/row: row0 → bulk 384 B + 16 B tail, row1 → parent full pad); wide rows only on the last tile per row (D>4096).
- WHY_IT_MAY_HELP: below threshold the hot path returns to one descriptor + zero second-op bookkeeping, matching the doc's "pad is not cheaper than DataCopy" guidance — the split should only be paid for when the remainder is large enough for the pad's partial-block handling to matter.
- WHY_IT_MAY_FAIL: if pad cost is per-byte (not per-call), thresholds never help and always-split wins; if MTE queues hide the extra descriptor, all settings equalize; constants add shape-dependence that can regress wide-D last tiles.
- ASCEND_FEASIBILITY: high — pure API-usage change on already-legal call forms; no new instruction; barrier structure untouched (`kernel.asc:260,327`).
- UB/CORE/DMA_IMPACT: UB identical; host one-core policy for non-32B rowBytes untouched (`kernel.asc:438-466`); DMA ops per non-aligned tile 2 → 1 below threshold.
- SYNC_IMPACT: none — same `PipeBarrier` sequence, fewer in-flight DMA completions.
- PRECISION_RISK: none. Counts stay `valid`; CopyIn tail uses `padParams{false,0,0,0}` (dummy = first element) but only `valid` elements are consumed; CopyOut dummy is auto-discarded (`api-datacopy.md:112-119`). Thresholds change which legal call runs, never the data.
- ABI: two call forms only — `DataCopy(localT, globalT, count)` (3-arg) vs `DataCopyPad(..., DataCopyExtParams, DataCopyPadExtParams<T>)` (Load) / `DataCopyPad(globalT, localT, DataCopyExtParams)` (Store, CopyOut overload with no pad params). Threshold evaluation is scalar-unit work (`and`/`cmp`/`branch` on `byteCount`), not MTE; Ext vs non-Ext forms are the same hardware instruction per doc, so ABI width is not a variable. H1 must keep Store's 4-arg overload and Load's Ext form exactly as V001 has them, otherwise it stops being a one-change OFAT.
- DUPLICATE_CHECK: not V001 (no threshold), not parent (never splits); no active route owns a split *policy* — SCHED = core ownership, BATCH = batched multi-row DMA after param residency, REDUCE = invscale math, ASYNC = MTE3 overlap, UB-LIVENESS = buffer aliasing. R010 donor is a different tiling with no threshold.
- MINIMAL_OFAT_DIFF: exactly one constant changed per binary relative to V001. Binary set (each is its own build, no multi-change):
  - `OFAT-R1`: `R_min=64B`, `B_min=32B` (V001 with remainder floor only)
  - `OFAT-B1`: `R_min=0`, `B_min=512B` (V001 with bulk floor only)
  - `OFAT-BOTH`: `R_min=64B`, `B_min=512B` (only after R1 and B1 are each measured alone — never both at once)
  - references: parent (never-split) and V001 (`R=0,B=32`), both already built.
- FALSIFICATION (what kills H1, stated before any data):
  1. *Resolution kill* — if V001 vs parent on the exact shape lands inside the same-binary floor in both blocks (|Δ| ≤ floor), the entire split family is below measurement resolution; thresholds cannot show a win ⇒ H1 demoted from READY_FOR_MAIN_REVIEW.
  2. *Direction kill* — if the sweep is monotone toward never-split (parent ≥ all thresholds ≥ V001), the split itself is wrong and thresholds are not the lever ⇒ H1 as stated is false; the correct next step would be "drop the split", not "tune it".
  3. *Mechanism kill* — one msprof comparison (parent vs V001, same shape) showing the extra DataCopy adds an instruction but ≈0 cycles (MTE issue hidden) ⇒ descriptor-cost mechanism false, thresholds can only regress.
  4. *Flat kill* — all four settings equal within floor ⇒ shape-insensitive; keep V001 or parent as-is on correctness grounds, no revision.
- EXPECTED_LOCAL_PROBES: same-binary floor for the shape must PASS first (currently blocked, see STATUS UPDATE); then interleaved P/C with device events, warmup≥10, ≥31 samples, ≥4 pairs; threshold binaries only after Main's NEXT_HYPOTHESIS.
- CLASSIFICATION: READY_FOR_MAIN_REVIEW (unchanged) — but execution now depends on a *usable measurement path for 2×100*, not on a V001 result that no longer exists.

## HYPOTHESIS-3 — DEEPENING 2026-09-25 (OFAT / falsification / precision-ABI)

- MECHANISM: V001 splits **both** directions identically; variant L = split in `Load` only (`kernel.asc:94-150`), uniform parent pad in `Store`; variant S = split in `Store` only (`kernel.asc:151-198`), uniform parent pad in `Load`. One variant = one conceptual change.
- BOTTLENECK: direction-specific MTE2 (GM→UB) vs MTE3 (UB→GM) handling of a non-32B `blockLen`. Structural asymmetry is already in the op census: per tile **6 loads : 1 store** (pass1 2 loads; pass2 4 loads + 1 store), so any per-op saving is ~6× larger on the load side — that arithmetic, not a measured cost, is the current motivation.
- EXPECTED_SHAPES: any D%8≠0; probe 2×100 hits both directions every row; gamma/bias loads (`kernel.asc:311-312`, column offset, unaligned when D%8≠0) add load-side ops that stores do not have.
- WHY_IT_MAY_HELP: concentrates the split where op count is highest and lets the low-frequency direction keep the simpler single-op path; if pad cost is direction-asymmetric, only one side may be worth splitting.
- WHY_IT_MAY_FAIL: the documented direction difference is *semantic*, not performance — CopyIn has fill rules, CopyOut auto-discards dummy; no doc or local evidence says MTE2 and MTE3 pad costs differ. If costs are symmetric, each variant regresses vs V001 by exactly the removed split on that direction.
- ASCEND_FEASIBILITY: high — only documented call forms; both variants already exist textually inside V001 (deleting one branch is a reduction, not an addition).
- UB/CORE/DMA_IMPACT: no UB change, no core change; DMA ops per non-aligned tile change by ±1 per direction.
- SYNC_IMPACT: none.
- PRECISION_RISK: none. Load tail keeps `padParams{false,0,0,0}` → dummy = first element, consumed only up to `valid`; Store CopyOut discards dummy automatically. Neither variant changes counts, so no numeric difference is possible by construction — the only risk would be a wrong overload chosen for one direction (see ABI).
- ABI (direction-asymmetric by construction): Load needs the 4-argument form with `DataCopyPadExtParams<T>`; Store uses the 3-argument CopyOut form with no pad params. So variant L carries the pad-params struct setup and variant S does not — an ABI-surface difference that is *not* a perf variable but must be kept byte-identical to V001's existing code for each direction under test. Re-check that `DataCopyPadExtParams<T>` (T-typed paddingValue) is what Load already uses and that no switch to legacy `DataCopyPadParams` (uint64 paddingValue) happens during the edit — that would be a second change.
- DUPLICATE_CHECK: not V001 (V001 splits both), not parent (splits neither); no active route owns direction-specific copy policy. Distinct axis from H1 (size) and from H6 (predicate/control-flow placement).
- MINIMAL_OFAT_DIFF: delete the split in exactly one of `Load`/`Store`, keep the other byte-identical to V001; two binaries, no constants, no layout change.
- FALSIFICATION (pre-registered):
  1. If both L and S land within the same-binary floor of V001 ⇒ direction has no measurable cost, H3 dead (keep V001's symmetric split or drop it per H1's direction kill).
  2. If Δ_L + Δ_S ≈ Δ_V001 (additive within floor) and |Δ_L| ≈ |Δ_S| despite the 6:1 op ratio ⇒ op count does not drive cost ⇒ the load-side rationale collapses even if direction matters.
  3. If only one variant moves and the other is flat, H3 survives with that direction and the mechanism becomes "op-count-proportional cost", testable against H1's descriptor story.
- EXPECTED_LOCAL_PROBES: exact-shape pairs for L then S against the same parent, only after a usable floor exists for 2×100 (currently blocked).
- CLASSIFICATION: NEEDS_MORE_EVIDENCE (unchanged — semantic asymmetry documented, performance asymmetry unmeasured).

## HYPOTHESIS-6 (NEW 2026-09-25) — start-alignment-aware DataCopy predicate

- TITLE: `sizeAligned` is checked without `startAligned` on the full-tile path — add start alignment to the direct-DataCopy predicate.
- MECHANISM: `Load`/`Store` branch on `sizeAligned = (byteCount & 31)==0` **first** and take direct `DataCopy` unconditionally (`kernel.asc:113-117`, `159-163`); `startAligned` is only consulted inside the non-size-aligned branch (`kernel.asc:119`, `165`). For a full 4096-element tile (`byteCount=16384`, always size-aligned) with a GM start that is not 32 B aligned, the code therefore issues `DataCopy` from an unaligned GM address. Parent has the identical size-only predicate (`parent/kernel.asc:107-112`, `131-136`). H6 = predicate becomes `sizeAligned && startAligned`, so an unaligned-start full tile falls to the pad path (parent's full-pad form; V001 would additionally keep its bulk+tail split for that tile).
- BOTTLENECK: not a performance hypothesis — it is a completeness question about the alignment predicate that defines this route (aligned bulk vs pad tail). If the hardware/doc really requires a 32 B GM start for `DataCopy`, every non-aligned-D shape with D≥4096 and rows≥2 silently runs half its tiles on a forbidden call form.
- EXPECTED_SHAPES: D ∈ [4096, 32768] with D%8≠0 (row-1 start = D×4 bytes ≡ non-zero mod 32) and rows ≥ 2 — e.g. 2×4100, 2×4103, 3×8193. Below D=4096 every tile's `valid` is the whole row, so `byteCount` is non-aligned whenever the row start is, and the pad path is taken anyway (this is why 2×100 never exercises the edge). Gamma/bias loads use `col` offsets only (aligned when D%8≠0 rows are handled per-column) and are unaffected for col=0 tiles.
- WHY_IT_MAY_HELP: closes a potential data-corruption path with a two-operand predicate change; if the edge is real it explains any historical shape-dependent wrongness and it is a correctness-first repair (README allows correctness-only repairs with no performance variable).
- WHY_IT_MAY_FAIL: local references only state count alignment for `DataCopy` (`common-traps.md` 原因2: `count*sizeof(T)` must be a 32 B multiple; `api-datacopy.md:84-96` table is element counts) and state a 32 B start requirement only for the **UB side of DataCopyPad** (`api-datacopy.md:123`). No local doc says GM start must be 32 B aligned for `DataCopy`. The parent is historical phase3 online 17.64 / 15/15, which is evidence the edge either does not exist on dav-2201 or was never covered by those 15 cases — **UNKNOWN, not assumed**.
- ASCEND_FEASIBILITY: trivially high — a pure predicate change; both call forms already present.
- UB/CORE/DMA_IMPACT: UB unchanged; core policy unchanged; worst case adds one pad op per unaligned-start full tile (i.e. makes an illegal-looking transfer legal, at descriptor cost).
- SYNC_IMPACT: none.
- PRECISION_RISK: none by construction (pad path consumes `valid` only; CopyOut dummy discarded); the risk being *tested* is data corruption, not numeric drift.
- ABI: same two call forms; only the branch condition changes. No struct, no count, no offset changes.
- DUPLICATE_CHECK: no active route owns the alignment *predicate* (SCHED/BATCH/REDUCE/ASYNC/UB-LIVENESS operate on scheduling, batch residency, math, overlap, aliasing). Distinct from H1 (threshold on remainder/bulk sizes), H3 (direction), H2 (row layout, deferred), H5 (compute counts). Note this hypothesis also covers the parent, so it is a shared-surface finding Main should see regardless of route scope.
- MINIMAL_OFAT_DIFF: one `&&` operand added to each of the two predicates in V001's `Load` and `Store` (and, if Main agrees, the same in the parent — that would be a separate correctness repair, not part of any performance revision).
- FALSIFICATION (one decisive run, pre-registered):
  1. Run `atx_ref_parent_probe 4 2 4100 0 … 1 1 1 0` and the V001 binary on the same shape. `bad=0` on both ⇒ edge is not exercised on this device/driver ⇒ H6 downgraded to NEEDS_MORE_EVIDENCE (doc-check only, no repair). `bad>0` ⇒ real latent corruption in parent and V001 ⇒ escalate to Main as a correctness finding before any performance revision.
  2. Cross-check with 2×4103 (also tests H5's ReduceSum `valid=7 < 8` documented minimum, `api-reduce.md`) to separate "alignment edge" from "reduce-count edge".
  3. Doc check (no device needed): official CANN `DataCopy` Restriction section for a GM-address alignment clause. Until (1) or (3) resolves, the claim stays UNKNOWN.
- EXPECTED_LOCAL_PROBES: two correctness probes, no timing, ≤1 minute of device time — **blocked this cycle**: d4 (ASYNC), d5 (BATCH), d6 (REDUCE) all leased after 14:24Z, d7 forbidden.
- CLASSIFICATION: NEEDS_MORE_EVIDENCE (decisive falsification ready; do not treat as a bug report until probe or official doc confirms).

## TRACK-B CYCLE SUMMARY 2026-09-25

- Hypotheses worked this cycle: **3** — H1 deepened (OFAT matrix + 4 pre-registered kills + precision/ABI), H3 deepened (2 OFAT variants + 3 pre-registered kills + direction-specific ABI), H6 added (new, predicate completeness, one-run falsification designed).
- Scope respected: DATA COPY ALIGNMENT / PAD / TAIL only; no kernel modification, no new Revision, no Judge, no MCP/Plugin; external material used only as architecture references already recorded under EXTERNAL-IDEAS.
- Device used this cycle: **d4 only** (lease `SV-ALIGN-2x100`, 14:21:27Z–14:24:34Z); d5/d6 taken by other routes; d7 untouched.
- Blockers for Main: (1) 2×100 FP32 same-binary noise floor fails with systematic B1>B2 drift >0.25 — measurement layer, needs a harness/host decision (e.g. longer warmup, different gap, or drop this shape from timing) before any ALIGN P/C; (2) H6's one-run falsification needs a device slot; (3) malformed lease row (`device=2026-09-25T00:00:00Z`) in `server3-device-leases.tsv`.

## H6 DEEPENING 2026-09-25 — official DataCopy restriction check + exact 2×4100/2×4103 falsification design (correctness only, no timing)

This block completes H6's pre-registered falsifier #3 (doc check) and replaces the draft one-run
falsification with a decidable battery. Source refs re-verified this turn against
`workspaces/ALIGN-TAIL-X/{V001,parent}/kernel.asc`.

### 1. Official restriction check (falsifier #3 — RESOLVED)

Evidence read (docs only; no source copied):

- **PRIMARY OFFICIAL (local, machine-readable):** asc-devkit 9.2.0 doc
  `.cannbot/dependencies/ops-direct-invoke/asc-devkit/docs/zh/api/SIMD-API/basic_api/memory_vector_compute/data_move/DataCopy_GMAndUB_continuous.md`,
  operand notes L79-85 and 约束说明 L136-139, verbatim substance:
  1. Global Memory addresses must be aligned to the **data-type byte size** (float → 4 B; half → 2 B) — NOT to 32 B;
  2. UB addresses must be 32 B aligned;
  3. `count * sizeof(T)` must be 32 B aligned; if not, the transfer size is **silently rounded down** to a 32 B multiple (a real behavior note, but it does not fire here — both parent and V001 only issue `DataCopy` under `sizeAligned`).
- Corroborating local refs: skill `ascendc-api-best-practices/references/api-datacopy.md` §32字节对齐 (L84-93, a size table) and §UB端起始地址 (L121-123, UB-side start requirement applies to DataCopyPad); `ascendc-precision-debug/references/common-traps.md` 原因2 (L371-379, `count*sizeof(T)` only).
- Conflicting summary NOT used: `ascendc-docs-search/references/api-index.md` L189 "DataCopy：512 字节对齐" — a generic cheat-sheet line that contradicts the API doc's explicit 约束说明; most plausibly describes cube/AdvAPI copy paths, not basic GM↔UB `DataCopy`. Recorded so nobody re-litigates it.
- Public web: hiascend CANN 8.5 API pages return JS shells only (fetched 2026-09-25; no per-API text retrievable) → CANN 8.5 online wording UNCONFIRMED. Version caveat: the primary source is the 9.2.0 devkit while this route builds CANN 8.5.0.alpha002; this class of constraint is expected to be stable across versions but is formally unverified for 8.5.

**CONSEQUENCE:** no official GM-32B clause exists for `DataCopy`. Row starts are always
4-B aligned in fp32 (offset = row·D·4 bytes, D integral), the UB side sits on a buffer base
(32 B by InitBuffer), and tile sizes are 16384 B → the call form H6 flags is
**document-conforming**. The parent's size-only predicate is arguably correct as documented;
adding `startAligned` would be defense-in-depth, not a violation repair. H6's premise is now:
doc says LEGAL, dav-2201 hardware behavior is the only open question → the probe below decides.

### 2. Exact falsification battery (correctness only; zero timing)

Edge arithmetic (FP32, kTile=4096 elems — verified: V001 `kernel.asc:108-114` Load size-only
branch, `116` startAligned only inside the non-size-aligned path, `157-163`/`165` Store
mirror; parent `kernel.asc:109-112`/`133-136` same size-only test inline). Row-1 GM start =
4D bytes; D%8≠0 ⇒ start mod 32 = 4·(D mod 8) ≠ 0 while staying 4-B aligned ⇒ row-1 tile-0
has `byteCount=16384` (sizeAligned TRUE) at an unaligned GM start → direct `DataCopy` on
both Load and Store. Row 0 start = 0 → never exercises the edge. Below D=4096 the whole row
is one tile whose byteCount is non-aligned exactly when the start is → pad path → edge
unreachable (why 2×100 is blind). Host forces one core on non-32B rowBytes
(`kernel.asc:436-455`), so every shape below runs single-core regardless of the blocks arg.

**Confound:** for D ∈ (4096, 8192] the tail tile has valid = D−4096; D=4100 → 4, D=4103 → 7,
both below the Level-2 `ReduceSum` documented minimum of 8 (H5's separate edge, counts are
`valid`). Structurally in fp32, D%8≠0 ⇒ tail%8≠0, so an unaligned-start shape can never carry
a multiple-of-8 tail — the two edges cannot be split by fp32 tail arithmetic. They are split
by ROW COUNT (alignment edge needs rows≥2; reduce edge does not) plus one FP16 shape whose
tail is canonical:

| run | shape (dtype) | GM-start alignment edge (row≥1 tile-0) | ReduceSum-count edge (tail) | role |
|---|---|---|---|---|
| A | 2×4100 FP32 | YES — row1 start 16400, mod32=16 | YES (valid=4) | **required by scope** |
| B | 2×4103 FP32 | YES — row1 start 16412, mod32=28 | YES (valid=7) | **required by scope** (also H5's min-8 probe) |
| C | 1×4100 FP32 | NO (row0 aligned) | YES (valid=4) | reduce-only control |
| D | 2×4104 FP16 | YES — row1 start 8208 bytes, mod32=16; tile0 bytes=8192 sizeAligned | NO — tail valid=8 (≥8, multiple of 8, canonical) | **alignment-only isolator** |
| E | 1×4104 FP16 | NO | NO (tail 8 canonical) | clean control — anything here is a harness/other fault |

Commands (unified ref harness, usage `support/runner_ref.inc:175-196`, argc 10; width range
64..32768 covers all shapes; dtype 0=fp32, 1=fp16):

```
atx_ref_parent_probe <dev> <rows> <D> <dtype> <prefix> 1 1 1 0
atx_ref_v001_probe   <dev> <rows> <D> <dtype> <prefix> 1 1 1 0
```

warmup=1, samples=1, blocks=1, gap=0 → one launch each; harness prints `bad`/`max_abs`,
exits 3 iff `bad>0` (`runner_ref.inc:310,340,396`). 10 runs total (5 shapes × 2 binaries),
seconds of device time. Minimal decisive subset if the window is tight: E and D on parent +
D on V001 (3 runs).

Decision tree (pre-registered):

1. **D(bad>0) AND E(bad=0)** → GM-start edge REAL on dav-2201 despite docs → escalate to Main
   as a correctness finding before any performance revision (see §3 risks).
2. **D(bad=0)** → edge not exercised on this device/driver; combined with §1 (no GM-32B
   clause), H6's premise is false → H6 → **REJECTED_AS_VIOLATION**; the predicate `&&`
   survives only as optional hardening, not as a repair justification.
3. **C(bad>0)** → ReduceSum-count edge REAL → routes to H5 hardening; A/B results are then
   confounded and must NOT be attributed to alignment — judge alignment only via D vs E.
4. A or B bad>0 while D bad=0 → failing mechanism is the reduce tail (row-wide rms
   corruption), not alignment → same routing as (3); H6 per tree (2).
5. E(bad>0) → harness/generator fault → fix measurement first; no H6 verdict.

### 3. Risk if parent AND V001 share the bug

- **Same predicate, shared lineage.** Parent `kernel.asc:109-112/133-136` and V001
  `108-114/157-163` are the same size-only test; every future ALIGN revision inherits it. A
  real edge sits on BOTH sides of every P/C pair on triggering shapes.
- **Qualification blindness.** All local qualification to date used small probes (2×100) that
  mathematically cannot reach the edge — P/C on them returns bad=0 while the bug (if real)
  lives untouched on D≥4096 unaligned-start shapes.
- **Official-set evidence cuts toward "not real."** Parent is historical phase3 online 15/15
  (17.64). If the hidden official set contains any D∈[4096,32768] with D%8≠0 (plausible
  given the advertised D range with non-32B allowed), the edge would already have failed
  there → parent passing is suggestive that dav-2201 does not enforce a GM-32B rule.
  The official shape list is unknown, so this is supporting evidence, not proof.
- **What a shared bug would cost.** runner_ref aborts with exit 3 on bad>0, so timing cannot
  be silently wrong; the concrete loss is that D≥4096 unaligned-start shapes become
  unmeasurable and unscoreable until the predicate question settles, plus a potential
  wrong-answer class failure on any official shape in that band. If tree (1) fires, Main
  should treat the fix as a correctness-first repair on the parent surface (README allows
  correctness-only repairs with no performance variable) and decide whether the parent used
  as P reference is repaired too — that touches every ALIGN P/C on record.
- **Scope of this cycle:** design + docs only. No device slot used (d4/d5/d6 leased, d7
  forbidden), no kernel edit, no new revision.

### 4. Fields refresh (full set, H6 updated)

- MECHANISM: unchanged — add `&& startAligned` to the two size-only predicates (V001
  `kernel.asc:111`/`160`; parent inline equivalents); an unaligned-start full tile falls to
  the pad path. Newly specified: the GM side is always dtype-aligned for fp32 and the UB side
  is buffer-base aligned, so today's call form is document-conforming.
- BOTTLENECK: unchanged — predicate completeness, not performance.
- EXPECTED_SHAPES: required 2×4100 / 2×4103 FP32; isolators 1×4100 FP32, 2×4104 FP16,
  1×4104 FP16 (table §2). Below D=4096 unreachable (unchanged).
- WHY_IT_MAY_HELP: two-tier now — (a) if tree (1) fires, closes a real corruption path
  correctness-first; (b) if tree (2), no repair; value reduced to optional hardening.
- WHY_IT_MAY_FAIL: doc check RESOLVED against the premise (no GM-32B clause; only
  dtype-byte + UB-32B + size-32B); parent 15/15 consistent with "edge absent"; probe likely
  confirms legality. Also: version gap 9.2.0 doc vs 8.5 toolchain (UNCONFIRMED online).
- ASCEND_FEASIBILITY: trivially high — pure predicate change; both call forms already present.
- UB/CORE/DMA_IMPACT: unchanged (worst case: one pad op per unaligned-start full tile).
- SYNC_IMPACT: none.
- PRECISION_RISK: none by construction; the battery measures corruption (`bad`/`max_abs`),
  not drift.
- ABI: unchanged — same two call forms, branch condition only.
- DUPLICATE_CHECK: unchanged (no active route owns the alignment predicate); shared-surface
  note stands — this covers the parent, Main should see it regardless of route scope.
- MINIMAL_OFAT_DIFF: unchanged (one `&&` operand per predicate); now conditioned on a
  positive probe D and Main's go.
- FALSIFICATION_TEST: superseded by the 5-shape × 2-binary battery + decision tree §2
  (replaces the earlier single-run draft); doc half of falsifier #3 resolved in §1.
- CLASSIFICATION: NEEDS_MORE_EVIDENCE (refined) — doc half RESOLVED-AGAINST-H6; probe half
  ready and decidable in one short correctness window; outcome paths: tree (2) →
  REJECTED_AS_VIOLATION (no repair), tree (1) → escalate as shared parent+V001 correctness
  finding. Do not report as a bug before the probe runs.

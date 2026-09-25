# ALIGN-TAIL-X — Next Hypotheses (TRACK-B research, append-only)

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
- CLASSIFICATION: NEEDS_MORE_EVIDENCE (cross-route scope vs BATCH-RESIDENT-X and SCHED-ROWGROUP-X must be confirmed by Main; UB budget recompute required).

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

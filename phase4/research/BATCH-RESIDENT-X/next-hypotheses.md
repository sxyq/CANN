# BATCH-RESIDENT-X — TRACK-B Next-Hypothesis Research (append-only)

> **MAIN-2 APPROVALS 2026-09-25** — Scope arbitration: parameter staging / gamma-bias residency / row-batch parameter reuse fixed to this route (UB-H2 marked DUPLICATE·DEFER_TO_BATCH). APPROVED NEXT backlog: H1 two-stage row-group compute + vectorized row-local reduction state + fused epilogue — MUST be shrunk to ONE conceptual mechanism at Revision declaration; if implementation would change reduction topology + epilogue + batch ownership together, Main splits it; no three-donor single implementation. Track-A: migrate legacy wall-clock harness to unified device-event protocol before any further measurement; legacy wall-clock data (incl. +185%) never promotion evidence.

ROUTE: BATCH-RESIDENT-X
TRACK-A: candidate SHA `ad961c5837971bf989c9989b822cc2719a6a285c60969d35e1530463b7c8721d` unchanged; no kernel edits, no new revision, no device run this turn.
TRACK-B: architecture-level residency / batching research. No `.asc`/`.cpp` edits, no formal next revision.
Owner context: MAIN-2, worktree `/Users/sunyiyang/Desktop/Project/cann-next6/BATCH-RESIDENT-X`, branch `exp/next6-batch-resident-x`.

---

## CURRENT_CANDIDATE

| field | value |
|---|---|
| ROUTE / REVISION | BATCH-RESIDENT-X / V001 |
| CONTEXT_CLASS | HISTORICAL_DERIVED |
| DIRECT_PARENT | A001-V017 (R014 parameter-residency checkpoint, official 15/15, score 36.41) |
| PARENT_SOURCE_SHA256 | `c906bc45c0bfa0a1fb35710abac637a7c784b265d2658a66daa1d739da4054c3` |
| CANDIDATE_SOURCE_SHA256 | `ad961c5837971bf989c9989b822cc2719a6a285c60969d35e1530463b7c8721d` |
| SINGLE_HYPOTHESIS | After parameter residency (R014), contiguous multi-row batch DMA ownership on the FastKernel path only |
| mechanism now in code | `kMaxBatchRows = 4`; `batchRows_` chosen by UB formula `(3B+1)·d32·elemSize + 20·d32 + 512 ≤ 160000`; `ProcessBatch` / `CopyInBatch` / `CopyOutBatch` issue one multi-row `DataCopy` (`nBursts = B`) per tensor; queues remain `kSingleBuffer`; `ProcessSingle` is byte-level parent path for B==1 tails; ResidentKernel + host `run_kernel` byte-identical to parent |
| correctness | PASS for hypothesis path: 42/48 probe cases PASS; the 6 failures are col-split 507035 that the parent fails identically (pre-existing, not introduced) |
| status | decision `NEEDS_ONE_MORE_LOCAL` + `MEASUREMENT_BLOCKED`; probes IDLE; source pinned |

Measurement record (all WALL_CLOCK harness — the BATCH runner is listed in `local-timing-protocol.md` as warmup 5 / 21 samples / wall-clock, not yet converged to the unified device-event method):

| set | load class | median delta | direction |
|---|---|---|---|
| set1 | LOAD_CONTAMINATED | +6.4% | INCONSISTENT 2f/2s |
| set2 | DEV4_FREE_WINDOW_VLLM_RESIDENT | −23.9% | INCONSISTENT 3f/1s |
| set3 | DEV4_EXCLUSIVE_VLLM_RESIDENT | −20.3% | INCONSISTENT 3f/1s; pair3 +185.5% |
| round2 | parent-only reference across d4/d5/d6 | candidate skipped (parent CV 0.19–0.38, range/med 0.54–1.45) | NOT_MEASURED |

---

## CURRENT_BLOCKER

1. Measurement authority, not architecture: window qualification 2/2 UNQUALIFIED (d6 CV 0.257 / max-min 2.04; d4 CV 0.325 / max-min 2.82), so candidate pairs were never run under the current qualification rule. Round-2 parent-only reference blocks on all three devices (d4/d5/d6) were unstable.
2. The set3 pair3 +185.5% (FP16 64×1024) is classified OUTLIER / LOAD-RELATED: the same shape's parent swung 110.5→65.8 µs between set2 and set3 (−40%), and the candidate swung 103.9→188.0 µs between sets on identical source. Not trusted, not re-chased, not stripped from records.
3. Harness mismatch: BATCH still uses legacy wall-clock samples; `local-timing-protocol.md` requires device-event primary, warmup ≥10, in-process samples ≥11/21, interleaved pairs, and a per-route same-binary noise floor with the Direct Parent on the exact probe shape before any P/C.
4. Consequence for TRACK-B: no trustworthy local signal exists for V001, so no architecture conclusion may be drawn from the current deltas (direction INCONSISTENT, LOCAL_CONFIDENCE LOW across all sets). Research continues read-only; no device run this turn.

---

## BOTTLENECK_MODEL

Question: at the probe shapes, what actually costs time — GM traffic, instruction issue, or synchronization?

Effective bandwidth from set3 parent wall-clock samples (GM traffic = rows × D × elem × 3 tensors):

| shape | parent µs | GM bytes | effective GB/s |
|---|---:|---:|---:|
| FP32 100×256 | 126.2 | 307 KB | 2.4 |
| FP32 80×512 | 189.9 | 492 KB | 2.6 |
| FP16 64×1024 | 65.8 (set3) / 110.5 (set2) | 393 KB | 6.0 / 3.6 |
| FP32 128×128 | 87.9 | 197 KB | 2.2 |

Device HBM peak is order 10^12 B/s, so every probe shape runs 250–700× below bandwidth capability. **The probed regime is not GM-traffic bound.** The sample also contains host launch + `aclrtSynchronizeStream` (wall-clock method).

Cost inventory for the parent single-row path (per row): ~8 D-sized vector ops (ToFloat×2, Add, Mul, ReduceSum, Muls, Mul, Add, FromFloat) + ~6 short/scalar ops (Duplicate 8, Muls 8, Adds 8, Sqrt 8, GetValue scalar pull, reciprocal) + 12 single-buffer queue events (Alloc/EnQue/DeQue/Free on x, res, out) + 3 DataCopy calls. For 100 rows that is order 10^3 serial op/sync events inside a 126 µs sample — i.e. issue/sync bound, with per-op fixed cost dominating because D-sized ops are short (D=256 → 1 KB → 32 blocks).

What V001's mechanism actually attacks, and its ceiling:

- Multi-row batch collapses queue events from 12/row to 12/B rows and DataCopy calls from 3/row to 3/B rows. At B=4 that is a 4× cut of both terms. This matches the observed large wins at shapes where B=4 engages (pair2 −34%/−47%, pair4 −67%) and explains why a "DMA transaction" mechanism wins despite bandwidth being idle: the win is **synchronization and call-count amortization**, not HBM efficiency.
- Therefore the remaining headroom is NOT more DMA amortization. It is (a) per-row instruction-launch and V→scalar `GetValue` cost still paid inside the batch loop, (b) redundant per-core parameter residency (gamma+bias loaded and cast by every core) when rows/core is small, (c) MTE/V/MTE3 overlap (second-order at these sizes, first-order only on wide/hidden shapes), (d) measurement noise, which currently exceeds every candidate effect.
- Where the batch is dead: FP32 D=4096 → `(3·1+1)·16384 + 20·4096 + 512 = 147,968 ≤ 160000` holds for B=1 but B=2 needs 197,120 > 160,000, so **B=1 at D≈3072–4096 FP32** (V001 degenerates to parent there). Rows/core at the `rows·D ≤ 262144` ceiling is 2–3 anyway.
- Batch/UB frontier (why rowFactor and parameter residency trade against each other): the 20·d32 term is 5 D-wide float tiles including gammaF+biasF (8·D bytes FP32). Removing them from residency or segmenting D shifts B up by roughly one step per 1024 elements of D.

Hidden judge shapes are unknown (`architecture-evidence-map.md`); the model must hold for both small probe shapes and possible wider/longer shapes.

---

## DONOR_REGISTER (candidate donors inspected this turn)

### DONOR-1
- SOURCE_ROUTE: R014 (GammaBias residency, `independent__full-r014-parameter-residency-i001/files/kernel.asc`)
- MECHANISM: `ResidencyTilingData` + stripe loop — `for (parameterStart…; parameterStart += parameterLength) { LoadParameters(stripe); for (row…) { ComputeInverseRms(row); EmitRow(row, paramStart, paramLength, invRms); } }` — gamma/bias loaded once per stripe and reused across every row of that stripe.
- OLD_CONTEXT: phase3 R014 was EXPLORING with no champion; `local_reference = H001/V002 partial path; MIX combination not yet through accuracy checks`.
- CURRENT_CONTEXT: distilled into parent A001-V017 `LoadFloatParamsOnce` (once per core, `paramQueue_` V008 sync pattern) — already inside the pinned candidate, unchanged by V001.
- WHY_ORTHOGONAL: tests residency *lifetime scope* (stripe vs whole-vector vs per-core-once) rather than row batching, which is V001's mechanism.
- WHY_NOT_DUPLICATE: V001 keeps whole-D residency and only adds multi-row batch DMA; no revision in this route has restructured the parameter residency loop.

### DONOR-2
- SOURCE_ROUTE: R015 (multi-row combined DMA, `independent__full-r015-multi-row-dma-i001/.../op_kernel/kernel.txt`)
- MECHANISM: block owns `MULTI_ROW_TILE = 8` contiguous rows; `DataCopyPad` with `nBursts = rowsThisBlock_`, `blockLen = d·sizeof(T)`, pad for non-32B D; all rows copied in, computed in one loop, copied out once.
- OLD_CONTEXT: phase3 R015 EXPLORING, `local_reference = H001/V002 candidate path; safety and official score unverified`; idea-pool marks "true multi-row stride DataCopy" still unexplored as a dedicated redesign (owner listed MID-X).
- CURRENT_CONTEXT: V001 implements the post-residency combination with `kMaxBatchRows = 4` and a UB-derived B selector; batch in/out via `DataCopy` nBursts (32B-aligned only, pad path not batched).
- WHY_ORTHOGONAL: R015 is the DMA/ownership side of this route's charter; it is the *base* of V001, not a new hypothesis — it is registered so the next hypotheses can be screened against it.
- WHY_NOT_DUPLICATE: R015 alone had no parameter residency; R014 alone had no batch DMA; the combination under the champion path is exactly V001, already pinned and awaiting measurement.

### DONOR-3
- SOURCE_ROUTE: R016 (per-row core allocation / scheduling, `exp__full-r016-scheduling-arch-i001/.../kernel.txt`)
- MECHANISM: **cyclic row scheduling** — `for (row = blockIdx; row < outer; row += blockNum)` with comment "keeps the tail distribution independent of outer % blockNum"; rows processed in `kTileElements = 256` column tiles; `reduceTmp` 32 KB.
- OLD_CONTEXT: phase3 R016 EXPLORING, `local_reference = H001/V003 row-group static verification; H001/V005 multi-rank real NPU run`; idea-pool row+col hybrid listed as unexplored.
- CURRENT_CONTEXT: parent/candidate FastKernel uses contiguous ownership (`firstRow = block·base + …`, `rowsThisCore = base + (block < extra)`), which is a *prerequisite* for contiguous multi-row batch DMA; SCHED-ROWGROUP-X (sibling route) owns "32B-safe row-group ownership" from this donor.
- WHY_ORTHOGONAL: ownership geometry (contiguous vs cyclic vs group-cyclic) is a different design axis than batch size or residency lifetime, and it interacts with batch adjacency (cyclic destroys burst contiguity).
- WHY_NOT_DUPLICATE: V001 changed only how rows inside one core's contiguous range are copied; the assignment of rows to cores is byte-identical to parent. SCHED-ROWGROUP-X's scope is kernel-side 32B row-group ownership, not host blockFactor or group-cyclic assignment (per scheduler description only; that route's source is not read).

### DONOR-4
- SOURCE_ROUTE: R029 (official five-mode tiling used as runtime dispatch taxonomy, `full-r029-i001/.../kernel.txt`)
- MECHANISM: `enum RuntimeMode { SPLIT_D, SINGLE_N, MERGE_N, MULTI_N, NORMAL }`; host `ChooseMode(d, outer, cores)` thresholds (`d ≥ 8192 && outer ≤ cores → SPLIT_D`; `outer ≤ cores → SINGLE_N`; `outer ≤ cores·4 → MERGE_N`; `outer ≥ cores·32 → MULTI_N`); `rowsPerCore = ceil(outer/blockNum)`; per-row D tiled by `ChooseTile` bounded by `kUbBudgetBytes = 96 KB`, `kMaxTileElements = 2048`.
- OLD_CONTEXT: phase3 R029 status PLANNED, `local_reference = 官方分档与 UB 规划框架`; idea-pool still lists the five-mode taxonomy as unexplored as a dispatch taxonomy (owners MIX-A / R31B).
- CURRENT_CONTEXT: parent already has two-path dispatch (`useFast` vs ResidentKernel) but with fixed constants, not a mode taxonomy; this route's charter is residency/batching, not mode dispatch.
- WHY_ORTHOGONAL: mode *selection* is host dispatch policy; this route studies what happens inside one path after residency.
- WHY_NOT_DUPLICATE: no hypothesis here adds a new mode or changes `useFast`; R029 is used only for its rowFactor/ubFactor framing and threshold precedents.

### SUPPLEMENTARY INSPECTED (duplicate screening only)
- `fast__lane-d/.../FULL-R030-WIDE-PARAM-REUSE` and `FULL-R029-L002..L005-WIDE-CACHED-ROW`: historical wide param-reuse / cached-row series carrying an `R014-V002 parameter cache` block. Consequence: plain "cache gamma/bias on wide rows" is already historical — any param hypothesis in this route must be about *lifetime scope or stripe structure*, not about whether to cache at all.
- `phase4/control/architecture-evidence-map.md`: row batching = PROVEN_WIN; parameter residency = PROVEN_WIN; pipeline overlap = MIXED (A001 V017 x/res 2-slot small win; R31 MTE depth flat on T14); launch topology / host dispatch = NOT_PROPERLY_TESTED; epilogue fusion = NOT_PROPERLY_TESTED (EPI-X correctness FAIL before score).
- `phase4/control/idea-pool-29-routes.md`: still-unexplored entries relevant here are R014 "stripe-resident params for D>UB", R015 "true multi-row stride DMA", R016 "row+col hybrid", R029 five-mode taxonomy.

---

## EXTERNAL_IDEA_REGISTER

### EXT-IDEA-1
- EXTERNAL_IDEA: Official CANN `add_rms_norm` rowFactor/ubFactor tiling — in-kernel row batching derived from a UB budget equation instead of fixed constants.
- SOURCE: local archived research `归档/phase3-before-reset-20260920/调研/调研1/Agent03-ascend-opensource.md`, `调研4/agents/agent03-official-repos.md`, `调研5/agents/agent-03-官方开源仓库.md`, `调研6/agents/agent03-official-repos.md` (citing ops-nn / ops-transformer `op_host` tiling sources).
- MECHANISM: `MERGE_N` (numColAlign ≤ 2000): `rowFactor = ubSize/(numColAlign·weight + 260)`, `ubFactor = rowFactor·numColAlign`, multi-row joint reduction `ReduceSumMultiN` (requires D < 2040), gamma broadcast across the row group, rstd broadcast per row, double-buffer queues. `MULTI_N` (FP16 aligned): `rowFactor = (ubSize − 1024 − 256 − numColAlign·2)/(numColAlign·16 + 64)`, per-row reduce + rstd gather-expansion. `NORMAL`: `rowFactor = 64`, `ubFactor = 12288`. All row batches are `i_o_max = ceil(rowWork/rowFactor)` with an explicit `row_tail`; D tiled by `j_max = ceil(numCol/ubFactor)` with `col_tail`.
- WHY_DIFFERENT_FROM_EXISTING_ROUTES: official code computes factors on host from platform UB size and passes them in a tiling struct; this task has no tiling struct, and EXT-ASCEND-X (PARKED, LOCAL_REJECTED on correctness 27/27 in a fresh implementation) already tried porting the whole triple-factor selection. None of the six active routes applies the MERGE_N *joint-reduction + broadcast epilogue* shape inside a correctness-proven batch path.
- EXPECTED_BOTTLENECK: instruction-launch and scalar-handoff count per row at small D; multi-row joint ops replace per-row epilogues.
- APPLICABLE_ROUTE: BATCH-RESIDENT-X (HYPOTHESIS-1 mechanism idea only; no code copied).
- PROVENANCE_CLASS: EXTERNAL_OFFICIAL_SOURCE_ARCHIVED_LOCAL (grade A citations in the archived research; architecture/tiling/buffering idea only).

### EXT-IDEA-2
- EXTERNAL_IDEA: vLLM-Ascend public documentation as a source of fused-normalization batching architecture.
- SOURCE: `https://vllm-ascend.readthedocs.io/en/latest/` (reachable this turn); `https://github.com/vllm-project/vllm-ascend` transport error.
- MECHANISM: none extracted — the documentation entry point publishes deployment/quick-start material, no rowFactor/tiling/buffering detail was reachable without guessing paths. Recorded as a negative result so nobody re-spends a turn on it.
- WHY_DIFFERENT_FROM_EXISTING_ROUTES: n/a (no usable idea).
- EXPECTED_BOTTLENECK: n/a.
- APPLICABLE_ROUTE: n/a.
- PROVENANCE_CLASS: EXTERNAL_DOC_REACHED_NO_MECHANISM.

---

## HYPOTHESIS-1

**Title:** Two-phase row-group compute with vectorized row-local reduction state (small-D fused epilogue)

- MECHANISM: Keep V001's contiguous multi-row batch ownership and its `CopyInBatch`/`CopyOutBatch` exactly as-is, but restructure `ProcessBatch`'s inner loop from "fully finish row b" to two phases over the same B-row block: (A) reduction phase — for each b: `ToFloat(u)`, `ToFloat(work)`, `Add`, `Mul`, `ReduceSum` into slot b of a B-slot sum vector, plus one pre-zero of the B×8 region; (B) batched rstd phase — 4 vector ops over B elements (`Muls invD`, `Adds eps`, `Sqrt`, element-wise reciprocal/Div against a ones vector) producing a B-element rstd vector, then expand it into a B×D scale buffer (B × `Duplicate` of D elements); (C) one fused epilogue over the whole B×D block — `Mul(u, scale)`, `Mul(·, gammaF)`, `Add(·, biasF)`, `FromFloat` as four B×D ops instead of 4·B D-sized ops. Per-row `GetValue(0)` scalar pulls, per-row `Duplicate/Muls/Adds/Sqrt` short ops, and per-row epilogue launches disappear from the hot loop.
- BOTTLENECK: per-row instruction issue and V→scalar `GetValue` stalls plus short-op fixed cost at small D (see BOTTLENECK_MODEL: bandwidth idle, order 10³ serial events per sample). This hypothesis attacks the V-side cost; it does not touch GM traffic.
- EXPECTED_SHAPES: FastKernel-eligible shapes with rowsThisCore ≥ 2 and small D — strongest at D ≤ 1024 (probe shapes 128×128, 100×256, 80×512, 64×1024 FP16), where a D-sized op is only 0.5–4 KB; neutral-to-negative at D ≥ 3072 where ops are long and the B×D scale buffer eats the UB budget (formula effect below pushes B down one step at large D).
- WHY_IT_MAY_HELP: (1) removes B `GetValue` V→scalar stalls per batch; (2) short-op count per batch drops from ~4·B + 4·B to ~4·B + 8; (3) epilogue/cast launches drop from 4·B to 4; (4) gamma/bias apply naturally as element-wise ops over B×D with no per-row reload (gamma/bias reuse across the batch becomes one op instead of B); (5) synergy: it reuses V001's already-paid batch DMA and queue amortization rather than competing with it.
- WHY_IT_MAY_FAIL: (1) the B reductions themselves cannot be batched — three D-sized ops per row remain unavoidable; (2) the B×D scale expansion adds B·D·4 bytes of UB and B·D write+read traffic, so at large D the launch savings are smaller than the byte cost (hence the small-D shape expectation); (3) `ReduceSum<float, true>` microcode or TQue waits may be the real serial cost, in which case launch collapsing does nothing; (4) the reciprocal-over-vector step needs an element-wise Div or an rsqrt-style primitive — if neither is available on this toolkit, the rstd phase falls back to per-row `Sqrt`+`GetValue` (savings shrink but do not vanish); (5) measurement noise currently swamps every effect (CURRENT_BLOCKER), so a real win may stay invisible until the harness converges to device-event method.
- ASCEND_FEASIBILITY: high. Only standard primitives: `DataCopy` (unchanged), `ToFloat/FromFloat`, `Add/Mul/Muls/Adds/Sqrt/ReduceSum/Duplicate`, plus one element-wise Div over B elements (fallback: keep per-row `Sqrt`+`GetValue` for rstd only). No cross-core sync, no new pipe, no adv-api dependency (official `Brcb`/`BroadCast` avoided; plain `Duplicate` expansion used instead). New UB item folds into the existing B selector formula.
- UB/CORE/DMA_IMPACT: UB + `B·D·4` bytes for the scale buffer → batch formula becomes `(3B+1)·d32·elemSize + (20 + 4B)·d32 + 512 ≤ 160000`; at FP32 D=128 B stays 4 (≈10.2 KB total), at FP32 D=2048 B drops 4→3, at D=4096 B stays 1 (unchanged). CORE: none (blockDim and ownership untouched). DMA: unchanged — same 3 batch bursts per B rows.
- SYNC_IMPACT: strictly fewer — B `GetValue` scalar pulls per batch removed, queue EnQue/DeQue pattern unchanged from V001 (once per batch), no new `PipeBarrier<PIPE_ALL>` beyond what the ops already imply.
- PRECISION_RISK: low. Per-row sum uses the identical `ReduceSum` sequence; rstd is computed from the same inputs with the same FP32 ops (B-wide instead of 8-wide short vectors, element-wise identical); `Duplicate` expansion is exact; epilogue order stays normalize → ×gamma → +bias with `CAST_ROUND`. Expected bitwise-identical output; must still be confirmed against the golden matrix on all dtypes × D buckets before any timing (a precision regression would be a correctness finding, not a performance one).
- DUPLICATE_CHECK: vs own V001 — not a duplicate: V001 amortizes MTE-side queue/DMA events across rows, this reorganizes V-side compute phases inside the same batch. vs EXT-ASCEND-X (dynamic rowFactor/ubFactor/blockFactor, PARKED, correctness FAIL) — not a duplicate: B stays fixed and derived as today; only the compute passes inside one batch change, on a correctness-proven parent rather than a fresh implementation. vs UB-LIVENESS-X (UB lifetime/alias architecture, V003 judge-ready) — PARTIAL ADJACENCY on UB layout, distinct primary mechanism (compute-pass structure and reduction-state vectorization vs buffer lifetime/aliasing); that route's source is not read, flagged for Main. vs official MERGE_N — mechanism idea only, no code copied; the fresh-implementation context class differs. vs SCHED / ALIGN / REDUCE / ASYNC — different axes (ownership geometry / copy alignment / reduction redesign / pipeline overlap).
- MINIMAL_OFAT_DIFF: rewrite `ProcessBatch` internals only (same signature, same `batchRows_`, same `CopyInBatch`/`CopyOutBatch`, same `ProcessSingle` fallback, one added TBuf for the scale buffer, one added term in the B selector formula). `ResidentKernel`, `LoadFloatParamsOnce`, and host `run_kernel` byte-identical to V001. One conceptual change: *how a batch is computed*, not how rows are owned or copied.
- EXPECTED_LOCAL_PROBES: (1) same-binary noise floor first, Parent A001-V017 + candidate, exact probe shapes, unified device-event protocol (warmup ≥10, ≥11/21 in-process samples, interleaved pairs); (2) primary pairs: FP32 128×128 and FP32 100×256 (smallest D, largest launch fraction); FP32 80×512; FP16 64×1024 only after its block-level MAD/median passes (that shape has the outlier history); (3) expectation if the model is right: candidate advantage grows as D shrinks and B stays 4, and collapses toward 0 as D → 4096; (4) correctness matrix on all dtypes × D buckets before any timing; (5) if deltas stay inside the same-binary noise floor → LOCAL_REJECTED for this mechanism, not for batching overall.
- CLASSIFICATION: **READY_FOR_MAIN_REVIEW**

---

## HYPOTHESIS-2

**Title:** Parameter-stripe residency with D-segmented batch — restore rowFactor where the UB budget kills B (residency/batch UB arbitrage)

- MECHANISM: Two coupled moves inside FastKernel only, following R014's original stripe loop structure: (1) segment each row's D into tiles of width T (`ubFactor`) instead of requiring full-D residency of gamma/bias (gammaF/biasF become T-wide stripes loaded once per segment and reused across all B rows of the segment); (2) run the batch over segments — for each segment: `CopyInBatch` of the B×T block, reduction accumulation across segments per row (row-local reduction state = one running sum slot per row), apply-epilogue for that segment, `CopyOutBatch`. The B selector then prices `B×T` instead of `B×D`, which is the official rowFactor/ubFactor split applied inside this route's proven path.
- BOTTLENECK: batch-vs-UB deadlock at mid D. Quantified: FP32 D=4096 → B=1 today (B=2 needs 197,120 > 160,000) so V001 is byte-identical to parent there; gammaF+biasF alone occupy 8·D = 32 KB of the 160 KB budget, i.e. parameter residency is competing directly with rowFactor for UB. At mid D the V001 mechanism is present in code but inert.
- EXPECTED_SHAPES: FP32 D ∈ [2048, 4096] with rows ≤ 262144/D (e.g. 64×4096, 48×4096, 128×2048) where B is UB-capped 1–2; weaker at probe shapes (D ≤ 1024) where B already reaches the kMaxBatchRows=4 cap and segmentation only adds a loop; no effect outside `useFast` (thresholds untouched in the minimal diff).
- WHY_IT_MAY_HELP: (1) re-engages batch DMA + queue amortization at D where it is currently dead; (2) parameter stripe residency frees 8·D−2·T·4 bytes of UB which the B selector converts directly into a larger rowFactor — the residency↔batching trade becomes explicit and tunable; (3) segment ops are T-wide with T ≈ 2048, better vector efficiency than 4096-element ops are not required, and short-tail handling uses the existing 32B alignment rule; (4) row-local reduction state (one running sum per row across segments) is a new capability, not a re-tune.
- WHY_IT_MAY_FAIL: (1) rows/core at D=4096 under `rows·D ≤ 262144` and 24 cores is only 2–3, so even with B=2 engaged the batch is at most one full group — the absolute win is bounded; (2) segmenting re-reads nothing but adds loop/branch overhead per segment; for a 2-segment row that is +1 loop +2 batch round-trips vs today's 1 full-residency pass; (3) ResidentKernel already handles mid D with double-buffered x/res and a proven 36.41 parent score — the hypothesis implicitly claims Fast's simpler per-row path beats Resident's tiled path at mid D, and the evidence map has never isolated Fast-vs-Resident on the same shape; (4) loosening any of the fixed thresholds (`d ≤ 4096`, `rows·d ≤ 262144`) is out of scope and dangerous — V012 proved 262144 safe while V013's looser 2M selection let an unknown shape in and it runtime-errored; the minimal diff must not touch them; (5) EXT-ASCEND-X already explored UB-adaptive work sizing (fresh, correctness FAIL) — adjacency must be screened by Main.
- ASCEND_FEASIBILITY: medium. Segment loop, stripe `LoadParameters`-style `CopyIn` of T elements, and running-sum accumulation are all plain Ascend C; the risk is bookkeeping (partial D tails must use the existing DataCopyPad path, `nBursts` batch requires 32B-aligned T and D), and the apply phase must read x/res segments without re-reading rows already consumed (segment-major ordering with B rows resident per segment handles this).
- UB/CORE/DMA_IMPACT: UB restructured — fixed tiles scale with T, not D; B selector uses `(3B+1)·T·elemSize + (20+4? )·T …` (exact formula to be derived in a design draft; T chosen so one full batch + 5 float tiles fit 160 KB). CORE: unchanged (contiguous ownership kept). DMA: more, not fewer, bursts (one batch per segment × segments per row) — this hypothesis spends DMA calls to buy UB headroom for batching; net effect on call count vs parent must be computed per shape before implementation.
- SYNC_IMPACT: one extra queue round-trip per segment; `GetValue` still per row per reduction completion (once per row, not once per segment, because rstd is finalized after the last segment). Net vs V001 at B=1 shapes: strictly fewer syncs (batch engages); at B=4 probe shapes: slightly more (segment loop).
- PRECISION_RISK: medium-low. Sum accumulated across segments in FP32 changes summation grouping relative to single-pass `ReduceSum` over full D — associativity differs, so results may differ in the last ulp for FP32/BF16 paths (FP16 inputs are upcast to FP32 first, so the risk is grouping only). Must pass the full golden matrix with the route's precision tolerances; if a dtype bucket regresses, the segment path stays off for that bucket.
- DUPLICATE_CHECK: vs V001 — extends rather than duplicates (V001 = ownership batching at full-D residency; this = residency scope change that enables batching at mid D). vs historical R014 — R014 striped params but had no batch; vs historical R030 wide param-reuse series — those cache whole-D params on wide rows, no stripe/segment loop, no batch. vs EXT-ASCEND-X — PARTIAL ADJACENCY (their UB-adaptive work sizing); different route, different parent, fresh vs historical-derived, and their failure was correctness in a from-scratch port. vs WIDE-X idea-pool item "stripe-resident params for D>UB" — that item's listed owner was a round-1 route; flagged for Main so only one route carries it.
- MINIMAL_OFAT_DIFF: FastKernel only: add segment loop + stripe param load + running row-sum state; `batchRows_` formula re-priced on T; `ProcessSingle` fallback and all thresholds (`d ≤ 4096`, `rows·d ≤ 262144`, `fastUb ≤ 160000`) unchanged; ResidentKernel and host untouched. One conceptual change: *residency lifetime + batch granularity move together from D to T*.
- EXPECTED_LOCAL_PROBES: (1) first a read-only discrimination probe is required before any code: same-binary Parent timing on mid-D shapes (FP32 64×4096, 48×4096) to establish whether ResidentKernel is actually slower than Fast on shapes just under/over the D=4096 selection line (if Fast and Resident cost the same, this hypothesis has no target and should be dropped); (2) if a gap exists: correctness matrix first, then device-event paired probes on those two shapes plus the four probe shapes as regression; (3) decision rule: mid-D pair must clear the same-binary noise floor in ≥4 interleaved pairs with consistent direction before any online consideration.
- CLASSIFICATION: **NEEDS_MORE_EVIDENCE**

---

## HYPOTHESIS-3

**Title:** Batch-aligned blockFactor — host-side residency scope so gamma/bias is loaded by fewer cores and every core owns whole batches

- MECHANISM: Host-only change in `run_kernel`'s FastKernel branch: replace `blockCount = min(cores, rows)` with a batch-aware blockFactor — pick the largest blockCount ≤ min(cores, rows) that (a) makes each core's contiguous range a whole number of B-row groups (`rows % blockCount == 0` and range divisible by B where possible) and (b) gives each core at least a minimum row count when D is large (cap blockCount at `rows / minRowsPerCore` with minRowsPerCore ≈ 2·B for D ≥ 2048). The kernel already receives `blockDim` as a parameter and recomputes `firstRow_ / rowsThisCore_ / batchRows_` from it, so the kernel source stays byte-identical; only the launch dimension and its derived ownership change.
- BOTTLENECK: two per-core costs that scale with blockCount, not with rows — (1) redundant parameter residency: `LoadFloatParamsOnce` runs on every core, so gamma+bias GM traffic = `blockCount · 2 · D · elem`; (2) per-core fixed setup (TPipe `InitBuffer` ×12, param queue round-trip + two casts) amortized over only 1–3 rows when `rows ≈ cores`. Quantified: FP32 32×4096 → 24 cores × 32 KB param reads = 768 KB vs 1.57 MB of row traffic (param = 34%); FP32 64×2048 → 384 KB vs 1.57 MB (24%). At probe shapes the share is small (FP32 100×256: 48 KB vs 307 KB ≈ 15%), so this hypothesis targets mid-D/mid-rows shapes, not the current probe set.
- EXPECTED_SHAPES: FastKernel-eligible with D ∈ [1024, 4096] and rows ∈ [blockCount, 4·blockCount] (e.g. FP32 32×4096, 64×2048, 48×4096); also any shape where `rows % blockDim != 0` or `rowsThisCore % B != 0` (batch tails: at 100 rows / 24 cores / B=4, ~20% of rows fall in 1-row ProcessSingle tails that pay 12 queue events instead of 12/B). Weak/neutral when rows >> cores (blockCount stays cores) and at D ≤ 512 (param traffic negligible, added serial rows/core likely cost more than saved setup).
- WHY_IT_MAY_HELP: (1) param GM traffic and its two queue round-trips + casts are paid `blockCount` times — halving blockCount halves them; (2) whole-batch ownership removes the ProcessSingle tail inside cores, so V001's amortization applies to 100% of rows instead of ~80%; (3) fewer cores × more rows raises gamma/bias reuse depth per load (the residency route's own theme, at the multi-core scope instead of intra-core); (4) launch topology / block↔core binding is `NOT_PROPERLY_TESTED` in the evidence map — an untested axis.
- WHY_IT_MAY_FAIL: (1) the trade has unknown sign: parallelism drops proportionally (24 cores × 2.6 rows → 8 cores × 8 rows) and if per-row V-issue dominates rather than per-core fixed cost, fewer cores is strictly slower; (2) probe shapes have small param share and small setup share, so local probes may show nothing; (3) `blockDim` must stay consistent with the launch dim and with any sync scope (architecture-principles #7) — Fast path has no `SyncAll`, so the rule is satisfiable, but any mistake here is a correctness class failure; (4) hidden judge shapes may be rows >> cores, where the rule changes nothing (neutral, wasted revision).
- ASCEND_FEASIBILITY: high — this is host arithmetic plus an already-parameterized kernel; no new primitive, no UB change (B selector adapts to rowsThisCore automatically).
- UB/CORE/DMA_IMPACT: UB unchanged. CORE: launch topology changes (this is the point) — blockDim must equal the actual ownership divisor, verified by the correctness matrix across rows/cores combos; no core may run with `GetBlockIdx() ≥ blockDim`. DMA: total param bursts drop; row bursts unchanged per row (batch count per core rises, tail bursts vanish).
- SYNC_IMPACT: fewer param `EnQue/DeQue` pairs in total (one set per core); row-path pattern unchanged; no new cross-core sync (Fast path has none).
- PRECISION_RISK: none in principle — each row runs the identical per-row math in whichever core owns it; output is row-deterministic. Still verify with the golden matrix (ownership mistakes show up as whole-row corruption, an easy-to-spot correctness class).
- DUPLICATE_CHECK: HIGH-SCREEN REQUIRED. (a) EXT-ASCEND-X's stated hypothesis explicitly included "select blockFactor, rowFactor, and ubFactor dynamically" — it was PARKED after correctness FAIL in a fresh implementation, so the *architecture* is not disproven, but the *idea territory* overlaps; differentiator: single-factor change on a correctness-proven historical parent, aimed at param-residency amortization rather than full tiling-formula porting. (b) SCHED-ROWGROUP-X owns "shape-aware schedule + 32B row-group ownership" (R016-derived schedule) — their scope per scheduler description is kernel-side row-group geometry; this is host-side launch dimension. Partial adjacency, flagged for Main to arbitrate scope before either route implements. (c) vs own V001 — not a duplicate: V001's diff does not touch `blockCount`.
- MINIMAL_OFAT_DIFF: host `run_kernel` FastKernel branch only — one derived value (`blockCount`) plus the reasons table above; kernel `.asc`, ResidentKernel, param residency, batch selector all byte-identical to V001. One conceptual change: *how many cores own the rows*.
- EXPECTED_LOCAL_PROBES: (1) before any code — a read-only shape audit computing param-traffic share and rows/core for the four probe shapes plus mid-D candidates, to confirm the hypothesis has any target inside the current selection rules; (2) if viable: correctness matrix emphasizing rows not divisible by cores; (3) device-event paired probes on FP32 64×2048 and FP32 32×4096 (largest param share) with 100×256 as a control (expected ~flat); (4) negative control: a rows >> cores shape must show ~0 delta — if it moves, the measurement is contaminated, not the hypothesis confirmed.
- CLASSIFICATION: **NEEDS_MORE_EVIDENCE** (duplicate screening with EXT-ASCEND-X / SCHED-ROWGROUP-X scope required first)

---

## OPTIONAL-HYPOTHESIS-4

**Title:** Depth-2 pipelined batch — overlap copy-in(n+1) / compute(n) / copy-out(n−1) inside the batch loop

- MECHANISM: Keep V001's batch ownership; change `xQueue_ / resQueue_ / outQueue_` from `kSingleBuffer` to `kDoubleBuffer` and iterate batches as a pipeline: issue `CopyInBatch` for the next batch while computing the current one, and `CopyOutBatch` for the previous one. This is R013 double-buffer applied specifically to the batch path (parent ResidentKernel already carries the same 2-slot x/res pattern from A001-V017, where it was a small win).
- BOTTLENECK: in the current batch loop MTE2 → V → MTE3 are strictly serial per batch (single-buffer queues force a wait at each `EnQue/DeQue`). At probe shapes MTE time is small (bandwidth idle per BOTTLENECK_MODEL), so the recoverable span is small; at hidden wide/long shapes with more rows per core the serial MTE windows grow linearly.
- EXPECTED_SHAPES: rowsThisCore ≥ 2·B (enough batches to pipeline) and larger total bytes per batch — mid/wide shapes, D ≥ 1024; weak at probe shapes (rows/core 2–6 → at most one pipeline stage pair).
- WHY_IT_MAY_HELP: converts idle MTE windows into overlapped time; mechanism proven at small scale inside the parent (A001 V017 x/res 2-slot); straightforward extension of an existing pattern.
- WHY_IT_MAY_FAIL: (1) at probe shapes there is almost nothing to overlap — predicted deltas inside the noise floor; (2) UB cost: depth-2 doubles the batch slot cost, `(3B+1)` becomes `(6B+1)`-ish, so B drops (at FP32 D=2048 B falls 4→2, possibly negating the overlap win); (3) with B already ≥ 2 the batch loop is short and pipeline fill/drain eats most stages; (4) at these sample sizes wall-clock host overhead further dilutes any kernel-side gain.
- ASCEND_FEASIBILITY: high (standard `TQue` depth-2); UB impact is the binding constraint, not feasibility.
- UB/CORE/DMA_IMPACT: UB roughly doubles the x/res/out batch slots → B selector re-priced, B shrinks; CORE unchanged; DMA call count unchanged (overlap, not reduction).
- SYNC_IMPACT: more in-flight work by design; requires care that `FreeTensor` ordering does not race the next batch's `AllocTensor` (standard pattern, but a correctness-class risk if wrong).
- PRECISION_RISK: none (same math, same order within each row).
- DUPLICATE_CHECK: PARTIAL ADJACENCY — ASYNC-TRIPLE-X's entire charter is MTE2/V/MTE3 overlap on an R013-derived parent. Different route, different kernel path (their R013 lineage vs this route's Fast batch path), but same mechanism family. Recommend Main keep overlap assigned to ASYNC-TRIPLE-X and not run both unless scopes are explicitly split by kernel path.
- MINIMAL_OFAT_DIFF: queue depth constant + batch loop rotation only; ResidentKernel and host untouched.
- EXPECTED_LOCAL_PROBES: only if promoted — device-event pairs on D ≥ 2048 shapes with rowsThisCore ≥ 8; probe shapes serve as expected-neutral controls.
- CLASSIFICATION: **NEEDS_MORE_EVIDENCE** (do not implement while ASYNC-TRIPLE-X owns overlap)

---

## OPTIONAL-HYPOTHESIS-5

**Title:** Contiguous vs cyclic vs group-cyclic row ownership — ownership geometry as a batch-adjacency variable

- MECHANISM: three ownership geometries over the same rows, all evaluated against the same batch path: (a) current contiguous `base + extra` assignment; (b) R016 pure cyclic `row = idx; row += blockCount` (each core's rows strided — incompatible with multi-row `DataCopy` bursts unless re-batched, so in practice it would force per-row bursts); (c) hybrid group-cyclic: partition rows into contiguous groups of `G = k·B` rows, deal groups round-robin to cores — batches stay adjacent inside a group (burst preserved), group counts balance across cores (R016's tail-independence property), and each core's range is a whole number of groups (no ProcessSingle tail) whenever rows is a multiple of G.
- BOTTLENECK: tail structure of contiguous ownership — `rows % blockDim` gives cores unequal row counts (±1) and `rowsThisCore % B` gives batch tails that fall back to the 12-event single-row path. For uniform per-row cost the ±1 imbalance is already near-optimal, so the only real cost is the *batch-tail share* of rows, which at small D is paid in queue events, not in imbalance.
- EXPECTED_SHAPES: rows not divisible by blockDim with rowsThisCore % B ≠ 0 (FP32 100×256: 4–5 rows/core, B=4 → ~16–20% tail rows; FP16 64×1024: 2–3 rows/core → every core has a tail). Zero effect when rows divides evenly into B-row groups per core (control shape 96 rows / 24 cores / B=4).
- WHY_IT_MAY_HELP: group-cyclic makes ~100% of rows run through the batch path and equalizes rows/core exactly; pure cyclic's documented R016 property (tail distribution independent of `outer % blockNum`) generalizes to group granularity without losing adjacency.
- WHY_IT_MAY_FAIL: (1) ±1-row imbalance for uniform rows is tiny — the honest prior is near-neutral; (2) if rows is not a multiple of G, group-cyclic merely moves the partial group to one core (same total tail work); (3) contiguous ownership already gives adjacent cores adjacent GM regions; group-cyclic keeps this within groups, pure cyclic breaks it; (4) hidden shapes may divide cleanly → zero effect.
- ASCEND_FEASIBILITY: high — ownership arithmetic in `Init` plus a group→core mapping; no new primitive; pure cyclic variant (b) is feasible only with per-row bursts and would *undo* V001's DMA amortization, so (b) is listed as a contrast, not a candidate.
- UB/CORE/DMA_IMPACT: UB unchanged (B selector follows rowsThisCore). CORE: ownership mapping only, no `SyncAll` in Fast path. DMA: group-contiguous bursts preserved; pure cyclic would raise burst count back to per-row.
- SYNC_IMPACT: none beyond fewer ProcessSingle tails.
- PRECISION_RISK: none — identical per-row math, only the owner differs.
- DUPLICATE_CHECK: HIGH-SCREEN REQUIRED — SCHED-ROWGROUP-X's charter is "32B-safe row-group ownership" derived from R016; group-cyclic row ownership sits squarely in that territory. Differentiator available (this route needs it as a *batch-tail* variable, evaluated together with B), but running both would duplicate the ownership axis. Flag for Main scope decision before anyone implements. vs EXT-ASCEND-X — they fixed consecutive-row ownership, no cyclic variant. vs own V001 — V001 did not change ownership geometry.
- MINIMAL_OFAT_DIFF: FastKernel `Init` ownership arithmetic only (row start/count derived from group index); batch loop, copies, params, host dispatch untouched.
- EXPECTED_LOCAL_PROBES: (1) shape audit first — for each probe shape compute tail-row share under current ownership (read-only, from the shape math); (2) if tail share ≥ 15% on at least two probe shapes: correctness matrix, then device-event pairs on 100×256 and FP16 64×1024 with 96×256 (clean division) as negative control; (3) a delta on the clean-division control means measurement contamination, not confirmation.
- CLASSIFICATION: **NEEDS_MORE_EVIDENCE** (weak prior; scope collision with SCHED-ROWGROUP-X must be resolved by Main first)

---

## RECOMMENDED_NEXT

1. TRACK-A stays as-is: candidate SHA `ad961c58…` pinned, no kernel edits, no new revision, no device run this turn. When Main opens a device window, the order is: convert the BATCH harness to the unified device-event method (warmup ≥10, in-process ≥11/21 samples, interleaved pairs) → same-binary noise floor with parent A001-V017 on the exact probe shapes → P/C only on shapes whose MAD/median passes. The set1–3 wall-clock deltas (including the +185.5% pair) stay tagged as load-related and are never the basis for an architecture conclusion.
2. TRACK-B primary recommendation: **HYPOTHESIS-1 (two-phase batch with vectorized row-local reduction state)** is the best next architecture revision — it attacks the cost the bottleneck model identifies as dominant (per-row launches + V→scalar stalls), has a small single-mechanism diff inside `ProcessBatch`, no threshold change, low precision risk, and no scope collision with the other five active routes beyond a declared UB-layout adjacency to UB-LIVENESS-X.
3. Cheapest alternative: **HYPOTHESIS-3 (batch-aligned blockFactor)** is a host-only diff, but must clear duplicate screening against EXT-ASCEND-X's blockFactor territory and SCHED-ROWGROUP-X's scheduling scope first.
4. **HYPOTHESIS-2** needs its discrimination probe (same-binary Fast vs Resident on mid-D shapes) before any implementation; recommend Main fold that read-only measurement into the next device window this route is granted. Without it the hypothesis stays NEEDS_MORE_EVIDENCE.
5. **OPTIONAL-4 (depth-2 batch pipeline)** — defer: overlap is ASYNC-TRIPLE-X's charter and predicted gain at probe shapes is small.
6. **OPTIONAL-5 (group-cyclic ownership)** — do not implement while SCHED-ROWGROUP-X owns row-group geometry; only the tail-share shape audit is worth doing read-only.
7. Sequencing rule if Main issues `NEXT_HYPOTHESIS` after V001 resolves: return to the latest permitted parent (A001-V017 unless V001 is promoted) and take H1 as a single-mechanism revision; do not layer H1 onto an unresolved H3 or H2.

### SOURCES_INSPECTED (this turn, read-only)

- `phase4/control/execution-contract.md` §R (Long-Horizon Parallel Exploration) and `phase4/control/local-timing-protocol.md` (unified measurement method, outlier policy, shape-specific status, BATCH harness row).
- `phase4/control/idea-pool-29-routes.md`, `phase4/control/architecture-evidence-map.md`, `phase4/control/next-round-plan.md`, `phase4/control/scheduler.tsv` (route scopes for duplicate screening), `phase4/research/architecture-principles.md`.
- Worktree `cann-next6/BATCH-RESIDENT-X`: `ROUTE-BRIEF.md`, `phase4/workspaces/BATCH-RESIDENT-X/submission_v001.asc` (FastKernel batch path, ResidentKernel, host `run_kernel`), `phase4/local/BATCH-RESIDENT-X/V001/{source-meta.json, local-result.json, diff.patch, MAIN-REVIEW.md}`.
- Historical: `归档/phase3-before-reset-20260920/管理/路线状态/{R014,R015,R016,R029}.json`; `phase4/archive/historical-branches-20260924/` trees for R014 (`kernel.asc`), R015 (`op_kernel/kernel.txt`), R016 (`kernel.txt`), R029 (`kernel.txt`), R030/R029-L00x wide param-reuse series (`fast__lane-d`).
- Official tiling research archive: `归档/phase3-before-reset-20260920/调研/{调研1,调研4,调研5,调研6}` agent reports (rowFactor/ubFactor formulas, MERGE_N/MULTI_N/NORMAL/SPLIT_D thresholds).
- Public web: `vllm-ascend.readthedocs.io` reachable (no tiling mechanism on entry pages); `github.com/vllm-project/vllm-ascend` transport error — no external code consulted or copied.

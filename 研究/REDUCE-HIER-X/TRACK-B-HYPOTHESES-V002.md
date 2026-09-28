# TRACK-B HYPOTHESES V002 — REDUCE-HIER-X

ROUTE=REDUCE-HIER-X
WORKTREE=/Users/sunyiyang/Desktop/Project/cann-main2-r2/REDUCE-HIER-X
BRANCH=exp/main2-r2-reduce-hier
CURRENT_CANDIDATE=V001 (eager running-fold of tile partials)
V001_SOURCE_SHA=b9c618b3b53fd2988b667f0c6830fa4a7206aef0fde69f92cdb5545ba6503521
V001_LOCAL_VERDICT=NEEDS_ONE_MORE_LOCAL (clean delta -3.8% on 8x8192 FP32 only)
DIRECT_PARENT_FOR_V002=V001 (if Main accepts V001 as parent) else FROZEN_R31B_V011
OFFICIAL_ANCHOR=45.16
MODE=TRACK-B research only — no kernel edit until Main approves exactly one hypothesis

## 1. What V001 taught us

V001 changed only WHEN tile partials merge (eager fold into one accumulator,
collapse `ReduceSum` removed). Result: -3.8% on 8x8192 FP32 (tileCount=2),
within noise; 4/5 probe shapes were `MEASUREMENT_PROTOCOL_BLOCKED_FOR_SHAPE`
under d5 VLLM contention. Conclusion: the end-of-row collapse was **not** the
dominant reduction cost. The remaining per-tile cost is unchanged.

## 2. Where the reduction V/S handoffs actually sit (post-V001)

`kTileElems = 4096` (not 1024). Real tileCount on the multi-tile paths:

| D | path | tileCount | per-row ReduceSum calls (post-V001) | per-row GetValue |
|---|---|---:|---:|---:|
| 4096 | mid / full-tile | 1 | 1 | 2 (squareSum + invRms) |
| 8192 | `ProcessFp32FullRowOutputPipelined` / generic | 2 | 2 | 2 |
| 16384 | `ProcessWideFp32FullCacheRows` | 4 | 4 | 2 |
| 32768 | `ProcessWideFp32FullCacheRows` | 8 | 8 | 2 |

Each `AscendC::ReduceSum` performs an internal V/S handoff (comment at
`submission.asc:331` area, still true post-V001). So the reduction-side V/S
count per row is `tileCount + 2`. For D=32768 that is 10 V/S round-trips per
row, 8 of which exist only because the per-tile reduce is a `ReduceSum` API
call. This is the lever V001 left untouched.

V/S handoff count is the hypothesis axis for V002.

## 3. Win estimate model (vs V001)

H1 removed 1 of 3 V/S on tileCount=2 and produced ≈ -3.8% (weak, noisy).
Rough share of runtime in reduction V/S: ~10–15% at tileCount=2, growing with
tileCount because the output pass (gamma/bias Muls/Adds + stores) is roughly
D-proportional and flat per element, while V/S count grows with tileCount.
Estimated V/S share of the first pass: ~15–20% at tileCount=4, ~20–30% at
tileCount=8. A change that drops V/S from `tileCount+2` to `2` should therefore
exceed H1 by a factor of `tileCount` on the reduction-tail share.

---

## HYPOTHESIS-2 — Full manual vector reduction tree, 0–1 GetValue (R011)

- **MECHANISM**
  Replace every per-tile `AscendC::ReduceSum(...)` on the square tile with an
  explicit UB vector reduction tree: pairwise (or 4-way) `Add` halving of the
  `valid` squares down to one FP32 element, then the existing V001 eager fold
  `Add(acc, acc, tmp, 1)`. Zero intermediate `GetValue`. The only scalar reads
  per row remain the existing two (squareSum accumulator, invRms). Per-tile
  `ReduceSum` width, square `Mul`, mean/epsilon tail, and output pass are
  unchanged. The reduction becomes pure V-pipe work plus 2 V/S per row,
  independent of tileCount.

- **EXPECTED_BOTTLENECK**
  Per-tile `ReduceSum` internal V/S handoff, paid `tileCount` times per row
  (2/4/8 at D=8192/16384/32768). On large-D rows this serializes the first
  pass: each handoff drains the V pipe before the next tile’s square can be
  reduced.

- **FILES/FUNCTIONS TO TOUCH**
  - `Process()` first-pass loop (`submission.asc:297–332`)
  - `ProcessFp32FullRowOutputPipelined` (`submission.asc:1164–1190`)
  - `ProcessWideFp32FullCacheRows` (`submission.asc:2116–2150`)
  - Optional same-revision sites if the helper is shared: `ProcessNarrowMidOverlap`
    (`:555`), `ProcessBf16FullRowOutputPipelined` / `ProcessFp16FullRowOutputPipelined`
    (`:927`, `:1045`), `ProcessFp32FullRowOutputPipelined` sibling paths.
    For strict OFAT land S1/S2/S3 first and leave the rest as a follow-up.
  - A small local `VectorReduceSum1(dst, src, work, count)` helper in
    `reduceFp32Buf_` / a dedicated work area. No host / CMake / runner.

- **WHY_ORTHOGONAL_TO_MAIN1**
  In-kernel RMS reduction primitive only. No row scheduling, core ownership,
  DMA layout, dtype dispatch, gamma/bias policy, epilogue, or host change.
  MAIN-1 `cann-sixlane` remains untouched and is not a parent.

- **WHY_NOT_DUPLICATE_EXISTING_MAIN2**
  - Not V001/H1: H1 changed merge timing (partial-sum lifetime); this changes
    the per-tile reduction primitive (vector tree vs `ReduceSum` API).
  - Not `REDUCE-INVSCALE-X` H3: that widens the `ReduceSum` span and keeps the
    API; this removes the API call and its V/S handoff at the same span.
  - Not `REDUCE-INVSCALE-X` H4: that is a single whole-row `ReduceSum` for
    UB-fitting rows; this keeps per-tile spans and uses a manual tree.
  - Not `REDUCE-INVSCALE-X` H1/H2: those are reciprocal-tail / output-load
    scheduling. The invRms math sequence is out of this route’s scope.
  - Donor R011 (“full vector tree with 0–1 GetValue”) is idea-pool owner
    REDUCE-X and still unexplored. Prior R011 attempts used GetValue-heavy
    trees and correctness-failed; this variant is 0-intermediate-GetValue.

- **EXPECTED_WIN_SHAPES** (vs V001)
  - D=4096 (tileCount=1): small but real — removes the only per-tile V/S.
    Est. -3 to -6%.
  - D=8192 (tileCount=2): est. -8 to -15%.
  - D=16384 (tileCount=4): est. -12 to -22%.
  - D=32768 (tileCount=8): est. -18 to -35%. Best target; matches the
    “Official case 14 is 4.4x slower” large-work profile.
  - Multi-row × wide (8x8192, 8x32768): same per-row win, amplified by row
    count on the first pass.

- **EXPECTED_RISK_SHAPES**
  - Small D (D≤1024) where a manual tree of length 64–1024 may be slower than
    a single `ReduceSum` (tree depth 6–10 vs one API call). Control with
    D=256/1024 probes.
  - Unaligned `valid` tails (D=100, 65, 257): tree must handle non-power-of-2
    counts.
  - Shapes where the output pass dominates (small D, many rows): reduction
    share is small so delta may sit inside the noise floor.

- **CORRECTNESS_RISK**
  Moderate–high. Idea-pool record: prior R011-style trees correctness-failed.
  Hazards: odd/non-power-of-2 `valid`, tree index math, UB work-area aliasing
  with `xFp32`/`residualFp32`, FP32 association drift vs `ReduceSum`’s internal
  tree (tolerance should hold against a double golden but must be measured).
  Mitigation: one helper, prove at one site family first, keep `ReduceSum` as
  a documented fallback path only if it is not a second performance mechanism.

- **MEASUREMENT_PLAN**
  1. Declaration block (ROUTE / REVISION / DIRECT_PARENT / PARENT_SOURCE_SHA /
     PARENT_SCORE=45.16 / SINGLE_HYPOTHESIS / CONTEXT_CLASS / WHY_NOT_DUPLICATE).
  2. Build + link on server3 exact source; NPU correctness FP32/FP16/BF16
     including multi-tile (D=8192/16384/32768) and single-tile control (D=4096,
     D=256) plus unaligned (D=100/65).
  3. Same-binary qualification of Direct Parent at each probe shape under
     `local-timing-protocol.md` (device events, warmup ≥ 45, samples ≥ 21,
     MAD/med ≤ 0.10, drift ≤ 0.10). Prefer a quieter window than the V001 d5
     run; d5 residual VLLM made 4/5 shapes fail same-binary.
  4. Interleaved P/C pairs ≥ 4 on qualified shapes. Primary probes:
     1x8192, 1x16384, 1x32768 FP32 (tileCount 2/4/8) — these must show the
     trend to confirm the V/S story.
  5. Falsification: if the manual tree is not faster than `ReduceSum` at
     D=4096 (tileCount=1) on the same binary pair, the V/S-handoff hypothesis
     is wrong for this toolchain and H2 is parked.

---

## HYPOTHESIS-3 — Hierarchical two-level UB merge tree (R006)

- **MECHANISM**
  Keep per-tile `ReduceSum` (V/S per tile unchanged). Change only the V001
  eager fold into a hierarchical two-level tree in UB: partition the `tileCount`
  tile partials into groups of fan-in G (G=2 or 4), `Add` each group to a
  level-1 partial, then one final `Add`/short `ReduceSum` over the level-1s.
  `GetValue` count unchanged (2). For `tileCount ≤ G` this degenerates to
  V001’s flat fold.

- **EXPECTED_BOTTLENECK**
  The flat fold is already cheap (V001 proved the collapse was not dominant).
  This hypothesis can only shorten the ADD chain of the fold, not the per-tile
  `ReduceSum` V/S handoffs. Expected win is therefore much smaller than H2.

- **FILES/FUNCTIONS TO TOUCH**
  Fold sites in `Process()` (`:330`), `ProcessFp32FullRowOutputPipelined`
  (`:1177–1190`), `ProcessWideFp32FullCacheRows` (`:2116–2150`).

- **WHY_ORTHOGONAL_TO_MAIN1**
  Merge-tree shape only inside this kernel. No MAIN-1 coupling.

- **WHY_NOT_DUPLICATE_EXISTING_MAIN2**
  Distinct from H1 (flat fold) and H2 (per-tile primitive). Distinct from
  `REDUCE-INVSCALE-X` H3/H4 (span / stage count). Donor R006 “hierarchical UB
  tree” is still unexplored.

- **EXPECTED_WIN_SHAPES** (vs V001)
  - D=8192/16384/32768: est. 0 to -5%. Only the fold’s Add chain changes;
    per-tile V/S is untouched, so this cannot approach H2’s scale.

- **EXPECTED_RISK_SHAPES**
  - D=8192 (tileCount=2, G=2): no-op versus V001.
  - Large tileCount with G=2: more Add ops than a flat fold of 8 elements.

- **CORRECTNESS_RISK**
  Low. Same summands, different association. Hazards: odd `tileCount`, level-1
  slot aliasing.

- **MEASUREMENT_PLAN**
  Same protocol as H2, but expect deltas inside noise at most shapes. Useful
  mainly as a cheap negative control if H2 is blocked. Falsification is
  immediate: if it cannot beat V001 at D=32768, park.

---

## HYPOTHESIS-4 — Chunked vector tree (G tiles per tree, bounded UB)

- **MECHANISM**
  Middle point between H2 and H3. For each group of G consecutive tiles
  (G=2 or 4), write the G tiles’ squares into a contiguous UB region (reusing
  the y-cache / work tile where available) and run **one** vector reduction
  tree over that group’s `G * valid` elements, then fold the group result into
  the accumulator. V/S count per row becomes `ceil(tileCount / G) + 2` instead
  of `tileCount + 2`. UB cost ≈ `G * kTileElems` floats for the concatenated
  squares (G=2 → 32 KB, G=4 → 64 KB). Per-tile `Mul` square, mean/epsilon
  tail, and output pass unchanged.

- **EXPECTED_BOTTLENECK**
  Same per-tile `ReduceSum` V/S handoff as H2, but reduced by a factor of G
  rather than eliminated. UB pressure is the constraint: the squares of G
  tiles must coexist with the y cache and x/residual staging.

- **FILES/FUNCTIONS TO TOUCH**
  `ProcessWideFp32FullCacheRows` first (largest tileCount, clearest signal),
  then `ProcessFp32FullRowOutputPipelined` and generic `Process()`. Requires a
  concatenated-squares scratch region — either a new `InitBuffer` slot or a
  documented reuse of `valueFp32Buf_` after y is written (watch aliasing).

- **WHY_ORTHOGONAL_TO_MAIN1**
  Reduction organization + workspace only. No MAIN-1 coupling.

- **WHY_NOT_DUPLICATE_EXISTING_MAIN2**
  - Not H2: H2 is one tree per tile; this is one tree per G tiles.
  - Not `REDUCE-INVSCALE-X` H3: that widens the `ReduceSum` API span; this
    removes `ReduceSum` from the grouped region entirely and uses a manual
    tree. Different primitive, different UB cost model.
  - Not H3 of this document: H3 keeps per-tile `ReduceSum` and reshapes the
    fold; H4 removes most per-tile `ReduceSum` calls.

- **EXPECTED_WIN_SHAPES** (vs V001)
  - D=8192 (tileCount=2, G=2): equivalent to H2 (one tree per row).
  - D=16384 (tileCount=4, G=2): 2 trees instead of 4 `ReduceSum`; est. -8 to -15%.
  - D=32768 (tileCount=8, G=4): 2 trees instead of 8 `ReduceSum`; est. -12 to -22%.
  - Falls short of H2 at large D because V/S is reduced, not eliminated.

- **EXPECTED_RISK_SHAPES**
  - Wide FP32 full-Y where `valueFp32Buf_` already holds complete rows —
    squeezing G tiles of squares may force `wideFullYRows_` to shrink, which
    would change row occupancy (a second variable). Must be avoided or split.
  - UB-exhaustion at G=4 on top of the full-y cache.

- **CORRECTNESS_RISK**
  Moderate. Tree indexing across concatenated tiles, UB aliasing with the y
  cache, and the same non-power-of-2 tail issues as H2. Higher than H3, lower
  than H2 if G=2 and the tree is small.

- **MEASUREMENT_PLAN**
  Same protocol as H2. Primary probes: 1x16384 and 1x32768 FP32. Only pursue
  this if H2 is correctness-blocked and H3 shows no signal — it is the hedge,
  not the lead.

---

## WHY this can move Official score

Official case 14 is reported **4.4x slower than the best known result**, which
is the signature of a large-work shape (large D and/or many rows). On such
shapes the RMS first pass must reduce every tile’s squares, and post-V001 each
tile still pays a full `ReduceSum` V/S handoff. The handoff count grows with
tileCount (2/4/8 at D=8192/16384/32768) while the output pass is flat per
element — so the reduction tail is exactly the part that scales badly and
keeps this kernel off the Official curve at large D.

H2 attacks that scaling directly: V/S per row drops from `tileCount + 2` to
`2`, so the first pass becomes almost entirely V-pipe and can overlap with the
next tile’s MTE2. That is the single largest remaining reduction-side lever on
this route. V001 targeted the merge (small, proven weak); V002 must target the
per-tile primitive.

Secondary Official effect: worst-case shapes (wide FP32, multi-row) are the
ones the Official harness likely weights in the tail of the score. A
large-D-first win improves the worst case rather than the average, which is
what a 4.4x gap needs.

---

## Recommendation

**HYPOTHESIS-2 — Full manual vector reduction tree with 0–1 GetValue (R011).**

Why: it is the only candidate that removes the per-tile `ReduceSum` V/S
handoff rather than reshaping the (already cheap) merge. Expected win scales
with tileCount, which is exactly the large-D band where Official case 14
loses. H3 cannot deliver that scale by construction; H4 is a fallback if H2
is correctness-blocked.

## REQUEST_MAIN_APPROVAL

Requesting Main approval for exactly one V002 revision:

**HYPOTHESIS-2 — Full manual vector reduction tree (R011, 0–1 GetValue)**

- ROUTE: REDUCE-HIER-X
- REVISION: V002
- DIRECT_PARENT: V001 (SOURCE_SHA b9c618b3b53fd2988b667f0c6830fa4a7206aef0fde69f92cdb5545ba6503521)
  if Main accepts V001 as the parent; otherwise FROZEN_R31B_V011
  (a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3) and the
  fold from H1 is re-applied as part of V002’s delivered source.
- PARENT_SCORE: 45.16 (OFFICIAL_ANCHOR); V001 local delta -3.8% on 8x8192
  only (NEEDS_ONE_MORE_LOCAL)
- SINGLE_HYPOTHESIS: replace per-tile `ReduceSum` on the square tile with an
  explicit UB vector reduction tree (pairwise/4-way `Add`) feeding the V001
  eager fold; 0 intermediate `GetValue`; 2 scalar reads per row unchanged.
- CONTEXT_CLASS: FROZEN_STRONG_BASELINE_REDUCTION_TOPOLOGY
- WHY_NOT_DUPLICATE: see H2 section (distinct from H1 merge timing,
  REDUCE-INVSCALE-X H3 span / H4 stage-count, and from H3/H4 of this file)

No kernel, host, CMake, runner, shared-control, or CANNJudge action until
this approval is issued.

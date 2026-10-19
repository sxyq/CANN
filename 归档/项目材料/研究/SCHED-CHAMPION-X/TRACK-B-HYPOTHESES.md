# TRACK-B HYPOTHESES — SCHED-CHAMPION-X

ROUTE=SCHED-CHAMPION-X
WORKTREE=/Users/sunyiyang/Desktop/Project/cann-main2-r2/SCHED-CHAMPION-X
BRANCH=exp/main2-r2-sched-champion
DIRECT_PARENT=FROZEN_R31B_V011
PARENT_SOURCE_SHA=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
OFFICIAL_ANCHOR=45.16
CONTEXT_CLASS=FROZEN_STRONG_BASELINE_TRANSPLANT
TRACK=Track-B (read-only research; no kernel/host edit this turn)

## Scope lock (single performance variable)

Every hypothesis below varies **one** scheduling decision inside the domain
`row ownership / core assignment / row-group scheduling`. Nothing else moves.

Frozen-seed facts used as the transplant surface (verified this turn):

| fact | location |
|---|---|
| row partition is row-unit base/extra only | `Process()` lines 171–176 (`blockIdx/baseRows/extraRows/beginRow/localRows`) |
| no `rowGroup = 32/gcd(rowBytes,32)` anywhere | whole-file symbol scan: 0 hits for rowGroup/gcd ownership |
| host launch width is already core-fill at row granularity | `run_kernel` `requestedBlocks = min(availableCoreNum, rowCount)` then `blockCount` |
| batched/pipelined paths require `localRows > 1` | mode dispatch lines 178, 187, 196, 202, 207, 213, 218, 228 |
| wide path has its own base/extra split | `ProcessWideFp32*` / `ProcessWideLowPrecision` lines 1860+, 2088+, 2254+, 2387+, 3076+ |
| narrow/normal path is `rowWidth <= kCacheElems (8192)` | `widePath_` set at line 60–61 |

NON_DUPLICATION_AUDIT=PASS (Main 2026-09-27) — frozen champion has **no** whole-group
ownership. This file is therefore hypotheses, not `DO_NOT_IMPLEMENT_DUPLICATE`.

DEFERRED in every hypothesis (unchanged byte-for-byte): mode selection conditions,
rows/block batch widths (`wideFullYRows_`, `batchLimit`, `kSmallFp32*`), DMA
segmentation (`DataCopyExtParams`/`DataCopyPad`), wide path functions, dtype paths,
reduction arithmetic, UB lifetime/InitBuffer layout, epilogue arithmetic.

## Donor mechanism being transplanted (read-only)

Source: `phase4/workspaces/SCHED-ROWGROUP-X/WHY_NOT_DUPLICATE.md` and
`SCHED-ROWGROUP-X-V001-submission.asc` header (concept only, not copied).

Validated donor rule:

1. `rowGroup = 32 / gcd(rowBytes, 32)` consecutive rows form one 32B-aligned group
2. each complete group is owned wholly by one core (never split)
3. task extents round to whole groups
4. (parent-only, not applicable here) remove "row not 32B-aligned → single core"

Donor result on weak parent R016: Official 17.14 → 22.27 (15/15); clean local
33×100 −51.38% 4/4. Donor's dominant win channel was **lifting R016's single-core
fallback** on unaligned rows. Frozen R31B-V011 has no such fallback — it already
launches `min(cores, rowCount)` blocks and splits rows across them — so the
transplant win channel is **different**: group-aligned ownership boundaries,
`localRows > 1` batched-path eligibility, and group-quantized core scheduling.

## rowGroup values on probe-class shapes (narrow path, D ≤ 8192)

| shape | dtype | rowBytes | gcd(rowBytes,32) | rowGroup | H1 effect |
|---|---|---:|---:|---:|---|
| 17×256 | FP32 | 1024 | 32 | 1 | no-op (control) |
| 33×100 | FP32 | 400 | 16 | 2 | active |
| 17×257 | FP32 | 1028 | 4 | 8 | active |
| 7×65 | FP32 | 260 | 4 | 8 | active |
| 17×257 | FP16 | 514 | 2 | 16 | active (strongest) |
| 1×6144 | FP32 | 24576 | 32 | 1 | no-op; single row anyway |
| any D>8192 | any | — | — | — | wide path, out of scope |

---

## HYPOTHESIS-1: GROUP-ALIGNED BALANCED OWNERSHIP

**MECHANISM**
Replace the row-unit base/extra split at `Process()` 171–176 with a group-unit
base/extra split. Compute `rowGroup = 32 / gcd(rowWidth * sizeof(T), 32)`,
`totalGroups = ceil(rowCount / rowGroup)`, then
`baseGroups = totalGroups / blockCount`, `extraGroups = totalGroups % blockCount`,
`beginGroup = blockIdx * baseGroups + (blockIdx < extraGroups ? blockIdx : extraGroups)`,
`localGroups = baseGroups + (blockIdx < extraGroups ? 1 : 0)`,
`beginRow = beginGroup * rowGroup`,
`localRows = min(localGroups * rowGroup, rowCount - beginRow)`.
Each active core owns whole row groups; the final group is clipped to `rowCount`.
Host `requestedBlocks` is **not** changed. This is the literal donor rule
(whole-group ownership) applied to R31B's contiguous balanced model.

**EXPECTED_BOTTLENECK**
Non-group-aligned core boundaries: the current row-unit split cuts 32B-aligned
groups across cores, so per-core GM spans lose 32B alignment on unaligned shapes,
and fragment cores with `localRows == 1` fall off every batched/pipelined path
(`localRows > 1` conditions) onto the generic single-row loop.

**FILES/FUNCTIONS TO TOUCH**
`AddRmsNormBiasKernel::Process()` row partition only, frozen seed lines 171–176
(beginRow/localRows). Optional one-line rowGroup helper in the same class.
Not touched: `run_kernel` requestedBlocks (that is H2), `ProcessWide*`
(1860+/2088+/2254+/2387+/3076+), mode dispatch conditions, batch widths.

**WHY_ORTHOGONAL_TO_MAIN1**
MAIN-1 worktrees under `cann-sixlane/` are READ/WRITE DEFERRED and were not read.
Per the idea-pool mapping recorded in SCHED-ROWGROUP-X `WHY_NOT_DUPLICATE.md`,
row-group ownership / rows-task scheduling is the R012+R016 class owned by the
SCHED lane (this route). MAIN-1 champion lanes own multimode dispatch, wide-D
reduction/layout, and tiling-factor provenance — different mechanism classes.
This revision touches only the beginRow/localRows arithmetic; it does not collide
with any MAIN-1 DMA, reduction, epilogue, dtype, or UB change.

**WHY_NOT_DUPLICATE_EXISTING_MAIN2**
Not a rerun of SCHED-ROWGROUP-X V001: that donor combined R016 band scheduling +
R012 ownership on a weak parent whose win came from removing a single-core
fallback (1→3 cores on 33×100). R31B has no single-core fallback and a different
parent split (base/extra vs D-band tasks). H1 adds only whole-group boundary
alignment on the champion; expected win channels (group-aligned spans,
`localRows>1` path unlock) are not the donor's channel. Frozen-seed audit PASS
confirms no equivalent mechanism exists in MAIN-2's champion. Also distinct from
ALIGN-TAIL-X (tail copy path), ASYNC-TRIPLE-X (pipeline), BATCH-RESIDENT-X
(residency/multi-row DMA), REDUCE-INVSCALE-X (reduction/invRms), UB-LIVENESS-X
(UB layout) — none owns row-group core assignment.

**EXPECTED_WIN_SHAPES**
Unaligned narrow shapes where `rowGroup > 1` and `totalGroups < blockCount`
today fragment rows: 33×100 FP32 (rowGroup=2), 17×257 FP32 (rowGroup=8),
7×65 FP32 (rowGroup=8), 17×257 FP16 (rowGroup=16). Two win channels fire
together: (a) whole-group 32B-aligned per-core spans, (b) `localRows` rises from
1 to ≥rowGroup on previously fragmenting cores, unlocking the existing
`localRows > 1` batched paths. Expected magnitude: moderate on the champion
(already multicore) — well below the donor's −51% core-enablement jump; target
is a stable delta beyond the shape's same-binary floor.

**EXPECTED_RISK_SHAPES**
- Any `rowGroup == 1` shape (17×256 FP32, 1×6144, any 32B-aligned rowBytes):
  partition is bit-identical to today → delta must be ≡0. This is the
  falsification control, not a win shape.
- Wide shapes D>8192: untouched (wide path deferred) → ≡0.
- Shapes where `totalGroups >= blockCount` and `rowGroup == 1`: no change.
- If launch count drops (`blockCount > totalGroups` → idle blocks), any loss
  would show as worse balance when `rowCount` is large; probe shapes are safe.

**CORRECTNESS_RISK**
Low. Rows are independent; ownership only moves boundaries. Two things to prove
in the 15-case suite: (1) last-group clip `min(localGroups*rowGroup, rowCount-beginRow)`
never overruns `rowCount` and never leaves a gap or overlap across cores
(`beginGroup` ranges are disjoint and cover `0..totalGroups`); (2) cores whose
`localRows` newly satisfies `> 1` take batched paths that already exist and
already handle arbitrary `localRows ≥ 2` — mode-selection code itself is not
edited, so a pass there is a regression of existing paths, not a new path.
Numerical risk: none (per-row arithmetic untouched).

**MEASUREMENT_PLAN**
Per `phase4/control/local-timing-protocol.md`:
1. Same-binary qualification of Direct Parent R31B-V011 (SHA a8c19a19…) on each
   exact probe shape, unified protocol (device events primary, warmup≥45,
   ≥21 samples in-process, MAD/median ≤0.10 and block drift ≤0.10).
2. Only PASS shapes enter P/C. Required shapes: **33×100 FP32** and
   **17×257 FP32** (or 7×65 once qualified) as rowGroup>1 targets;
   **17×256 FP32** as the rowGroup=1 control (expect ≡0).
3. Interleaved P/C pairs ≥4 (PC/CP alternating), one device from
   `server3-device-leases.tsv`, no concurrent route on that device.
4. Primary stats only: per-block median DEVICE_EVENT_US, MAD/median, p10/p90,
   paired block delta. Raw samples retained; no post-hoc outlier rules.
5. Falsification: any nonzero delta on the 17×256 control (rowGroup=1) means
   the edit is not a pure ownership change → SINGLE_CHANGE_AUDIT re-audit.
6. Verdict vocabulary is Main's (LOCAL_ACCEPTED / LOCAL_REJECTED /
   NEEDS_ONE_MORE_LOCAL / …); local % ≠ Official Score.

---

## HYPOTHESIS-2: GROUP-QUANTIZED LAUNCH WIDTH

**MECHANISM**
Keep the device row-unit partition byte-identical. Change only the host launch
width in `run_kernel` from `requestedBlocks = min(availableCoreNum, rowCount)` to
`requestedBlocks = min(availableCoreNum, totalGroups)` with
`totalGroups = ceil(rowCount / rowGroup)` and the same `rowGroup` formula.
Fewer blocks launch; each active block receives more rows through the unchanged
base/extra split. This isolates the **core-scheduling half** of the donor rule
(group-quantized core count) without touching ownership boundaries.

**EXPECTED_BOTTLENECK**
Block launch/init overhead and row-granularity oversubscription. Today
`blockCount = min(cores, rowCount)` can exceed `totalGroups` whenever
`rowGroup > 1` (e.g. 33 rows × rowGroup=16 → 33 blocks for 3 groups), so many
blocks pay TPipe Init + entry cost for 1-row fragments while the batched paths
stay out of reach (`localRows == 1`).

**FILES/FUNCTIONS TO TOUCH**
`run_kernel` only: the `requestedBlocks` derivation in the host section
(`requestedBlocks = availableCoreNum …; if (requestedBlocks > rowCount) …`).
Needs host-side `rowGroup` from `rowWidth` + dtype size. No device edit at all.

**WHY_ORTHOGONAL_TO_MAIN1**
Host launch width is core assignment in the SCHED class. No MAIN-1 file is read
or written; no overlap with MAIN-1's DMA /
reduction / epilogue / dtype / UB mechanism classes.

**WHY_NOT_DUPLICATE_EXISTING_MAIN2**
Distinct from SCHED-ROWGROUP-X V001 (which changed device task granularity on a
band-table parent and owned groups on-device) and from H1 (which moves device
boundaries). H2 changes only how many blocks launch; it does not implement
whole-group ownership. Distinct from BATCH-RESIDENT-X (multi-row DMA/residency)
and from all other MAIN-2 routes' mechanisms. Frozen seed has no group-quantized
launch width today.

**EXPECTED_WIN_SHAPES**
Shapes where `blockCount_today > totalGroups`, i.e. `rowGroup > 1` and
`rowCount` small-to-moderate: 33×100 FP32 (33 blocks → 17), 7×65 FP32
(rowGroup=8 → 8 groups from 7 rows ⇒ 7 blocks → 1, careful: see risk),
17×257 FP16 (rowGroup=16 → 17 blocks → 2). Win channel is pure launch/entry
overhead removal plus larger `localRows` on survivors (which can unlock batched
paths as a consequence — same downstream effect as H1, different lever).

**EXPECTED_RISK_SHAPES**
- `rowGroup == 1` shapes: `totalGroups == rowCount` ⇒ launch width unchanged ⇒ ≡0.
- Shapes where `rowCount < rowGroup` (e.g. 7×65 FP16 with rowGroup=16):
  `totalGroups = 1` collapses the launch to 1 block — this **reintroduces
  single-core behavior** and is the failure mode of H2. Must be measured
  explicitly or the formula clamped to `min(cores, rowCount)` still, with
  `totalGroups` only as an upper bound. H2 as written is honest about this risk.
- Large-rowCount shapes where `rowCount ≫ cores`: `totalGroups` still ≥ cores
  usually, so launch width stays at core count → ≡0.

**CORRECTNESS_RISK**
Low-to-medium. Coverage is unchanged (base/extra still covers `rowCount`
exactly). The real risk is performance, not math: collapsing to 1 block when
`rowCount < rowGroup` is correct but can be a large regression versus today's
multi-core fragment split. Numerical risk: none. Mitigation candidate (a later,
separate hypothesis, not folded here): clamp
`requestedBlocks = min(availableCoreNum, max(totalGroups, …))` — but that would
be a second variable, so it is **out of scope for H2's OFAT**.

**MEASUREMENT_PLAN**
Same protocol as H1 (same-binary on Direct Parent first, then interleaved pairs,
device events, primary stats). Target shapes: 33×100 FP32 (clean block-count
drop 33→17), 17×257 FP16 if qualified (33→2, big drop — highest sensitivity to
the single-core-collapse risk). Control: 17×256 FP32 must be ≡0. Add one
`rowCount < rowGroup` shape if it can be qualified (7×65 FP16) precisely to
measure the collapse risk. Falsification: nonzero delta on rowGroup=1 control.

---

## HYPOTHESIS-3: GROUP-STRIDE OWNERSHIP (cyclic group mapping)

**MECHANISM**
Keep `rowGroup` / `totalGroups` / `blockCount` as in H1, but map groups to cores
cyclically instead of contiguously: core `i` owns groups `i, i+blockCount,
i+2*blockCount, …` (whole groups only), converted to `beginRow/localRows` by a
small per-core group loop or by a stride loop over group indices. Single
variable: **ownership pattern** (contiguous packed range → cyclic stride).

**EXPECTED_BOTTLENECK**
Inter-core load imbalance when `totalGroups > blockCount` and the final group is
short: contiguous packing hands the short tail group to one core and leaves the
others with full groups; cyclic stride spreads groups evenly by count and breaks
up any local cost heterogeneity. Secondary effect: cyclic destroys per-core GM
row-range locality (interleaved rows across cores).

**FILES/FUNCTIONS TO TOUCH**
`Process()` row partition 171–176, rewritten as a group-stride loop (the loop
shape is the change; batch widths / mode dispatch untouched). Wide functions
untouched.

**WHY_ORTHOGONAL_TO_MAIN1**
Ownership-pattern choice inside the SCHED row-group lane. No MAIN-1 access; no
shared mechanism class with MAIN-1.

**WHY_NOT_DUPLICATE_EXISTING_MAIN2**
Donor V001 used contiguous task extents (its `scheduleMode` 1 path) plus a cyclic
path as a band-selected option on a different parent. H3 is the pattern half
alone on R31B with group units; it does not re-donor R016 bands, R012 group
math, or the donor's core-enablement. Not BATCH/ASYNC/ALIGN/REDUCE/UB mechanisms.

**EXPECTED_WIN_SHAPES**
Only shapes with `totalGroups > blockCount` (large rowCount relative to cores)
and uneven group costs — e.g. large-R small-D where the last group is partial.
On today's small probe shapes (`totalGroups ≤ 17` while cores ≈ 40) cyclic and
contiguous **coincide** (each active core gets one group), so expect ≡0 there.

**EXPECTED_RISK_SHAPES**
- All currently likely probe shapes (33×100, 17×256, 17×257): `totalGroups ≤
  blockCount` after H1-style fill ⇒ pattern inert ⇒ delta must be ≡0. A nonzero
  local delta on those shapes falsifies the edit (it is not pattern-only) or the
  harness.
- Large-R shapes: cyclic interleaving can hurt GM burst locality → possible
  regression even while balance improves.

**CORRECTNESS_RISK**
Low. Stride ownership still partitions `0..totalGroups` exactly once if the loop
is `for (g = blockIdx; g < totalGroups; g += blockCount)`. Rows stay independent.
Partial final group clip identical to H1. Numerical risk: none.

**MEASUREMENT_PLAN**
This hypothesis is **not probeable on current PASS shapes** (provably inert when
`totalGroups ≤ blockCount`). Measurement plan: (1) qualify a large-rowCount
narrow shape (e.g. 256×100 FP32 or larger) with same-binary first; (2) only then
run interleaved P/C of H3 vs H1 (pattern-only pair) under the unified protocol.
Until that shape exists, H3 stays NEEDS_MORE_EVIDENCE and is not the first
revision. Control set still includes 17×256 (≡0 expected).

---

## HYPOTHESIS-4: TAIL-GROUP FOLDING (remainder ownership policy)

**MECHANISM**
Take H1's whole-group ownership, but change only how the **partial final group**
is owned: instead of giving the short remainder group (`rowCount % rowGroup`
rows, 1..rowGroup-1 rows) to its own core, fold it into the core that owns the
preceding full group. Core group-counts still differ by ≤1 among the remaining
cores; the folding core takes `rowGroup + (rowCount % rowGroup)` rows. Single
variable: **remainder-group ownership** (standalone fragment vs folded tail).

**EXPECTED_BOTTLENECK**
Tail straggler / critical path. On 33×100 (rowGroup=2) the fragment is 1 row; on
17×257 FP16 (rowGroup=16) the fragment can be 1 row against 16-row peers. A
1-row core pays full launch + Init + generic path cost and sets the makespan
floor whenever peers are much longer.

**FILES/FUNCTIONS TO TOUCH**
`Process()` row partition 171–176 — the `localRows` clip for the last active
core only (and the active-core count if the last core becomes idle). No host
change, no wide-path change, no other edits.

**WHY_ORTHOGONAL_TO_MAIN1**
Remainder ownership policy in the SCHED row-group lane. MAIN-1 untouched.

**WHY_NOT_DUPLICATE_EXISTING_MAIN2**
Donor V001 rounded tasks to whole groups and left the short tail to a task; it
did not fold the fragment into a neighbor. H4 is not a rerun of donor
core-enablement or of H1's boundary unit change (H1 is the precondition; H4
varies only the tail rule). Not any other MAIN-2 route's mechanism.

**EXPECTED_WIN_SHAPES**
Shapes with `rowCount % rowGroup != 0` and enough rows that the fragment core is
otherwise tiny: 33×100 FP32 (remainder 1 of rowGroup=2), 17×257 FP32
(remainder 1 of 8), 7×65 FP32 (rowCount 7 < rowGroup 8 — folding degenerates,
see risk). Gain is the removal of the 1-row straggler, usually a small fraction
of total time unless the fragment core is the sole critical path.

**EXPECTED_RISK_SHAPES**
- `rowCount % rowGroup == 0`: no fragment ⇒ H4 ≡ H1 ⇒ ≡0 delta. Control.
- `rowCount < rowGroup` (7×65 FP16 class): everything is one partial group;
  folding has nothing to fold → ≡0 or falls back to H1 semantics.
- Shapes where peers are already short: folding lengthens one core (rowGroup+rem)
  and may worsen balance instead of helping.

**CORRECTNESS_RISK**
Low. Coverage stays exact (the folded core owns a contiguous row range of
`rowGroup + rem` rows; the former fragment core owns nothing and does not write).
Needs the same last-range clip discipline as H1. One extra edge: the folded span
is no longer exactly one 32B-aligned group — the "whole group" invariant is
relaxed only for the tail. Numerical risk: none.

**MEASUREMENT_PLAN**
H4 is a refinement **on top of H1**, so it can only be measured after H1 is
locally accepted (otherwise P/C confounds two variables). Plan: same-binary on
Direct Parent for the exact shapes; then interleaved P/C of H4 candidate vs the
H1 Local Best (not vs frozen parent — otherwise the pair mixes H1+H4). Shapes:
33×100 FP32 primary, 17×257 FP32 secondary; `rowCount % rowGroup == 0` shape as
≡0 control. Unified protocol otherwise unchanged.

---

## RECOMMENDATION — first OFAT revision

**RECOMMENDED FIRST: HYPOTHESIS-1 (GROUP-ALIGNED BALANCED OWNERSHIP).**

Why:

1. It is the **literal validated donor rule** (whole-group ownership) — the
   mechanism Main already accepted as Local + Official validated on the weak
   parent. H2/H3/H4 are single levers around it; H1 is the mechanism itself.
2. It is a true single variable: only the boundary unit of the existing
   base/extra split changes (rows → groups). Host launch width, mapping pattern,
   remainder policy, mode selection, and all deferred surfaces stay put.
3. It is **self-falsifying and safe**: on every `rowGroup == 1` shape the
   partition is bit-identical to frozen R31B-V011, so the 17×256 control must
   read ≡0. Any nonzero control delta rejects the implementation before it can
   pollute the champion.
4. It has a **second, already-built win channel** unique to this strong parent:
   fragment cores whose `localRows` rises to `≥ rowGroup (> 1)` enter the
   existing batched/pipelined paths. That channel does not exist in the donor
   (row-at-a-time `ProcessRow`) and is not a second variable — it is a downstream
   consequence of the same ownership change through unchanged dispatch code.
5. H2's `rowCount < rowGroup` collapse risk and H4's tail-only effect make them
   better as follow-ups; H3 is inert on current probe shapes and is not first.

Order after H1: H2 (if launch overhead shows up), then H4 (if the tail fragment
is the critical path), then H3 (only after a large-rowCount shape is qualified).

## Collision risk (summary)

- MAIN-1: none observed. `cann-sixlane/*` not read or written; mechanism class
  (row-group ownership / core scheduling) is this route's recorded lane.
- Existing MAIN-2: no duplication with SCHED-ROWGROUP-X (different parent,
  different win channel), nor with ALIGN/ASYNC/BATCH/REDUCE/UB lanes. Frozen
  champion audit PASS (no rowGroup ownership present).
- Intra-file: H1 ⊥ H2 (boundaries vs launch width) ⊥ H3 (pattern) ⊥ H4 (tail
  rule); H4 requires H1 as precondition and must be paired against the H1 Local
  Best, not the frozen parent.

REQUEST_MAIN_ROUTE_RECORD: HYPOTHESIS-1 (GROUP-ALIGNED BALANCED OWNERSHIP) — record this single hypothesis as the first OFAT revision of SCHED-CHAMPION-X on DIRECT_PARENT=FROZEN_R31B_V011 (PARENT_SOURCE_SHA=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3).

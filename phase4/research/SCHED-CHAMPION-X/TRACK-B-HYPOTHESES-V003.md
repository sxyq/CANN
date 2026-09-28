# TRACK-B HYPOTHESES V003 — SCHED-CHAMPION-X

ROUTE=SCHED-CHAMPION-X
DIRECT_PARENT=V002 (SOURCE_SHA de1e93c7…, LOCAL_ACCEPTED −4.4% 5/5)
OFFICIAL_ANCHOR=45.16
CONTEXT_CLASS=OWNERSHIP_LANE_CONTINUATION

## Structural constraint discovered

On FP32, `rowGroup > 1` iff `width % 8 != 0`. Batched compute paths
(`ProcessSmallFp32ContiguousBatched`, `ProcessSmallFp32Batched`) require
`width % kSmallFp32ScalarStride(8) == 0`. These conditions are mutually
exclusive: shapes where group ownership matters are exactly shapes where
batched compute is unavailable.

Win channel on unaligned shapes is therefore **param residency only**
(`cacheParams = localRows > 1` → gamma/bias loaded once per core). V002's
−4.4% on 33×100 is this channel. On aligned shapes (`width%8==0`),
`rowGroup=1` → group split is a no-op; parent row-unit split already gives
`localRows > 1` and batched paths.

**Implication: ownership lane has limited remaining headroom.**

---

## H1: TAIL-GROUP FOLDING

**MECHANISM**: When `useGroupSplit` and `rowCount % rowGroup != 0`, fold the
partial final group (1..rowGroup-1 rows) into the preceding group's owner.
No standalone fragment core. Same group-unit split otherwise.

**EXPECTED_BOTTLENECK**: Tail straggler. On 33×100 (rowGroup=2, 33 rows):
core 16 gets 1 row vs 2 rows for cores 0-15. Critical path = 2 rows. Folding
gives core 15 rows 30-32 (3 rows) → critical path = 3 rows. **Actually worse
for this shape.** Only helps when the fragment is large relative to full groups.

**FILES/FUNCTIONS TO TOUCH**: `Process()` group-split branch, `localRows` clip.

**WHY_ORTHOGONAL_TO_MAIN1**: Ownership tail policy. No MAIN-1 access.

**WHY_NOT_DUPLICATE_EXISTING_MAIN2**: V002 keeps fragment as separate group.
H4 in original Track-B; not yet implemented.

**EXPECTED_WIN_SHAPES**: Shapes where `rowCount % rowGroup` is large (e.g.,
rowGroup=8, rowCount=20 → fragment=4). Marginal.

**EXPECTED_RISK_SHAPES**: 33×100 (fragment=1, folding makes straggler worse).

**CORRECTNESS_RISK**: Low. Coverage exact; folded core owns contiguous rows.

**MEASUREMENT_PLAN**: Same-binary + P/C on 33×100 (expect ≈0 or slight loss).

**EXPECTED_GAIN**: ≤1%. Marginal.

---

## H2: CYCLIC GROUP ASSIGNMENT (large-R shapes)

**MECHANISM**: When `useGroupSplit` and `totalGroups > blockCount`, map groups
to cores cyclically (core i takes groups i, i+blockCount, …) instead of
contiguous base/extra.

**EXPECTED_BOTTLENECK**: Inter-core load balance when groups have heterogeneous
costs (partial tail group). Cyclic spreads evenly.

**FILES/FUNCTIONS TO TOUCH**: `Process()` group-split branch, mapping pattern.

**WHY_ORTHOGONAL_TO_MAIN1**: Ownership pattern. No MAIN-1 access.

**WHY_NOT_DUPLICATE_EXISTING_MAIN2**: H3 in original Track-B. Not implemented.

**EXPECTED_WIN_SHAPES**: Large-R shapes where `totalGroups > blockCount`
(e.g., 100×100 FP32). Neutral on current probe shapes (33×100: totalGroups=17
< blockCount=33 → each core gets 1 group, cyclic == contiguous).

**EXPECTED_RISK_SHAPES**: Current probes (provably inert when
`totalGroups ≤ blockCount`). Large-R shapes: cyclic may hurt GM locality.

**CORRECTNESS_RISK**: Low. Stride loop covers groups exactly once.

**MEASUREMENT_PLAN**: Requires qualified large-R shape. Not probeable now.

**EXPECTED_GAIN**: ~0 on current shapes. Needs new shape.

---

## H3: GATE THRESHOLD TUNING

**MECHANISM**: Change gate from `totalGroups * 2 >= activeRowUnits` to
`totalGroups * 1.5 >= activeRowUnits` (or other multiplier). Activates group
split on more intermediate shapes.

**EXPECTED_BOTTLENECK**: Gate sensitivity. Current 2x threshold is calibrated
so 33×100 (ratio 0.515) activates and 17×257 FP16 (ratio 0.118) does not.

**FILES/FUNCTIONS TO TOUCH**: `Process()` gate constant.

**WHY_ORTHOGONAL_TO_MAIN1**: Gate parameter. No MAIN-1 access.

**WHY_NOT_DUPLICATE_EXISTING_MAIN2**: V002's gate is 2x. Tuning is parameter
sweep, not new mechanism.

**EXPECTED_WIN_SHAPES**: Shapes with ratio between 0.5 and 0.67 (e.g.,
totalGroups=20, activeRowUnits=33). None in current probe set.

**EXPECTED_RISK_SHAPES**: 33×100 if threshold loosened above 0.515. 17×257
if tightened below 0.118 (won't happen with 1.5x).

**CORRECTNESS_RISK**: None. Pure parameter change.

**MEASUREMENT_PLAN**: Same shapes. Expect ≈0 delta (gate already correct for
probes).

**EXPECTED_GAIN**: ~0 on current shapes.

---

## RECOMMENDATION: PARK after H1

**Rationale**: The structural constraint (`rowGroup>1` iff `width%8≠0`, batched
compute requires `width%8==0`) means ownership's win channel is param residency
only, already captured at −4.4%. H1 (tail folding) is the only remaining
single-variable ownership change with a plausible (though marginal) effect.
H2/H3 have no probeable surface. The big lever (batched compute on unaligned
shapes) requires mode-selection changes (forbidden in this lane).

**Recommend**: Implement H1 as one final OFAT (expected ≤1% gain). If it shows
no improvement, PARK the ownership lane and redirect to mode/DMA lanes where
the batched-compute headroom lives.

REQUEST_MAIN_APPROVAL: HYPOTHESIS-H1 (TAIL-GROUP FOLDING) as final ownership OFAT before PARK evaluation.

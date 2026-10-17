# current route recordS — SCHED-CHAMPION-X V002

Date: 2026-09-27
Recordr: MAIN-2
DIRECT_PARENT=FROZEN_R31B_V011
PARENT_SOURCE_SHA=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
OFFICIAL_ANCHOR=45.16
V001_DISPOSITION=NEEDS_ONE_MORE_LOCAL (33x100 FP32 clean -8.8% favor V001; 17x257 FP16 +145.9% parallelism collapse)

## SELECTED SINGLE HYPOTHESIS (V002)

Group-aligned balanced ownership WITH active-core preservation.

Same conceptual mechanism as V001 (32B-safe row-group ownership / core assignment),
repaired so whole-group ownership is applied only when it does not collapse
core parallelism.

Rule:
1. Compute rowGroup = 32 / gcd(rowBytes, 32) as in V001.
2. Compute the parent row-unit split (beginRow0, localRows0) exactly as frozen
   Process() lines 171-176.
3. If totalGroups = ceil(rowCount / rowGroup) >= blockCount:
   use V001 group-unit split (whole-group ownership).
4. Else:
   keep parent row-unit split (beginRow0, localRows0). No group snap.
   // This preserves the 17x257 FP16 17-core path when rowGroup would
   // collapse to 2 cores.

Rationale: V001 timing showed the win channel is group-aligned spans when
there are enough groups to fill cores (33x100 rowGroup=2). When rowGroup is
large vs rowCount (17x257 FP16 rowGroup=16 -> 2 groups), whole-group ownership
destroys parallelism and is a large regression.

DEFERRED: same as V001 — no host requestedBlocks change, no mode/DMA/wide/dtype/
reduction/UB/epilogue change.

V001's correctness SyncMTE3ToV repair stays (it is required for multi-row ownership).

## After V002

- Rebuild + correctness (same matrix as V001).
- Timing: same 4 shapes. Expect 33x100 delta ~= V001 (-8%), 17x257 FP16 delta ~= 0 (control).
- Then local verdict and ONLINE_WORTHY decision.

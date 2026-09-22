# A001 Route Thread (export)

Source: MiMo Desktop actor `general-3` (and predecessor `general-3` after stream recovery) plus Main feedback/judge records in this session.
Full machine transcript not exported by platform; this file preserves ordered handoffs and Main/judge feedback as recorded in-session.

## Index
1. Spawn prompt (Fresh Phase4 Control)
2. V010 handoff
3. Main feedback V010
4. V011 handoff
5. Main feedback V011 (RE)
6. V012 handoff
7. Main feedback V012
8. V013 handoff
9. Main feedback V013 (RE)
10. V014 handoff
11. Main feedback V014
12. V015 handoff
13. Main feedback V015 (RE)
14. V016 handoff
15. Main feedback V016
16. V017 handoff
17. Main feedback V017
18. V018 handoff (ONLINE_READY, not submitted)
19. Park notice

---

## 1. Spawn prompt (summary)
ROLE: Fresh Phase4 Control. ONLY modify `phase4/workspaces/A001/`.
ISOLATION: Do NOT read historical R031, R31A, R31B compute sources.
Baseline given: FRESH BEST A001-V008 28.10; V009 27.72 regression — base next work on V008.
Need architecture-level change, not chunk/queue/DataCopy tweaks.
Handoff keys: ROUTE REVISION HYPOTHESIS CHANGED COMPILE SOURCE COMPILE_LOG CLEANUP ONLINE_READY EXPECTED_AFFECTED_CASES.
No git, no CANNJudge.

## 2. V010 handoff
ROUTE: A001 / REVISION: V010
HYPOTHESIS: V008 loses to (1) second x+residual read after RMS, (2) gamma/bias reload every chunk, (3) row-only mapping idling cores when R < cores (T14 r=31.5).
CHANGED: `submission_v010.asc` full-u UB, gamma/bias cache, 2D D-split, output-tail partial RMS.
COMPILE: PASS Ascend910B3/dav-2201.
ONLINE_READY: yes.
EXPECTED: T14/T07/T04/T06/T08; all shapes from single-read u.

## 3. Main feedback V010
PASS 15/15 SCORE 33.25 NEW FRESH BEST (was 28.10).
T14 53176.88 vs V008 118183.70; T04 23.66 vs 85.33; T07 69.75 vs 225.75.
REGRESSIONS: T01/T02/T05 small-case overhead.
Next: recover small-case latency or attack T14/T08. Isolation unchanged.

## 4. V011 handoff
HYPOTHESIS: V010 small-case regression from full-u setup + two GetValue drains; T14 still overhead-bound.
CHANGED: FastKernel small totals; single GetValue; 2-slot x/res.
ONLINE_READY: yes.

## 5. Main feedback V011
PASS 3/15 Runtime Error. T01 WA 100%; T05 RE; rest skipped.
FastKernel idea is right when correct (T02 3.25 T03 6.55 T04 18.08).
Fresh Best remains V010 33.25.
Next: fix FastKernel correctness/crash; fall back to V010 Resident.

## 6. V012 handoff
HYPOTHESIS: V011 T01 WA = Init-time gamma/bias MTE without queue sync; T05 RE = one-shot D buffers near D=8192.
CHANGED: param load per row via queue; host UB check; else V010 Resident.
ONLINE_READY: yes.

## 7. Main feedback V012
PASS 15/15 SCORE 34.38 NEW FRESH BEST.
T01 3.97 T02 3.80 (FastKernel wins). T14 53243 still r=14.2. T04 89.58 still slow.

## 8. V013 handoff
HYPOTHESIS: V012 T04 slow because per-row param GM reload; widen Fast gate to 2M with UB still enforced.
CHANGED: LoadFloatParamsOnce; kFastMaxElems=2097152; Resident single-GetValue + double-buffer.
ONLINE_READY: yes.

## 9. Main feedback V013
PASS 4/15 Runtime Error. T05 RE again. FastKernel T01-T04 fast (3.93/3.49/7.17/19.22).
2M gate re-admitted T05. Fresh Best remains V012 34.38.

## 10. V014 handoff
HYPOTHESIS: tighten gate to proven domain; keep param-once only.
CHANGED: D<=4096 && R*D<=262144 && fastUb<=160000; Resident = V012/V010 copy.
ONLINE_READY: yes.

## 11. Main feedback V014
PASS 15/15 SCORE 35.89 NEW FRESH BEST.
T01 4.14 T02 3.52 T04 18.53. T14 53260 still dominant.

## 12. V015 handoff
HYPOTHESIS: T14 cost is Resident per-row GetValue×2 + serialized MTE.
CHANGED: single GetValue + 2-slot issue-ahead + D-split when D>=8192 && R<=2*cores.
ONLINE_READY: yes.

## 13. Main feedback V015
PASS 4/15 Runtime Error. T05 RE. Multi-delta stack invalid. Fresh Best remains V014 35.89.
Next: V014 byte-stable + ONE Resident change only.

## 14. V016 handoff
CHANGED: only BuildUKeepSum + InvRmsFromSum (one GetValue). No double-buffer. No D-split.
ONLINE_READY: yes.

## 15. Main feedback V016
PASS 15/15 SCORE 36.35 NEW FRESH BEST. T14 50830. Single GetValue validated.

## 16. V017 handoff
CHANGED: only 2-slot x/res issue-ahead. peakTile = 6*T+5*float. No D-split.
ONLINE_READY: yes.

## 17. Main feedback V017
PASS 15/15 SCORE 36.41 NEW FRESH BEST. Double-buffer safe. T14 50050.

## 18. V018 handoff
CHANGED: host-only wideFew = (d>=8192 && rows<=cores*2); up to 4 column groups when R>=cores.
ONLINE_READY: yes. Expected: T14 if wide few-row; else no-op.

## 19. Park notice
Main: RETIRED ROUTE ARCHIVE. A001 leaves fixed slots. Best V017 36.41 retained as idea donor for MIX-A. V018 unfinished (not submitted). Do not continue optimizing A001.

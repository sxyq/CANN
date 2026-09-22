# H001 Route Thread (export)

Source: MiMo Desktop actor `general-5` plus Main feedback/judge records.
Ordered handoffs and Main/judge feedback preserved.

## Index
1. Spawn prompt
2. V001 handoff / first CE
3. Main feedback V001 CE
4. V002 handoff / still CE
5. Main feedback V002 CE
6. V003 handoff / first online run
7. Main feedback V003
8. V004 handoff
9. Main feedback V004 (first 15/15)
10. V005 handoff
11. Main feedback V005 TLE
12. V006 handoff
13. Main feedback V006
14. V007 handoff
15. Main feedback V007 (23.29)
16. V008 handoff
17. Main feedback V008 (29.04)
18. V009 handoff
19. Park notice

---

## 1. Spawn prompt (summary)
Fresh Small-D / High-R. ONLY `phase4/workspaces/H001/`. No R031 family. Own BF16/run_kernel/ABI.
State: compile-03 `not support bf16 type cast`; run_kernel needs verify.

## 2–3. V001
Handoff: BF16 via ToFloat/ToBfloat16 + Cast; multi-row tile; run_kernel ABI. LOCAL all PASS.
Main: online Compile Error 0/15 (6ab28d8a). Local ≠ judge.

## 4–5. V002
Handoff: npu_kernel_dev template; preflight 5/5.
Main: online still CE (6ab29347). Compare B001 shape; isolate constructs.

## 6–7. V003
Handoff: B001 skeleton; three dtype entries; GM tiling run_kernel.
Main: FIRST ONLINE RUN 9/15. T05/T09/T11/T12/T13/T15 ~99.9% WA. Template solved. Fix wide path.

## 8–9. V004
Handoff: ProcessWide two-pass full-row RMS; uint64 offsets.
Main: 15/15 FIRST FULL CORRECTNESS score 12.54. Next small-D speed.

## 10–11. V005
Handoff: fused ApplyRow + ReduceSum + depth-2 DMA.
Main: 0/15 TLE. Revert to V004; one change only. Watch TQue Alloc-before-Free.

## 12–13. V006
Handoff: V004 structure + fused ApplyRow only.
Main: 15/15 12.55 flat. Next ReduceSum for sum(u*u).

## 14–15. V007
Handoff: ReduceSum in hot ApplyRow; 8KiB tmp.
Main: 15/15 23.29 BIG JUMP. T03 4.40 T04 14.40 T06 24 T07 41 T08 95 T10 98 T14 22943.

## 16–17. V008
Handoff: ReduceSum in wide ChunkSumSquares.
Main: 15/15 29.04 NEW BEST. Wide cases collapsed (T05 36.77 T09 220 T15 23771).

## 18. V009 handoff
Wide resident FP32 gamma/bias when fit ≤80KiB. ONLINE_READY. Not submitted.

## 19. Park notice
Main: RETIRED ROUTE ARCHIVE. H001 parked after loop close (V006 already online earlier; V008 is best). Explore slots rotate to MID-X/REDUCE-X etc.

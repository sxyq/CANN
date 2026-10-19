# G001 Route Thread (export)

Source: MiMo Desktop actor `general-4` (G001 resident-y one-read) plus Main feedback/judge records.
Full machine transcript not exported by platform; ordered handoffs and Main/judge feedback preserved.

## Index
1. Spawn prompt
2. V002 handoff
3. Main feedback V002
4. V003 handoff
5. Main feedback V003
6. V004 handoff
7. Main feedback V004
8. V005 handoff
9. Main feedback V005
10. V006 handoff
11. Main feedback V006
12. V007 handoff
13. Main feedback V007
14. V008 handoff
15. Main feedback V008
16. V009 handoff
17. Main feedback V009
18. Park / cancel notice

---

## 1. Spawn prompt (summary)
ROLE: Fresh One-Read / Resident-Y. ONLY `phase4/workspaces/G001/`.
ISOLATION: no R031/R31A/R31B compute. G001 owns all source fixes including sqrtf/BF16/ABI/run_kernel.
State: V001 already online 6/15 WA calc 9.95; Main previously integrated ABI/sqrt; further source changes reserved for G001 Child.
Fix correctness first (T12/T14/T15 100% err), keep resident-y.

## 2. V002 handoff
HYPOTHESIS: V001 WA from (1) FromFloat BF16 low-16 bits bug, (2) native-dtype u before RMS overflow, (3) dtype code remap vs judge.
CHANGED: all intermediate FP32; FromFloat bits>>16; judge encoding end-to-end; resident when 4*dim≤160KiB.
COMPILE: PASS. ONLINE_READY: true.
EXPECTED: T12/T14/T15 Pass; T02/T04/T06/T07 improve.

## 3. Main feedback V002
PASS 11/15. calc 9.686. T12/T14/T15 Pass (was 100% err).
Remaining WA: T02 3.84% T04 5.81% T06 2.63% T07 3.84% T11 0.022%.
Best correctness so far. Next V003 diagnose remaining WA.

## 4. V003 handoff
HYPOTHESIS: golden quantizes u native then invRms multiply chain.
CHANGED: AddNativeU + u*invRms*gamma+bias.
ONLINE_READY: true.

## 5. Main feedback V003
PASS 7/15 REGRESSION. T12/T14/T15 18–21% err. T02/T04/T06/T07 still 2–6%.
V002 is better baseline. Revert quantize/associativity for large-D.

## 6. V004 handoff
CHANGED: restore V002 arithmetic; explicit RNE FromFloat half/BF16.
ONLINE_READY: true.

## 7. Main feedback V004
PASS 10/15. T12/T14/T15 Pass again. T02/T04/T06/T07 still 2.6–6%. T11 0.011% WA.
RNE did not clear target cases.

## 8. V005 handoff
HYPOTHESIS: golden `output=norm+bias` with norm already native → T(T(u/rms*gamma)+bias).
CHANGED: NormPlusBias intermediate native cast; revert RNE.
ONLINE_READY: true.

## 9. Main feedback V005
PASS 6/15 REGRESSION. T12/T14/T15 ~20% again. T02/T04/T06/T07 2–6%.
STOP stacking numerical experiments that break large cases.

## 10. V006 handoff
CHANGED: only gamma order `u*gamma/rms+bias` on V002 baseline.
ONLINE_READY: true.

## 11. Main feedback V006
PASS 9/15. T02/T04/T06/T07 frozen. T11/T13 tiny WA. Gamma order not the fix.

## 12. V007 handoff
CHANGED: only intermediate cast of u/rms (O(1)) before *gamma.
ONLINE_READY: true.

## 13. Main feedback V007
PASS 6/15 REGRESSION. u/rms cast broke T12/T14/T15. All isolated numerical levers nearly exhausted.

## 14. V008 handoff
CHANGED: only pairwise binary-counter sumSquares.
ONLINE_READY: true.

## 15. Main feedback V008
PASS 10/15. Pairwise did not fix T02/T04/T06/T07. Levers exhausted.
SHIFT: restore V002 then performance on pass set.

## 16. V009 handoff
CHANGED: exact V002 arithmetic + gamma/bias UB cache for multi-row performance.
ONLINE_READY: true.

## 17. Main feedback V009
PASS 9/15. T11/T13 0.15% WA new. T02/T04/T06/T07 still 2–6%. Best remains original V002 11/15.

## 18. Park / cancel notice
Main: RETIRED ROUTE ARCHIVE / park G001. Scalar GetValue family not continued. Actor general-4 cancelled. Future one-read = G002 Fresh Vectorized Route.

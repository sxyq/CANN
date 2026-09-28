# I001 Route Thread (export)

Source: MiMo Desktop actors `general-6`, then `general-9` after stream failure, plus Main feedback/judge records.

## Index
1. Spawn prompt (empty workspace)
2. V001 handoff / CE
3. Main feedback V001
4. V002 handoff / CE
5. Main feedback V002
6. V003 handoff / first online
7. Main feedback V003
8. V004 handoff
9. Main feedback V004 (15.53)
10. V005 handoff
11. Main feedback V005
12. V006 handoff
13. Main feedback V006
14. V007 handoff
15. Park notice

---

## 1. Spawn prompt (summary)
Fresh Wide-D Specialist. ONLY `phase4/workspaces/I001/` (then empty). No R031 family.
Explore retained-y, large-tile streaming, low-sync cooperative reduction, parameter-stripe — not just chunk 4096→8192.

## 2–3. V001
Handoff: retained-y / two-pass / D-stripe; local full link PASS.
Main: online CE 0/15. Self-contained judge file required.

## 4–5. V002
Handoff: template ABI rewrite; preflight 5/5.
Main: still CE. Match B001 run_kernel ABI exactly.

## 6–7. V003
Handoff: GM_ADDR + const TensorGroupInfo& + int64_t + aclrtStream; two-pass body.
Main: FIRST ONLINE RUN 10/15. T02/T03/T11 WA; T13 RE. ABI solved.

## 8–9. V004
Handoff: rightPadding; ReduceSum 8KiB tmp; store n; tile 256.
Main: 15/15 FIRST FULL score 15.53. Then wide-D speed.

## 10–11. V005
Handoff: retained-y + large-tile 2048 + D-split rows≤4 cols≥4096.
Main: 13/15 calc 25.27. T02/T03 ~100% WA. Speed wins real (T09/T11/T15). Fix tiny WA.

## 12–13. V006
Handoff: uKeep +32 slack for padded workN.
Main: 13/15 still T02/T03 WA. Padding hypothesis incomplete.

## 14. V007 handoff
Force D<256 to V004 two-pass; retained-y only D≥256. ONLINE_READY. Not submitted.

## 15. Park notice
Main: RETIRED ROUTE ARCHIVE. I001 parked. Wide T14 gap vs best remains huge. Future wide work goes to WIDE-X exploration slot.

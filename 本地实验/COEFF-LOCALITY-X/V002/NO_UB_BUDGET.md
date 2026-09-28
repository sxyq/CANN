# COEFF-LOCALITY-X V002 — NO_UB_BUDGET

Date: 2026-09-27
Route: COEFF-LOCALITY-X (branch exp/main2-r2-coeff-locality)
Approval: MAIN-APPROVAL-V002.md (H2 stripe residency, tileElems pinned at 4096)
Parent: R31B-V011, PARENT_SOURCE_SHA=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3

Outcome: **NO_UB_BUDGET** — no useful gamma/bias stripe fits at tileElems=4096.
No kernel change. No compile. No correctness run. No device timing.

## Decision rule (from approval)

Implement H2 only with leftover UB at tileElems=4096 (parent's 1-tile io
layout). If no useful stripe fits, write NO_UB_BUDGET and stop. Do not
shrink tileElems.

## UB budget at tileElems=4096 (parent FP32 wide layout)

Parent Init wide-FP32 branch allocates (parent.asc:71-80):

| buffer | size at tileElems=4096 |
|---|---|
| xBuf_ | 4096 × 4 = 16384 B |
| residualBuf_ | 4096 × 4 = 16384 B |
| valueFp32Buf_ (full y rows) | rowWidth × rows × 4 |
| reduceFp32Buf_ | rows × 16 × 4 |

Budget: kWideFullYBudgetBytes = 176 KiB = 180224 B.
ChooseWideFullYRows(rowWidth, 4, ioTiles=2, 4, workTiles=0, tileElems) —
the io term is the xBuf_/residualBuf_ pair above.

Per-shape leftover (tileElems stays 4096 in every case below):

| D | rows chosen | y bytes | io bytes | reduce | need | leftover |
|---|---|---|---|---|---|---|
| 32768 | 1 | 131072 | 32768 | 64 | 163904 | **16320** |
| 16384 | 2 | 131072 | 32768 | 128 | 163968 | **16256** |
| 12288 | 2 | 98304 | 32768 | 128 | 131200 | 49024 |
| 10240 | 3 | 122880 | 32768 | 192 | 155840 | 24384 |
| 9216 | 3 | 110592 | 32768 | 192 | 143552 | 36672 |

Cost of one resident K-tile stripe (K tiles of gamma plus K tiles of bias,
FP32): K × 2 × 4096 × 4 = K × 32768 B.

At the two wide shapes the hypothesis targets:

- D=32768: leftover 16320 B < 32768 B (K=1 pair). Short by 16448 B.
  Even a gamma-only single tile (16384 B) misses by 64 B — exactly the
  reduce-partial bytes. A bias-only tile misses the same way.
- D=16384: leftover 16256 B < 32768 B. Short by 16512 B. Gamma-only
  single tile misses by 128 B.

D=12288 and D=9216 have room for K=1, but neither is a measured shape,
and at those widths wideFullYRows_ is 2–3 so the batch loop rarely
exceeds one iteration anyway. D≤8192 is not the wide path
(kCacheElems=8192 boundary); those cores already keep full gamma/bias.

Conclusion: at tileElems=4096 no K≥1 gamma/bias stripe fits at D=32768
or D=16384. Per the approval stop rule → NO_UB_BUDGET.

## Second blocker: the reuse never fires under the runner

The stripe only pays when the batch loop runs more than once per core
(localRows > batchLimit), so pass 2 can skip already-resident tiles.

Host launch (submission.asc run_kernel): blockCount =
min(availableCoreNum, rowCount), with availableCoreNum =
ACL_DEV_ATTR_VECTOR_CORE_NUM (the full device vector-core count).

| shape | rowCount | blockCount | localRows | batchLimit (D=32768 → 1) | batch iterations |
|---|---|---|---|---|---|
| 1×32768 | 1 | 1 | 1 | 1 | 1 |
| 2×16384 | 2 | 2 | 1 | 2 | 1 |
| 8×32768 | 8 | 8 | 1 | 1 | 1 |

Every measured shape gives localRows = 1 on every core. The batch loop
runs exactly once, so there is no cross-batch gamma/bias reload to skip.
Track-B's "8×32768 with 4 cores → localRows=2" scenario does not occur
on d4: the host clamps blockCount to rowCount before it can fall below
it. Even where leftover UB could hold a stripe (FP16 D=32768, leftover
≈ 32.7 KB, K=1 half-tile pair would fit), the expected delta under this
protocol is ≈ 0.

FP16-side stripe is also outside this approval: "No dtype split" is an
explicit constraint, and the hypothesis is scoped to
ProcessWideFp32FullCacheRows.

## What was and was not done

- Done: UB budget arithmetic from the parent's own Init allocation and
  ChooseWideFullYRows; batch-iteration count from the host launch rule.
- Not done (by stop rule): kernel edit, compile/link, correctness run,
  device timing, V002 submission.asc. Workspace submission.asc remains
  the committed V001 candidate (SOURCE_SHA 6e47a9a0ad50db5eef81f73911f846edd29f38d3d2222aaea868b10f4a8e59b4).
  No V002 source SHA exists.

## Implications for the route

1. Stripe residency cannot be had at tileElems=4096 on D=32768/16384
   without either shrinking tileElems (V001's failure mode, forbidden)
   or taking bytes from the y-row residency (a second factor).
2. Cross-batch param reload is structurally zero on the local runner's
   shapes regardless of UB: localRows=1 everywhere. H2's expected
   multi-batch win shapes need rowCount > vector-core count to exist at
   all; the current timing set never enters that regime.
3. Remaining COEFF-LOCALITY-X levers are thin. H1 (prefetch) regressed
   the primary shape. H2 has no UB and no reuse window. H3 (one-shot
   full-row param load on generic single-row multi-tile cores) is the
   last hypothesis in the Track-B list; its ceiling is small (generic
   path, D 4096–8192).

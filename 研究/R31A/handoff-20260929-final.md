# R31A Track-A 收口 — LANE_NEEDS_PLANNING_REVIEW

Date: 2026-09-29
Route: R31A (Champion Conservative Evolution), LANE M1-3
Worktree: `/Users/sunyiyang/Desktop/Project/cann-m1-r31a`, branch `m1/r31a-exploit`
Lane status: **`LANE_NEEDS_PLANNING_REVIEW`** (report + recommendation; **not** a PARK — the lane lifecycle decision belongs to the Planning / Review Layer)

---

## 1. Campaign summary

R31A is the Champion Conservative Evolution lane: single-variable OFAT revisions from the V016 champion starting point (`dd130938…`, Official 45.00). The campaign produced a **Local Best chain of four `LOCAL_ACCEPTED` revisions** spanning two complementary shape regimes.

### Local Best chain

| rev | hypothesis | site | shape regime | delta vs parent | verdict |
|---|---|---|---|---|---|
| V016 | (start) | — | — | — | OFFICIAL_BEST 45.00 |
| **V024** | H2 invRms row-scale Muls hoist | CachedRows pass 2 | D=32768 | **−3.66%** | LOCAL_ACCEPTED |
| **V025** | H4 pass-1 residual staging liveness | CachedRows pass 1 | D=32768 | **−4.12%** | LOCAL_ACCEPTED |
| **V026** | H5 pass-2 parameter staging liveness | CachedRows pass 2 | D=32768 | **−1.69%** | LOCAL_ACCEPTED |
| **V028** | H9 batch affine barrier removal | batch affine loop | D<32768 | **−1.09%** | LOCAL_ACCEPTED |

`LOCAL_BEST = V028` (source SHA `ee52831c435f07f51ea3917af0e7faed9a012f852ab19641882c401fb78d10f1`).

### Dual-domain coverage

- **CachedRows / D=32768 regime** (the champion path): V024 + V025 + V026 ≈ **−8.3% vs V016** (chain-estimated; the direct V016-referenced cross-check measured V025 at −6.8% and V026's incremental −1.69% is consistent).
- **batch / D<32768 regime**: V028 ≈ **−1.09% vs V026** (8/8 clean blocks, two devices).

V028 parents from V026, so **V028 is a strict superset of all local gains** and covers both regimes. It does not regress the CachedRows path (the D=32768 same-code control is within noise).

### Online candidate status

- **ONLINE_RECOMMENDATION = WORTHY**. V028 (the full chain) is R31A's final primary Online candidate, replacing V026. The package is ready under `线上结果/R31A/` (exact source SHA `ee52831c…`).
- Earlier packages for V024/V025/V026 are also prepared. Per Main's note, V028 supersedes them as the primary candidate (it carries the complete chain).
- I have **not** submitted Online. Submission is the unified Judge Owner's action after Planning's `ONLINE_DECISION`.

---

## 2. Information gained across the campaign

Each revision produced non-duplicate information even when it did not advance Local Best:

| rev | verdict | information gained |
|---|---|---|
| V021 | NEEDS_ONE_MORE_LOCAL | Path-dispatch fact: rows=2/D=24576 takes `ProcessWideFp32Batched`, not `ProcessWideFp32CachedRows` where V021's edit lived. Affects measurement interpretation of every CachedRows change. |
| V022 | LOCAL_REJECTED | H3 refuted at D=24576 (batch beats CachedRows there); noise band quantified; the gap=5s/d6 same-binary window recipe. |
| V023 | NEEDS_ONE_MORE_LOCAL | **`Rsqrt` is an approximate ~2^-10 primitive on 910B3** and breaks the 1e-4 FP32 budget. Transferable rule for every route's denominator work. |
| V024 | LOCAL_ACCEPTED | Per-tile `Muls` hoist is a real lever (−3.66%); barrier/pipe-stall removal in the value path pays. |
| V025 | LOCAL_ACCEPTED | Pass-1 staging liveness (free a slot early + prefetch) is a real lever (−4.12%). |
| V026 | LOCAL_ACCEPTED | Pass-2 staging liveness is also real (−1.69%), smaller because its overlap window is narrower. |
| V027 | NEEDS_ONE_MORE_LOCAL | H6 boundary overlap is a **small** lever (one overlap per row vs per tile); bounds the pass-boundary class as weak. |
| V028 | LOCAL_ACCEPTED | **`SetFlag<HardEvent::V_*>` 前的 `PipeBarrier` 冗余且安全可删** (ARCHITECTURE_FINDING, forwarded to other lanes). But each such barrier costs only ~0.01 µs — **barrier position matters more than count** (chain-middle barriers pay, chain-tail ones near the SetFlag are cheap). |

---

## 3. Research space exhausted

The assigned research space (CachedRows affine / path selection / local data flow, then the batch path's clean OFAT) is **exhausted**:

- **CachedRows affine/dataflow:** H1 (Rsqrt) rejected for precision; H2 (Muls hoist) done as V024; H3 (batch row threshold) refuted; H4 (pass-1 liveness) done as V025; H5 (pass-2 liveness) done as V026; H6 (boundary release) within noise as V027; H7 (pass-2 double-staging) moot after H5; H8 (cross-row gamma residency) INFEASIBLE (UB).
- **Batch path:** H9 (barrier removal) done as V028; H10 (RMS finalize onto reduce buffer, H6 analog) low expectation (H6 within noise) and not worth a revision; H11 (fold invRms into gamma) INFEASIBLE (UB ~1 KB free vs 24 KB needed, and overwriting gamma destroys parameter reuse).

**UB budget is the binding constraint** on both paths (CachedRows D=32768 ≈ 4 KB free; batch D=24576 ≈ 1 KB free). No new buffer pair is possible at the current tile width; every remaining idea would need a tile-policy change (SHAPE-TILING, out of scope) or would redo a covered mechanism.

---

## 4. LANE_NEEDS_PLANNING_REVIEW — recommendation

Per the campaign rules this lane is now flagged `LANE_NEEDS_PLANNING_REVIEW`. The facts for the Planning / Review Layer:

1. **Local Best chain is strong:** four `LOCAL_ACCEPTED` revisions (V024/V025/V026/V028), dual-domain coverage, combined ≈ −8.3% on D=32768 and ≈ −1.09% on D<32768 (batch). Each accepted result has two-device, noise-band-controlled, clean-block confirmation.
2. **V028 is the Online candidate** (`ONLINE_RECOMMENDATION = WORTHY`, package ready, exact source SHA `ee52831c…`). It is a strict superset of the local gains (parents from V026).
3. **The assigned research space is exhausted** (CachedRows affine/dataflow + batch clean OFAT). The remaining ideas (H10) have low expectation and the rest are UB-blocked.
4. **Budget:** 8 closed-loop revisions this campaign (V021–V028), of which 4 `LOCAL_ACCEPTED`, 4 information-gaining non-accepted. The soft budget of 6 is passed but the hard budget of 10 is not reached; the credible-gain chain justifies the spend.

**Recommendation to Planning:** decide between
- **(a) push Online** with V028 (the full chain, dual-domain) — the local evidence is strong and the research space is spent, or
- **(b) reassign / close** the lane if the marginal local gain (H10, expected within noise) is not worth another revision.

**This document does not PARK the lane.** The `PARK / CLOSE / MERGE / REPLACE` decision is exclusively the Planning / Review Layer's. The lane is reported as `LANE_NEEDS_PLANNING_REVIEW` and awaits that decision.

---

## 5. What is NOT done (boundaries held)

- **No Online submission** — the Judge Owner acts only after Planning's `ONLINE_DECISION`.
- **No PARK / CLOSE / MERGE / REPLACE** — the lane lifecycle is the Planning Layer's call.
- **Shared ledgers not modified** (`技术路线/*.tsv`, `调度/*`) — MAIN-1 owns them. The records needing MAIN-1 sync are:
  - `技术路线/路线成绩表.tsv` / `技术路线/全版本记录.tsv`: the `LOCAL_BEST=V028` advance, the full Local Best chain, and the V021–V028 verdicts.
  - `调度/服务器设备使用.tsv`: the 2026-09-28/29 device-run lease rows for all measured revisions.
  - `调度/线上候选.tsv` / Online records: V028 as the final Online candidate.

---

## 6. Evidence index

All evidence is retained under the per-revision directories (nothing deleted):

| rev | evidence |
|---|---|
| V021 | `本地实验/R31A/V021/` (full package + the D24576-path-mismatch finding) |
| V022 | `本地实验/R31A/V022/` (H3 refuted; noise band; 3-row batch control) |
| V023 | `本地实验/R31A/V023/` (Rsqrt precision failure + fix) |
| V024 | `本地实验/R31A/V024/` (Muls hoist; LOCAL_ACCEPTED) |
| V025 | `本地实验/R31A/V025/` (pass-1 liveness; LOCAL_ACCEPTED) |
| V026 | `本地实验/R31A/V026/` (pass-2 liveness; LOCAL_ACCEPTED) |
| V027 | `本地实验/R31A/V027/` (boundary release; NEEDS_ONE_MORE_LOCAL) |
| V028 | `本地实验/R31A/V028/` (barrier removal; LOCAL_ACCEPTED + ARCHITECTURE_FINDING) |

Research: `研究/R31A/next-hypotheses.md` (round 1), `next-hypotheses-v2.md`, `next-hypotheses-v3.md`, and the per-revision handoffs `handoff-20260929*.md`.

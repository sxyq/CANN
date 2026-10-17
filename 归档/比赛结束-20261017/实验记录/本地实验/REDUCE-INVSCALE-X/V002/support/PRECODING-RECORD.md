# REDUCE-INVSCALE-X V001 — pre-coding record

Recorded BEFORE any coding, per route protocol.

```text
ROUTE             : REDUCE-INVSCALE-X
REVISION          : V001
CONTEXT_CLASS     : HISTORICAL_DERIVED
DIRECT_PARENT     : FULL-R006-V001-REDUCTION-ARCH
PARENT_SOURCE     : 归档/phase3-before-reset-20260920/实验/online/upstream-2026-09-18/sources/FULL-R006-V001-REDUCTION-ARCH_kernel.txt
PARENT_SOURCE_SHA : 94ab0ef96a1a907fa187797b6c72361b2bdacfe6faf7a95d6471bd537a913266
PARENT_LINES      : 865
PARENT_SCORE      : LOCAL_PARENT
PARENT_FORMAL     : Correctness Fail 1/15, official_score null
                   (归档/.../实验/online/upstream-2026-09-18/results/FULL-R006-V001_RESULT.json,
                    submission recorded; formal score unusable, so the LOCAL_PARENT reference is
                    established by this route on server3 before any delta is claimed)
SINGLE_HYPOTHESIS : From the R006 reduction-architecture parent, replace ONLY the
                    post-reduction normalization (B0-style Duplicate + Div) with the
                    R019-style scalar reciprocal + vector Muls. Nothing else changes.
DONOR_R006        : FULL-R006-V001-REDUCTION-ARCH (partials-collector reduction;
                    header lists "reciprocal/Muls normalization replacement" under
                    "Explicitly absent"; code comment: "Keep B0-style division rather
                    than importing R019's reciprocal normalization route")
DONOR_R019        : FULL-R019-NORMALIZATION L003, sha256
                    ce7be90cccd4e08cc24b44e3701bb5b598e043dd8c6bd0a1c0ca6fd2faec4954,
                    online Pass 15/15 official 18.68; also R019-V001 external note
                    "标量求倒数 + 向量 Muls 取代 Duplicate+Div"
```

## Why this parent is legal

- It is a recorded, archived, formally submitted source inside this repository
  (`实验/online/upstream-2026-09-18/sources/` + `results/FULL-R006-V001_RESULT.json`
  with submission record), i.e. provenance is verifiable byte-for-byte.
- It is the only R006 reduction-architecture source whose post-reduction
  normalization is NOT already invscale, so the single change in this revision
  is real rather than a no-op.
- It is not the external-track `候选-R006-V001-PARTIALS` candidate
  (`归档/.../提交/外部轨道-ChatGPT编译并修复/源码/候选-R006-V001-PARTIALS/`),
  which already claims R006 partials + R019 normalization but is explicitly marked
  "未经本仓库服务器复核" (never verified by this repository) and was never submitted.
- It is not `online-independent/FULL-R006-REDUCTION-ARCH/I001` (15/15, 23.53) or
  `D001`/`D003`, because those already compute `1.0f / rms` on the scalar side and
  apply it with vector `Muls`; adding "R019-style scalar reciprocal + vector Muls"
  on top of them would be a no-op and could not test orthogonality.

## WHY_THIS_COMBINATION_IS_NEW

R006 organization and R019 invscale were historically separate, and this repo's
verified evidence never stacked them:

| source | reduction organization | post-reduction normalization | verified here |
|---|---|---|---|
| FULL-R006-V001-REDUCTION-ARCH | R006 partials collector + one final vector ReduceSum | B0-style `Duplicate` + `Div` | yes, formal submission (Correctness Fail 1/15) |
| FULL-R019-NORMALIZATION L003 | R013 double-buffer pipeline (not R006) | scalar reciprocal + vector `Muls` (folded into gamma) | yes, Pass 15/15, 18.68 |
| FULL-R006-REDUCTION-ARCH I001 / D003 | R006 chunked ReduceSum with scalar accumulator | already scalar reciprocal + vector `Muls` | yes, Pass 15/15, 23.53 / 23.80 |
| 候选-R006-V001-PARTIALS (external track) | R006 partials | R019 reciprocal + Muls | **no** — "未经本仓库服务器复核", never submitted |

No source in this repository combines the **R006 partials-collector reduction
organization** with the **R019 scalar-reciprocal + vector-Muls normalization** and
has been compiled, run on NPU, and probed here. R006's own header names that exact
piece as the one thing it deliberately left out. This revision is that combination.

## WHY_NOT_DUPLICATE

1. Not a duplicate of R019 L003: L003's reduction is the R013 ping-pong double-buffer
   pipeline with a per-tile `ReduceSum` and no partials collector. This route keeps
   the R006 partials collector untouched and changes only the normalization.
2. Not a duplicate of FULL-R006-V001: that source explicitly keeps `Duplicate` + `Div`
   and lists reciprocal/Muls as absent. The whole point of V001 is to add it.
3. Not a duplicate of R006 I001/D003: those already use scalar reciprocal + `Muls`,
   so there is no R019 change left to add; this route does not start from them.
4. Not a duplicate of external-track `候选-R006-V001-PARTIALS`: that file was never
   verified, compiled, or submitted by this repository and is not an admissible
   parent; its existence is recorded here as prior art, not as this route's source.

## Change budget (OFAT)

Exactly one conceptual change: post-reduction normalization.

Untouched by construction (they remain byte-identical to the parent):

- reduction tile size (`kReduceTileElems = 6144`)
- partials collector (`kPartialCapacity = 64`) and `CollapsePartials`
- `ReduceTileToPartial` and the per-tile `ReduceSum`
- pipeline / event sync primitives (`SyncMte2ToVector`, `SyncVectorToMte2`, ...)
- row scheduling (round-robin `row += coreNum`, non-aligned rows → 1 core)
- parameter residency (gamma/bias still reloaded tile-by-tile)
- host launch, dtype dispatch, validation, UB layout

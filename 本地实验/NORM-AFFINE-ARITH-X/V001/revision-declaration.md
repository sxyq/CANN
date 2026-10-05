# NORM-AFFINE-ARITH-X V001 — Revision Declaration

- ROUTE: `NORM-AFFINE-ARITH-X`
- REVISION: `V001`
- DIRECT_PARENT: exact `R31B V011`
- PARENT_SOURCE_SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- PARENT_SCORE / OFFICIAL_ANCHOR: `45.16`
- SINGLE_HYPOTHESIS: In the FP32 branch of `ProcessNarrowMidOverlap` only, reverse the two independent affine multiplies so vector gamma `Mul` precedes scalar `Muls(invRms)`, keeping the existing bias `Add`. This tests arithmetic ordering in the post-reduction affine chain without changing operation count or data movement.
- CONTEXT_CLASS: `ROUTE_STUDIO_OFAT`
- WHY_NOT_DUPLICATE: Existing parent path applies `Muls(invRms)`, vector `Mul(gamma)`, then vector `Add(bias)`. This changes only the arithmetic factorization in the mid-size single-tile function; it does not alter reduction, row/core ownership, scheduling, or UB layout. No prior Route-local revision exists.
- EDIT_SCOPE: FP32 branch of `ProcessNarrowMidOverlap` epilogue only; all other code remains byte-identical to parent.
- PLANNED_ORDER: `ONE CHANGE -> COMPILE -> CORRECTNESS -> LOCAL -> RESULT -> COMMIT`
- ONLINE: prohibited for this agent.
- DECLARED_AT_UTC: 2026-10-05

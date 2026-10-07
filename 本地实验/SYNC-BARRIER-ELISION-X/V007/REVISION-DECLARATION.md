# SYNC-BARRIER-ELISION-X V007

- ROUTE: SYNC-BARRIER-ELISION-X
- REVISION: V007
- DIRECT_PARENT: R31B-V011
- PARENT_SOURCE_SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- PARENT_SCORE: Official 45.16; this is not the Local score scale
- CURRENT_LOCAL_BEST: R31B-V011
- SINGLE_HYPOTHESIS: Remove the single `PIPE_V` barrier between the square `Mul` and `ReduceSum` in `ProcessNarrowMidOverlap`.
- SINGLE_CHANGE_BOUNDARY: One synchronization-operation deletion only.
- WHY_NOT_DUPLICATE: V006 removed a different `PIPE_V` barrier after `Muls`; V007 tests only the barrier immediately before the tile `ReduceSum`.
- V006_LOCAL_RESULT_AT: `2026-10-07T05:38:37Z`
- V007_SOURCE_EDIT_AT: `2026-10-07T05:58:43Z`
- LOCAL_RESULT_TO_NEXT_EDIT_SECONDS: `1206` (MISS against the 180-second SLA)
- ONLINE_CANDIDATE: NO

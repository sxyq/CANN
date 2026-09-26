# REVISION DECLARATION — REDUCE-HIER-X

ROUTE=REDUCE-HIER-X
REVISION=V001
DIRECT_PARENT=FROZEN_R31B_V011
PARENT_SOURCE_SHA=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
OFFICIAL_ANCHOR=45.16
CURRENT_LOCAL_BEST=NONE
SINGLE_HYPOTHESIS=HYPOTHESIS-1 Eager running-fold of tile partials (partial-sum lifetime): each tile ReduceSum folds immediately into one FP32 accumulator via 1-element Adds; remove end-of-row collapse ReduceSum; GetValue unchanged. Do not change per-tile ReduceSum width, square Mul, mean/epsilon tail, output pass, scheduling, dtype, DMA.
CONTEXT_CLASS=FROZEN_STRONG_BASELINE
WHY_NOT_DUPLICATE_MAIN1=NON_OVERLAP_MATRIX in phase4/control/main2-r2-route-registry.md
WHY_NOT_DUPLICATE_MAIN2=parent is frozen R31B-V011; old routes are evidence donors only
MAIN_APPROVAL=YES
APPROVAL_DOC=phase4/research/REDUCE-HIER-X/MAIN-APPROVAL-V001.md
SINGLE_CHANGE_AUDIT=PENDING_IMPLEMENTATION

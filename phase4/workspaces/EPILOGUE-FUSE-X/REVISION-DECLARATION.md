# REVISION DECLARATION — EPILOGUE-FUSE-X

ROUTE=EPILOGUE-FUSE-X
REVISION=V001
DIRECT_PARENT=FROZEN_R31B_V011
PARENT_SOURCE_SHA=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
OFFICIAL_ANCHOR=45.16
CURRENT_LOCAL_BEST=NONE
SINGLE_HYPOTHESIS=HYPOTHESIS-1 SCALE-FOLD: replace (Muls value invRms) then (Mul value gamma) with (Muls scale gamma invRms) then (Mul value scale); Add bias unchanged. One arithmetic identity (y*a)*b -> y*(a*b). Uniform across T; FP32 intermediates; no dtype split; no sync change; no reduction/scheduling/mode change.
CONTEXT_CLASS=FROZEN_STRONG_BASELINE
WHY_NOT_DUPLICATE_MAIN1=NON_OVERLAP_MATRIX in phase4/control/main2-r2-route-registry.md
WHY_NOT_DUPLICATE_MAIN2=parent is frozen R31B-V011; old routes are evidence donors only
MAIN_APPROVAL=YES
APPROVAL_DOC=phase4/research/EPILOGUE-FUSE-X/MAIN-APPROVAL-V001.md
SINGLE_CHANGE_AUDIT=PASS (one mechanism: ApplyScaleFold normalize-into-gamma; half-domain arms and broadcast-batch arms and wide paths untouched; barrier count/placement unchanged)

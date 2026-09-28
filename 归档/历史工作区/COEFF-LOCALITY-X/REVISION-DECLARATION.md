# REVISION DECLARATION — COEFF-LOCALITY-X

ROUTE=COEFF-LOCALITY-X
REVISION=V001
DIRECT_PARENT=FROZEN_R31B_V011
PARENT_SOURCE_SHA=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
OFFICIAL_ANCHOR=45.16
CURRENT_LOCAL_BEST=NONE
SINGLE_HYPOTHESIS=H1 param double-buffer prefetch in FP32 wide output pass (lowp sibling pattern)
CONTEXT_CLASS=FROZEN_STRONG_BASELINE
WHY_NOT_DUPLICATE_MAIN1=coeff load timing only; no multi-row DMA / mode / wide redesign / dtype path
WHY_NOT_DUPLICATE_MAIN2=no other route owns gamma/bias MTE2 residency in FP32 wide output pass
MAIN_APPROVAL=YES
APPROVAL_DOC=phase4/research/COEFF-LOCALITY-X/MAIN-APPROVAL-V001.md
SINGLE_CHANGE_AUDIT=PASS (one mechanism: 2-deep param MTE2 prefetch; UB-budget tileElems adaptation is the automatic consequence)

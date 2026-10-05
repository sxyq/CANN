# DTYPE-LOWP-FASTPATH-X V001

ROUTE=DTYPE-LOWP-FASTPATH-X
REVISION=V001
DIRECT_PARENT=R31B-V011
PARENT_SOURCE_PATH=线上结果/R31B/V011/submission.asc
PARENT_SOURCE_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
PARENT_OFFICIAL_SCORE=45.16
CONTEXT_CLASS=FROZEN_STRONG_BASELINE_TRANSPLANT
SINGLE_HYPOTHESIS=Remove the duplicate BF16 wide-path gamma/bias conversion pair in pass two.
ALLOWED_AXIS=FP16/BF16 conversion count and cast placement only.
TARGET_SHAPES=Medium/wide BF16 path; no tiny kernel and no core ownership change.
WHY_NOT_DUPLICATE=The edit targets the BF16-only wide pass-two duplicate conversion pair in this route; it does not change the FP16 native path, FP32 path, scheduling, ownership, or store mechanism.
ONLINE=FORBIDDEN
SINGLE_CHANGE_AUDIT=PASS

Declaration precedes Candidate source editing. The parent is the retained exact Git/source evidence in
`线上结果/R31B/V011/`, not another Candidate revision.

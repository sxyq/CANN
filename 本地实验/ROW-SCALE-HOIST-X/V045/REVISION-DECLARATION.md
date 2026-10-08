ROUTE=ROW-SCALE-HOIST-X
REVISION=V045
DIRECT_PARENT=V026
PARENT_SOURCE=本地实验/ROW-SCALE-HOIST-X/V026/submission.asc
PARENT_SOURCE_SHA256=7c2569d9e3830c845f2b5dc4587a73a05a0bdf041168614cd55ad84204e063f9
SINGLE_HYPOTHESIS=In the FP32 one-row ProcessNarrowMidOverlap path, apply the existing invRms Muls to the freshly loaded gamma tile before multiplying the output tile by gamma.
FOCUS=FP32 row-scale operand placement in ProcessNarrowMidOverlap when localRows==1 only
TARGET_BRANCH=ProcessNarrowMidOverlap FP32 branch; residentParams=false
TARGET_SHAPE=FP32 [40,2050], 40 vector cores, one row/core
OFAT_BASE=V026; V045 is a sibling, not a continuation of V044
WHY_NOT_DUPLICATE=V020/V029 alter scale placement/order on the output-value operand; V042 moves that output-value scale relative to the parameter-ready wait. V039 applies scale to gamma in the separate FP32 full-row consumer. V043/V044 apply gamma-operand scale placement in the narrow-mid BF16/FP16 consumers. No prior revision applies the existing FP32 scale to gammaLocal in the FP32 narrow-mid one-row branch.
UNCHANGED=FP32 dtype and arithmetic precision; invRms formula/value; one scale Muls and one gamma Mul; input and parameter DMA; waits/events; reduction; dispatch and row tiling; bias addition; resident multi-row path
EXCLUDED=Reduction/math redesign; dtype or precision changes; scale extent changes; scheduling/tiling changes; other consumers; Online; shared-record writes; other worktrees
COMPILE=PASS; device and submission targets
CORRECTNESS=PASS; Parent and Candidate FP32 [40,2050], matched_ratio=1.0
LOCAL=MEASUREMENT_BLOCKED; descriptive score=92.6170800932; Candidate +7.9714453310% slower; high CV, block drift, and mixed paired direction
CURRENT_LOCAL_BEST=V026
OFFICIAL_SCORE=NONE
ONLINE=NOT_RUN

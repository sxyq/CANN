ROUTE=ROW-SCALE-HOIST-X
REVISION=V044
DIRECT_PARENT=V026
PARENT_SOURCE=本地实验/ROW-SCALE-HOIST-X/V026/submission.asc
PARENT_SOURCE_SHA256=7c2569d9e3830c845f2b5dc4587a73a05a0bdf041168614cd55ad84204e063f9
SINGLE_HYPOTHESIS=In the FP16 one-row ProcessNarrowMidOverlap case, apply the existing half invRms Muls in place to the per-row gamma tile before multiplying the output tile by gamma.
FOCUS=FP16 row-scale operand placement in ProcessNarrowMidOverlap when localRows==1 only
TARGET_BRANCH=ProcessNarrowMidOverlap FP16 branch; residentParams=false
TARGET_SHAPE=FP16 [40,3072], 40 vector cores, one row/core
OFAT_BASE=V026; V044 is a sibling, not a continuation of V043
WHY_NOT_DUPLICATE=V021 moves scale across FromFloat in the same consumer; V035 changes the order of output-tile scale and gamma multiplies in the multi-row resident-parameter case. V044 instead scales the freshly loaded gamma operand in-place for localRows==1. No earlier revision moves scale onto gamma in this FP16 consumer. V039/V043 operand placement targets different FP32/BF16 consumers.
UNCHANGED=FP16 dtype and rounding at each existing op; invRms formula/value; one scale Muls and one gamma Mul; dispatch, row tiling, buffers, events, synchronization, conversion boundary, bias addition
EXCLUDED=Multi-row resident gamma cache; reduction/math redesign; dtype or precision changes; scheduling/tiling changes; other consumers; Online; shared-record writes; other worktrees
CURRENT_LOCAL_BEST=V026
COMPILE=PASS; device and submission targets
CORRECTNESS=PASS; FP16 [40,3072], Parent and Candidate, matched_ratio=1.0
LOCAL=MEASUREMENT_BLOCKED; descriptive score=82.6086976256; Candidate +21.0526286871% slower; Parent stability drift/jitter prevents reliable classification
RESULT=RECORDED; V026 remains CURRENT_LOCAL_BEST
OFFICIAL_SCORE=NONE
ONLINE=NOT_RUN

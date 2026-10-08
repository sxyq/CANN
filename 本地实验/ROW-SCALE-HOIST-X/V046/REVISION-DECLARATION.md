ROUTE=ROW-SCALE-HOIST-X
REVISION=V046
DIRECT_PARENT=V026
PARENT_SOURCE=本地实验/ROW-SCALE-HOIST-X/V026/submission.asc
PARENT_SOURCE_SHA256=7c2569d9e3830c845f2b5dc4587a73a05a0bdf041168614cd55ad84204e063f9
SINGLE_HYPOTHESIS=In the FP32 resident multi-row ProcessNarrowMidOverlap path, apply the existing invRms Muls to an FP32 scratch copy of cached gammaLocal, then multiply valueLocal by that scaled scratch; leave cached gammaLocal unchanged for subsequent rows.
FOCUS=FP32 row-scale operand placement in ProcessNarrowMidOverlap when residentParams=true only
TARGET_BRANCH=ProcessNarrowMidOverlap FP32 branch; residentParams=true
TARGET_SHAPE=FP32 [128,2050], 40 vector cores, 3-4 rows/core
OFAT_BASE=V026; V046 is a sibling, not a continuation of V045
WHY_NOT_DUPLICATE=V020/V029 change value-operand order; V039 targets ProcessFp32FullRowOutputPipelined; V042 changes scale placement relative to the parameter-ready wait; V043 applies gamma-operand scale in the BF16 narrow-mid branch; V044 and V045 apply it only in one-row FP16/FP32 branches. No prior revision scales a gamma scratch in the FP32 resident multi-row narrow-mid branch.
UNCHANGED=FP32 dtype and operation precision; invRms formula/value; one scale Muls and one value/gamma Mul; cached gamma and bias DMA; reduction; dispatch and row tiling; buffers, event IDs, waits, stores, bias addition, and all other dtype/path behavior
EXCLUDED=Reduction/math redesign; dtype or precision changes; scale extent changes; scheduling/tiling changes; other consumers; Online; shared-record writes; other worktrees
CURRENT_LOCAL_BEST=V026
COMPILE=PASS; device and submission targets; compile-local-20261008T0126Z.log
CORRECTNESS=PASS; Parent and Candidate FP32 [128,2050], matched_ratio=1.0; correctness-parent-20261008T0135Z.log and correctness-candidate-20261008T0135Z.log
LOCAL=MEASUREMENT_BLOCKED; descriptive score=95.0260266050; Candidate +5.2343274498% slower; high jitter/block variance and mixed paired direction
RESULT=RECORDED; V026 remains CURRENT_LOCAL_BEST
OFFICIAL_SCORE=NONE
ONLINE=NOT_RUN

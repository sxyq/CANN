ROUTE=ROW-SCALE-HOIST-X
REVISION=V039
DIRECT_PARENT=V026
PARENT_SOURCE=本地实验/ROW-SCALE-HOIST-X/V026/submission.asc
PARENT_SOURCE_SHA256=7c2569d9e3830c845f2b5dc4587a73a05a0bdf041168614cd55ad84204e063f9
SINGLE_HYPOTHESIS=In ProcessFp32FullRowOutputPipelined, apply the existing per-row invRms to an FP32 scratch copy of each gamma tile, then multiply the value tile by that scaled gamma instead of applying invRms to the value tile.
FOCUS=FP32 full-row row-scale operand placement only
TARGET_BRANCH=ProcessFp32FullRowOutputPipelined
TARGET_SHAPE=FP32 [128,8192]
OFAT_BASE=V026; V032-V038 remain independent probes and are not parents
WHY_NOT_DUPLICATE=V037 changes row-scale extent while retaining scale-before-gamma. V038 swaps the order of the value-tile gamma and scale operations. V039 leaves the cached gamma untouched and changes only the operand receiving the existing invRms scale, using the existing xFp32 scratch after scalar extraction.
UNCHANGED=FP32 dtype and precision; per-tile extent; inverse-RMS calculation; output loop, dispatch, buffers, events, synchronization, and all other paths; bias remains after gamma-scaled value
EXCLUDED=Reduction/math redesign; dtype changes; scale extent changes; scheduling/tiling changes; Online; shared-record writes; other worktrees
CURRENT_LOCAL_BEST=V026
COMPILE=PASS; device and submission targets; compile-local-20261007T2240Z.log
CORRECTNESS=PASS; parent and candidate FP32 [128,8192], matched_ratio=1.0; correctness-local-20261007T2240Z.log
LOCAL=MEASUREMENT_BLOCKED; descriptive score=90.2999979850; candidate +10.7419736782% slower; local-result.json
LOCAL_POOLED_MEDIANS_US=PARENT 18.0600005; CANDIDATE 20.000001
CURRENT_LOCAL_BEST=V026
OFFICIAL_SCORE=NONE
ONLINE=NOT_RUN

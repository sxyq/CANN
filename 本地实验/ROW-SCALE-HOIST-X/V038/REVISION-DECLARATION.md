ROUTE=ROW-SCALE-HOIST-X
REVISION=V038
DIRECT_PARENT=V026
PARENT_SOURCE=本地实验/ROW-SCALE-HOIST-X/V026/submission.asc
PARENT_SOURCE_SHA256=7c2569d9e3830c845f2b5dc4587a73a05a0bdf041168614cd55ad84204e063f9
SINGLE_HYPOTHESIS=In ProcessFp32FullRowOutputPipelined, move the existing per-tile FP32 row-scale Muls from before gamma Mul to after gamma Mul and before bias Add.
FOCUS=FP32 full-row output row-scale ordering only
TARGET_BRANCH=ProcessFp32FullRowOutputPipelined
TARGET_SHAPE=FP32 [128,8192]
OFAT_BASE=V026; V032-V037 remain independent probes and are not parents
WHY_NOT_DUPLICATE=V014/V015/V017/V027/V028/V029 target other consumers; V025 targets BF16 full-row. V037 changes only scale extent from two tiles to the full row while preserving scale-before-gamma. No prior revision changes the per-tile scale/gamma order in ProcessFp32FullRowOutputPipelined.
UNCHANGED=FP32 dtype and precision; per-tile scale extent; reduction and inverse-RMS formula; output loop, dispatch, buffers, events, synchronization, and all other paths; bias remains after scale and gamma
EXCLUDED=Rsqrt/math redesign; reduction changes; dtype changes; scheduling or tiling changes; scale extent changes; Online; shared-record writes; other worktrees
CURRENT_LOCAL_BEST=V026
COMPILE=PASS; device and submission targets; compile-local-20261007T2222Z.log
CORRECTNESS=PASS; parent and candidate FP32 [128,8192], matched_ratio=1.0; correctness-local-20261007T2222Z.log
LOCAL=MEASUREMENT_BLOCKED; descriptive score=101.3937282230; candidate -1.3745704467% faster by pooled medians but direction/stability inconsistent; local-result.json
LOCAL_POOLED_MEDIANS_US=PARENT 20.370000; CANDIDATE 20.090000
CURRENT_LOCAL_BEST=V026
OFFICIAL_SCORE=NONE
ONLINE=NOT_RUN

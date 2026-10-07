ROUTE=ROW-SCALE-HOIST-X
REVISION=V040
DIRECT_PARENT=V026
PARENT_SOURCE=本地实验/ROW-SCALE-HOIST-X/V026/submission.asc
PARENT_SOURCE_SHA256=7c2569d9e3830c845f2b5dc4587a73a05a0bdf041168614cd55ad84204e063f9
SINGLE_HYPOTHESIS=In ProcessSmallFp32FullTileBatched, move the existing per-row FP32 row-scale Muls from before gamma Mul to after gamma Mul and before bias Add.
FOCUS=FP32 small full-tile row-scale placement/order only
TARGET_BRANCH=ProcessSmallFp32FullTileBatched
TARGET_SHAPE=FP32 [128,4096]
OFAT_BASE=V026; V032-V039 remain independent probes and are not parents
WHY_NOT_DUPLICATE=V027 covers ProcessSmallFp32Batched rows outside its full-tile early-return helper; V028 covers ProcessSmallFp32ContiguousBatched; V029's exact ProcessSmallFp32FullTileBatched epilogue retains the V026 scale-before-gamma order. V037-V039 target ProcessFp32FullRowOutputPipelined, not this 4096-wide consumer.
UNCHANGED=FP32 dtype and precision; row-scale scalar and formula; reduction; tile and batch extents; row/block dispatch; buffers, events, synchronization, and stores; bias remains after gamma and scale
EXCLUDED=Reduction/math redesign; dtype changes; scale extent changes; scheduling/tiling changes; other consumers; Online; shared-record writes; other worktrees
CURRENT_LOCAL_BEST=V026
COMPILE=PASS; device and submission targets; compile-local-20261007T230043Z.log
CORRECTNESS=PASS; Parent and Candidate FP32 [128,4096], matched_ratio=1.0; correctness-local-20261007T230215Z.log
LOCAL=MEASUREMENT_BLOCKED; descriptive score=99.3211540293; candidate +0.6834857864% slower; mixed paired direction and high jitter; local-result.json
LOCAL_POOLED_MEDIANS_US=PARENT 19.0200005; CANDIDATE 19.1499995
CURRENT_LOCAL_BEST=V026
OFFICIAL_SCORE=NONE
ONLINE=NOT_RUN
OFFICIAL_SCORE=NONE
ONLINE=NOT_RUN

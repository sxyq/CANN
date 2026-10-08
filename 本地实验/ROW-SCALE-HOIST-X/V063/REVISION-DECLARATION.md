ROUTE=ROW-SCALE-HOIST-X
REVISION=V063
DIRECT_PARENT=V026
CURRENT_LOCAL_BEST=V026
PARENT_SOURCE=本地实验/ROW-SCALE-HOIST-X/V026/submission.asc
PARENT_SOURCE_SHA256=7c2569d9e3830c845f2b5dc4587a73a05a0bdf041168614cd55ad84204e063f9
SINGLE_HYPOTHESIS=In ProcessSmallFp32ContiguousBatched, only for rowWidth <= kFp32RepeatMaxWidth, move the existing per-row FP32 invRms multiply from before gamma multiplication to after gamma multiplication and before bias.
FOCUS=row-scale/hoist placement only
TARGET_BRANCH=ProcessSmallFp32ContiguousBatched narrow branch
TARGET_SHAPE=FP32 [128,128], 40 vector cores, local rows 3-4
OFAT_BASE=V026; V061 and V062 are preserved siblings and are not parents
WHY_NOT_DUPLICATE=The recorded contiguous FP32 order probe V028 targets the wide branch; V051 moves invRms onto a gamma scratch in the narrow branch. V063 changes only the value-operand order in the narrow contiguous FP32 branch and does not change the operand, dtype, dispatch, buffers, synchronization, or store path.
UNCHANGED=rsqrt and reduction; dtype policy; dispatch predicates; core ownership; UB/buffer allocation; DMA; event/synchronization structure; bias operation; all other dtypes and consumers; other Routes
CORRECTNESS_GATE=Parent and Candidate FP32 [128,128] must pass before Local
LOCAL_PROTOCOL=device-event timing, 45 warmups, 32 samples per invocation, Parent stability plus four interleaved P/C blocks; retain all raw samples and load/process snapshots
ONLINE=FORBIDDEN
SHARED_RECORDS=NOT_MODIFIED

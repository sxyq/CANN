ROUTE=ROW-SCALE-HOIST-X
REVISION=V064
DIRECT_PARENT=V026
CURRENT_LOCAL_BEST=V026
PARENT_SOURCE=本地实验/ROW-SCALE-HOIST-X/V026/submission.asc
PARENT_SOURCE_SHA256=7c2569d9e3830c845f2b5dc4587a73a05a0bdf041168614cd55ad84204e063f9
SINGLE_HYPOTHESIS=In ProcessSmallFp32ContiguousBatched, only for rowWidth <= kFp32RepeatMaxWidth, move the existing per-row FP32 invRms multiply from between gamma and bias to after bias.
FOCUS=row-scale/row-level scale hoist placement only
FOCUS_AXIS=row-scale/hoist placement only
FOCUS_VALUE=FP32 contiguous-batched narrow branch, width=128, gamma -> bias -> invRms
TARGET_BRANCH=ProcessSmallFp32ContiguousBatched narrow branch
TARGET_SHAPE=FP32 [128,128], 40 vector cores, local rows 3-4
OFAT_BASE=V026; V061, V062, and V063 are preserved siblings and are not parents
UNCHANGED=rsqrt and reduction; dtype policy; dispatch predicates; core ownership; UB/buffer allocation; DMA; event/synchronization structure; all other dtypes and consumers; other Routes
CORRECTNESS_GATE=Parent and Candidate FP32 [128,128] must pass before Local
LOCAL_PROTOCOL=device-event timing, 45 warmups, 32 samples per invocation, Parent stability plus four interleaved P/C blocks; retain all raw samples and load/process snapshots
ONLINE=FORBIDDEN
SHARED_RECORDS=NOT_MODIFIED

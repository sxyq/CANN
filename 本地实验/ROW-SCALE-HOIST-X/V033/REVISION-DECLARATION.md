ROUTE=ROW-SCALE-HOIST-X
REVISION=V033
DIRECT_PARENT=V026
PARENT_SOURCE=compile/src/parent_submission.asc
PARENT_SOURCE_SHA256=7c2569d9e3830c845f2b5dc4587a73a05a0bdf041168614cd55ad84204e063f9
SINGLE_HYPOTHESIS=In ProcessFp16FullRowOutputPipelined, keep FromFloat and bias placement unchanged, but move the existing native-half row-scale Muls before the existing native-half gamma Mul in each output tile.
FOCUS=FP16 full-row row-scale placement/consumption ordering only
TARGET_BRANCH=ProcessFp16FullRowOutputPipelined
TARGET_SHAPE=FP16 [128,8192]
OFAT_BASE=V026; V031 and V032 are not the parent
WHY_NOT_DUPLICATE=V026 applies row-scale after gamma in this full-row function; V030/V032 apply FP32 row-scale before output conversion; V031 changes arithmetic precision and is out of scope. V008's intermediate order is in ProcessWideFp16CachedRows, not this target function. No previous declaration applies this single ordering swap to ProcessFp16FullRowOutputPipelined.
UNCHANGED=Reduction and inverse-RMS calculation; data type and FromFloat conversion; row/block dispatch; tile loop and synchronization; bias placement; all non-target branches
EXCLUDED=Rsqrt/math redesign; reduction changes; dtype changes; scheduling changes; Online; shared-record writes
COMPILE=PASS; local host hwnput3; device and submission targets; compile-local-20261007T1056Z.log
CORRECTNESS=PASS; parent and candidate FP16 [128,8192], matched_ratio=1.0; correctness-local-20261007T1101Z.log
LOCAL=MEASUREMENT_BLOCKED; descriptive score=94.9924127466; candidate +5.2715654952% slower; parent stability failed; local-result.json
CURRENT_LOCAL_BEST=V026
OFFICIAL_SCORE=NONE
ONLINE=NOT_RUN

ROUTE=ROW-SCALE-HOIST-X
REVISION=V034
DIRECT_PARENT=V026
PARENT_SOURCE=compile/src/parent_submission.asc
PARENT_SOURCE_SHA256=7c2569d9e3830c845f2b5dc4587a73a05a0bdf041168614cd55ad84204e063f9
SINGLE_HYPOTHESIS=In ProcessFp16FullTileBatchedOutputPipelined, keep FromFloat and bias placement unchanged, but move the existing native-half row-scale Muls before the existing native-half gamma Mul in each output row.
FOCUS=FP16 full-tile row-scale placement/consumption ordering only
TARGET_BRANCH=ProcessFp16FullTileBatchedOutputPipelined
TARGET_SHAPE=FP16 [128,4096]
OFAT_BASE=V026; V033 is not the parent
WHY_NOT_DUPLICATE=V024 changes this function from pre-conversion scale to post-gamma scale; V008 tests post-conversion/pre-gamma order in ProcessWideFp16CachedRows; V033 tests it in ProcessFp16FullRowOutputPipelined. No earlier declaration applies the selected ordering to this full-tile batched function.
UNCHANGED=Reduction and inverse-RMS calculation; output conversion; row/batch dispatch; tile loop and synchronization; bias placement; all non-target branches
EXCLUDED=Rsqrt/math redesign; reduction changes; dtype changes; scheduling changes; Online; shared-record writes
COMPILE=PASS; local host hwnput3; device and submission targets; compile-local-20261007T113356Z.log
CORRECTNESS=PASS; parent and candidate FP16 [128,4096], matched_ratio=1.0; correctness-local-20261007T1134Z.log
LOCAL=MEASUREMENT_BLOCKED; descriptive score=105.1524710831; candidate -4.9000000000% faster; parent stability failed; local-result.json
CURRENT_LOCAL_BEST=V026
OFFICIAL_SCORE=NONE
ONLINE=NOT_RUN

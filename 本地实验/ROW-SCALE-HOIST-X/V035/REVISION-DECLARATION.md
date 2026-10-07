ROUTE=ROW-SCALE-HOIST-X
REVISION=V035
DIRECT_PARENT=V026
PARENT_SOURCE=compile/src/parent_submission.asc
PARENT_SOURCE_SHA256=7c2569d9e3830c845f2b5dc4587a73a05a0bdf041168614cd55ad84204e063f9
SINGLE_HYPOTHESIS=In ProcessNarrowMidOverlap's FP16 epilogue, keep FromFloat and bias placement unchanged but move the existing native-half row-scale Muls before the native-half gamma Mul.
FOCUS=One FP16 row-scale consumption-order change in ProcessNarrowMidOverlap only
TARGET_BRANCH=ProcessNarrowMidOverlap FP16 branch
TARGET_SHAPE=FP16 [128,3072]
OFAT_BASE=V026; V032, V033, and V034 are not parents
DISPATCH=width 3072 is greater than kSmallLowPrecisionContiguousMaxWidth 2048 and within (kMidRowMinWidth 128, kTileElems 4096]; runner uses 128 rows and 40 blocks, so the FP16 narrow-mid path has multiple rows per block
WHY_NOT_DUPLICATE=V021 moved FP16 row-scale across FromFloat to after gamma in this branch; V029 moved scaling into FP32 before output conversion across multiple paths. This probe preserves the conversion boundary and half arithmetic, changing only native-half Muls/Mul order. V033/V034 target different full-row/full-tile consumers.
UNCHANGED=Reduction and inverse-RMS calculation; all data types and conversions; row/block dispatch; buffers, events, and synchronization; bias placement; non-FP16 branches
EXCLUDED=Rsqrt/math redesign; reduction changes; dtype changes; scheduling changes; Online; shared-record writes
CURRENT_LOCAL_BEST=V026
COMPILE=PASS; compile-local-20261007T121751Z.log
CORRECTNESS=PASS; FP16 [128,3072], Parent and Candidate, matched_ratio=1.0; correctness-local-20261007T1219Z.log
LOCAL=MEASUREMENT_BLOCKED; descriptive score=102.4054982818; candidate -2.3489932886% faster; parent stability and paired measurements had high jitter; local-result.json
OFFICIAL_SCORE=NONE
ONLINE=NOT_RUN

ROUTE=ROW-SCALE-HOIST-X
REVISION=V030
DIRECT_PARENT=V026
PARENT_SOURCE=本地实验/ROW-SCALE-HOIST-X/V030/compile/src/parent_submission.asc
PARENT_SOURCE_SHA256=7c2569d9e3830c845f2b5dc4587a73a05a0bdf041168614cd55ad84204e063f9
CANDIDATE_SOURCE=本地实验/ROW-SCALE-HOIST-X/V030/compile/src/submission.asc
CANDIDATE_SOURCE_SHA256=c75d584e86b1bd58cc54066c59691d143c186352bac413f32a275221e2df3ef3
SINGLE_HYPOTHESIS=In ProcessFp16FullRowOutputPipelined, move row-scale multiplication from after FP16 output conversion and gamma multiplication to FP32 before output conversion.
FOCUS=FP16 full-row output row-scale placement only
COMPILE=PASS locally on hwnput3 with CANN 8.5.T8.0.B060 (version_dir=8.5.0.alpha002) after sourcing /usr/local/Ascend/ascend-toolkit/set_env.sh
CORRECTNESS=PASS on device0 for parent and candidate; matched_ratio=1.0; max_abs_error=0.001953125
LOCAL=NEEDS_ONE_MORE_LOCAL; interleaved device0 P-C-P-C-P-C; high time-varying contention
LOCAL_SCORE=144.862155388 (descriptive pooled route-local speedup index; not accepted as stable)
LOCAL_DELTA=-30.968858132% (pooled median; negative means candidate faster)
CURRENT_LOCAL_BEST=V026
OFFICIAL_SCORE=NONE
ONLINE=FORBIDDEN
ATTEMPTS=compile-v030-20261007T0411Z.log; compile-v030-retry-20261007T0413Z.log; compile-v030-local-20261007T045222Z.log
CORRECTNESS_EVIDENCE=correctness-result-local-20261007T045843Z.json
LOCAL_EVIDENCE=local-result.json; local-paired-20261007T050208Z.log
BLOCKER=NONE; the two prior server3 SSH/DNS attempts failed before CMake and are retained

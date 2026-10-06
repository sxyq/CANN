ROUTE=ROW-SCALE-HOIST-X
REVISION=V027
DIRECT_PARENT=V026
PARENT_COMMIT=8a374ae7
PARENT_SCORE=NONE_COMPILE_ONLY
SINGLE_HYPOTHESIS=In ProcessSmallFp32Batched, move FP32 row-scale from before gamma multiplication to after gamma multiplication and before bias.
FOCUS=ProcessSmallFp32Batched epilogue only
WHY_NOT_DUPLICATE=V026 changed only ProcessFp16FullRowOutputPipelined; V027 changes only the small FP32 batched epilogue and preserves inherited low-precision and wide-path order.
V027_EDIT_TIMESTAMP=2026-10-06T21:18:55.856709508Z
SLA_DEADLINE_UTC=2026-10-06T21:18:53.341265114Z
CHILD_SLA_FAIL=YES
COMPILE=PASS
COMPILE_COMMAND=cmake -S 本地实验/ROW-SCALE-HOIST-X/V027/compile -B /tmp/cann-row-scale-hoist-x-v027-20261006T211855Z; cmake --build /tmp/cann-row-scale-hoist-x-v027-20261006T211855Z --target device submission -j2
COMPILE_START_UTC=2026-10-06T21:19:33.277386480Z
COMPILE_END_UTC=2026-10-06T21:19:52.300806327Z
COMPILE_EXIT_CODE=0
DEVICE_TARGET=PASS
SUBMISSION_TARGET=PASS
COMPILE_CONTEXT=LOCAL_WORKTREE_ASCEND_TOOLCHAIN
SERVER3_COMPILE=NOT_RUN (USER_PROHIBITED_SSH)
CORRECTNESS=NOT_RUN (USER_PROHIBITED)
LOCAL=NOT_RUN (USER_PROHIBITED)
ONLINE=NOT_RUN (USER_PROHIBITED)

CONTINUATION_UPDATE_UTC=2026-10-06T22:26:04Z
WORKTREE=/home/data4t2/lelinfeng/cann-row-scale-hoist-x-w4
BRANCH=exp/row-scale-hoist-x-w4
HEAD_BEFORE_RESULT_COMMIT=8a374ae73a4c779126aeef86d807217a6aebae40
CORRECTNESS=PASS; device=0; dtype=fp32; shape=[128,3072]; matched_ratio=1.0; max_abs_error=3.09944153e-06
LOCAL_SCORE=11.03; LOCAL_SCORE_UNIT=candidate_pooled_median_device_us_single_shape
LOCAL_DELTA=+8.7771203156%; parent_pooled_median=10.14us; candidate_pooled_median=11.03us
LOCAL_VERDICT=NEEDS_ONE_MORE_LOCAL; paired_rep_delta=+9.9081%,-5.9126%,+9.2593%; high_jitter_and_mixed_direction
LOCAL_DEVICE=0; FREE_HBM_BEFORE_MB=5315; FREE_HBM_AFTER_MB=5312; AICORE=0%; OTHER_PROCESS=89395:VLLMWorker_TP
ONLINE=FORBIDDEN_NOT_RUN
EVIDENCE=correctness-result.json; local-result.json; local-device-load.txt; correctness-build-evidence.txt; link-blocker-evidence.txt

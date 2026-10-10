# N01 REGISTER-LIVE-RANGE-X V004

ROUTE=N01 REGISTER-LIVE-RANGE-X
REVISION=V004
PARENT=N01 V003
REFERENCE=R31B V011 / Official 45.16
PARENT_SOURCE=本地实验/N01-REGISTER-LIVE-RANGE-X/V003/submission.asc
PARENT_SOURCE_SHA256=f355ef215f9e717b3207010ade0893fed34c7d6078bd1f2d0e7dbd7c966c1da0
CANDIDATE_SOURCE=本地实验/N01-REGISTER-LIVE-RANGE-X/V004/submission.asc
CANDIDATE_SOURCE_SHA256=b7fdfc0aff03fffd52f72c55f2ed3b9e61db1ab795de4c0e9684834782f012c1
FUNCTION=ProcessWideFp32FullCacheRows
SOURCE_CHANGE=Reorganize the first-pass tile work and post-reduction normalization into explicit executable scopes. The reduction scope owns xLocal, residualLocal, and valueTile through the unchanged Load/Add/Mul/ReduceSum sequence. The following scalar bridge scope reads squareSum, then a separate normalization scope rebinds the expired residual UB vector for Duplicate/Sqrt and the final scalar read.
PERFORMANCE_HYPOTHESIS=Ending the reduction vector scope after its final GetValue and rebinding the same dead residual vector for normalization reduces overlap among wide-row FP32 temporaries visible to the compiler, without changing the vector operation sequence.
LIFETIME_EXPECTATION=Per-tile xLocal, residualLocal, and valueTile end at the first scope boundary. Per-row reduction output is consumed by GetValue(0); normalizationValue is created only after that read and uses the same residualBuf_ storage.
UNCHANGED=Arithmetic operation order and operands; Load/Store addresses and counts; Sync and PipeBarrier sequence; UB buffers and sizes; row/tile ownership; reduction layout; parameter/output pass; core assignment; dispatch.
FORBIDDEN_OVERLAP=UB Bank; Core allocation; formula changes; DMA synchronization changes.
DEVICE_PLAN=server3 NPU 2 only if the live snapshot is empty and the existing no-reset paired runner is safe; NPU 3 not used

## RULE_REFRESH_RECEIPT

HEAD_BEFORE_REVISION=8a00bda7d473db4f666e26f99f383f47b7f5ffd6
HEAD_PARENT=19b156ea0cf20560b6a27800437fd82f31c75994
BRANCH=route/n01-register-live-range-x
WORKTREE=/Users/sunyiyang/Desktop/Project/cann/worktrees/n01-register-live-range
RULES_READ=AGENTS.md; .agents/skills/cann-route-executor/SKILL.md; 项目规则/服务器实验规范.md; 项目规则/本地性能测试规范.md; 项目规则/Git工作流程.md; 调度/服务器设备使用.tsv
MECHANISM=ProcessWideFp32FullCacheRows FP32 temporary vector lifetime and reuse point
NEXT_ACTION=Compile the V004 Candidate through the existing V016 single-version device/submission/full_link targets; use the existing no-reset NPU2 paired runner only when the live device snapshot remains safe.

## Results

COMPILE=PASS
COMPILE_CONFIGURE_RC=0
COMPILE_BUILD_RC=0
COMPILE_COMMAND=cmake --build /home/data4t2/lelinfeng/phase4-review-repro/R31B-V016/N01-V004/build --target device submission full_link --parallel 2
COMPILE_ARTIFACT_DEVICE_SHA256=74e18219566986d6daf2307bafe6bbe206fd2b7c11bd91b5fb066068c09871a3
COMPILE_ARTIFACT_SUBMISSION_SHA256=1904eb438d3616c454c00e3457f91eb65c6896b040944bb51c761a83fa94345c
CORRECTNESS=NOT_RUN
LOCAL=NONE
LOCAL_SCORE=NONE
FULL_SOURCE_SIMILARITY=NOT_MEASURED
DEVICE=server3 NPU 2; NPU 3 not used
NPU2_COMPILE_SNAPSHOT=HBM_USAGE_RATE=5%; AICORE_USAGE_RATE=0%; process query unsupported by npu-smi
RUNNER=NOT_RUN
NEXT_ACTION=Use the existing no-reset paired runner on NPU2 only in a later runtime revision; do not invoke a reset runner.
PUSH=NO
EVIDENCE=diff.patch; evidence/dependency-check.md; evidence/compile-server3.log; evidence/runtime-status-npu2.md

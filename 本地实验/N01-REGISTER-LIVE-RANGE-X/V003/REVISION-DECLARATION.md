# N01 REGISTER-LIVE-RANGE-X V003

ROUTE=N01 REGISTER-LIVE-RANGE-X
REVISION=V003
PARENT=N01 V002
REFERENCE=R31B V011 / Official 45.16
PARENT_SOURCE=本地实验/N01-REGISTER-LIVE-RANGE-X/V002/submission.asc
PARENT_SOURCE_SHA256=59ea1096afa7fa72ae7768a8b7fb51389917d6fda28660ee77ca68a9821b3a8d
CANDIDATE_SOURCE=本地实验/N01-REGISTER-LIVE-RANGE-X/V003/submission.asc
CANDIDATE_SOURCE_SHA256=f355ef215f9e717b3207010ade0893fed34c7d6078bd1f2d0e7dbd7c966c1da0
SOURCE_CHANGE=In ProcessWideFp32FullCacheRows, reuse residualLocal for the scalar normalization vector after its reduction result has been read into squareSum; xLocal remains the ReduceSum source and is no longer reused for normalization.
FUNCTION=ProcessWideFp32FullCacheRows
PERFORMANCE_HYPOTHESIS=Moving the post-reduction Duplicate/Sqrt temporary to residualLocal may shorten the live range of xLocal and reuse the vector whose prior value ended at GetValue(0), reducing FP32 Vector register pressure on full-cache rows.
ARITHMETIC_DEPENDENCY=ReduceSum writes residualLocal; SyncVToS and GetValue(0) produce squareSum; after squareSum is copied, the old residualLocal vector contents are dead. SyncSToV precedes the new Duplicate, so residualLocal is a valid normalization destination.
UNCHANGED=Add/Mul/ReduceSum operations and operands; numerical operation order and values; reduction structure; PipeBarrier and Sync calls; Load/Store DMA; UB allocation; core assignment; tile geometry; dispatch; output path.
FORBIDDEN_OVERLAP=UB Bank; Core allocation; formula changes; DMA synchronization changes.
DEVICE=server3 NPU 2; NPU 3 not used

## RULE_REFRESH_RECEIPT

HEAD_BEFORE_REVISION=16256ee77fb207449693bacc012f91a34f7198bb
HEAD_PARENT=d66185f8d6912ab42a92a4cd1ba256306bd389cc
BRANCH=route/n01-register-live-range-x
WORKTREE=/Users/sunyiyang/Desktop/Project/cann/worktrees/n01-register-live-range
RULES_READ=AGENTS.md; .agents/skills/cann-route-executor/SKILL.md; 项目规则/服务器实验规范.md; 项目规则/本地性能测试规范.md; 项目规则/Git工作流程.md
MECHANISM=ProcessWideFp32FullCacheRows FP32 temporary vector lifetime and reuse point
NEXT_ACTION=Keep V003 as a compile-only local revision; later retry the existing no-reset runner build with the corrected include path, then run Correctness and minimal interleaved Local on an available NPU.

## Results

COMPILE=PASS; existing V016 single-version targets device, submission, full_link configured and built with BUILD_RC=0; candidate module SHA is recorded in runtime-status-npu2.md
RUNNER_BUILD=FAIL; first no-reset runner build stopped at C++ header parsing because the retry command used an incorrect target include path
CORRECTNESS=NOT_RUN; no V003 runner executable was produced
LOCAL=NONE; no V003 runner invocation and no raw latency samples
DEVICE=server3 NPU 2; SOC=Ascend910B3; CANN=8.5.0.alpha002
RUNNER_POLICY=Use only the existing temporary paired runner with kDevice=2 and aclrtResetDevice removed; never invoke the original reset runner.
RUNNER_FAILURE=runner-build-server3.log records CONFIGURE_RC=0, BUILD_RC=2, and the original compiler diagnostics; NPU 2 was not used by the failed build
EVIDENCE=evidence/dependency-check.md; evidence/compile-server3.log; evidence/runner-build-server3.log; evidence/correctness-local-server3.log; evidence/runtime-status-npu2.md; evidence/paired-runner-no-reset.patch
NEXT_ACTION=Retry the existing no-reset runner build with the corrected C++ include path, then run V003 Correctness and minimal interleaved Local only on an available NPU; do not invoke the original reset runner.

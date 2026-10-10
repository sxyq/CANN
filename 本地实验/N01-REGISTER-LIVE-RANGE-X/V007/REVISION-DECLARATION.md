# N01 REGISTER-LIVE-RANGE-X V007

ROUTE=N01 REGISTER-LIVE-RANGE-X
REVISION=V007
PARENT=N01 V006
REFERENCE=R31B V011 / Official 45.16
PARENT_SOURCE=本地实验/N01-REGISTER-LIVE-RANGE-X/V006/submission.asc
PARENT_SOURCE_SHA256=85dc7e6b431a1160255fb09243666fda6fadf766034328e25098b181ae573f68
CANDIDATE_SOURCE=本地实验/N01-REGISTER-LIVE-RANGE-X/V007/submission.asc
CANDIDATE_SOURCE_SHA256=7bedcda7c757370afaf5dc3d7453f3a4c3e40a271ca923912edca415c5de9184
FUNCTION=ProcessWideFp32FullCacheRows
SOURCE_CHANGE=After the full-row reduction scalar read, bind the normalization Duplicate/Sqrt temporary to the now-dead xBuf_ FP32 staging vector; keep the reduction destination and all input/output roles unchanged.
PERFORMANCE_HYPOTHESIS=Reusing xBuf_ after its input lifetime ends can shorten the live range of the reduction partial storage and keep the normalization value in an already available FP32 vector without changing the arithmetic sequence.
LIFETIME_EXPECTATION=xBuf_ is live for row input and reduction setup, dead after the reduction scalar read, then reused for meanSquare Duplicate/Sqrt and inverse-RMS read before it is loaded as the parameter staging vector.
UNCHANGED=Arithmetic order and values; Load/Store addresses and counts; Sync and PipeBarrier sequence; UB allocation and sizes; row/tile ownership; reduction destination and count; parameter/output pass; core assignment; dispatch.
FORBIDDEN_OVERLAP=Scalar address; UB Bank; mathematical formula; DMA; synchronization; ownership; V005 square/ReduceSum role exchange; V006 reduceLocal first-slot reuse; V004 scope reorganization.
DEVICE_PLAN=server3 NPU 2 only; use the existing V016 no-reset paired entry; record load and do not use NPU 3

## RULE_REFRESH_RECEIPT

HEAD=d4cfbf34c6e050644f444204ceb3e5bb0d16ee3a
HEAD_PARENT=79a9a8b76bddf31269b65908f4836fd5da0c0a08
BRANCH=route/n01-register-live-range-x
WORKTREE=/Users/sunyiyang/Desktop/Project/cann/worktrees/n01-register-live-range
RULES_READ=AGENTS.md; .agents/skills/cann-route-executor/SKILL.md; 项目规则/服务器实验规范.md; 项目规则/本地性能测试规范.md; 项目规则/Git工作流程.md; 调度/服务器设备使用.tsv; /Users/sunyiyang/.agent-global/environment.md
MECHANISM=ProcessWideFp32FullCacheRows FP32 temporary vector lifetime and reuse point
FORBIDDEN=Scalar address; UB Bank; mathematical formula; DMA/synchronization; ownership; V005 role exchange; V006 reduceLocal first-slot reuse; V004 scope reorganization
NEXT_ACTION=Upload V007 to the existing V016 single-version source path and compile device submission full_link; then use only the existing NPU2 no-reset paired entry.

## Results

COMPILE=PASS
COMPILE_CONFIGURE_RC=0
COMPILE_BUILD_RC=0
COMPILE_TARGETS=device submission full_link
COMPILE_COMMAND=cmake --build /home/data4t2/lelinfeng/phase4-review-repro/R31B-V016/N01-V007/build --target device submission full_link --parallel 2
COMPILE_ARTIFACT_DEVICE_SHA256=d573ff6a582347e77df6792acf004398e74ecffaa6cc9a5c9e2c63eb15461e19
COMPILE_ARTIFACT_SUBMISSION_SHA256=2cebd83e1163c48953b34379062058c72b4f1fcb651d44e8d070ee33eb423f10
CORRECTNESS=NOT_RUN
CORRECTNESS_REASON=User requested closure before starting the existing no-reset paired runner; no device runner was invoked and no reset call was executed.
LOCAL_SCORE=NONE
LOCAL_DELTA=NONE
LOCAL_REASON=Correctness was not run, so no timing samples were generated.
DEVICE_ID=server3 NPU2
FREE_HBM_MB=2186
DEVICE_PRE=HBM=63350/65536 MB; AICore=64%; Aivector=NOT_REPORTED_BY_npu-smi_info; PID=658669 python memory=59977 MB
DEVICE_POST=HBM=63350/65536 MB; AICore=46%; Aivector=NOT_REPORTED_BY_npu-smi_info; PID=658669 python memory=59977 MB
RUNNER=existing /home/data4t2/lelinfeng/phase4-review-repro/R31B-V016/N01-V003/paired-runner-build/paired_runner_v016; V007 execution=NOT_RUN; V006 recorded RESET_SCAN=NONE
EVIDENCE_NOTE=evidence/compile-server3.log retains the full environment, input identities, raw npu-smi snapshots, build output, and artifact identities; evidence/correctness-local-server3.log records the unexecuted device stage.
PUSH=NO

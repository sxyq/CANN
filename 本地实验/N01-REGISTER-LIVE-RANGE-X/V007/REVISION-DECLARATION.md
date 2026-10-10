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
NEXT_ACTION=V007 device validation is complete; retain the raw result and select a new independent FP32 lifetime factor for the next N01 revision.

## Results

COMPILE=PASS
COMPILE_CONFIGURE_RC=0
COMPILE_BUILD_RC=0
COMPILE_TARGETS=device submission full_link
COMPILE_COMMAND=cmake --build /home/data4t2/lelinfeng/phase4-review-repro/R31B-V016/N01-V007/build --target device submission full_link --parallel 2
COMPILE_ARTIFACT_DEVICE_SHA256=d573ff6a582347e77df6792acf004398e74ecffaa6cc9a5c9e2c63eb15461e19
COMPILE_ARTIFACT_SUBMISSION_SHA256=2cebd83e1163c48953b34379062058c72b4f1fcb651d44e8d070ee33eb423f10
CORRECTNESS=PASS
CORRECTNESS_CASES=4/4; PARENT_FAILURES=0; CANDIDATE_FAILURES=0; RUNNER_RC=0
CORRECTNESS_REASON=The existing no-reset paired runner passed all four Parent/Candidate cases.
LOCAL_SCORE=NONE
LOCAL_DELTA=fp16-tail=-0.840us; fp16-wide=-0.900us; bf16-tail=-0.480us; bf16-wide=+0.060us
LOCAL_REASON=All four paired-device delta p10-p90 intervals crossed zero; FP16 groups and BF16-tail also contained large outliers under the recorded NPU2 load.
DEVICE_ID=server3 NPU2
FREE_HBM_MB=2186 pre; 2183 post
DEVICE_PRE=HBM=63350/65536 MB; AICore=61%; Aivector=NOT_REPORTED_BY_npu-smi_info; PID=658669 python memory=59977 MB
DEVICE_POST=HBM=63353/65536 MB; AICore=59%; Aivector=NOT_REPORTED_BY_npu-smi_info; PID=658669 python memory=59977 MB
RUNNER=existing /home/data4t2/lelinfeng/phase4-review-repro/R31B-V016/N01-V003/paired-runner-build/paired_runner_v016; V007 execution=PASS; RESET_SCAN=NONE
RUNNER_SOURCE_SHA256=87da3944c4054eca8e91f62dd6bd90bfed1f50ff6221d83794181a66317b49fa
RUNNER_BINARY_SHA256=3198cc87f3f2d4f59c445e93f46923589fa4e9d934e3d6f4bcbd7c43578687aa
RUNNER_CONFIGURE_RC=0; RUNNER_BUILD_RC=0
EVIDENCE_NOTE=evidence/compile-server3.log retains Compile inputs and modules; evidence/correctness-local-server3.log retains the initial rg-tool failure, the grep reset scan, all raw samples, snapshots, and final summaries.
PUSH=NO

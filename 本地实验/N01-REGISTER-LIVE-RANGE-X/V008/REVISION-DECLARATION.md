# N01 REGISTER-LIVE-RANGE-X V008

ROUTE=N01 REGISTER-LIVE-RANGE-X
REVISION=V008
PARENT=N01 V007
REFERENCE=R31B V011 / Official 45.16
PARENT_SOURCE=本地实验/N01-REGISTER-LIVE-RANGE-X/V007/submission.asc
PARENT_SOURCE_SHA256=7bedcda7c757370afaf5dc3d7453f3a4c3e40a271ca923912edca415c5de9184
CANDIDATE_SOURCE=本地实验/N01-REGISTER-LIVE-RANGE-X/V008/submission.asc
CANDIDATE_SOURCE_SHA256=8d4270c20ceca2eee9a56ca2560dda40dd2028741a6ea8025dca17ca2f8f0741
FUNCTION=ProcessWideFp32FullCacheRows
SOURCE_CHANGE=After the final per-row ReduceSum scalar read, bind the normalization Duplicate/Sqrt temporary to the last tile partial slot of reduceLocal instead of the V007 xBuf_ staging vector.
PERFORMANCE_HYPOTHESIS=The last reduction partial is dead after the aggregate scalar read; reusing that distinct slot can end the xBuf_ normalization lifetime before the parameter pass while preserving the existing FP32 vector work areas.
LIFETIME_EXPECTATION=The tail reduceLocal partial is live only through the aggregate reduction and becomes reusable after GetValue(0); it carries meanSquare through Duplicate/Sqrt and the inverse-RMS scalar read, while xBuf_ remains available for the later parameter load.
FACTOR_AUDIT=V001 valueFp32 in-place square; V002 FP32 square/reduction operand roles; V003/V004 residualBuf_ normalization and scope; V005 post-Add square/ReduceSum role exchange; V006 reduceLocal first-slot reuse; V007 xBuf_ normalization reuse. V008 uses the distinct tail reduction partial slot.
UNCHANGED=Arithmetic order and values; Load/Store addresses and counts; Sync and PipeBarrier sequence; UB allocation and sizes; row/tile ownership; reduction destination and count; parameter/output pass; core assignment; dispatch.
FORBIDDEN_OVERLAP=Scalar address; UB Bank; mathematical formula; DMA; synchronization; ownership; V001-V007 factors listed in FACTOR_AUDIT.
DEVICE_PLAN=server3 NPU 2 only; use the existing V016 no-reset paired entry; record load and do not use NPU 3

## RULE_REFRESH_RECEIPT

HEAD=a417bd831ee8e5a0653c658572c0f3ccd992fa56
HEAD_PARENT=a685340316782e5e069e75216f99e875db707887
BRANCH=route/n01-register-live-range-x
WORKTREE=/Users/sunyiyang/Desktop/Project/cann/worktrees/n01-register-live-range
RULES_READ=AGENTS.md; .agents/skills/cann-route-executor/SKILL.md; 项目规则/服务器实验规范.md; 项目规则/本地性能测试规范.md; 项目规则/Git工作流程.md; 调度/服务器设备使用.tsv; /Users/sunyiyang/.agent-global/environment.md
MECHANISM=ProcessWideFp32FullCacheRows FP32 temporary vector lifetime and reuse point
FORBIDDEN=Scalar address; UB Bank; mathematical formula; DMA/synchronization; ownership; V001-V007 factors listed in FACTOR_AUDIT
NEXT_ACTION=V008 device validation is complete; retain the raw result and select another independent FP32 lifetime factor for the next N01 revision.

## Results

COMPILE=PASS
COMPILE_CONFIGURE_RC=0
COMPILE_BUILD_RC=0
COMPILE_TARGETS=device submission full_link
COMPILE_COMMAND=cmake --build /home/data4t2/lelinfeng/phase4-review-repro/R31B-V016/N01-V008/build --target device submission full_link --parallel 2
COMPILE_ARTIFACT_DEVICE_SHA256=64a8153145e02c200f9f6c189a0a0bf8fb50a90511d3cbf87b0adeeac049668e
COMPILE_ARTIFACT_SUBMISSION_SHA256=6255d2443fc026adba9768ca8450712f64410640851212b6072475da952c02ea
CORRECTNESS=PASS
CORRECTNESS_CASES=4/4; PARENT_FAILURES=0; CANDIDATE_FAILURES=0; RUNNER_RC=0
LOCAL_SCORE=NONE
LOCAL_DELTA=fp16-tail=-4.760us; fp16-wide=+6.380us; bf16-tail=+1.140us; bf16-wide=-3.880us
LOCAL_REASON=All four paired-device delta p10-p90 intervals crossed zero; every group contained large outliers under the recorded NPU2 load.
DEVICE_ID=server3 NPU2
FREE_HBM_MB=2186 pre; 2183 post
DEVICE_PRE=HBM=63350/65536 MB; AICore=59%; Aivector=NOT_REPORTED_BY_npu-smi_info; PID=658669 python memory=59977 MB
DEVICE_POST=HBM=63353/65536 MB; AICore=55%; Aivector=NOT_REPORTED_BY_npu-smi_info; PID=658669 python memory=59977 MB
RUNNER=existing /home/data4t2/lelinfeng/phase4-review-repro/R31B-V016/N01-V003/paired-runner-build/paired_runner_v016; RESET_SCAN=NONE
RUNNER_SOURCE_SHA256=87da3944c4054eca8e91f62dd6bd90bfed1f50ff6221d83794181a66317b49fa
RUNNER_BINARY_SHA256=3198cc87f3f2d4f59c445e93f46923589fa4e9d934e3d6f4bcbd7c43578687aa
RUNNER_CONFIGURE_RC=0; RUNNER_BUILD_RC=0
EVIDENCE_NOTE=evidence/compile-server3.log retains the complete Compile environment and modules; evidence/correctness-local-server3.log retains reset scan, runner build, all raw samples, snapshots, and summaries.
PUSH=NO

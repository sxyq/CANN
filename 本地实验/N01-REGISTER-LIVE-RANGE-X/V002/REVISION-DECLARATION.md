# N01 REGISTER-LIVE-RANGE-X V002

ROUTE=N01 REGISTER-LIVE-RANGE-X
REVISION=V002
PARENT=N01 V001
REFERENCE=R31B V011 / Official 45.16
PARENT_SOURCE=本地实验/N01-REGISTER-LIVE-RANGE-X/V001/submission.asc
PARENT_SOURCE_SHA256=c00a2ac61ea722a84a098d9f047ecf39c539e2dfda98d49e2fec910dd063f2dc
CANDIDATE_SOURCE=本地实验/N01-REGISTER-LIVE-RANGE-X/V002/submission.asc
CANDIDATE_SOURCE_SHA256=59ea1096afa7fa72ae7768a8b7fb51389917d6fda28660ee77ca68a9821b3a8d
SOURCE_CHANGE=In the BF16 branch of ProcessWideLowPrecision pass 1, reuse xFp32 for the final square and use that same vector as the ReduceSum input after the Add has consumed xFp32 as an input.
FUNCTION=ProcessWideLowPrecision
PERFORMANCE_HYPOTHESIS=Reusing the post-Add xFp32 temporary may shorten the live range of the BF16 square result and reduce Vector register pressure on wide rows while preserving retained valueTile.
UNCHANGED=Numerical operation order and values; reduction order and work buffer role; UB allocation; core assignment; DMA; synchronization; tile geometry; dispatch; output path.
FORBIDDEN_OVERLAP=UB Bank; Core allocation; formula changes; DMA synchronization changes.
DEVICE=server3 NPU 2 (updated user assignment; no NPU 3 use)

## RULE_REFRESH_RECEIPT

HEAD_BEFORE_REVISION=d66185f8d6912ab42a92a4cd1ba256306bd389cc
HEAD_PARENT=1c51a5b2eb9b5b9dffc031c1fbbbc35e9506b49f
BRANCH=route/n01-register-live-range-x
WORKTREE=/Users/sunyiyang/Desktop/Project/cann/worktrees/n01-register-live-range
RULES_READ=AGENTS.md; .agents/skills/cann-route-executor/SKILL.md; 项目规则/服务器实验规范.md; 项目规则/本地性能测试规范.md; 项目规则/Git工作流程.md
MECHANISM=Wide-row Vector temporary FP32 value lifetime in ProcessWideLowPrecision
NEXT_ACTION=Retain V002 as the current local result; later N01 work starts from this revision and uses only the no-reset runner on an explicitly available NPU.

## Results

COMPILE=PASS; first configure failed because runtime_probe.cpp was omitted (CONFIGURE_RC=1); retry configure=0 and device submission full_link BUILD_RC=0
CORRECTNESS=PASS; four cases, both V011 Parent and V016 Candidate passed with failures=0
LOCAL=RECORDED_RAW; four cases, 21 interleaved Parent/Candidate pairs each, device-event timing
LOCAL_SCORE=NONE; one timing block per case and high sample dispersion, so no stable score is claimed
DEVICE=server3 NPU 2; SOC=Ascend910B3; CANN=8.5.0.alpha002
RUNNER_POLICY=Temporary copy of the existing paired runner with kDevice=2 and aclrtResetDevice removed; original runner was not invoked
RUNNER_RUNTIME_FIX=LD_PRELOAD=/usr/lib/aarch64-linux-gnu/libstdc++.so.6 for the existing HCC runtime-library selection
SHAPES_DTYPES=rows=2; fp16 width=12288 and 32768; bf16 width=12288 and 32768
RUNNER_PROTOCOL=--paired; warmup_each=45; pairs_each=21; timing=DEVICE_EVENT_PRIMARY; wall=DIAGNOSTIC
LOCAL_MEDIANS=fp16-tail-d12288 P=8.520us C=9.300us paired_delta=+0.320us; fp16-wide-d32768 P=11.840us C=13.380us paired_delta=+0.500us; bf16-tail-d12288 P=15.160us C=16.420us paired_delta=-0.140us; bf16-wide-d32768 P=15.960us C=14.800us paired_delta=-0.380us
LOCAL_MEDIAN_DELTA=C-P: fp16-tail +9.15%; fp16-wide +13.01%; bf16-tail +8.31%; bf16-wide -7.27%
FREE_HBM=62259MB equivalent to 5% used at NPU2 pre/post snapshots; OTHER_PROCESS_PRESENT=NO
EVIDENCE=evidence/compile-server3.log; evidence/runner-build-server3.log; evidence/correctness-local-server3.log; evidence/runtime-status-npu2.md; evidence/paired-runner-no-reset.patch
NEXT_ACTION=Retain V002 as the current local result; a later N01 revision starts from this commit and uses only the no-reset runner on an explicitly available NPU.

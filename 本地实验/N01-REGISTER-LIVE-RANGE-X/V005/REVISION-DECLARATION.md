# N01 REGISTER-LIVE-RANGE-X V005

ROUTE=N01 REGISTER-LIVE-RANGE-X
REVISION=V005
PARENT=N01 V004
REFERENCE=R31B V011 / Official 45.16
PARENT_SOURCE=本地实验/N01-REGISTER-LIVE-RANGE-X/V004/submission.asc
PARENT_SOURCE_SHA256=b7fdfc0aff03fffd52f72c55f2ed3b9e61db1ab795de4c0e9684834782f012c1
CANDIDATE_SOURCE=本地实验/N01-REGISTER-LIVE-RANGE-X/V005/submission.asc
CANDIDATE_SOURCE_SHA256=07f3c0d291b772385eb47bcad831cd28a063b14c093e9f23a5274cab786d768b
FUNCTION=ProcessWideFp32FullCacheRows
SOURCE_CHANGE=After Add has consumed xLocal and residualLocal as inputs, store the FP32 square in residualLocal and pass the now-dead xLocal as the ReduceSum temporary buffer. The ReduceSum destination and valid count remain unchanged.
PERFORMANCE_HYPOTHESIS=Swapping the post-Add square and ReduceSum temporary roles shortens the live range of the original xLocal value and lets the reduction work value reuse that dead FP32 vector without changing the arithmetic sequence.
LIFETIME_EXPECTATION=The Add result remains in valueTile. residualLocal becomes the square vector only after Add; xLocal is retained only as the ReduceSum temporary input after its source lifetime ends.
UNCHANGED=Arithmetic order and values; Load/Store addresses and counts; Sync and PipeBarrier sequence; UB allocation and sizes; row/tile ownership; reduction destination and layout; parameter/output pass; core assignment; dispatch.
FORBIDDEN_OVERLAP=Scalar address; UB Bank; mathematical formula; DMA; synchronization; ownership; V004 scope reorganization.
DEVICE_PLAN=server3 NPU 2 only; use the existing V016 no-reset paired entry; record load and do not use NPU 3

## RULE_REFRESH_RECEIPT

HEAD=125be8085386f4ec0b7f13dab19b0dd1e39afe64
HEAD_PARENT=06163b61b0cfa370ea76f25dcd1d408c9d9f0c38
BRANCH=route/n01-register-live-range-x
WORKTREE=/Users/sunyiyang/Desktop/Project/cann/worktrees/n01-register-live-range
RULES_READ=AGENTS.md; .agents/skills/cann-route-executor/SKILL.md; 项目规则/服务器实验规范.md; 项目规则/本地性能测试规范.md; 项目规则/Git工作流程.md; 调度/服务器设备使用.tsv
MECHANISM=ProcessWideFp32FullCacheRows FP32 temporary vector lifetime and reuse point
FORBIDDEN=Scalar address; UB Bank; mathematical formula; DMA/synchronization; ownership; V004 scope reorganization
NEXT_ACTION=Upload V005 to the existing V016 single-version source path and compile device submission full_link; then use only the existing NPU2 no-reset paired entry.

## Results

COMPILE=PASS
COMPILE_CONFIGURE_RC=0
COMPILE_BUILD_RC=0
COMPILE_TARGETS=device submission full_link
COMPILE_COMMAND=cmake --build /home/data4t2/lelinfeng/phase4-review-repro/R31B-V016/N01-V005/build --target device submission full_link --parallel 2
COMPILE_ARTIFACT_DEVICE_SHA256=43e250fedc02919c5eb2d4de62a649977e3cb32e7e2001c531e17b0ce1e41412
COMPILE_ARTIFACT_SUBMISSION_SHA256=f11cea1826ef41f1fe7d87468696a2d7a6607499d72d41b903e14cc626f19d04
CORRECTNESS=PASS
CORRECTNESS_CASES=4/4; PARENT_FAILURES=0; CANDIDATE_FAILURES=0; RUNNER_RC=0
LOCAL=RECORDED_RAW
LOCAL_SCORE=NONE
LOCAL_REASON=NPU2 HBM usage was 96% with PID 658669 present; AICore/Aivector were nonzero and device-event samples had large outliers. All four paired-delta p10-p90 intervals crossed zero, so no stable Local score is claimed.
LOCAL_STATS=fp16-tail delta_median=-1.100us delta_MAD=18.360us p10=-76.840 p90=35.660; fp16-wide delta_median=-1.040us delta_MAD=11.640us p10=-15.320 p90=41.220; bf16-tail delta_median=-0.020us delta_MAD=24.420us p10=-92.400 p90=71.820; bf16-wide delta_median=-4.580us delta_MAD=22.680us p10=-58.280 p90=30.660
RUNTIME_PARENT_SOURCE=/home/data4t2/lelinfeng/phase4-workspaces/R31B/R31B-V011-LP-ROW-PIPELINE_kernel.asc
RUNTIME_PARENT_SOURCE_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
RUNTIME_CANDIDATE_SOURCE=/home/data4t2/lelinfeng/phase4-review-repro/R31B-V016/N01-V005/R31B-V016-WIDE-TILE-SEED_kernel.asc
RUNTIME_CANDIDATE_SOURCE_SHA256=07f3c0d291b772385eb47bcad831cd28a063b14c093e9f23a5274cab786d768b
RUNNER_SOURCE=/home/data4t2/lelinfeng/phase4-review-repro/R31B-V016/N01-V003/paired-runner-src; existing no-reset source reused with V005 include path
RUNNER_CONFIGURE_COMMAND=cmake -S /home/data4t2/lelinfeng/phase4-review-repro/R31B-V016/N01-V003/paired-runner-src -B /home/data4t2/lelinfeng/phase4-review-repro/R31B-V016/N01-V003/paired-runner-build -DR31B_V016_SOURCE_DIR=/home/data4t2/lelinfeng/phase4-review-repro/R31B-V016/N01-V005
RUNNER_BUILD_COMMAND=cmake --build /home/data4t2/lelinfeng/phase4-review-repro/R31B-V016/N01-V003/paired-runner-build --target paired_runner_v016 --parallel 1 -- -B
RUNNER_CONFIGURE_RC=0
RUNNER_BUILD_RC=0
RUNNER=/home/data4t2/lelinfeng/phase4-review-repro/R31B-V016/N01-V003/paired-runner-build/paired_runner_v016 --paired
RUNNER_SOURCE_SHA256=87da3944c4054eca8e91f62dd6bd90bfed1f50ff6221d83794181a66317b49fa
RUNNER_BINARY_SHA256=3198cc87f3f2d4f59c445e93f46923589fa4e9d934e3d6f4bcbd7c43578687aa
RUNNER_RESET_SCAN=NONE
SHAPES_DTYPES=rows=2; fp16 width=12288 and 32768; bf16 width=12288 and 32768; warmup_each=45; pairs_each=21
NPU2_PRE_RUN=2026-10-10T19:32:04Z; HBM_USAGE_RATE=96%; AICORE_USAGE_RATE=56%; AIVECTOR_USAGE_RATE=55%; PID=658669; PROCESS_MEMORY_MB=59977
NPU2_POST_RUN=2026-10-10T19:32:12Z; HBM_USAGE_RATE=96%; AICORE_USAGE_RATE=51%; AIVECTOR_USAGE_RATE=55%; PID=658669; PROCESS_MEMORY_MB=59977
FREE_HBM_MB=UNKNOWN; npu-smi reported rounded HBM usage rate only
NEXT_ACTION=V005 evidence is complete locally; a future N01 revision must use a separate FP32 temporary lifetime factor and the existing no-reset entry only.
DEVICE=server3 NPU 2; NPU 3 not used
PUSH=NO
EVIDENCE=diff.patch; evidence/compile-server3.log; evidence/correctness-local-server3.log; evidence/runtime-status-npu2.md

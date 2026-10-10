# N01 REGISTER-LIVE-RANGE-X V006

ROUTE=N01 REGISTER-LIVE-RANGE-X
REVISION=V006
PARENT=N01 V005
REFERENCE=R31B V011 / Official 45.16
PARENT_SOURCE=本地实验/N01-REGISTER-LIVE-RANGE-X/V005/submission.asc
PARENT_SOURCE_SHA256=07f3c0d291b772385eb47bcad831cd28a063b14c093e9f23a5274cab786d768b
CANDIDATE_SOURCE=本地实验/N01-REGISTER-LIVE-RANGE-X/V006/submission.asc
CANDIDATE_SOURCE_SHA256=85dc7e6b431a1160255fb09243666fda6fadf766034328e25098b181ae573f68
FUNCTION=ProcessWideFp32FullCacheRows
SOURCE_CHANGE=After the per-row reduction scalar read, reuse the first dead reduceLocal FP32 partial slot as normalizationValue for Duplicate/Sqrt and the inverse-RMS scalar read. This is a new reuse point; V005's post-Add square and ReduceSum operand roles remain unchanged.
PERFORMANCE_HYPOTHESIS=Reusing the dead reduction partial slot for the scalar normalization vector avoids extending the residual input buffer's FP32 normalization lifetime into the next stage while preserving the existing arithmetic sequence.
LIFETIME_EXPECTATION=The row's reduceLocal partials become dead after GetValue(0); their first slot carries meanSquare through Duplicate/Sqrt until the inverse-RMS read. No FP32 buffer allocation changes.
UNCHANGED=Arithmetic order and values; Load/Store addresses and counts; Sync and PipeBarrier sequence; UB allocation and sizes; row/tile ownership; reduction destination and count; parameter/output pass; core assignment; dispatch.
FORBIDDEN_OVERLAP=Scalar address; UB Bank; mathematical formula; DMA; synchronization; ownership; V005 square/ReduceSum role exchange; V004 scope reorganization.
DEVICE_PLAN=server3 NPU 2 only; use the existing V016 no-reset paired entry; record load and do not use NPU 3

## RULE_REFRESH_RECEIPT

HEAD=79a9a8b76bddf31269b65908f4836fd5da0c0a08
HEAD_PARENT=125be8085386f4ec0b7f13dab19b0dd1e39afe64
BRANCH=route/n01-register-live-range-x
WORKTREE=/Users/sunyiyang/Desktop/Project/cann/worktrees/n01-register-live-range
RULES_READ=AGENTS.md; .agents/skills/cann-route-executor/SKILL.md; 项目规则/服务器实验规范.md; 项目规则/本地性能测试规范.md; 项目规则/Git工作流程.md; 调度/服务器设备使用.tsv
MECHANISM=ProcessWideFp32FullCacheRows FP32 temporary vector lifetime and reuse point
FORBIDDEN=Scalar address; UB Bank; mathematical formula; DMA/synchronization; ownership; V005 role exchange; V004 scope reorganization
NEXT_ACTION=Upload V006 to the existing V016 single-version source path and compile device submission full_link; then reuse only the existing NPU2 no-reset paired entry for Correctness and paired Local.

## Results

COMPILE=PASS
COMPILE_CONFIGURE_RC=0
COMPILE_BUILD_RC=0
COMPILE_TARGETS=device submission full_link
COMPILE_COMMAND=cmake --build /home/data4t2/lelinfeng/phase4-review-repro/R31B-V016/N01-V006/build --target device submission full_link --parallel 2
COMPILE_ARTIFACT_DEVICE_SHA256=05652b5a78eae7b3d7c7e975bedfdbe7b45ff81c56ad36310d1595f764539a2e
COMPILE_ARTIFACT_SUBMISSION_SHA256=f29d91abfa9912534fba8e64d828f604c35c61b0b426b4b494a804854a9eafab
CORRECTNESS=PASS
CORRECTNESS_CASES=4/4; PARENT_FAILURES=0; CANDIDATE_FAILURES=0; RUNNER_RC=0
LOCAL=RECORDED_RAW
LOCAL_SCORE=NONE
LOCAL_REASON=NPU2 HBM usage was 96% with PID 658669 present; AICore/Aivector were nonzero and device-event samples contained large outliers. All four paired-delta p10-p90 intervals crossed zero.
LOCAL_STATS=fp16-tail delta_median=-0.560us MAD=7.640us p10=-15.740 p90=11.240; fp16-wide delta_median=-0.520us MAD=2.540us p10=-10.140 p90=11.380; bf16-tail delta_median=8.760us MAD=9.140us p10=-12.380 p90=178.880; bf16-wide delta_median=1.080us MAD=4.320us p10=-5.500 p90=12.520
RUNTIME_PARENT_SOURCE=/home/data4t2/lelinfeng/phase4-workspaces/R31B/R31B-V011-LP-ROW-PIPELINE_kernel.asc
RUNTIME_PARENT_SOURCE_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
RUNTIME_CANDIDATE_SOURCE=/home/data4t2/lelinfeng/phase4-review-repro/R31B-V016/N01-V006/R31B-V016-WIDE-TILE-SEED_kernel.asc
RUNTIME_CANDIDATE_SOURCE_SHA256=85dc7e6b431a1160255fb09243666fda6fadf766034328e25098b181ae573f68
RUNNER_SOURCE=/home/data4t2/lelinfeng/phase4-review-repro/R31B-V016/N01-V003/paired-runner-src; existing no-reset source reused with V006 include path
RUNNER_CONFIGURE_COMMAND=cmake -S /home/data4t2/lelinfeng/phase4-review-repro/R31B-V016/N01-V003/paired-runner-src -B /home/data4t2/lelinfeng/phase4-review-repro/R31B-V016/N01-V003/paired-runner-build -DR31B_V016_SOURCE_DIR=/home/data4t2/lelinfeng/phase4-review-repro/R31B-V016/N01-V006
RUNNER_BUILD_COMMAND=cmake --build /home/data4t2/lelinfeng/phase4-review-repro/R31B-V016/N01-V003/paired-runner-build --target paired_runner_v016 --parallel 1 -- -B
RUNNER_CONFIGURE_RC=0
RUNNER_BUILD_RC=0
RUNNER=/home/data4t2/lelinfeng/phase4-review-repro/R31B-V016/N01-V003/paired-runner-build/paired_runner_v016 --paired
RUNNER_SOURCE_SHA256=87da3944c4054eca8e91f62dd6bd90bfed1f50ff6221d83794181a66317b49fa
RUNNER_BINARY_SHA256=3198cc87f3f2d4f59c445e93f46923589fa4e9d934e3d6f4bcbd7c43578687aa
RUNNER_RESET_SCAN=NONE
SHAPES_DTYPES=rows=2; fp16 width=12288 and 32768; bf16 width=12288 and 32768; warmup_each=45; pairs_each=21
NPU2_PRE_RUN=2026-10-10T19:58:56Z; HBM_USAGE_RATE=96%; AICORE_USAGE_RATE=68%; AIVECTOR_USAGE_RATE=35%; PID=658669; PROCESS_MEMORY_MB=59977
NPU2_POST_RUN=2026-10-10T19:59:04Z; HBM_USAGE_RATE=96%; AICORE_USAGE_RATE=65%; AIVECTOR_USAGE_RATE=34%; PID=658669; PROCESS_MEMORY_MB=59977
FREE_HBM_MB=UNKNOWN; npu-smi reported rounded HBM usage rate only
NEXT_ACTION=V006 evidence is complete locally; a future N01 revision must use a separate FP32 temporary lifetime factor and the existing no-reset entry only.
DEVICE=server3 NPU 2; NPU 3 not used
PUSH=NO
EVIDENCE=diff.patch; evidence/compile-server3.log; evidence/correctness-local-server3.log; evidence/runtime-status-npu2.md

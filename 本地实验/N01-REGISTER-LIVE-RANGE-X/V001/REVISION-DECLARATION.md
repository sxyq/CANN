# N01 REGISTER-LIVE-RANGE-X V001

ROUTE=N01 REGISTER-LIVE-RANGE-X
REVISION=V001
PARENT=R31B V011 / Official 45.16
PARENT_SOURCE=线上结果/R31B/V011/submission.asc
PARENT_SOURCE_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
CANDIDATE_SOURCE=本地实验/N01-REGISTER-LIVE-RANGE-X/V001/submission.asc
CANDIDATE_SOURCE_SHA256=c00a2ac61ea722a84a098d9f047ecf39c539e2dfda98d49e2fec910dd063f2dc
SOURCE_CHANGE=In the FP16 branch of ProcessWideLowPrecision pass 1, square the widened y value in valueFp32 in place and use that same vector as the ReduceSum input.
FUNCTION=ProcessWideLowPrecision
PERFORMANCE_HYPOTHESIS=Reusing the conversion vector for its final square may remove a separate FP32 vector result from the tile's live range and reduce Vector register pressure on wide FP16 rows. Runtime effect is unverified.
UNCHANGED=Numerical operation order and values; reduction order and work buffer; UB allocation; core assignment; DMA; synchronization; tile geometry; dispatch; output path.
FORBIDDEN_OVERLAP=UB Bank; Core allocation; formula changes; DMA synchronization changes.
SUPERSEDED_PROBE_SOURCE_SHA256=fb0ea21d9fa06aed935c0bdbab7d1afdf3765344d7ad6a06b5672a2be0542f37
SUPERSEDED_PROBE=evidence/scalar-probe/ (not built or measured; retained and excluded from V001 Candidate)
DEVICE=server3 NPU 3 (user assignment; existing runtime_probe selects the device from its CLI argument)

## RULE_REFRESH_RECEIPT

HEAD=1c51a5b2eb9b5b9dffc031c1fbbbc35e9506b49f
DIRECT_PARENT=1c51a5b2eb9b5b9dffc031c1fbbbc35e9506b49f
HEAD_PARENT=c9510be4e4a9da0084433e45442468da1eb75c72
BRANCH=route/n01-register-live-range-x
WORKTREE=/Users/sunyiyang/Desktop/Project/cann/worktrees/n01-register-live-range
RULES_READ=AGENTS.md; .agents/skills/cann-route-executor/SKILL.md; 项目规则/服务器实验规范.md; 项目规则/本地性能测试规范.md; 项目规则/Git工作流程.md
MECHANISM=Wide-row FP16 Vector temporary FP32 value lifetime in ProcessWideLowPrecision
NEXT_ACTION=When server3 NPU 3 no longer has PID 1859841, use the existing paired runner for Correctness and interleaved Local; retain fresh load snapshots.

## Results

COMPILE=PASS; existing R31B V016 single-version targets device, submission, full_link; paired_runner_v016 build PASS
CORRECTNESS=NOT_RUN; server3 NPU 3 had PID 1859841 (python3.11, 14238 MB); paired runner calls aclrtResetDevice(device)
LOCAL=NONE; paired runner was not executed; no latency samples collected
DEVICE=server3 NPU 3; SOC=Ascend910B3; CANN=8.5.0.alpha002
EVIDENCE=evidence/compile-server3.log; evidence/runtime-status-npu3.md

## Source Diff

```diff
--- R31B-V011/线上结果/R31B/V011/submission.asc
+++ N01-V001/本地实验/N01-REGISTER-LIVE-RANGE-X/V001/submission.asc
@@ -3183,10 +3183,10 @@
                     AscendC::Muls(yTile, xLocal, half(1.0f), valid);
                     AscendC::PipeBarrier<PIPE_V>();
                     ToFloat(valueFp32, xLocal, valid);
                     AscendC::PipeBarrier<PIPE_V>();
-                    AscendC::Mul(xFp32, valueFp32, valueFp32, valid);
+                    AscendC::Mul(valueFp32, valueFp32, valueFp32, valid);
                     AscendC::PipeBarrier<PIPE_V>();
                     AscendC::ReduceSum(
-                        reduceLocal[batchRow * reduceStride + tile], xFp32,
+                        reduceLocal[batchRow * reduceStride + tile], valueFp32,
                         residualFp32, valid);
```

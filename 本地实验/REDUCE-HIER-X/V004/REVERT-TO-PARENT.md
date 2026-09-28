# REVERT TO PARENT — REDUCE-HIER-X V004

Date: 2026-09-29
Trigger: LOCAL_REJECTED on V004 (N1 binned streaming single-stage square-sum)
Per: MAIN-2 V004 instruction —「LOCAL_REJECTED 时：保留失败证据 → commit/push → 显式 revert 到 FROZEN_R31B_V011 → push」

## 状态

```text
CURRENT_SOURCE      = FROZEN_R31B_V011
CURRENT_SOURCE_SHA  = a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
V004_SOURCE_SHA     = 1926a2f91491981f06424502dde96356661b5d5c5dc30ae190a0e5935265218d (retained as failure evidence)
LOCAL_VERDICT       = LOCAL_REJECTED
```

## 回退动作

1. 失败证据全部保留（不删除）：`本地实验/REDUCE-HIER-X/V004/` 含声明、submission.asc、
   submission.sha256、source-meta.json、diff.patch、local-result.json、build/correctness
   结果、完整 raw timing（171 文件）。
2. server3 工作区 `phase4-workspaces/REDUCE-HIER-X/submission.asc` 已覆盖回
   `parent.asc`（SHA 核对 = a8c19a19…）。
3. 本 worktree 无独立于证据目录的工作源文件；路线当前源码身份回到 FROZEN_R31B_V011。
4. 未改任何共享成绩记录（路线成绩表/全版本记录由 Main-2 统一登记）。

## V004 结论摘要

- 正确性 PASS（宽 FP32 16k/32k 为父版预存 golden 差异）。
- 主探针双侧 same-binary 合格：1x32768 +23.89%（6/0 favP）、1x16384 +21.17%（6/0 favP）。
- 8x8192 干净 pair 也 favP +10..18%。1x4096 对照无信号（S5 未动）。
- 结论：N1 被证伪。分箱流式累加（零 barrier、正确性安全）比父版的 per-tile ReduceSum
  + collapse 更慢——手工分解出的大量小 Add 指令的调度开销超过省下的 ReduceSum 调用。

## 对路线空间的含义

四个 large-D 归约拓扑变体全部证伪（V001 fold / V002 tree / V003 short-span /
V004 binned single-stage）。两种结构端点都输或持平：减少 ReduceSum 调用（手工 Add）
更慢；缩短 span（预折）中性。per-tile `ReduceSum` API 调用在本工具链已接近最优。
归约拓扑轴在 FROZEN_R31B_V011 谱系的剩余空间视为耗尽（事实陈述；路线生命周期由
Planning 决定）。

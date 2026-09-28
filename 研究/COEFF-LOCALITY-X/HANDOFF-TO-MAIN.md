# HANDOFF-TO-MAIN — COEFF-LOCALITY-X V004 完成

ROUTE=COEFF-LOCALITY-X
WORKTREE=/Users/sunyiyang/Desktop/Project/cann-m2-coeff
BRANCH=m2/coeff-locality
FROM=Route Agent（LANE M2-3）
TO=Main-2
DATE=2026-09-29
PHASE=V004 执行完成 — 本地结论已下，LANE 事实包已写，STOP

---

## 1. 需求与状态

Main-2 批准 NH-1（=H3 精化，generic 单行多 tile 整行预加载）作为 V004。
要求：声明先 commit；完整流程；带 1x4096 控制组；LOCAL_REJECTED 则回退并写 LANE 事实包。

**状态：V004 流程完成。本地结论 NEEDS_ONE_MORE_LOCAL；按 Main-2「若 NH-1 也无收益」条款，LANE 事实包已写，建议 COEFF 转 LANE_NEEDS_PLANNING_REVIEW。**

## 2. 本轮实际完成

1. V004 声明两次 commit（初版 + 实现前范围收窄：预加载块与多行路径共用，合并 Load 会波及范围外population，故 diff 收窄为仅条件一行）。
2. Kernel 修改：`Process()` 的 `cacheParams` 条件一行——
   `cacheRow && localRows > 1` → `cacheRow && (localRows > 1 || rowWidth > kTileElems)`。
   UB 增量 0、tileElems 钉 4096、MTE2 次数不变（仅移出输出通道关键路径）。
3. server3 编译链接 PASS（SHA 848165b6…）。
4. NPU 正确性 18/18 与 parent 逐形一致（wide 1x16384/1x32768 同 parent 双侧失败——证明 wide 路径未动）。
5. 三轮测时（d4 标准、d5 复测、d4 加长采样），带双控制组。
6. 本地结论 NEEDS_ONE_MORE_LOCAL；LANE 事实包 `本地实验/COEFF-LOCALITY-X/V004/LANE-FACTS.md`。

## 3. 修改或操作对象

| 对象 | 路径 |
|---|---|
| V004 源码/声明/diff/元数据/结论 | `本地实验/COEFF-LOCALITY-X/V004/` |
| 三轮测时证据 | `本地实验/COEFF-LOCALITY-X/V004/support/`（results-timing-v004*/ 全部 raw） |
| LANE 事实包 | `本地实验/COEFF-LOCALITY-X/V004/LANE-FACTS.md` |
| handoff | `研究/COEFF-LOCALITY-X/HANDOFF-TO-MAIN.md` |

未改共享总账（`技术路线/`、`调度/`）。

## 4. 验证结果

| 阶段 | 结果 |
|---|---|
| 编译/链接 | PASS（exe 2c7a800f…） |
| 正确性 | 与 parent 逐形一致（16 PASS + 2 同 parent shared-golden fail） |
| 三轮配对干净中位（负=candidate 快） | 1x6144：-4.83% / -10.22% / **-4.25%**（合计 15/18 favC）；1x8192：-4.35% / -0.52% / -0.68% |
| **控制组 1x32768（路径未动）** | R1 -1.14% / R2 +1.03% / **R3 -2.77%（5/6 favC）** |

**结论**：1x8192 按 DECLARATION 判据**证伪**（Δ≤控制偏置）。1x6144 三轮方向稳定但超额仅 ~1.5%，且控制组同号偏置达 -2.77%——残留信号不可与 binary-wide 装置偏置干净分离。判定 NEEDS_ONE_MORE_LOCAL。

## 5. 剩余工作与风险

- **建议 COEFF-LOCALITY-X → LANE_NEEDS_PLANNING_REVIEW**（依据 LANE-FACTS.md）：三个机制维度（时序/驻留/预加载）已全部触及，large-D 证伪，mid-D 残留不可分离。
- **备选**：null-binary 对照（parent-rebuild vs parent，无 kernel 改动）可裁决 1x6144 残留性质——若 Planning 要彻底关账可批此项。
- LOCAL_BEST=NONE，CURRENT_CANDIDATE=V004（不叠加新变化），OFFICIAL_BEST=44.16（V001）不变。
- 短 kernel 测量经验（warmup≥60/samples≥81/sb×3、必须带控制组）已写入 LANE-FACTS 供其他路线复用。

**本轮结论已下，STOP，等待 Main-2 / Planning 对 COEFF 路线状态的裁定。**

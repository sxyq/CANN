# HANDOFF-TO-MAIN — ASYNC-OVERLAP-CHAMPION-X V002

收件：Main-2
状态：V002 本地流程走完，**STOP**。不 Online、不开 V003。

## 当前需求与状态

W2 NarrowMid 行间发射重排已实现并完成本地测量。正确性干净，性能方向混杂。

## 本轮实际完成

1. V002 声明（DIRECT_PARENT=R31B-V011，从 Champion 种子干净起）。
2. OFAT 源码：`ProcessNarrowMidOverlap` 中 FP32/FP16 把下一行 x/res Load 提前到
   invRms 标量尾之前；`SyncMTE3ToV` 下沉到覆写 store 源之前；BF16 因 store 读
   xBuf_ 保持原序。SHA `f8f65e09…`。
3. server3 编译链接 PASS。
4. NPU 正确性：7 形状 OUTHASH 与 parent 逐位一致（`PASS_VS_PARENT`）。
5. same-binary + 交错 P/C：2 形状合格，方向均混杂。
6. 证据分步 commit+push。

## 修改或操作对象

| 对象 | 位置 |
|---|---|
| V002 源码 | `本地实验/ASYNC-OVERLAP-CHAMPION-X/V002/submission.asc` |
| 声明/元数据/diff | `…/REVISION-DECLARATION.md`, `source-meta.json`, `diff.patch` |
| 构建正确性 | `…/BUILD-CORRECTNESS.md`, `build-correctness.log` |
| 本地性能 | `…/LOCAL-PERFORMANCE.md`, `local-result.json`, `results/`（52 文件） |
| server3 | `phase4-workspaces/ASYNC-OVERLAP-CHAMPION-X/{V002,PARENT-CORR,timing-v002}` |

## 验证结果

| 阶段 | 结论 |
|---|---|
| BUILD / LINK | PASS |
| CORRECTNESS | `PASS_VS_PARENT`（NarrowMid FP32/FP16/BF16 + 控制组 OUTHASH 全等） |
| SINGLE_CHANGE_AUDIT | PASS |
| LOCAL | **`NEEDS_ONE_MORE_LOCAL`** |

## 本地结论依据

| shape | SB | 干净对方向 |
|---|---|---|
| 128×2048 fp16 | 双方合格 | 2 偏 V002 / 1 偏 parent（−8.9% ~ +8.7%） |
| 128×4096 fp32 | 双方合格 | 1 偏 V002 / 1 中性 / 1 偏 parent |

两形状均为混杂方向，p10 也混杂。无稳定改善、无稳定回退。
NarrowMid kernel 只有 ~8–10 µs，invRms 标量尾短，提前 Load 的覆盖窗口可能不足。

## 选项 A 补测（2026-09-28，终局）

条件：localRows≥2/核（512 rows / 40 cores = 12–13 行/核）、≥6 组交错、双方 SB 合格。

| shape | 干净对方向 | 判定 |
|---|---|---|
| 512×2048 fp32 | 2 偏 V002 / 1 偏 parent / 1 中性 | 混杂 |
| 512×1024 fp32 | 1 偏 V002 / 3 偏 parent / 2 中性 | **偏 parent** |

**终局 `NEEDS_ONE_MORE_LOCAL`**，不再烧 W2 测量预算。
机制天花板：NarrowMid per-row 固定成本由 GetValue / V-S 往返主导，
提前发下一行 Load 砍不动这些往返。

## 下一假设建议

**倾向 W4 — GetValue handoff 收敛**（见 `TRACK-B-HYPOTHESES-V002.md`）：

- 每行两轮 `SyncVToS → GetValue → SyncSToV`（L565–574）是最大串行段。
- 在保持算术顺序不变的前提下合并/减少标量往返。
- 风险：精度，须先做 OUTHASH 逐位对照。
- 备选：W1（FullCache inter-pass，对准 case 14 若为 FP32 wide）。

## 剩余工作与风险

- W2 信号混杂且 512×1024 偏 parent，不推进 LOCAL_BEST。
- 未改共享总账、未 Online、未开 V003、未 revert。

## 请求 Main-2

- 确认 W2 终局 `NEEDS_ONE_MORE_LOCAL`。
- 是否批准下一假设 W4（或 W1）进入 V003 研究/声明。

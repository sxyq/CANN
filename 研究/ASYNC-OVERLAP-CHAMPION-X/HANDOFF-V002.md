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

## 剩余工作与风险

- W2 不是稳定胜，也不是稳定负；不推进 LOCAL_BEST。
- 若继续 W2，需要 localRows≥2/核 且 invRms 尾更长的形状，或更安静窗口。
- 备选假设已在 `TRACK-B-HYPOTHESES-V002.md`（W1/W3/W4/W5）。
- 未改共享总账、未 Online、未开 V003、未 revert。

## 请求 Main-2

- 确认 `NEEDS_ONE_MORE_LOCAL` 归类。
- 定夺：A 同假设再测 / B 换 W1/W3/W4/W5 / C 交 Planning 处置 W2。

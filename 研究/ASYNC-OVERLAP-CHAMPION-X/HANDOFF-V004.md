# HANDOFF-TO-MAIN — ASYNC-OVERLAP-CHAMPION-X V004

收件：Main-2
状态：V004 本地流程走完，**STOP**。不 Online、不开 V005。

## 当前需求与状态

W1 FullCache inter-pass prologue 已实现并完成本地测量。控制组逐位一致；
FullCache 性能方向完全对半开。

## 本轮实际完成

1. V004 声明（DIRECT_PARENT=R31B-V011）。
2. OFAT 源码：FullCache pass-2 tile-0 参数 Load 提前到 invRms 尾；
   Duplicate/Sqrt 移到 residualBuf_ 以释放 xBuf_。
3. **bugfix**：首版在 GetValue 前 Load 覆盖 xBuf_（max_abs 1.85），已修复。
4. 正确性：控制组 OUTHASH 逐位一致；FullCache 为 parent 非确定路径，
   max_abs 双方同量级（0.10–0.13），已标注 UB-GAP-CLUE。
5. 测时：128×16384 fp32 双方 SB 合格（kernel ~23 µs），6 组交错 P/C。
6. 本地结论：`NEEDS_ONE_MORE_LOCAL`。

## 修改或操作对象

| 对象 | 位置 |
|---|---|
| V004 源码 | `本地实验/ASYNC-OVERLAP-CHAMPION-X/V004/submission.asc` |
| 声明/元数据/diff | `…/REVISION-DECLARATION.md`, `source-meta.json`, `diff.patch` |
| 正确性 | `…/BUILD-CORRECTNESS.md`, `correctness-outhash.log` |
| 性能 | `…/LOCAL-PERFORMANCE.md`, `local-result.json`, `results/`（36 文件） |
| server3 | `…/ASYNC-OVERLAP-CHAMPION-X/{V004,timing-v004}` |

## 验证结果

| 阶段 | 结论 |
|---|---|
| BUILD | PASS |
| CORRECTNESS | `PASS_WITH_NONDET_PATH`（控制组逐位一致；FullCache 标注 UB-GAP-CLUE） |
| LOCAL | **`NEEDS_ONE_MORE_LOCAL`** |

## 本地结论依据

128×16384 fp32（kernel ~23 µs，双方 SB 合格）6 组干净对：

| 方向 | 对数 |
|---|---|
| 偏 V004 | 3 |
| 偏 parent | 3 |

幅度 1–5%，p10 同样 3/3，落在噪声带内。

## 结构观察

W1 在 FullCache 上的重叠窗口比 H1 在 LP 上小：invRms 把 xBuf_ 当 scratch
用到最后一次 GetValue，提前 Load 只能盖住 Duplicate/Sqrt + 最后标量尾。
要扩大窗口需把 invRms scratch 挪到专用小缓冲——那是 buffer 用法变化，
须单独声明（可能触碰「不加 buffer」边界的解释）。

## 剩余工作与风险

- W4+W1（NarrowMid handoff + FullCache prologue）均未给出稳定胜。
- 若走 Online：FullCache 非确定路径本身有 WA 风险（parent 同样）。
- 未改共享总账、未 Online、未开 V005、未 revert。

## 请求 Main-2

- 确认 `NEEDS_ONE_MORE_LOCAL` 归类。
- 本路线 V001–V004 四个机制（H1/W2/W4/W1）均已测：
  - H1：Official REJECT（本地小胜不迁移）
  - W2：混杂
  - W4：混杂（+ 1-bit 精度差异）
  - W1：混杂
- **建议交 Planning 评估本路线是否 PARK**，或明确下一个机制轴
  （例如 invRms scratch 重定位以扩大 prologue 窗口，但那是新声明）。

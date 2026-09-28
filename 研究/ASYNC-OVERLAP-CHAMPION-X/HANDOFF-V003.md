# HANDOFF-TO-MAIN — ASYNC-OVERLAP-CHAMPION-X V003

收件：Main-2
状态：V003 本地流程走完，**STOP**。不 Online、不开 V004。

## 当前需求与状态

W4 GetValue handoff 收敛已实现并完成本地测量。精度门按裁定接受容差内差异；
性能方向混杂。

## 本轮实际完成

1. V003 声明（DIRECT_PARENT=R31B-V011）。
2. OFAT 源码：NarrowMid invRms 尾两轮 V-S 往返合并为一轮，
   `invRms = 1.0f / std::sqrt(meanSquare)`。SHA `d23593ce…`。
3. OUTHASH 对照：6/7 逐位一致，32×4096 fp32 差 1 bit（容差内）。
   写 `PRECISION-STOP.md` → Main-2 裁定接受 → 写 `PRECISION-DECISION.md`。
4. same-binary + 6 组交错 P/C（512×1024 / 512×2048 fp32，d5）。
5. 本地结论：`NEEDS_ONE_MORE_LOCAL`。

## 修改或操作对象

| 对象 | 位置 |
|---|---|
| V003 源码 | `本地实验/ASYNC-OVERLAP-CHAMPION-X/V003/submission.asc` |
| 精度记录 | `…/PRECISION-STOP.md`, `PRECISION-DECISION.md`, `precision-outhash.log` |
| 性能 | `…/LOCAL-PERFORMANCE.md`, `local-result.json`, `results/`（72 文件） |
| server3 | `…/ASYNC-OVERLAP-CHAMPION-X/{V003,timing-v003}` |

## 验证结果

| 阶段 | 结论 |
|---|---|
| BUILD | PASS |
| OUTHASH | 6/7 逐位一致；1 处容差内差异（已裁定接受） |
| SAME-BINARY | 仅 512×1024 d5 parent 合格；B2 系统性漂移 1.12–1.41 |
| LOCAL | **`NEEDS_ONE_MORE_LOCAL`** |

## 本地结论依据

| shape | 干净对方向（6 组） |
|---|---|
| 512×1024 fp32 | 2 偏 V003 / 2 偏 parent（±10%） |
| 512×2048 fp32 | 2 偏 V003 / 2 偏 parent / 1 中性 |

W2 与 W4 在 NarrowMid 上均未给出稳定胜。per-row 固定成本的瓶颈可能在
**GetValue 往返的绝对次数**（每行至少读一次 squareSum），而非往返间隔或发射时机。

## 剩余工作与风险

- W4 不推进 LOCAL_BEST。
- 若走 Online：32×4096 类形状的 1-bit 差异在更严容差下可能 WA（已记入
  `PRECISION-DECISION.md` 回滚点）。
- 未改共享总账、未 Online、未开 V004、未 revert。

## 请求 Main-2

- 确认 `NEEDS_ONE_MORE_LOCAL` 归类。
- W4 处置建议：
  - 若继续同假设：需要能分辨 ~1% 效应的测量窗口（kernel >20 µs 或更安静设备）。
  - 若换假设：W1（FullCache inter-pass，对准 case 14）或交 Planning 处置本路线。
- NarrowMid 轴（W2+W4）建议合并评估后再决定是否 PARK。

# SHAPE-TILING-CHAMPION-X Handoff — 2026-09-29（多行补测 + case 14 归属）

来源：MAIN-1 DIRECTION after V002（指令 A 补测 + 指令 B 分向）。
性质：V002 的测量补全（同一 binary），不开 V003。

## 指令 A：多行补测结果

同一 V002 binary + parent，device 4，warmup=45，21 交错 pair，device events。
same-binary 5/5 PASS（含新形状 16×32768，median 20.46 µs）。

空白对照噪声带（当轮现测）**[-0.08, -0.06] µs**：

| 形状 | paired_delta_median | 方向（负/正） | first10 / last10 | 判读 |
|---|---|---|---|---|
| fp32-blank-d16384-r8 | -0.060 µs | 11 / 10 | -0.24 / +0.18 | 空白 |
| fp32-blank-d16384-r2 | -0.080 µs | 12 / 9 | -0.08 / -0.04 | 空白 |
| fp32-change-d32768-r2 | -0.080 µs | 13 / 8 | -0.04 / -0.08 | 带边缘 |
| fp32-change-d32768-r8 | -0.060 µs | 11 / 10 | -0.06 / +0.02 | 带内 |
| **fp32-change-d32768-r16** | **-0.160 µs** | 12 / 9 | **-0.26 / +0.08** | median 探出带，但方向不稳 |

**r16 的 −0.160 不能算信号**，两个理由：

1. **方向不稳**：12/9 只是弱偏向，first10=-0.260 而 last10=+0.080，后半段反号。
2. **幅度与机制不匹配**（这是更强的理由）：
   - 单行 D=32768 成本 ≈ 8.28 µs（16× 减 8× 的 same-binary medians）
   - tile 往返 8→6，降幅 25%
   - 若往返固定开销主导，应省 0.2–2 µs/行
   - 实测 −0.16 µs，比机制预测小一个数量级以上
   → 该幅度只能是噪声，不是 tile 档位的作用

结论：**多行补测仍无稳定信号**。按 MAIN-1 判据，这构成「tile 轴在该机制下无收益」的强证据，
V003 不做 tile 常量类改动。`local-result.json` 已追加 `MULTISCALE_TIMING` 节，原闭环记录未改。

## 指令 B：case 14 瓶颈归属（只读，已完成）

报告：`研究/SHAPE-TILING-CHAMPION-X/CASE14-BOTTLENECK-ATTRIBUTION.md`

核心结论：

| 段 | 判定 | 依据 |
|---|---|---|
| tile 往返 | **已排除** | 多行幅度核对：往返降 25% 但实测 < 机制预测 1/10 |
| 归约数学 | 非主缺口 | 8 次 partial + 1 次收尾，相对每 tile 5 个向量算子占比小；invRms 非确定是正确性问题不是 4.4x 来源 |
| store | 次要可收项 | STORE-EPILOGUE-X 单项已知收益 0.09 分量级，填不满 4.4x |
| **每 tile 同步 / 流水深度** | **最可疑主缺口** | 每行约 48 次 `PipeBarrier<PIPE_V>` + 16 次 `SyncMTE2ToV/SyncVToMTE2`；按 8.28 µs/行摊，同步段与 8.28 同量级 |

跨 D 单价：D 翻倍每行 +3.9~4.2 µs（两组独立测点一致）→ 成本随元素数线性，
主导是 O(D) 的向量工作与每 tile 同步，不是行固定开销、也不是往返固定开销。

## NEXT（供 Main 选，本路线不自行开 V003）

1. **分段计时**（只读测量补全，不占 Revision 预算）：在 paired_runner 里给 Pass1/Pass2 分别打 event，
   量出两段占比；同时列每行 `PipeBarrier`/`Sync*` 计数，估单次同步单价。
   这能把「同步主导」从推断变成实测。
2. **若确认同步主导** → 下一个性能假设应是「减少每 tile 同步次数 / 加深 MTE 流水」，
   按分工归 R31B 或 EPILOGUE-ARITH lane，**不归本 lane**（本 lane 只做 tiling/tile-policy）。
   ASYNC-TRIPLE-X 在此方向有 507035 失败史，需更强正确性护栏。
3. **tile 轴**：建议停止 tile 常量类改动（H1/H2 两档 + 多行补测均无信号且幅度核对失败）。
   H3 SPLIT_D / H4 MERGE_N 正确性风险高，本阶段不开（MAIN-1 已重申）。

## 预算状态

闭环性能 Revision 仍为 **2/6**（本轮是测量补全，不开新 Revision）。
连续无收益 2，但本轮产出了机制幅度核对这一新信息（tile 轴证伪），按 MAIN-1 判断不触发 `LANE_NEEDS_PLANNING_REVIEW`。

## 证据位置

- 多行补测：`本地实验/SHAPE-TILING-CHAMPION-X/V002/logs/multiscale-{same-binary,paired}.log`
- 追加记录：`本地实验/SHAPE-TILING-CHAMPION-X/V002/local-result.json` → `MULTISCALE_TIMING`
- case 14 归属：`研究/SHAPE-TILING-CHAMPION-X/CASE14-BOTTLENECK-ATTRIBUTION.md`
- runner（新增 `--multiscale` 用例集）：`本地实验/SHAPE-TILING-CHAMPION-X/V002/support/paired/`
- server3：`/home/data4t2/lelinfeng/shape-tiling-champion-x/V002/`

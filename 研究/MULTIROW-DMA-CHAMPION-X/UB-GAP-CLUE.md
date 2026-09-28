# UB-GAP-CLUE — Frozen parent FP32 wide 路径输出非确定性

ROUTE: MULTIROW-DMA-CHAMPION-X（记录路线；缺陷不属于本路线）
日期: 2026-09-28
状态: EVIDENCE_ONLY — 交 Planning；本路线不修、不建 UB V004
登记依据: Main-2 V001 Main Review 指示「正式登记交 Planning，你不修」

## 线索内容

Frozen parent（R31B-V011，SHA `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`）的 **FP32 wide 路径**在确定性输入下输出逐次不同。

同一 binary（`mrx_parent_probe`）在 d4 连跑 3 次，形状 8×16384 FP32，输入由固定公式生成（`InputValue(i,37,11)` / `InputValue(i,17,3)`，非随机）：

| 次序 | bad | gross_bad | max_abs |
|---|---|---|---|
| run 1 | 96269 | 91373 | 1.299292 |
| run 2 | 92999 | 88084 | 1.260196 |
| run 3 | 94249 | 89585 | 1.235985 |

候选 binary（`mrx_candidate_probe`，V001）同类波动（bad=97390/95945/95245，max_abs 1.235985/1.235985/1.299292）——V001 未触及 FP32 路径，波动来自父本行为。

对照：**LP（FP16/BF16）wide 路径完全确定**。48×16384 FP16 parent/candidate 各 3 连跑，max_abs 恒为 0.003424077、bad 恒为 3，逐位一致。

## 指向

- 路径：`ProcessWideFp32FullCacheRows`（FP32 全部 wide 形状的唯一分发目标，`ProcessWideFp32` 一句直通）。
- 性质：确定性输入 + 确定性数学 → 输出应逐次一致。run-to-run 漂移指向 kernel 侧**未初始化 UB 读取或缓冲别名/复用行为**（对 binary 布局或执行时序敏感），不是 harness golden 单方面算错。
- 与既有记录的关系：main2-r2-route-registry 已记「Frozen parent already fails some local-runner goldens on wide FP32 D=16384/32768 (max_abs≈0.44–1.2)」。本线索补强：该偏差至少部分是输出本身非确定，而非仅 golden 公式差异。
- 相邻线索：SCHED-CHAMPION-X 曾发现 `ProcessNarrowMidOverlap` BF16 的 `xBuf_` 别名竞态（`SetFlag<V_MTE2>(inputRelease)` 早于 MTE3 Store 排空），说明冠军内缓冲复用语义存在脆弱点。FP32 wide 的非确定性可能同类。

## 证据位置

- 复跑命令与输出：见 V001 handoff 会话记录；探针为 `server3:/home/data4t2/lelinfeng/phase4-workspaces/MULTIROW-DMA-CHAMPION-X/build/mrx_{parent,candidate}_probe`。
- Correctness 矩阵（FP32 wide 两形状 parent/candidate 均 FAIL，bad 计数不同）：`本地实验/MULTIROW-DMA-CHAMPION-X/V001/support/correctness-summary.tsv`。
- LP 路径确定性对照：同 support 目录与 `local-result.json`。

## 明确边界

- 本路线**不修**此问题。
- **不建 UB V004**、不改 `ProcessWideFp32*`、不动 UB 生命周期/别名结构。
- 处置权：Planning / Review Layer；如需修复由指定路线（UB-LIVENESS 血统或新授权）执行。

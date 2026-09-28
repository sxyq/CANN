# HANDOFF-TO-MAIN — MULTIROW-DMA-CHAMPION-X V001 完成

ROUTE: MULTIROW-DMA-CHAMPION-X（LANE M2-5）
WORKTREE: `/Users/sunyiyang/Desktop/Project/cann-m2-multirow`
BRANCH: `m2/multirow-dma-champion`
DIRECT_PARENT: R31B-V011 / `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3` / 45.16
CANDIDATE_SHA: `5dea0eaf1752fd2ef671ec8d69bc8f27d490413c2043ea64bebe0ddf943dee5f`
PHASE: V001 本地结论完成，STOP

## 结论（一句话）

**V001 LOCAL_REJECTED**：H1 stride 多行输入在主战场 48×16384 FP16 上 4/4 一致退步（device-event median +3.2%…+13.4%，中位 +8.4%）；事务减半没抵过失去 V011 2-deep 预取（零 UB 增长设计所迫）。正确性在触及路径上与 parent 逐位一致。

## 流程与证据

| 阶段 | 结果 | 证据 |
|---|---|---|
| Revision 声明（改码前） | 完整字段 + UB 数字推演 | `本地实验/MULTIROW-DMA-CHAMPION-X/V001/REVISION-DECLARATION.md` |
| UB 推演（风险前置） | 2-deep×B 暂存超出可用 UB（FP16 16384 B=2 差 128B）→ 走「缩 B」分支，chunk=2，`ChooseWideFullYRows` 语义未动 | 同上 |
| 编译/链接 | PASS（Build Fix：`DataCopy`→`DataCopyPad`+ExtParams，API 形态适配） | source-meta.json |
| Correctness | 触及 LP 路径 bit-identical；BF16 48×12288 0/0；回归矩阵干净 | `support/correctness-summary.tsv` |
| Same-binary | 三形状 PASS（warmup=45，2×31 device-event） | `support/timing-results/` |
| 交错 P/C | 4 对/形状，顺序交替 | 同上 + local-result.json |
| 本地结论 | **LOCAL_REJECTED** | `local-result.json` |

## P/C 摘要（device-event medians, µs）

| 形状 | 角色 | P | C | Δ% | 方向 |
|---|---|---|---|---|---|
| 48×16384 FP16 | 主场（batchRows=2） | 13.30/12.92/12.94/13.66 | 13.72/14.16/14.68/14.64 | +3.2/+9.6/+13.4/+7.2 | **4/4 退步** |
| 48×12288 BF16 | 主场 | 13.16/13.10/13.00 | 12.88/13.70/13.20 | -2.1/+4.6/+1.5 | 噪声内（第 4 对 LOAD_CONTAMINATED 留证） |
| 8×16384 FP16 | 阴性对照（localRows=1） | 8.48/8.16/8.12/8.20 | 8.36/7.78/8.16/7.86 | -1.4/-4.7/+0.5/-4.1 | 同代码噪声底 ≈5pp |

## 机制结论（不是构建问题）

零 UB 增长下无法同时保留 2-deep 与 B 行暂存；实测证明在该平台上「MTE2 命令减半」<「失去 (行,tile) 流 2-deep 预取」。若继续 stride 方向，需要能容纳 2×chunk 暂存的 UB（即动 `ChooseWideFullYRows` 的计入方式）或换保持流水的发事务形态——两者都是新假设，须 Main/Planning 决定，V001 本身按规则回退到 FROZEN parent。

## 附带发现：UB correctness gap 线索（只记 evidence，未修）

Frozen parent **FP32 wide 路径**在确定性输入下输出逐次不同（同一 binary 三跑 bad=96269/92999/94249；max_abs 1.23–1.30）；candidate binary 同类波动。LP 路径完全确定且与 parent 逐位一致。解读：已记录的 wide FP32 golden 偏差至少部分是 kernel 侧未初始化/别名行为（未触及的 `ProcessWideFp32FullCacheRows`），不是纯 harness 差异。交 Main-2/Planning；本路线不修、不建 UB V004。

## 请 Main-2 处理

1. 登记本地结论 `LOCAL_REJECTED`（LOCAL_BEST 回到 FROZEN R31B-V011）。
2. 共享 lease 记录：本路线测量用 `M2-MULTIROW-V001-D4-PERF-20260928`（d4），已记在 local-result.json；`调度/服务器设备使用.tsv` 的行请 Main 补登记（Route Agent 未改共享文件）。
3. UB correctness gap 线索（FP32 wide 非确定性）转 Planning。
4. 下一假设方向（仅建议，待批）：H3 ALIGNED-DATACOPY-FORM（独立、diff 更小）；或「保持流水的 stride 形态」（需 UB 计入方式讨论）；H2/H4 建议不再单独立项（同机制家族，且 H1 已给出负面信息）。

## 边界遵守

- OFAT：只改输入侧发事务形态；dispatch/ownership/y 驻留/Store/gamma-bias/tile 宽度未动。
- 未跑 server3 计时以外的额外实验；未提交 Online；未改共享总账（含 lease 表）；未修 FP32 wide 问题。
- 失败/污染样本全部保留（`support/timing-results/`，含 pc_B4_C 双峰污染样本）。

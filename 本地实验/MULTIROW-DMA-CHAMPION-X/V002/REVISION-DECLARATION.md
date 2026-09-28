# V002 REVISION-DECLARATION — MULTIROW-DMA-CHAMPION-X

（修改前记录。Main-2 裁定 2026-09-28：批准 C1 ALIGNED-DATACOPY-FORM；C2 不批准本 Revision；C3 不启用。）

## 声明字段

| 字段 | 值 |
|---|---|
| ROUTE | MULTIROW-DMA-CHAMPION-X |
| REVISION | V002 |
| REVISION_KIND | PERFORMANCE_SINGLE_HYPOTHESIS |
| DIRECT_PARENT | R31B-V011（FROZEN；V001 已 LOCAL_REJECTED，不作父） |
| PARENT_SOURCE_SHA | `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3` |
| PARENT_SCORE | 45.16（Official 15/15） |
| OFFICIAL_ANCHOR | 45.16 |
| CONTEXT_CLASS | FROZEN_STRONG_BASELINE_TRANSPLANT |
| SINGLE_HYPOTHESIS | C1 ALIGNED-DATACOPY-FORM：`Load`/`Store` 在 `count*sizeof(T)` 为 32B 整数倍时改发非 Pad `AscendC::DataCopy`（`DataCopyParams` 形态），非对齐保持 `DataCopyPad`。只改单条搬运指令形态；流水（2-deep）、UB、所有权、dispatch、数学全部不动 |
| EXPECTED_SHAPES | 命令密集段：主探针 48×16384 FP16（rows≥2×cores）+ 48×12288 BF16；对照 8×16384 FP16（localRows=1）；回归 33×100、2×4096、1×8192 等 |
| OFAT_AUDIT | 只改指令形态（Pad→非 Pad on 对齐点）；一个概念变化 |

## WHY_NOT_DUPLICATE

- 非 H1（不引入 nBursts>1/stride；V001 的 stride 多行是另一形态且已 LOCAL_REJECTED，不重跑）。
- 非 R015 / BATCH-RESIDENT-X / MODE-X-R015C / R31B V003 四排除形态（不涉 ownership、batch、row_copy、full-y 驻留）。
- 不与 ASYNC lane 重叠：不改事件/流水/发射顺序。
- 不与 DTYPE lane 重叠：不改 dtype 计算路径。
- 不动 `ChooseWideFullYRows`：C2 的 UB 计入问题不在本 Revision 处理。

## 实现前置：DataCopyParams 单位探针（Main-2 要求）

实现前用最小 device 探针确认 `DataCopyParams.blockLen` 的单位与精确传输能力。若不允许精确非 Pad 传输（单位语义无法表达 32B 整数倍长度、或行为不可控），记 evidence 停下报 Main-2，不硬改。

探针结果：见 `support/API-PROBE-RESULT.md`（执行后补记）。

## UB / CORE / DMA / SYNC / PRECISION IMPACT

| 项 | 影响 |
|---|---|
| UB | 0 字节变化 |
| CORE | 0 |
| DMA | 事务条数 0 变化；仅指令编码形态（Pad→非 Pad）在对齐点变化 |
| SYNC | 0 变化（事件/流水结构不动） |
| PRECISION_RISK | 无（同字节搬运）。仍跑全 golden 矩阵 |

## MINIMAL_OFAT_DIFF

`Load`/`Store` 两个助手中各加一个对齐分支：`count*sizeof(T)%32==0` → `DataCopy`+`DataCopyParams`；否则原 `DataCopyPad`。其余全部字节级不变（含 V011 unit 流、pass-2、host）。

## 风险注记（NPU 实测前保留）

Pad/非 Pad 可能同微码——若 P/C 落在同代码噪声底（≈±5pp）内，是机制中性结论，不是测量失败。主探针仍用 48×16384 FP16（V001 同场），便于与 V001 的 -8% 对照读数。

---

## V002 执行记录（修改后补记）

- API 探针 PASS（blockLen=32B 单位，精确传输成立），未触发停机条款。见 `support/API-PROBE-RESULT.md`。
- 实现：仅 `Load`/`Store` 各一个对齐分支（32B 整数倍 → 非 Pad `DataCopy`+`DataCopyParams`；否则原 Pad）。SOURCE_SHA=`2c23ce32752498a96031d410bb90a5437b2046798584c948a0b77a5963ee5da6`。编译/链接 PASS。
- Correctness：20/22 形状对逐位一致；仅 FP32 wide 两形状不同（已登记的父本非确定路径，V002 未触及）。
- Same-binary 三形状 PASS（drift≤5.8%）。
- P/C：主形状 8 对 6/8 为正、median −1.95%；BF16 4 对 3/4 为正、median −3.2%；同代码对照散布 −7.1%..+4.2%（1 对污染留证）。
- **LOCAL_VERDICT = NEEDS_ONE_MORE_LOCAL**：方向温和为正但幅度全部落在噪声带内；不构成 LOCAL_ACCEPTED，也不是 LOCAL_REJECTED。Candidate 保留，不叠加新变化。

## V002 补测窗口（Main-2 指令，终局补记）

- same-binary 重跑（d4）：48×16384 FP16 drift 0.019、8×16384 FP16 drift 0.025，均 PASS。
- 主形状补测 8 对（累计 16 对）：轮 2 **4/8 混合、median −0.28%**；合并 **median −1.84%、10/16 为正向、范围 −9.4%…+4.3%**。
- 同代码对照（累计 5 个可采信对）：median **−1.48%**、散布 **−7.1%…+5.4%**。对照自身的中位数也是负的——噪声不对称。
- 判定：候选中位数（−1.84%）与对照中位数（−1.48%）不可分，轮间方向不一致 → **NEEDS_ONE_MORE_LOCAL 终局**（Main 规则第 3 条）。
- **结论：C1 指令形态非主导成本。** 与 V001 合并解读：该 hot path 主导成本是 MTE2 流水重叠与 V 端指令/事件，不是单条搬运指令形态或命令计数。
- 污染留证：v2_C1_C、r2_K2_C、r2_K3_C（双峰尖峰，MAD 7–18）。
- C2 三选项事实包：`研究/MULTIROW-DMA-CHAMPION-X/C2-OPTIONS-FACT-PACKAGE.md`，交 Planning。

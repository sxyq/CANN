# TRACK-B HYPOTHESES NEXT — VECTOR-MATH-X

ROUTE=VECTOR-MATH-X
DATE=2026-09-29（第一阶段，只读研究；不改 Kernel、不建 V003、不跑正式 performance、不提交 Online）
WORKTREE=/Users/sunyiyang/Desktop/Project/cann-m2-vector (branch m2/vector-math)
PARENT_OF_RECORD=FROZEN_R31B_V011
PARENT_SOURCE_SHA=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
OFFICIAL_ANCHOR=45.16
ROUTE_OFFICIAL_BEST=V001 44.22（batched 32x256 -7.3% 3/3）
SCOPE=FP32 intermediate / invD / mean / sqrt / reciprocal / normalization arithmetic order / vector math sequence。统一 FP32 中间量。
OUT_OF_SCOPE=reduction topology、store、DMA、row scheduling、tiling、dtype 特化（DTYPE-SPECIAL-X / MAIN-1 边界）、Rsqrt（fast-approx 已拒，不重提）。

---

## 0. 当前事实（只从正式记录读）

| 事实 | 来源 |
|---|---|
| V001 = VM-H2 vector denominator，Official 44.22（ROUTE_OFFICIAL_BEST） | `线上结果/VECTOR-MATH-X/V001/result.json`、`技术路线/全版本记录.tsv` |
| V001 本地 batched 32x256 -7.3% 3/3、8x256 ~-8%；中/大形状无信号 | `本地实验/VECTOR-MATH-X/V001/local-result.json`（JSON 语法错误，verdict 以 registry 为准） |
| V002 = VM-H3a partial（broadcast Mul），中形状 4x2048 -14.0% 5/5、8x1024 -9.2% 3/3，无中形状回归 | `本地实验/VECTOR-MATH-X/V002/support/QUALIFICATION-SUMMARY.md` |
| V002 same-binary 在 d6/d5/d4 三次资格化全 FAIL（5–6µs MAD/med 0.104–0.395），1x32768（12µs）三次 PASS | 同上 + `SHORT-KERNEL-MEASUREMENT-NOTE.md` |
| V002 LOCAL_VERDICT=MEASUREMENT_BLOCKED，包 frozen，不是 LOCAL_BEST | `技术路线/路线成绩表.tsv` |
| DIV_FEASIBILITY_PROBE 已执行且 PASS（P1/P5 max_ulp=0.000；P2 静默数据损坏；P3 ACL 507035） | `DIV-FEASIBILITY-PROBE.md` + `本地实验/VECTOR-MATH-X/SEQ-FUSE-2-PROBE/probe-raw-logs.txt` |
| V003 草稿 DIRECT_PARENT=FROZEN_R31B_V011（不是 V002），HYPOTHESIS_ID=SEQ-FUSE-2 | `SEQ-FUSE-2-SPEC.md` DRAFT REVISION-DECLARATION |

---

## 1. 短 kernel same-binary 测量边界（本轮重点结论）

### 1.1 边界是什么

`项目规则/本地性能测试规范.md` 第 5 节：same-binary 主统计量 **全样本 MAD/median ≤ 0.10 且 |B1_med−B2_med|/median ≤ 0.10** 才 PASS。V002 三次认真资格化（d6/d5/d4，s=41/81、blocks=2/3、batch_n=4 皆已试）证明：

| kernel 长度 | 最好 MAD/med | 门槛 | 结论 |
|---|---:|---:|---|
| 5–6 µs（4x2048 / 8x1024 / 8x256） | 0.104 | 0.10 | 三次全 FAIL，不可达标 |
| ≥12 µs（1x32768） | 0.019 | 0.10 | 三次全 PASS |

对本路线的直接影响：**主目标形状（Official 弱 case 7/6/4/8 类的中等形状）恰好落在不可测带**。V002 最强配对信号（-14% / -9.2%，方向 5/5、3/3 一致）因此不可采信为 LOCAL_ACCEPTED，也无法推进 LOCAL_BEST。V003 若仍把 4x2048/8x1024 当唯一判定形状，会再次撞同一堵墙。

边界性质：这是**测量层/宿主干扰问题**（VLLM 常驻、稀疏 outlier 抬高全分布 MAD），不是数学机制失败，也不是 Candidate 缺陷。规范明确禁止为“修”same-binary 去改 Candidate kernel。

### 1.2 不改测时规范前提下如何拿到可采信证据

规范不可改（唯一测时方法权威；不得发明第三种方法）。以下四条策略全部合规：

**S1. 形状加长，而不是改统计（首选）**
同一数学机制换到 kernel 长度 ≥10–12 µs 的**行密集**形状测：行数拉长总时长，每行分母 tail 仍是被测机制。候选：16x2048、32x1024、64x512、16x4096 一档（需按实际 kernel 时长标定，以 same-binary 能 PASS 为准）。行密集保证 per-row tail 仍是显著开销，机制不被摊薄。这给出真正可进 LOCAL_BEST 判定的形状级证据。

**S2. 控制形状锚定 + 中形状只作方向证据**
1x32768（12µs，same-binary PASS）作 no-regression / tail-amortized 控制（预期 ≈0）。4x2048/8x1024/8x256 的配对 delta 一律标 **directional only**，永不写入 LOCAL_BEST、永不单独支撑 ONLINE_RECOMMENDATION。跨形状方向一致（V002 已有 5/5、3/3）作支持性证据保留。

**S3. 机制级 micro-probe 给出上界（新提议）**
仿 DIV_FEASIBILITY_PROBE 的做法，做一个独立的 **tail-cost micro-probe**：合成 kernel 只跑分母 tail 链（scalar handoff 版 vs 纯 V 版），用 device event 在足够长的循环（如 2000 次 tail）上计时，总时长落在可测带内。产出：µs/row 的 tail 成本差 = 机制收益上界。Candidate 中形状的配对 delta 与该上界对照，可信度显著高于裸配对百分比。不碰 Candidate、不改协议（micro-probe 不是 performance revision，同 DIV probe 级别）。

**S4. 不自改门槛**
长度分层门槛（SHORT-KERNEL-MEASUREMENT-NOTE 建议 1）是**策略问题**，归 Planning / Main-2 决定。本路线在未获裁定前不采用任何非标准 MAD 门槛，不用 trimmed CV 覆盖主统计量，不用 repeat-batch（已证更差），不事后改 outlier 规则。

### 1.3 证据组合的判定顺序

```text
S1 形状（same-binary PASS）上的 P/C  → 形式判定（LOCAL_ACCEPTED/REJECTED）
S3 micro-probe 机制上界            → 收益幅度合理性
S2 中形状方向证据 + 控制形状       → 泛化与回归面
以上齐全才做 ONLINE_RECOMMENDATION；任何一条缺失保持 KEEP_ACCUMULATING
```

---

## 2. 假设列表（全部在数学路径内）

### VMX-N1 — SEQ-FUSE-2：inline reciprocal denominator（纯 V 尾）

成熟度：**READY_FOR_MAIN_REVIEW**

- **MECHANISM**：整条分母 tail 留在 V pipe：`Muls(meanSq, reduceDest, invD, 1)` → `Adds(+ε)` → `Sqrt` → `Div(invRms, ones, meanSq, 1)`，然后 broadcast `Mul` 应用到 value tile。消除 2 次 V/S round-trip、2 次 GetValue pull、1 次 push Duplicate、3 个标量算子。Form A（0 pull）/ Form B（1 pull + Muls）由 probe 已裁定 Form A 可行。
- **BOTTLENECK**：frozen 父版每行 2 次 V→S + 2 次 S→V、2 次 GetValue、标量 mul+add+div、1 次 push back（站点表见 `SEQ-FUSE-2-SPEC.md` BOTTLENECK_EVIDENCE，12 个 live site）。
- **EXPECTED_SHAPES**：中带 4x2048 / 8x1024 / 2x4096（Official 弱 case 7/6/4/8 类）；小带 8x256 / 2x256；大 1x32768 控制。
- **WHY_IT_MAY_HELP**：V002 已在中形状给出 -14% / -9.2% 的方向信号（broadcast 部分），SEQ-FUSE-2 把剩余标量 handoff/除法一并消掉，覆盖 V001+V002 未做完的同一轴端到端；probe 证明 Div 精确 IEEE（0 ulp），数值风险已闭合。
- **WHY_IT_MAY_FAIL**：中形状测量边界未解时信号不可采信；broadcast `Mul` 比标量 `Muls` 多一次 V pass，若 tail 不是绑定成本则增益≈0；大形状 tail 摊薄（预期 ≈0，不作胜点）。
- **ASCEND_FEASIBILITY**：**已证实**。probe P1/P5：`Div(out, ones, denom, 8)` 3 个互异 32B 对齐 slot，max_ulp=0.000；count=1 形式 element0 精确。安全形已锁定。507035 根因=4B 偏移操作数（P3 复现）；dst==src1 是静默数据损坏（P2），必须 dst 与两源互异。
- **UB/CORE/DMA_IMPACT**：+4 个 32B 对齐 8-float UB slot（onesSlot 初始化一次、meanSqSlot、invRmsSlot、bcastSlot，可复用 dead-after-reduction scratch，注意 xFp32Buf_ 在 BF16 路径持有 gamma 不可别名）。无 DMA、无 core 分工变化。
- **SYNC_IMPACT**：每行净 -4 pipe handoff（2×V→S + 2×S→V 消失），剩余 PipeBarrier 只在 V 链内部。不重排任何剩余 wait 的位置（与 MIX-A 边界清晰）。
- **PRECISION_RISK**：低。Div=精确 IEEE 单精度除法（probe 0 ulp vs double 参考），与标量 `1.0f/x` 等价；ε、invD 数值不变；无 fast-approx。
- **DUPLICATE_CHECK**：V001（VM-H2）只把 meanSquare 移进 V，保留全部 handoff 与标量除法——是本轴的 partial；V002（VM-H3a partial）只换 apply 为 broadcast Mul 且 Div 回退——同轴 partial。SEQ-FUSE-2 是该轴端到端形态，变量单一（tail placement）。STORE-H2B（writeback）、EPILOGUE VMLA/SCALE-FOLD（affine）、REDUCE-HIER（reduction）、SCHED（row ownership）、DTYPE-SPECIAL（per-dtype）均不触碰。
- **MINIMAL_OFAT_DIFF**：仅替换 12 个 live site 的分母 tail 为纯 V 链 + apply 形式；无第二机制；Form A/B 是同一 fused tail 的 apply 形式，由 probe 裁定，不构成第二个 revision。
- **EXPECTED_LOCAL_PROBES**：S1 形状（16x2048 等 ≥10µs 行密集）上 same-binary+P/C；1x32768 控制；S3 tail-cost micro-probe 上界；中形状方向证据。STOP_CONDITION 见 spec（probe 失败 / 精度回归 / 中带全噪声 / 测量策略未决）。

### VMX-N2 — BATCHED-DENOM-CHAIN：批量 B 行分母单链（VM-H4 的 SEQ-FUSE-2 后继）

成熟度：**NEEDS_MORE_EVIDENCE**

- **MECHANISM**：batched 路径（`ProcessSmall*Batched*`、`Process*FullTileBatched*`）把每 batchRow 一条的 meanSq→Sqrt→reciprocal 链合并成一次 B 元素向量链：`Muls/Adds/Sqrt/Div(count=B)` 一次出 B 个 invRms，再批量 apply。
- **BOTTLENECK**：batched 路径每 batchRow 重复整条分母尾（B×handoff、B×标量算子）。
- **EXPECTED_SHAPES**：batched 8x256、32x256、16x128、64x64。
- **WHY_IT_MAY_HELP**：B 行摊 handoff 与发射次数；V001 在 batched 32x256 的 -7.3% 说明该路径 tail 有感。
- **WHY_IT_MAY_FAIL**：依赖 N1 落地后的纯 V 形态；B 不固定时向量链需要掩码/尾块处理；若瓶颈在发射延迟而非 handoff 数，收益有限。
- **ASCEND_FEASIBILITY**：Muls/Adds/Sqrt/Div 均为标准 V API（toolkit 8.5.0.alpha002 header 已确认 `Div`/`Muls`/`Adds` 存在）；B 元素 count 合法性沿用 probe 的 8 对齐形（B≤8 一拍，更大 B 分块）。
- **UB/CORE/DMA_IMPACT**：+1 个 B×8 scratch；无 DMA / core 变化。
- **SYNC_IMPACT**：每 batch 的 handoff 从 O(B) 降到 O(1)。
- **PRECISION_RISK**：低（同一 FP32 公式，批量化不改运算次序）。
- **DUPLICATE_CHECK**：是 N1 的发射粒度后继，不是重复（N1=单行 tail placement，N2=跨行批量发射）。需在 N1 落地并有可采信结果后单独声明。
- **MINIMAL_OFAT_DIFF**：只改 batched 分母链的发射粒度，不改 reduction、store、调度。
- **EXPECTED_LOCAL_PROBES**：同 N1 的 S1/S2/S3 组合，batched 形状为主。

### VMX-N3 — ZERO-PULL-BCAST apply：消除最后一次 Duplicate/pull（VM-H3c 后继）

成熟度：**NEEDS_MORE_EVIDENCE**

- **MECHANISM**：invRms 算出后不 pull、不 Duplicate 标量；用 Level-0 `Mul` 的 src stride-0 块广播（8 元素块重复）把 invRms 块直接作用到 value tile。
- **BOTTLENECK**：apply 步的 1 次 pull + 1 次 Duplicate（N1 Form A 已把降到 bcastSlot 构造一次；N3 进一步消构造）。
- **EXPECTED_SHAPES**：行密集中带（每行 apply 次数多）。
- **WHY_IT_MAY_HELP**：在 N1 之上再省每行一次 UB 构造；行多时累积。
- **WHY_IT_MAY_FAIL**：8 元素块广播 ≠ 单元素广播；填充 `xFp32[0:8]` 为同一 invRms 的 UB 内构造途径未验证（V002 已确认 `Duplicate` 只收宿主标量、Brcb 类 API 在本 toolkit header 未检出）；可能 INFEASIBLE。
- **ASCEND_FEASIBILITY**：**未验证**，需要独立 feasibility micro-probe（同 DIV probe 级别）：能否无 pull 地把 1 个 UB 值填成 8 块。
- **UB/CORE/DMA_IMPACT**：无新 buffer（复用 N1 的 invRms/bcast slot）；无 DMA / core 变化。
- **SYNC_IMPACT**：进一步减少 S 侧参与，无新增同步。
- **PRECISION_RISK**：低（同一 invRms 值的广播应用）。
- **DUPLICATE_CHECK**：与 N1 的 apply 步重叠但变量不同（构造方式 vs tail placement）；必须排在 N1 之后，不可合并进同一 revision。
- **MINIMAL_OFAT_DIFF**：只换 bcastSlot 的构造方式。
- **EXPECTED_LOCAL_PROBES**：先 feasibility probe；通过后同 N1 测量组合。

### VMX-N4 — PACKED-RECIPROCAL：多行 1 元素尾巴打包成 count=8 V 操作

成熟度：**NEEDS_MORE_EVIDENCE**

- **MECHANISM**：窄/短行场景下，把最多 8 行各自的 1 元素 meanSq→Sqrt→reciprocal 打包成一次 count=8 的 V 操作序列（操作数在 UB 中连续排列），只改**发射粒度**，不改哪行在哪算。
- **BOTTLENECK**：短行多次 1 元素 V op 与 handoff 的发射开销。
- **EXPECTED_SHAPES**：8x256、2x256、短行密集 batched。
- **WHY_IT_MAY_HELP**：1 元素 V op 对齐约束严格（4B 偏移即 507035），打包成 8 对齐块既提发射效率又避开对齐坑。
- **WHY_IT_MAY_FAIL**：要求 8 行 partial 在 UB 连续——若 reduction 输出布局不连续，需要额外搬运（那就出数学路径，触 reduction/DMA 边界 → 本假设必须停下并报告）；收益可能小于 N1/N2。
- **ASCEND_FEASIBILITY**：count=8 Div/Sqrt 已由 probe 证实；连续布局前提是 open question。
- **UB/CORE/DMA_IMPACT**：可能需要 1 个 8 元素 pack scratch；若需要 DataCopy 拼接则判 OUT_OF_SCOPE 并停止。
- **SYNC_IMPACT**：handoff 次数下降（每 8 行 1 次级别）。
- **PRECISION_RISK**：低（逐元素同一公式）。
- **DUPLICATE_CHECK**：与 N2 的差别是跨行打包 vs 批量链；与 N1 差别是发射粒度。单变量是「1 元素 op → 8 元素打包 op」。
- **MINIMAL_OFAT_DIFF**：只改分母 tail 的操作数宽度。
- **EXPECTED_LOCAL_PROBES**：先布局可行性核对（只读查 partial 在 UB 的落位）；再 S1 组合。

---

## 3. 汇总表

| ID | 单变量 | 预期收益 | 可行性 | 成熟度 |
|---|---|---|---|---|
| **VMX-N1 SEQ-FUSE-2** | 分母 tail 标量 handoff 链 → 纯 V 链 + broadcast apply | 中带 3–8% | probe PASS（0 ulp，安全形已锁定） | **READY_FOR_MAIN_REVIEW** |
| VMX-N2 BATCHED-DENOM-CHAIN | 每行分母链 → 批量 B 元素链 | batched 追加 2–5% | API 存在，需 N1 先落地 | NEEDS_MORE_EVIDENCE |
| VMX-N3 ZERO-PULL-BCAST | Duplicate+pull 构造 → stride-0 块广播 | 行密集追加 1–3% | 未验证，需 feasibility probe | NEEDS_MORE_EVIDENCE |
| VMX-N4 PACKED-RECIPROCAL | 1 元素 V op → 8 元素打包 | 短行 2–5% | count=8 已证；布局前提 open | NEEDS_MORE_EVIDENCE |

**建议优先级：N1（SEQ-FUSE-2）**。理由：机制端到端、probe 已清障、与 V001/V002 同轴但不重复（partials superseded）、测量计划已随 S1–S4 补强。N2–N4 均需 N1 之后单独声明，禁止合并。

---

## 4. 请求

1. Main-2 批准 VMX-N1 = SEQ-FUSE-2 作为 V003 单假设（DIRECT_PARENT=FROZEN_R31B_V011，见 spec 草稿声明）。
2. 同时裁定测量策略：S1 形状加长 + S3 micro-probe 是否纳入 V003 证据要求；长度分层门槛是否提交 Planning 讨论（本路线不自改规范）。
3. N2/N3/N4 保持 backlog；其中 N3 的 feasibility probe 是否排期待定。

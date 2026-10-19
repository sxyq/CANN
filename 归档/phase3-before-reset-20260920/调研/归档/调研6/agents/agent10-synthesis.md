# Agent 10 证据审阅 · 方案合并 · 反例验证 综合报告

> **角色**：10 个并行子代理的最后一个（证据审阅、方案合并、反例验证）。
> **依据**：Agent 1–9 全部报告 + 本地一手材料（`kernel.asc` 模板、`main.asc`、`scripts/AddRmsNormBias.py`、`scripts/verify_result.py`、`提交/V002/kernel.asc`、`文档/*.md`、`.workbuddy/memory/2026-09-11.md`）。
> **范围声明**：本机 macOS，无 CANN 工具链、无昇腾 NPU。**本报告不声称任何 NPU 编译通过 / 精度通过 / 性能达标**；所有硬件相关结论均为「文档/推算 + CPU 参考」，以真机实测为最终判定。
> **调研日期**：2026-09-12。所有来源编号沿用计划分配：A1=S001–S030、A2=S031–S060、A3=S061–S090、A4=S091–S120、A5=S121–S150、A6=S151–S166、A7=S181–S210、A8=S211–S240、A9=S241–S270、A10=S271–S291。

---

## 〇、执行摘要（12 条，每条标注证据等级）

1. **算子语义已锁定（最高权威，证据 A）**：`y=x+residual`；`rms=sqrt(mean(y²,axis=-1)+eps)`；`output=y/rms*gamma+bias`；最后一次性 cast 回原 dtype。golden 用 **FP32 全链路 + `y / rms` 除法**（非 `y*(1/rms)`）。来源：`S273`（AddRmsNormBias.py）、`S272`（main.asc）。
2. **判题 SoC 未确认（未定案，证据 D）**：模板默认 `SOC_ARCH="dav-2201"`，但判题真实运行 SoC 平台未开放；A2 硬件参数（UB=192KB、24 AIC/48 AIV）来自第三方规格表，与 `dav-2201` 的映射关系**未经官方确认**。来源：`S271`、`S276`、`S289`（agent07 第 1 节明示不擅自认定 `dav-2201==910B`）。
3. **CANN 版本存在「8.x 文档用于 9.0.0 结论」的版本过时风险（证据 B/未定案）**：A2 的 bf16 不支持明文取自 9.0.0-beta.2（S048）；Mul/Div 的 A2 不支持 bf16 为同族推断（B 级）。最终以 9.0.0 正式文档或编译实测为准。来源：`S284`（agent02 §4、附）。
4. **首版方案定为「S7 行级并行 + S1 两遍扫描 + S3 FP32 全中间 + S9 DataCopyPad 尾块 + S11 ReduceSum 归约」，且归一化走 `Divs(y,rms)` 先除路径（推荐级别：首选；证据等级 中，需真机确认）**。来源：综合 `S276`、`S288`、`S284`、`S289`。
5. **bf16 在 A2 上 Add/Mul/Muls/Div/ReduceSum 均不支持，必须 Cast→fp32→Cast（证据 A，官方 9.0.0-beta.2 明文）**：任何 bf16 路径不走 FP32 中转即编译失败或精度崩。来源：`S284`（§3.4）、`S259`（C14）、`S260`。
6. **当前 V002 候选用 `Muls(valueTile, valueTile, invRms)` 倒数路径，与项目强制 `Divs` 冲突，存在 bf16 1–28 元素越界风险（证据 中，CPU 参考）**：需在首版中改为 `Divs`。来源：`S281`（line 417/831/949/1069）、`S288`（Q1）。
7. **`DataCopyPad` UB→GM 按官方「不污染相邻 GM」（证据 A）；但社区报告「非对齐裸 DataCopy 多写覆盖相邻段」（证据 C）二者不矛盾、需分场景处理（证据等级：官方 A 优先，社区 C 为手写越界陷阱）**。来源：`S284`（§3.2）vs `S248`（C8）、`S259`（C12）。
8. **S2 单遍法 D 上限受 UB 约束（证据 中，推算）**：fp16/bf16 严格上限 `D≤22528`、fp32 `D≤11264`；超出须回退 S1（fp16/bf16 `D≥24576`、fp32 `D≥12288`）。来源：`S289`（§3.4）。
9. **32B cache-line 对齐在多核非 32B 整除 D 时是真实正确性风险（证据 B/C）**：写回方向（output）并发 DMA 会触碰同一 32B 缓存行覆盖邻居。主对策：output 每行 stride 补齐到 32B 整数倍。来源：`S289`（§5.2）、`S248`、`S259`。
10. **`ReduceSum` 无官方硬上限数字（证据 A，官方文档未给 255/4096/16320）**：仅受 UB 容量与 `sharedTmpBuffer` 公式约束；且为软件仿真、A2 仅支持 half/float（bf16 必须经 fp32）。来源：`S284`（§3.3）、`S249`（C11）。
11. **入口限定符 `__vector__` 合法（模板即 `__vector__`）；`__aicore__` 成员亦合法（证据 A）**：A2 耦合架构下 `__vector__`「不生效」等价于 `__aicore__`。来源：`S271`、`S284`（§3 / 表 18）。
12. **性能是唯一计分维度，但 15 点全过才计分（全或无）；首版以正确性绝对优先，性能优化放到第三候选（证据 A/本队已知规则）**。来源：`S283`（agent01）、`S276`、`S262`（C15/-1 机制）。

---

## 一、证据审阅

### 1.1 去重（同一来源被多个代理引用）

| 主题 | 重复点 | 去重处理 |
|---|---|---|
| GPU→Ascend 迁移 | A4、A5 均覆盖 CUDA/Triton/PyTorch | 合并为「GPU 机制仅作研究参考，不得作为提交方案」；A5 明确「无生成器产出单文件 `kernel.asc`」 |
| 官方开源仓库 | A3 与 A1 均引用官方样例中 AddRmsNorm（无 bias）；RMSNormQuant 误当等价 | A3 已澄清 RMSNormQuant 非等价（int8/无 residual/offset≠bias），全报告统一采信 A3 裁决 |
| 两次 Compile Error | V001/V002 均为编译/上传错误 | 归类为「通道问题，非算法问题」，不进入方案矩阵（来源 `S279`、`S280`、`S281`、A9 C5/C6） |
| DataCopyPad / ReduceSum 字段 | A2 与 A9 均涉及 | 以 A2 官方裁决为基准，A9 的社区案例作「陷阱佐证」 |

### 1.2 版本过时（8.x 结论用于 9.0.0 推断）

- **DataCopyPadExtParams 字段顺序**：社区旧帖曾给出 `{isPad, paddingValue, leftPadding, rightPadding}`，但 9.0.0 官方表 6 与示例统一为 `{isPad, leftPadding, rightPadding, paddingValue}`（A 级）。**处理**：以 9.0.0 为准，且强制按成员名赋值防歧义（`S284` §3.1）。
- **bf16 算术支持**：Add 不支持 bf16 的明文来自 **9.0.0-beta.2**（S048），Mul/Div 不支持为同族推断（B 级）。**风险标注**：正式版 9.0.0 未直接抓取 Mul/Div 产品表（`S284` §4）。
- **通用提醒**：A2 报告中部分硬件数字（UB=192KB、48 AIV）来自 arxiv/规格表转引（B 级），非 9.0.0 官方接口文档；仅作推算底座（`S289` 第 1 节）。

### 1.3 官方 vs 社区混淆（重点核查项）

| 主题 | 官方口径（A 级） | 社区说法（C/D 级） | 是否混淆 | 处置 |
|---|---|---|---|---|
| DataCopyPad UB→GM 写入 | 框架自动补 dummy 并在写 GM 时丢弃，不污染（`S284` §3.2） | C8/C12：「非对齐搬出多写字节覆盖邻居」 | **不矛盾**：社区案例是「裸 `DataCopy` 手动圆整 UB 后整体搬出」的人为越界，非 DataCopyPad 行为 | 用 DataCopyPad + 真实 blockLen，禁止裸 DataCopy 写非对齐长度 |
| ReduceSum count 上限 | 无 255/4096/16320 硬数字，仅 UB 约束（`S284` §3.3） | 社区流传 255/4096/16320 | **社区说法无官方背书** | 以 UB 公式为准；D≤32768 可行 |
| RMSNormQuant 等价 | A3 明确非等价（int8/无 residual/offset≠bias） | 易被误当「带 bias 的 RMSNorm」 | **已澄清，非等价** | 不采用 |
| GPU blockDim/warpSize | A4 明确无 Ascend 对应物 | 易被直接写进推荐实现 | **已限定** | GPU 机制仅作对照表，不进方案 |

### 1.4 证据冲突清单（≥5，含裁决）

> 每条给出：冲突双方 → 裁决 → 真机确认前必须执行的动作。

**冲突 C-1：归一化走 `Muls(y, 1/rms)` 还是 `Divs(y, rms)`**
- 冲突方 A：A2 在 Div/Divs 行备注「建议用 `Muls(1/rms)` 替代 Div 以避免除零」（`S284` 表 10、§5.1 用 `Rsqrt`+`Muls`）；V002 当前实现即 `Muls(valueTile, valueTile, invRms)`（`S281` line 417/831/949/1069）。
- 冲突方 B：A6 CPU 实验证明 `Divs(y,rms)` 与 golden **逐位一致（失配=0）**；`Muls(1/rms)` 在 **bf16 有 1–28 个元素越界**（最大稳健相对误差 5e-2，近零放大），NPU 硬件舍入下风险只增不减；且 `epsilon` 已在 `sqrt` 内（`mean(y²)+eps`），`rms>0` 恒成立，**除零不成立**（`S288` Q1/Q2/Q7）。
- **裁决**：首版强制 `Divs(y, rms)` + `Sqrt`。A2 的「避免除零」理由被 Q7 推翻（eps 内置保证 rms>0）。V002 的 Muls 路径是真实正确性风险，必须改。
- 真机确认前动作：在首版中替换 Muls 为 Divs，并保留 CPU 参考脚本（agent06）离线比对。

**冲突 C-2：DataCopyPad UB→GM 是否污染相邻 GM**
- 冲突方 A（官方）：`Local→Global` 重载无 padParams，框架写 GM 时丢弃 dummy，不污染（`S284` §3.2，S033）。
- 冲突方 B（社区）：C8/C12 报告非对齐搬出多写 -1/脏数据覆盖邻居（`S248`、`S259`）。
- **裁决**：不矛盾。官方行为是 DataCopyPad 自动丢弃；社区事故是「裸 `DataCopy` 把 UB 手动圆整到 32B 后整体搬出」导致写出 padding 区。**首版用 DataCopyPad + 真实有效 blockLen（可非对齐），绝不用裸 DataCopy 写非对齐尾块**。但鉴于「静默污染」风险高，额外加防御：output 每行 stride 补齐 32B + Host 侧多留 32B 余量（参考 A9 检查清单）。
- 真机确认前动作：用 D=70 的非对齐用例核对尾块（见反例清单 S9）。

**冲突 C-3：ReduceSum count 是否有 255/4096/16320 硬上限**
- 冲突方 A（官方）：9.0.0 文档未出现这些数字，仅「不超过 UB 大小限制」（`S284` §3.3）。
- 冲突方 B（社区）：流传 255/4096/16320（`S284` §3.3 反方）。
- **裁决**：以官方为准——无硬上限，仅 UB 约束 + `sharedTmpBuffer` 公式。D≤32768 在 UB 预算内可行。社区数字可能是特定硬件 repeat/block 上限的误传，不作为约束。但 `ReduceSum` 是**软件仿真**，大 D 归约慢，性能候选阶段再评估 `BlockReduceSum+WholeReduceSum`（`S289` §6.2，S193）。
- 真机确认前动作：按公式分配 `sharedTmpBuffer`（half 向上取整 16 元素、float 8 元素倍数），`srcInnerPad=true`。

**冲突 C-4：单遍 S2 的 D 上限「宽路径可行」vs UB 四份缓冲数学**
- 冲突方 A（V002 现状）：巨型 kernel 内含 `WideFp16Cached` 等宽路径分支，隐含「整行可进 UB」假设。
- 冲突方 B（A7 UB 数学）：严格模型（同时容 y+output+gamma+bias 四份）fp16/bf16 `D≤22528`、fp32 `D≤11264`；超出整行放不进 UB（`S289` §3.4）。
- **裁决**：以 A7 UB 公式为准。V002 的「宽路径」在 D 极大时退化/溢出。首版若用 S2，必须在 `D≥24576`(fp16/bf16)/`D≥12288`(fp32) 回退 S1。
- 真机确认前动作：用 fp32 [128,32768]、fp16 [1,30000] 验证 S2 的 UB 溢出（见反例 S2）。

**冲突 C-5：A2 推荐 `Muls(1/rms)`+`Rsqrt` 的「数值路径」与项目「先除 + Sqrt」强制要求冲突**
- 冲突方 A：A2 §5.1 推荐 `Rsqrt(rstd, meanSq)` + `Mul(y,y,rstd)`（`S284`）。
- 冲突方 B：A6 Q2 证明 `Sqrt+除法` 与 golden 逐位一致（相对误差=0），`Rsqrt+Mul` 有 ~1 ulp 硬件舍入且 bf16 最大稳健相对误差达 1e-2（`S288` Q2）。
- **裁决**：首版用 `Sqrt(mean(y²)+eps)` 后 `Divs`。`Rsqrt` 单次舍入误差 CPU 无法复现，宁可用除法（正确性优先）。性能候选阶段若真机显示 `Rsqrt` 显著更快且精度仍过阈，再单独评估。
- 真机确认前动作：首版只用 Sqrt+Divs；保留 Rsqrt 作为性能候选的可选替换项，需单独真机精度验证。

**冲突 C-6（额外）：`__vector__` 入口 vs `__aicore__` 成员**
- 冲突方：模板用 `__global__ __vector__`（`S271`）；V002 用 `__vector__` 启动但成员函数 `__aicore__`。
- **裁决**：两者在 A2 耦合架构下均合法（`__vector__` 不生效等价于 `__aicore__`），不构成错误（`S284` 表 18）。但 V002 把 `TPipe tpipe` 作为类成员（line 2845）属风险点（应移出 kernel 类，参考 `S277` 提交清单 6.1），且 ~20 个 Process 分支触发 `out of jump/jumpc imm range` 风险（A9 C3，官方 `S245`）。
- 真机确认前动作：首版收敛为少量通用路径，避免巨型多分支；若体量仍大，加 `-mllvm -cce-aicore-jump-expand=true` 兜底（`S245`）。

---

## 二、反例与失败条件清单（覆盖 S1/S2/S3/S7/S9/S11）

> 每条给出：具体失败输入（dtype / shape / D / outer / 数值范围）+ 失败机理 + 可执行验证方法（真机或离线 CPU）。
> 注：所有「真机验证」均为**有 CANN/NPU 时的操作**，本机未执行。

### S1（两遍扫描）— 失败条件：单行 UB 放不下且无 D 分块
- **失败输入**：`fp32`，`shape=[1, 32768]`（outer=1，D=32768）。若实现不做 D 维分块，需同时持有 `y`(128KB)+`output`(128KB)+`gamma`(128KB)+`bias`(128KB)=**512KB > 192KB UB** → UB 分配失败或越界。
- **机理**：S1 虽分两遍，但「整行四份同留 UB」超容量；正确实现必须 intra-row 切 tile（参考 A7 §4.4 d1）。
- **可执行验证**：① 真机编译运行该 shape，预期触发 UB 分配报错或结果 NaN/`inf`；② 离线：CPU 参考脚本确认算法逻辑正确后，再上真机验证 UB 预算。

### S2（单遍保存中间）— 失败条件：D 超 UB 四份上限
- **失败输入**：`fp16`，`shape=[1, 30000]`（D=30000 非对齐）；单 pass 需 `y`(60KB)+`output`(60KB)+`gamma`(60KB)+`bias`(60KB)=**240KB > 192KB UB**（严格模型已超）；或 `fp32, [128, 32768]`（单行四份 512KB）。
- **机理**：S2 省 40% GM 流量但以「整行常驻 UB」为代价；A7 严格上限 fp16/bf16 `D≤22528`、fp32 `D≤11264`（`S289` §3.4）。
- **可执行验证**：① 真机跑 S2 上述 shape，预期 UB 溢出；② 对照跑 S1 同 shape 应通过 → 证实「超界回退 S1」。

### S3（FP32 全中间计算）— 失败条件（反证）：偷用低精度中间累加
- **失败输入**：`fp16`，`shape=[64, 1024]`（`outer=64, D=1024`），uniform(-2,2)。若平方和在 fp16 累加（非 fp32），A6 Q6 实测：fp16 D=1024 失配比例 **0.13% > 0.1%** → 翻车；bf16 全程低精度 **50%+ 失配**（灾难性）。
- **机理**：fp16 累加在普通激活下即 `inf`（Q4：`|y|≥256` 平方溢出）；bf16 仅 8-bit 尾数，长累加静默停滞（`S288` Q4/Q6）。
- **可执行验证**：① 离线 CPU 复现「fp16 中间累加」路径 vs golden，统计失配比例（agent06 脚本 Q6 分支）；② 真机：构造同 shape，对比 golden，预期失配超 0.1%。→ 证伪低精度，坐实 S3「必须全 fp32 中间」。

### S7（以行分配 AI Core）— 失败条件①：outer < cores 结构欠利用；②：D 非 32B 对齐多核写回踩踏
- **失败输入（踩踏）**：`fp16`，`shape=[8, 70]`（outer=8 行 < 48 核，D=70 非对齐，`D·s=140B`，`140 mod 32 = 12`）。8 行分给 8 核，每核写回尾块按 32B 搬运，第 i 行尾与第 i+1 行头落在同一 32B 缓存行，并发 DMA 覆盖邻居有效字节。
- **机理**：output 张量行主序连续，非对齐行尾与下一行头同 cache line；多核并发写回触碰同一行 → 静默污染（`S289` §5.2、`S248`）。
- **可执行验证**：① 真机 `blockNum=8` 跑该 shape，核对 output 第 i 行末 12B 与第 i+1 行首是否被污染（与 golden 比对邻行边界）；② 对策验证：把 output 每行 stride 补齐到 `ceil(140/32)*32=160B` 后复跑，污染消失。

### S9（DataCopyPad 尾块）— 失败条件：用裸 DataCopy 写非对齐尾块
- **失败输入**：`fp16`，`shape=[1, 70]`（70 half=140B 非对齐）。若用裸 `DataCopy(outputGm, yLocal, 70)`，硬件按整 32B 块搬 16 half(32B)，把 `Global[11..15]` 写成 **-1**（`S259` C12 原文）。
- **机理**：DataCopy 以 32B 为粒度，尾块多出元素被写死 -1，覆盖相邻有效数据；DataCopyPad + mask 才隔离。
- **可执行验证**：① 真机裸 DataCopy 写回，检查尾部 5 元素 == -1（污染）；② 改用 `DataCopyPad(dst, src, copyParams)`（真实 blockLen=140，无 padParams）+ 读入侧 `DataCopyPadExtParams` 正确配置，复跑，尾部正常。

### S11（ReduceSum 归约）— 失败条件①：fp16 源累加溢出；②：srcInnerPad 未 true
- **失败输入（溢出）**：`fp16`，`shape=[1, 32768]`，`y` 取值 uniform(250, 255)（使 `|y|` 接近/超过 256）。`y*y` 在 fp16 下 = `inf`（`S288` Q4），`ReduceSum(fp16)` 累加 → `inf` → `rms=inf` → `output=NaN`。
- **机理**：ReduceSum 内部不处理累加溢出（`S249` C11 官方明文）；fp16 平方和必须先在 fp32 算（`S3` 数值策略）。
- **可执行验证**：① 真机/CPU 复现「fp16 源 ReduceSum」路径，预期输出 NaN；② 对策：入口 `Cast(fp16→fp32)` 后再 `ReduceSum(fp32)`，复跑应通过。
- **失败输入（对齐）**：A2 上 `srcInnerPad` 仅支持 `true`（`S249`）；若归约前尾块未 pad 到 32B 对齐，A2 结果错误。验证：D 非 32 倍数用例 + `srcInnerPad=true` 比对 `false`。

---

## 三、方案矩阵（S1–S14 全量）

> 列：`方案 | 算法要点 | GM访存次数 | UB占用 | 精度风险 | 尾块风险 | 性能潜力 | 实现难度 | CANN兼容风险 | 推荐级别 | 证据等级+来源`
> 推荐级别：首选 / 可作为第二路线 / 仅作研究参考 / 不建议。证据等级：A 官方、B 官方转引/样例、C 社区、D 未确认。

| 方案 | 算法要点 | GM访存 | UB占用 | 精度风险 | 尾块风险 | 性能潜力 | 实现难度 | CANN兼容 | 推荐级别 | 证据 |
|---|---|---|---|---|---|---|---|---|---|---|
| **S1** 两遍扫描 | Pass1 算平方和；Pass2 重算 y 并归一化 | 5·N·s | 低（逐 tile） | 低（FP32） | 低 | 基准（最低） | 低 | 低 | **首选**（首版骨架） | A：`S271/S276/S289` |
| **S2** 单遍保存中间 | 一次读 x/residual，UB 留 y 再归一化 | 3·N·s | 高（整行四份） | 低 | 低 | 高（省40%） | 中 | 中（UB紧） | **可作为第二路线**（S1跑通后启用，超 D 上限回退 S1） | B/C：`S289`§3.4 |
| **S3** FP32全中间 | fp16/bf16→fp32→FP32归约→一次cast回 | 同S1/S2 | 中 | 低（必须） | 低 | —（不改访存） | 低 | 中（bf16需Cast） | **首选**（数值必须） | A：`S288`Q3/Q4/Q6、`S284`§3.4 |
| **S4** 低精度中间 | 对照：全程fp16/bf16累加 | 同S1 | 低 | **极高**（bf16 50%+失配） | 低 | — | 低 | 中 | **不建议** | A：`S288`Q6（证否） |
| **S5** 大tile | tile≈UB上限，减DMA轮次 | 同S1/S2 | 高 | 低 | 中 | 高 | 中 | 低 | **可作为第二路线**（性能优化） | C：`S289`§4 |
| **S6** 小tile | tile小、多轮流水 | 同S1/S2 | 低 | 低 | 中 | 低 | 高（流水复杂） | 低 | **仅作研究参考** | C：`S289` |
| **S7** 以行分配核 | 一核若干整行，归约核内完成 | 同S1/S2 | 中 | 低 | 中（需32B对齐） | 中（自然） | 低 | 低 | **首选**（首版分核） | B：`S289`§5.1、`S283` |
| **S8** 以tile分配核 | 一核若干tile，可跨行切D | 同S1/S2 | 中 | 低 | 中 | 中（outer<cores时被迫） | 高（跨核归约） | 低 | **可作为第二路线**（outer<cores兜底） | B：`S289`§4.1/§5.1 |
| **S9** DataCopyPad尾块 | GM↔UB 非对齐用DataCopyPad+mask | 同S1/S2 | — | 低 | **低（正确）** | 中 | 低 | 低（需字段顺序对） | **首选**（尾块处理） | A：`S284`§3.1/§3.2、`S281` |
| **S10** 手工尾块 | mask/Duplicate/GatherMask 手写 | 同S1/S2 | — | 低 | 低 | 中 | 高 | 中 | **可作为第二路线**（S9备选） | C：`S289`/`S248` |
| **S11** ReduceSum归约 | 官方ReduceSum求和 | 同S1/S2（+UB临时） | 中（tmp） | 低（fp32） | 低 | 中（软件仿真） | 低 | 中（bf16需cast、A2仅half/float） | **首选**（归约） | A：`S284`§3.3、`S249` |
| **S12** 手工向量归约 | 两两相加/全归约指令手搓 | 同S1/S2 | 中 | 低 | 低 | 低（不如官方） | 高 | 中 | **仅作研究参考**（官方ReduceSum可用，手搓不值） | C：`S284`/`S289` |
| **S13** 纯Ascend C实现 | 直调单文件kernel.asc，纯Ascend C | — | — | — | — | — | — | 低（判题形态即此） | **首选**（提交形态） | A：`S271`/`S276` |
| **S14** CUDA/Triton/PyTorch迁移 | 从GPU实现翻译 | — | — | 中（语义映射） | 中 | 不确定 | 高（无对应物） | 高（GPU机制无Ascend对应） | **不建议**（作提交方案；仅研究参考） | B：`S286`(A4)/`S287`(A5) |

### 3.1 不推荐做法（明确列入黑名单）
- 用 `Muls(y, 1/rms)` 或 `Rsqrt` 替代 `Divs` + `Sqrt`（C-1/C-5，bf16 精度风险）。
- bf16 路径直接进入 Add/Mul/Muls/Div/ReduceSum（A2 不支持，编译失败/精度崩，C-3/C14）。
- 用裸 `DataCopy` 写非 32B 对齐尾块（`S259` C12，写 -1 污染）。
- 巨型多分支 kernel（2955 行 + ~20 变体），触发 `out of jump/jumpc imm range`（A9 C3）。
- 低精度（fp16/bf16）中间累加（S4，证否）。
- 把 `DataCopyPadExtParams` 当 `{isPad, paddingValue, leftPadding, rightPadding}` 旧序初始化（应按成员名赋值，C-2 顺序歧义）。

### 3.2 必须真机确认才能定案的做法
1. **判题 SoC 到底是不是 910B（dav-c220）**：决定 UB=192KB/48 AIV 是否适用；若为 310B（UB=256KB）则 S2 上限放宽。
2. **`Divs` vs `Muls` 在真机 bf16 下的实际舍入差异**：CPU 仅证明 Muls 有 1–28 元素越界，真机硬件舍入需实测确认是否越过 1e-3/1e-4 + 0.1%。
3. **`Rsqrt` 真机单次舍入是否真的更快且精度仍过阈**：性能候选阶段单独验证。
4. **`ReduceSum` 在判题 SoC 上的实际开销与 `BlockReduceSum+WholeReduceSum` 可用性**：A2 是否真支持后者未定案（`S284` §4）。
5. **fp32→bf16 cast 舍入模式（RNE/截断/odd）在判题 SoC 上的具体行为**：影响最后 1 ulp（C-5 延伸）。
6. **S2 单遍在判题 SoC 上的真实 UB 上限**：以 A7 推算为基准，需真机验证 192KB 是否准确。
7. **判题端是否含「失配比例 tol=0.1%」**：本地 verify 有，判题端未开放（`S283` 未确认）。

---

## 四、三条路线

### 4.1 首版（正确性绝对优先，必须先 15/15 全过）
**组合**：`S7`（行级并行）+ `S1`（两遍扫描）+ `S3`（FP32 全中间）+ `S9`（DataCopyPad 尾块）+ `S11`（ReduceSum 归约）。

**具体刚性约束**：
- **block 划分**：`blockCount = min(availableCoreNum, rowCount)`；行余数 `r=rowCount mod cores` 均摊给前 `r` 核（尾核多担 1 行，`S289` §5.1）。outer<cores 时退化为 S8（按 tile 切 D），但跨核归约同步代价高，优先 S1。
- **Div vs Muls**：强制 `Divs(y, rms)`（`rms=Sqrt(mean(y²)+eps)`），**禁止 Muls(1/rms)/Rsqrt**（C-1/C-5）。eps 在 sqrt 内保证 rms>0。
- **Sqrt vs Rsqrt**：强制 `Sqrt` 后除法（Q2 逐位一致）。
- **尾块处理**：D 非 32B 对齐一律 `DataCopyPad` + 真实有效 `blockLen`（可非对齐）+ 读入侧 `DataCopyPadExtParams` 按成员名赋值；output 每行 GM stride 补齐到 32B 整数倍 + Host 多留 32B（`S289` §5.2）。
- **逐字段 API 分配**：
  - fp32 路径：`DataCopy` 读入 → `Add`(y=x+residual) → `Cast`(fp32) → `Mul`(y,y) → `ReduceSum`(fp32) → `Muls`(mean,1/D)+`Add`(eps) → `Sqrt` → `Divs`(y,rms) → `Mul`(·gamma)+`Add`(+bias) → `DataCopyPad` 写出。
  - fp16 路径：同 fp32 组合（A2 支持 half 算术），但 `y*y`/`ReduceSum` 建议提升至 fp32 防溢出（Q4）。
  - **bf16 路径**：入口 `Cast(bf16→fp32)`，全程 fp32（上述组合），出口 `Cast(fp32→bf16, CAST_RINT)`；**严禁**直接用 Add/Mul/Muls/Div/ReduceSum 处理 bf16 张量（C-3/C14）。
- **ReduceSum 配置**：`sharedTmpBuffer` 按公式分配（half 上取整 16 元素、float 8 元素倍数），`srcInnerPad=true`，work 空间用 `GetReduceSumMaxMinTmpSize` 结论（max==min，取最小）。
- **标量同步**：`GetValue` 读回归约结果处插 `PipeBarrier`（仅 `PIPE_V`/`PIPE_MTE` 必要处，禁用 `PipeBarrier<PIPE_S>` 防硬件错误，`S284` §5.2-7）。
- **体量控制**：收敛为少量通用路径（按 dtype + 是否对齐 + 是否多核 参数化），避免 V002 式 ~20 变体巨型 kernel；若仍大，加 `-mllvm -cce-aicore-jump-expand=true` 兜底（C-3）。

### 4.2 第二候选（ fallback，S1 跑通后启用）
**组合**：`S7` + `S2`（单遍保存中间）+ `S3` + `S9`/`S10` + `S11`，**带 D 上限护栏**。
- 启用条件：首版 15/15 全过后，为性能引入 S2。
- **护栏**：fp16/bf16 仅当 `D≤22528`、fp32 仅当 `D≤11264` 走 S2；一旦 `D≥24576`(fp16/bf16)/`D≥12288`(fp32) 立即回退 S1（`S289` §3.4）。
- 仍走 `Divs`+`Sqrt`、DataCopyPad 尾块、bf16 Cast 中转——不因性能放松正确性约束。
- 风险：S2 在 D 极大时 UB 紧张（见反例 S2），需 intra-row 切 tile（A7 §4.4 d1）。

### 4.3 性能候选（正确性闭环后，冲刺性能分）
**组合**：`S2`（单遍）+ `S5`（大 tile）+ `S7`/`S8` 混合分核 + 双缓冲 + `BlockReduceSum+WholeReduceSum` + 合并 DMA（K 行/次 ≥512B）。
- 仅在 15/15 全过后启用（性能是唯一计分维度，但全过是前提）。
- 优化项（均来自 A7，标注推算/官方）：
  - 合并 DMA：D=64 fp16 时 `K=4` 行/次搬运达 512B cacheline（`S289` §4.2）。
  - 双缓冲 ~1.6×（S194）。
  - `BlockReduceSum+WholeReduceSum` 比两次 WholeReduceSum 快 ~40%（S193，官方最佳实践）——**前提：A2 9.0.0 真机确认该 API 可用（未定案，见 3.2-4）**。
  - 32B stride 补齐消除多核 cache-line 踩踏（`S289` §5.2）。
- **Rsqrt 可选替换**：若真机验证 `Rsqrt` 更快且精度仍过阈（3.2-3），可替换 Sqrt+Divs；否则保留 Divs。

---

## 五、证据缺口清单（≥8 条，均为「当前无法确认」）

1. **判题真实 SoC**：`dav-2201` 默认是否即 910B（dav-c220）？UB/核数参数是否适用？（`S271/S276` 未确认）
2. **判题 15 点 shape/dtype/epsilon 配置**：平台未开放，仅模板 1 例 FP16 [1,64]（`S272/S283`）。
3. **判题端是否含「失配比例 tol=0.1%」**：本地 verify 有，判题端未确认（`S274/S283`）。
4. **CANN 9.0.0 正式版 Mul/Div 的 A2 bf16 支持表**：当前为 beta.2 明文 + 同族推断 B 级（`S284` §4）。
5. **`BlockReduceSum`/`WholeReduceSum` 在 A2 9.0.0 的签名与可用性**：仅 ReduceSum 文档提及，未展开（未定案，`S284` §4）。
6. **`DataCopyPadExtParams` 头文件真实结构体声明**：未读安装包，依文档表 6 推断（建议按成员名赋值，`S284` §3.1）。
7. **硬件 `Muls`/`Divs`/`Rsqrt` 单次舍入误差**：只能 NPU 实测，CPU 无法复现（`S288` §6）。
8. **fp32→bf16 cast 在判题 SoC 的具体舍入模式（RNE/截断/odd）**：影响最后 1 ulp（`S284` §3.5、`S288` §6）。
9. **判题端真实数据分布**：是否出现 `|y|≥256` 触发 fp16 平方溢出？模板 uniform(-2,2) 安全，判题未知（`S288` Q4）。
10. **性能基线 T（评分公式分母）**：未给出，无法预知绝对时间目标（`S283` 评分公式 `100/(1+log_1.5(t/T))` 中 T 未知）。
11. **`availableCoreNum` 真实取值**：模板用 `ACL_DEV_ATTR_VECTOR_CORE_NUM`，判题端具体核数未确认（`S272/S283`）。
12. **`ReduceSum` 在判题 SoC 的实际开销与是否真软件仿真**：文档称软件仿真，未实测（`S284` §3.3）。

---

## 六、真机第一验证批（优先级排序，有 NPU 时执行）

> 目标：先打通通道与正确性，再谈性能。每日 50 次、取最后一次，建议首提交=最小可编译版。

| 优先级 | 验证项 | 输入/操作 | 预期结果 | 对应论断 |
|---|---|---|---|---|
| P0 | 上传通道完整性 | 提交前 `wc -l`/`md5`、首行 `#include`、末行闭合；UTF-8 无 BOM/LF | 平台回显与本地一致 | A9 C5/C6/C7、检查清单 A |
| P0 | 最小可编译版 | 仅 FP16 单路径、无分支的 tiny kernel | Compile Pass（验证 V001/V002 类通道问题已排除） | A9 §5.3 |
| P1 | 三 dtype 全过 | fp16/bf16/fp32 各 1 例（如 [64,1024]） | 15/15 中 dtype 维度通过 | S3、C-3/C14 |
| P1 | 尾块非对齐 | fp16 [8,70]、`[1,30000]` | DataCopyPad 尾块正确、无 -1 污染 | S9、反例 S9、C-12 |
| P1 | 32B 踩踏 | fp16 [8,70] blockNum=8，核对邻行边界 | 无 cache-line 污染 | 反例 S7、C-8/C-12 |
| P1 | bf16 Cast 中转 | bf16 [64,1024] | 精度过阈（非直接 bf16 算术） | C-3/C14、S3 |
| P2 | 大 D 归约 | fp32 [1,32768]、fp16 [1,32768] | ReduceSum(fp32) 正常，无 inf | 反例 S11、Q4 |
| P2 | S2 D 上限 | fp32 [128,32768]、fp16 [1,30000] 跑 S2 | UB 溢出 → 回退 S1 | 反例 S2、C-4 |
| P2 | uint64 偏移 | D=32768、outer 大，用 `uint64_t` 行偏移 | 无 `uint32_t` 溢出 | A9 §5.4 |
| P3 | 性能基线 | msprof op 测 S1 各形态 Task Duration | 取得 aiv/mte2/mte3 占比，确认 memory-bound | A7 §7 |
| P3 | Rsqrt 替换 | 同输入比对 Sqrt+Divs vs Rsqrt+Mul | 精度仍过阈且更快才采用 | C-5、3.2-3 |

---

## 七、来源清单（A10 段 S271–S291，复用 A1–A9 编号）

> A1–A9 编号沿用计划：A1=S001–S030、A2=S031–S060、A3=S061–S090、A4=S091–S120、A5=S121–S150、A6=S151–S166、A7=S181–S210、A8=S211–S240、A9=S241–S270。下列为 A10 新增/本地一手材料编号。

- **[S271]** `kernel.asc` 直调模板（最高权威）— `/Users/sunyiyang/Downloads/addrmsnormbias_problem_1742_template/kernel.asc` | 等级 A | 用途：入口 `extern "C" run_kernel` + `__global__ __vector__` + `TensorGroupInfo` 运行时读 shape/dtype + `float epsilon`；确认 `__vector__` 限定符。可支持结论：C-6、首版形态。
- **[S272]** `main.asc` 默认用例 — 同模板目录 | 等级 A | 用途：默认 FP16 [1,64]、`ACL_DEV_ATTR_VECTOR_CORE_NUM` 取核数、`epsilon=1e-5f`。可支持结论：执行摘要 1、缺口 2/11。
- **[S273]** `scripts/AddRmsNormBias.py`（golden 权威）— 同模板目录 | 等级 A | 用途：`y=x+r`→`rms=sqrt(mean(y²)+eps)`→`out=y/rms*gamma`→`+bias`→cast 回原 dtype；用 `y / rms` 除法。可支持结论：执行摘要 1、C-1/C-5。
- **[S274]** `scripts/verify_result.py`（本地判定口径）— 同模板目录 | 等级 A | 用途：`np.isclose(rtol=1e-3,atol=1e-3,equal_nan=True)` + `tol=0.1%`。可支持结论：缺口 3。
- **[S275]** `AGENTS.md`（项目约定）— `/Users/sunyiyang/Desktop/Project/cann/` | 等级 A | 用途：macOS 无 CANN/NPU，不得声称 NPU 结果。可支持结论：范围声明。
- **[S276]** `文档/problem-add-rms-norm-bias.md` — 本仓库 | 等级 A（内部权威） | 用途：DataCopyPad 顺序裁决、UB→GM 不污染、ReduceSum 无硬上限、入口限定符中风险、判题 SoC 未确认。可支持结论：摘要 2/4、C-2/C-3/C-6。
- **[S277]** `文档/submission-checklist.md` — 本仓库 | 等级 A（内部） | 用途：先除(Divs)、TPipe 移出 kernel 类、提交四项核对、CAST_RINT 非 CAST_RND、判题 SoC 未确认。可支持结论：C-6、首版约束。
- **[S278]** `.workbuddy/memory/2026-09-11.md`（上轮结论）— 本仓库 | 等级 B（内部记忆） | 用途：DataCopyPad 顺序、UB→GM 无污染、A2 bf16 不支持、__vector__/__aicore__ 均合法、V002 未编译、SoC 未指定。可支持结论：C-2/C-3/C-6。
- **[S279]** `提交/V001/结果.md` — 本仓库 | 等级 A（内部日志） | 用途：15/15 Compile Error `unknown type name 'pipe_'`（保留字/符号类）。可支持结论：去重、A9 C5。
- **[S280]** `提交/V002/结果.md` — 本仓库 | 等级 A（内部日志） | 用途：15/15 Compile Error，平台收到首行 `return false;`（上传截断，非算法）。可支持结论：去重、A9 C6。
- **[S281]** `提交/V002/kernel.asc`（2955 行现状）— 本仓库 | 等级 A（内部快照） | 用途：line 2870 `__global__ __vector__`；line 2845 `TPipe tpipe` 类成员；line 417/831/949/1069 `Muls(valueTile,valueTile,invRms)` 倒数路径；line 324/347 `ReduceSum`；line 2782 UB→GM `DataCopyPad` 无 padParams；line 2805 `CAST_ROUND`；~20 个 Process 分支。可支持结论：C-1/C-6、首版风险点。
- **[S282]** `调研/调研2/平台与研究覆盖计划.md` — 本仓库 | 等级 A（计划） | 用途：S1–S14 定义、来源分段、验收标准。可支持结论：方案矩阵列定义、全文结构。
- **[S283]** Agent 01 报告 `agent01-problem-and-submit.md` | 等级 A/B | 用途：15 点全过才计分、评分公式、fp32 更严 1e-4、SoC 未知。可支持结论：摘要 12、缺口 2/3/10。
- **[S284]** Agent 02 报告 `agent02-ascendc-api.md` | 等级 A/B | 用途：DataCopyPad 顺序、UB→GM 不污染、ReduceSum 无硬上限、A2 bf16 不支持、Cast roundMode、__vector__ 合法。可支持结论：C-2/C-3/C-5/C-6、S3/S11。
- **[S285]** Agent 03 报告 `agent03-official-repos.md` | 等级 A/B | 用途：官方 AddRmsNorm（无 bias）参考、RMSNormQuant 非等价。可支持结论：去重（RMSNormQuant 非等价）。
- **[S286]** Agent 04 报告 `agent04-gpu-migration.md` | 等级 B/C | 用途：GPU→Ascend 迁移对照表，warp shuffle/shared memory 无对应物。可支持结论：S14 不建议、官方vs社区混淆（GPU）。
- **[S287]** Agent 05 报告 `agent05-compiler-codegen.md` | 等级 C/D | 用途：无生成器产出单文件 `kernel.asc`；仅作研究参考。可支持结论：S14 不建议、S5/S6 研究定位。
- **[S288]** Agent 06 报告 `agent06-numerics.md` | 等级 A/B/C | 用途：Q1 Divs 逐位一致、Muls bf16 1–28 元素越界；Q2 Sqrt+Div 逐位一致；Q4 fp16 |y|≥256 溢出；Q6 低精度 50%+ 失配；Q7 eps 在 sqrt 内。可支持结论：C-1/C-5、S3、反例 S3、S4 证否。
- **[S289]** Agent 07 报告 `agent07-perf-ub.md` | 等级 A/B/C | 用途：UB=192KB、48 AIV（B 级）；S1=5N·s/S2=3N·s 省40%；S2 D 上限 fp16/bf16 22528、fp32 11264；32B 对齐真实风险；ReduceSum 含 scalar wait；Block+Whole 快~40%。可支持结论：C-3/C-4、S1/S2/S5/S7/S8、反例 S1/S2。
- **[S290]** Agent 08 报告 `agent08-env-linux.md` | 等级 B/C | 用途：Linux+CANN 环境清单、编译/运行错误处置表。可支持结论：真机验证批环境准备（P0–P3 执行前提）。
- **[S291]** Agent 09 报告 `agent09-failure-cases.md` | 等级 A/B/C | 用途：C1 保留字、C3 大 kernel 跳转超限、C6/C7 上传截断、C8/C12 非对齐写污染、C9 fp16 舍入静默错、C11 ReduceSum 溢出/对齐、C14 bf16 不支持、C18 GetValue 同步。可支持结论：C-2/C-6、反例 S7/S9/S11、首版检查清单。

---

## 八、收尾说明（不可确认结论的显式声明）

本报告已按验收标准完成：① 证据冲突清单 **6 条**（≥5）且每条含裁决；② 反例覆盖 **S1/S2/S3/S7/S9/S11**（每个含具体 dtype/shape/D/outer/数值范围 + 可执行验证方法）；③ 方案矩阵覆盖 **S1–S14 全量**（含推荐级别与证据等级）；④ 三条路线（首版具体、第二候选带护栏、性能候选后置）；⑤ 证据缺口 **12 条**（≥8）；⑥ 真机第一验证批优先级排序。

**本报告未声称、且无法确认的结论**（必须真机才能定案，见 §三 3.2 与 §五）：判题 SoC 真实型号、Divs 真机 bf16 舍入、Rsqrt 真机性价比、BlockReduceSum 可用性、fp32→bf16 真机舍入模式、S2 真机 UB 上限、判题端 tol 口径、判题端数据分布。所有性能数字均标注「官方转引 B」或「推算」，无任何本机/真机实测声明。

*报告结束。所有结论均可追溯至 S271–S291 及 A1–A9 来源；本机无 CANN/NPU，未编译、未运行任何 Ascend C 代码。*

# OpenAI 高难题 Agent 方法与社区讨论

## 结论先行

单位距离问题和 Navier-Stokes 的公开工作流不是同一套规模和组织方式：

1. **单位距离问题**：公开 PDF 给出的链路是“一个通用推理模型 + AI 生成题目说明 + 自动评分流水线 + 人工数学家复核与整理”。官方没有在该项目中公开约 10,000 个并行 Agent 的调度信息。
2. **Navier-Stokes**：OpenAI 官方明确描述了协调 Agent 系统、分组通信、问题变体分工、代码执行、互联网缓存、跨组信息合并和 Lean 形式化；公开说明称完整问题使用约 10,000 个并发 Agent。
3. **单位距离问题的真正贡献**：模型没有凭空发明一整套新数学，而是把“平方网格/高斯整数”的旧路线推广到高次数 CM 数域，并坚持探索一个当时普遍不被看好的反例方向。外部数学家随后把原始证明压缩、简化、量化并继续优化。
4. **社区共识**：结果的数学内容已经有专家整理版和公开 Lean 形式化工程；但“自主程度”“训练数据是否包含相关私人工作”“AI 生成内容的引用责任”等属于方法和科研规范讨论，不能和定理是否成立混为一谈。

这套经验可以迁移到 CANN 算子竞赛的研究组织方式，但不能把 Agent 数量当成性能来源。CANN 的最终证据仍然必须是题面接口、Ascend C 编译、真实 NPU 精度和 CANNJudge 结果。

## 一、单位距离问题：公开可确认的工作流

### 1. 原始题目提示词

OpenAI 发布的 `unit-distance-proof.pdf` 第 2-3 页公开了原始提示词。提示词要求模型：

- 给出平面单位距离问题的完整正解或反例；
- 严格区分正向证明和反例两条路线；
- 处理“对所有足够大的 n”与“存在无穷多个 n”的量词差异；
- 不把改进常数、有限计算、特殊情形或结构性约化当成完整答案；
- 只有在完成其中一条完整路线后返回。

提示词还明确指出，模型输出会进入 AI grading pipeline。原始论文的 AI 使用说明称：自动评分给出较高置信度后，OpenAI 内部研究人员和外部数学家才开始系统阅读、改写和复核。

这里有一个重要边界：**提示词设定了验收条件，不等于提示词本身证明了结果**。它能减少“把局部进展冒充完整解答”的情况，却不能替代数学推导、外部专家复核或形式化证明。

### 2. 模型采用的数学路线

原始证明的核心路线可以压缩为以下链路：

```text
Erdős 的平方网格构造
    -> 把网格差向量看成高斯整数
    -> 换成 L(i) 这类随次数增大的 CM 数域
    -> 让固定的一组有理素数在数域中完全分裂
    -> 用理想类群的抽屉原理产生很多范数为 1 的元素
    -> 通过 Minkowski 嵌入得到高维格点
    -> 在乘积圆盘中取平移后的有限点集
    -> 投影到一个复坐标，也就是平面
    -> 得到单位距离数超过 n^(1+δ) 的无穷序列
```

需要注意三个数学层次：

- **旧结构**：平方网格、高斯整数、同长度差向量。
- **关键迁移**：不固定数域，而是让数域次数不断增大；同时控制根判别式和类数损失。
- **几何收束**：把高维 Minkowski 空间中的单位模差向量投影到一个复坐标，投影后的差仍然是平面单位距离。

原始 PDF 使用了带 3 次幂 Galois 群的无穷塔，并通过 Frobenius 条件让固定素数完全分裂。专家整理版改用了更简洁的 2 次幂塔论证，并指出原始模型的路线在分裂素数和塔的参数上有不必要的复杂度。

### 3. 单位距离问题中“Agent”的真实角色

公开材料支持的角色分工是：

| 环节 | 公开证据 | 作用 |
| --- | --- | --- |
| 问题选择 | OpenAI 评估一组 Erdős 问题 | 扩大模型探索的问题面，而不是先人工指定唯一技术路线 |
| 生成路线 | 通用推理模型 | 尝试证明和反例，跨越组合几何、数论、代数几何等知识域 |
| 结果筛选 | AI grading pipeline | 对完整性和形式要求给出自动置信度信号 |
| 数学复核 | Daniel Litt 等外部数学家 | 逐步确认数论和几何论证是否成立 |
| 人工整理 | Alon、Bloom、Gowers、Litt、Sawin、Shankar、Tsimerman、Wang、Wood 等 | 简化论证、补背景、优化参数、说明历史脉络和局限 |
| 形式化 | 后续 Lean 工程 | 将结论转成机器可核验的定理和依赖结构 |

官方单位距离材料没有给出并行 Agent 数量、通信协议、模型版本、每轮提示词或完整失败样本。因此，不能把 Navier-Stokes 的 10,000 Agent 数字倒推到单位距离工作。

## 二、专家对结果的判断

### Noga Alon：跨领域迁移和反共识搜索

Alon 认为这是长期公开问题中的重要结果，关键惊喜在于：看似初等的几何问题，最后依赖深层代数数论。其观察可以转成研究方法：当主流方向长期没有突破时，应主动保留“反例搜索”和跨领域迁移两条路线。

### Thomas Bloom：AI 找到的是被忽略的方向

Bloom 强调，结果并没有带来一种全新的几何工具，而是让数论构造进入了单位距离问题。他还指出，专家整理版比原始输出更短、更一般，也更适合人类理解。

### W. T. Gowers：提示序列和搜索耐心

Gowers 用“给专家的提示序列”描述这类问题的难度。对单位距离问题，他认为几个关键提示可能是：

1. 认真尝试反例，而不是只尝试证明原猜想；
2. 推广最佳已知平方网格构造；
3. 从固定数域转向次数不断增加、但素理想范数保持受控的数域序列。

这说明模型的优势可能来自广泛知识、长时间尝试和不容易因为路线看似偏僻而提前放弃。它不等于模型能稳定解决任意数学问题。

### Daniel Litt：先有自动结果，再由专家确认

Litt 描述了自己被邀请复核数论部分的过程，并表示很快确认了论证的正确性和巧妙性。这个证词支持“模型先产出候选，专家再确认”的流程，也说明单位距离结果的可信度来自后续专家工作，而不是单靠模型自评。

### Will Sawin：增加数域次数是关键转折

Sawin 指出，若只在一个固定 CM 数域中扩大窗口，不能自然得到所需的增长；原始模型采用“固定参数窗口 + 数域次数增长”的方向，这个视角转换很难从固定域分析中直接看出来。Sawin 还给出显式 `δ=0.014` 的版本，使原始“存在某个 δ>0”变成了可计算的量化结果。

### Jacob Tsimerman：完成后的证明不代表发现过程简单

Tsimerman 强调，知道路线以后，很多参数选择看起来顺理成章；在不知道路线时，素数大小、窗口大小、分裂条件和判别式之间有大量互相牵制的选择。Agent 的价值之一，是可以并行或连续地试探这些组合，而不会像个人研究者一样很快受到时间和注意力限制。

### Melanie Matchett Wood：结果价值和成功率要分开

Wood 的判断最适合转成工程准则：

- 这个结果本身很漂亮；
- 但成功案例不能代表模型在所有开放问题上的成功率；
- 模型让专家注意到一个原本不一定会被集中研究的反例方向；
- 相关文献的引用和 AI 对已有思想的使用方式仍需要科研共同体形成规范。

## 三、与 Navier-Stokes 工作流的区别

OpenAI 关于 Navier-Stokes 的公开文章给出了更明确的多 Agent 方案：

1. 建立协调 Agent 系统；
2. 给 Agent 提供缓存互联网和代码执行工具；
3. 把 Agent 分成不同规模的小组，并允许组内通信；
4. 对同一问题的不同命题变体分别设任务。Navier-Stokes 被拆成 A/B 两个“寻求证明”的版本和 C/D 两个“寻求反例”的版本；
5. 先用约 100 个 Agent、约 50 小时处理 Euler 正则性问题；
6. 发现可行路线后，把资源转向 Navier-Stokes；
7. 让不同小组继续保持路线多样性，再通过 Codex 汇总有价值的中间结论；
8. 得到解析证明后，再用 Lean 形式化和验证。

官方文章声称完整 Navier-Stokes 工作涉及约 10,000 个并发 Agent、约 88 小时搜索，以及额外约 17 小时 Lean 形式化。这个数字和流程只适用于官方公开的 Navier-Stokes 项目，不能用于描述单位距离项目。

另一个边界是数学有效性：Clay Mathematics Institute 的公开页面仍将 Navier-Stokes 题目标记为 Active。OpenAI 的文章说明了自己的结果和形式化材料，但这不等于已经获得 Clay 奖项或完成学界最终接收。

## 四、开源社区与数学社区的讨论

### 1. Erdős Problems：从发布到复核的公开讨论

Erdős Problems 的第 90 题页面目前将问题标为 `DISPROVED (LEAN)`，并链接了 OpenAI 原始证明、Google DeepMind 的 formal-conjectures 题目陈述，以及多个独立形式化项目。该页面的讨论区显示 45 条评论，但页面也明确提示评论内容不代表平台已经逐条确认。

讨论中最有价值的几条线索：

- 社区把原始 OpenAI 证明、专家整理版和 Sawin 的显式参数版分开看待；
- 有人用 ChatGPT 搜索有限素数集合和参数，取得更强的数值证书，但这些结果必须由算术程序或专家重新验证；
- 社区成员指出，OpenAI 原始构造在历史上重要，但并没有针对显式指数或参数做充分优化；
- `plby/Erdos90` 被引用为无条件 Lean 形式化工程，另有早期工程仍带额外公理或条件依赖；
- 讨论还延伸到固定数域、单位距离图的图论性质和更高维推广。

这说明社区工作已经从“结果是真是假”进入了三条并行路线：证明复核、参数优化、结果推广。

### 2. MathOverflow：参数优化和可复现实验

MathOverflow 问题 [What is the unit distance exponent?](https://mathoverflow.net/questions/511514/what-is-the-unit-distance-exponent) 专门讨论如何提升 `δ`。页面中的公开信息显示：

- Sawin 的显式版本先给出约 `δ=0.014`；
- 社区成员随后围绕素数集合、Golod-Shafarevich 条件、重叠估计和归一化范数展开优化；
- 有人报告了约 `1.01526`、`1.03184` 等更强候选值，并公开计算过程或链接；
- Sawin 说明其论文中的参数还有优化空间，也指出仅靠同一类数域构造存在理论上限和技术障碍。

这些数值不是 OpenAI 原始定理的重新表述，也不自动等于正式发表的最终纪录。它们更像“可复现的计算证书候选”，需要锁定参数、运行验证器、确认每个数论条件并保存输入输出。

### 3. GitHub：形式化验证的不同层级

目前可见的开源项目可以分为四类：

| 项目 | 主要作用 | 不能直接推出的结论 |
| --- | --- | --- |
| [plby/Erdos90](https://github.com/plby/Erdos90) | OpenAI 2026 单位距离反例的 Lean 证明工程，区分 `src/original` 和 `src/submission` | 仅看到仓库不能替代本地 Lean 构建记录 |
| [kim-em/erdos-unit-distance](https://github.com/kim-em/erdos-unit-distance) | Alpöge 版本的 uniform-constant 反例形式化 | 它与 OpenAI 的固定幂次增长版本不是同一个定理 |
| [kim-em/erdos-unit-distance-comparator](https://github.com/kim-em/erdos-unit-distance-comparator) | 用 Comparator 独立比较挑战陈述、证明导出和公理依赖 | Comparator 项目的结论范围取决于它声明的挑战定理 |
| [google-deepmind/formal-conjectures](https://github.com/google-deepmind/formal-conjectures) | 收录 Erdős 问题的 Lean 题目陈述及形式化链接 | 题目陈述仓库不等于完整证明仓库 |

`plby/Erdos90` 的 README 说明原始模型证明和 lean-eval 提交版本分开存放；其提交目录还包含 `holes.json`、挑战文件和证明文件。社区把它称作无条件形式化，但真正使用时仍应读取构建日志和公理列表，不能只依据仓库标题判断。

### 4. Hacker News、Reddit 等通用社区

- Hacker News 上有原始伴随 PDF 的独立链接，但该条目本身讨论量很少；更广泛的 AI 数学讨论集中在“自动发现”和“人工确认”的定义、成功案例筛选偏差、模型是否能判断自己的证明正确等问题。
- Reddit 的相关页面受到匿名访问限制，本轮没有把 Reddit 帖子当作已核实证据。
- 因此，本报告把数学细节优先交给 OpenAI 原始 PDF、伴随论文、Erdős Problems 和 MathOverflow；把 Hacker News 作为舆论补充，把 Reddit 标为未获取。

## 五、可迁移到 CANN 的 Agent 研究流程

### 1. 把目标写成机器可判断的命题

CANN 题目必须先固定：

- 接口签名、输入输出 dtype、形状范围和广播关系；
- 每一步核心计算必须位于 Ascend C Kernel；
- 允许的误差、15 个测试点和性能统计方式；
- 禁止 Host 代算、空 Kernel、写死结果等绕过行为；
- 在没有真机时，哪些内容只能归为静态或 CPU 参考结果。

这对应单位距离提示词中的“正解/反例、量词、完整性”约束。目标越可判断，后续 Agent 越不容易用一份漂亮但不合格的总结结束任务。

### 2. 让不同 Agent 走相互独立的路线

建议将 CANN 研究拆成以下互斥主题：

| 主题 | 输入 | 输出 |
| --- | --- | --- |
| 题面与提交 | 官方题面、下载模板、提交页面 | 接口表、保护文件、提交格式 |
| Ascend C API | CANN 9.0.0 文档 | API 签名、产品范围、最小示例 |
| 参考语义 | Python/NumPy/PyTorch 参考 | 精度 oracle、边界用例 |
| Kernel 路线 | UB、TQue、DataCopy、ReduceSum | 逐行实现方案、资源估算 |
| 尾块与 dtype | D 非 32 倍数、FP16/BF16/FP32 | 掩码、补齐、转换和误差方案 |
| 性能 | 形状、核数、带宽、重复次数 | 候选 tiling 和测量计划 |
| 合规与提交 | 题面规则、提交包 | 可提交文件清单和风险项 |

每个 Agent 的交付格式固定为：结论、来源、源码位置、未确认点、最小可运行实验、建议下一步。不要只交一段“看起来合理”的技术描述。

### 3. 用共享资料库承接中间结果

共享状态至少包含五张表：

1. 事实表：题面原文、来源 URL、版本和访问日期；
2. API 表：签名、支持 SoC、当前代码用法和证据等级；
3. 假设表：尚未在真机确认的形状、对齐、性能和判题行为；
4. 反例表：编译错误、越界风险、精度误差和不成立的迁移方案；
5. 实验表：版本号、源码变更、命令、输入、输出、耗时和结果。

Agent 之间只共享可引用的条目。主 Agent 负责合并冲突、保留原始证据和决定是否进入实现，不让一个 Agent 的猜测自动变成项目事实。

### 4. 采用分阶段证据链

```text
题面和接口事实
    -> CPU 参考结果
    -> 普通 C++ / 静态结构验证
    -> CANN 9.0.0 编译
    -> 真机精度矩阵
    -> 真机性能测量
    -> CANNJudge 提交结果
    -> 用户确认后再进行最终提交
```

每一阶段只允许声明自己真正拥有的证据。单位距离项目中“自动评分高置信度”“数学家读过证明”“Lean 形式化”也是三种不同证据，CANN 项目同样要把“源码存在”“本机能编译”“15 点通过”“分数提升”分开写。

### 5. 对 CANN 的具体落地

当前仓库建议按下面的顺序运行：

1. `调研/调研2/` 继续保存按主题拆分的技术资料；
2. `源码/` 只保留唯一 Ascend C 工程；
3. `提交/V001`、`提交/V002` 等只保存已经准备过的提交代码和结果；
4. 每个版本记录“改了哪些文件、验证了什么、失败在哪里、下一步实验是什么”；
5. 没有 CANN 和 NPU 时只做静态、CPU 参考和接口核对，不写成真机结论；
6. 任何 CANNJudge 上传动作都留到本地证据齐全并获得用户确认之后。

## 六、证据边界

本报告将结论分为：

- **A**：OpenAI 官方页面、OpenAI 官方 PDF、官方 GitHub 仓库、arXiv 原始论文；
- **B**：Erdős Problems、MathOverflow、Lean 社区的公开原始页面和源码仓库；
- **C**：Hacker News 等通用社区的讨论；
- **D**：搜索摘要、无法打开的页面或未复现的帖子。

单位距离问题的数学结论已经有多份独立公开材料支持；但“成功尝试之外还有多少失败尝试”“具体模型内部如何评分和分配上下文”“原始提示词之外是否有隐藏控制策略”目前没有完整公开数据。凡是社区中的猜测，都不能写成 OpenAI 官方流程。

## 来源

1. OpenAI, [An OpenAI model has disproved a central conjecture in discrete geometry](https://openai.com/index/model-disproves-discrete-geometry-conjecture/), 2026-05-20。A，官方结果说明和方法边界。
2. OpenAI, [Planar Point Sets with Many Unit Distances](https://cdn.openai.com/pdf/74c24085-19b0-4534-9c90-465b8e29ad73/unit-distance-proof.pdf), 2026-05-20。A，原始提示词、AI 使用说明和原始证明。
3. Alon, Bloom, Gowers, Litt, Sawin, Shankar, Tsimerman, Wang, Wood, [Remarks on the Disproof of the Unit Distance Conjecture](https://cdn.openai.com/pdf/74c24085-19b0-4534-9c90-465b8e29ad73/unit-distance-remarks.pdf), 2026。A，专家整理版和反思文章。
4. Will Sawin, [An explicit lower bound for the unit distance problem](https://arxiv.org/abs/2605.20579), 2026-05-20。A，显式 `δ=0.014` 的参数化结果。
5. Michael T. M. Emmerich, [Optimizing Explicit Unit-Distance Lower-Bound Certificates](https://arxiv.org/abs/2606.03419), 2026-06-09。A，公开参数优化和验证程序说明。
6. OpenAI, [On the Navier-Stokes Millennium Prize Problem](https://openai.com/index/navier-stokes-solution/), 2026-09-08。A，多 Agent、路线分组、Codex 汇总和 Lean 形式化说明。
7. [Erdős Problems #90](https://www.erdosproblems.com/90)。B，问题状态、形式化入口和社区讨论索引。
8. [Erdős Problems #90 discussion](https://www.erdosproblems.com/forum/discuss/90)。B，原始证明、形式化工程和参数优化的社区讨论；评论正确性由发帖者自行负责。
9. [MathOverflow: What is the unit distance exponent?](https://mathoverflow.net/questions/511514/what-is-the-unit-distance-exponent)。B，显式指数优化和数论参数讨论。
10. [plby/Erdos90](https://github.com/plby/Erdos90)。B，OpenAI 单位距离反例的 Lean 工程。
11. [kim-em/erdos-unit-distance](https://github.com/kim-em/erdos-unit-distance)。B，Alpöge 版本的 uniform-constant 反例形式化，定理范围不同。
12. [kim-em/erdos-unit-distance-comparator](https://github.com/kim-em/erdos-unit-distance-comparator)。B，Comparator 独立验证工程。
13. [google-deepmind/formal-conjectures PR #4379](https://github.com/google-deepmind/formal-conjectures/pull/4379)。B，将形式化证明链接到 Erdős 问题目录的社区变更。
14. [Hacker News: Remarks on the Disproof of the Unit Distance Conjecture](https://news.ycombinator.com/item?id=48297807)。C，通用社区对伴随 PDF 的转发记录；讨论量有限。

访问日期：2026-09-12。

# OpenAI 公开 Agent 研究方式与本题迁移边界

## 结论

截至 2026-09-11，公开资料可以确认 OpenAI 发布了关于 Navier-Stokes Millennium Prize Problem 的 AI 生成解答，并附有数学写作说明和 Lean 形式化证明；同时公开声称模型在离散几何的 Erdős unit distance problem 上取得了结果。OpenAI 还公开了相关 Lean 形式化仓库。公开资料没有给出完整的内部 Agent 拓扑、提示词、模型组合、调度器实现或算力成本表。

这里的准确名称是 **Erdős unit distance problem/conjecture（埃尔德什单位距离问题/猜想）**。它不是“单位圆距离猜想”。“单位距离”指平面点集中的距离为 1 的点对数量问题，研究对象与当前 `AddRmsNormBias` 算子完全不同。

## 1. 官方资料核对

| 资料 | 已确认内容 | 证据状态 |
| --- | --- | --- |
| OpenAI News RSS：`On the Navier-Stokes Millennium Prize Problem` | 官方描述为分享 AI 生成的解答、写作说明和 Lean 形式化证明 | `verified`，官方 RSS |
| OpenAI News RSS：`An OpenAI model has disproved a central conjecture in discrete geometry` | 官方描述为模型解决有 80 年历史的单位距离问题，并推翻离散几何中的一个主要猜想 | `verified`，官方 RSS |
| OpenAI 预印本 `Finite Time Blowup for Navier-Stokes` | 论文摘要与定理 1.1 给出三维不可压 Navier-Stokes 的有限时间速度无界、动能有界构造；正文目录显示从物理描述、证明纲要到背景流、振荡脉冲、应力调整和全空间构造的完整证明组织 | `verified`，官方 PDF |
| OpenAI GitHub `NavierStokesAndEuler` | README 说明仓库包含 Lean 4 形式化，并给出 `lake exe cache get`、`lake build` 和 Comparator 独立核验入口；本机没有 Lean 环境，未执行构建 | `verified`，官方仓库 README |
| OpenAI 相关正文页面 | 页面地址和 Sitemap 可确认，当前命令行访问返回 Cloudflare 403，正文细节不能据此继续扩展 | `partial`，访问受限 |

官方 PDF 的结果形态值得单独记录：它不是一个数值模拟报告，而是一个带定理、构造、估计、附录和引用的数学证明文档。第 1 页给出论文题名、摘要和定理 1.1；第 6-7 页提供物理描述与证明纲要；后续章节按背景流、振荡脉冲、应力调整和局部化逐步建立结论。这个结构只能说明最终交付物如何组织，不能反推出内部 Agent 的具体分工。

OpenAI 官方页面的公开描述还包括以下高层编排信息：使用能读取互联网缓存和运行代码的协调 Agent；不同 Agent 组可以在组内通信；针对 Navier-Stokes 的不同表述分别探索可证和可否证方向；再用 Codex 汇总各组的中间洞见，把结果交叉传给后续 Agent。官方还称约 10,000 个并发 Agent 在约 88 小时后得到结果，Lean 形式化和验证另花约 17 小时，由 GPT-6 Astra 完成。这里仍然没有公开调度器、提示词模板、失败重试和候选淘汰的实现细节。

## 2. 独立报道披露的规模与 Agent 结构

Nature 的独立报道《OpenAI claims huge maths breakthrough on a famed 'Millennium Problem'》写道：简化版问题阶段约使用 1,000 个 Agent、持续约 50 小时；之后处理完整 Navier-Stokes 问题时扩大到约 10,000 个 Agent。报道同时指出结果仍需要数学界独立审阅。

CNBC 的独立报道进一步转述 OpenAI 的说法：约 10,000 个协作 Agent 可以读取互联网缓存、运行代码，并按组协作和在组内通信；报道给出约 88 小时的整体时间。这里能确认的是高层能力描述，不能由此推导出具体调度器、提示词或任务路由。

Clay Mathematics Institute 的 Navier-Stokes 页面在 2026-09-11 仍显示 `Active`。因此当前资料支持“OpenAI 已公开一份候选解答和形式化材料”，不支持“该结果已经得到 Clay 或数学界正式接受”。

这些信息的用途是帮助理解“分阶段推进 + 并行探索 + 共享工具 + 独立复核”的组织方式，证据等级为 C。它们不是 OpenAI 官方产品文档，也没有公开每个 Agent 的提示词、模型版本、任务分配、通信格式、失败重试、候选淘汰或评分规则，因此本项目不把这些细节补写成事实。

### 2.1 社区讨论的主要问题

社区讨论集中在三件事：结果是否已被独立理解和复现、公开资料能否说明优先权与数据来源、以及万级 Agent 的算力成本是否适合被普通研究者复制。Simon Willison 依据公开 token 价格做了数量级估算；Hacker News 讨论则关注抢发、训练数据边界和外部审阅。Tristan Buckmaster 的公开声明明确说他没有看过 OpenAI 的证明，也不知道模型具体做了什么，因此这些讨论不能替代数学审阅。

OpenAI 的 Navier-Stokes 公告明确表示不主张 Millennium Prize；Clay Mathematics Institute 页面在 2026-09-11 仍显示 `Active`。所以本报告把它记录为“官方发布的候选解答和形式化材料”，不写成已经得到奖项或数学界最终认可。

## 3. 从公开资料能提炼的研究组织模式

公开证据支持以下保守抽象：

1. 先定义最终问题和可接受的证明/实现结果。
2. 把困难问题拆成可独立推进的局部问题，允许多条路线同时探索。
3. 保留中间构造、反例、失败路线和引用关系，便于后续合并。
4. 用外部可执行工具验证关键产物。数学研究中可以是 Lean 形式化；算子工程中应是编译器、运行时、数值参考和性能测量。
5. 对关键结论安排独立复核，特别关注边界条件、版本差异和“看起来相似但语义不同”的实现。
6. 最终交付一份可读、可引用、能指出证据边界的报告，而不是只保留一个模型回答。

这是一种从公开材料提炼出的项目组织模型，不是 OpenAI 已公开的内部工作流说明。

## 4. 迁移到 AddRmsNormBias 的研究路线

当前比赛的研究路线应按工程证据排序：

```text
题面和上传格式
        ↓
CANN 9.0.0 / 判题 SoC / Ascend C API
        ↓
官方样例：DataCopy、DataCopyPad、ReduceSum、Cast、TQue
        ↓
相近实现：RMSNorm、AddRmsNorm、带 bias 的融合算子
        ↓
明确迁移差异：输入输出、epsilon、广播、D 尾块、dtype、Kernel 入口
        ↓
CPU 参考与静态审阅
        ↓
真实 NPU 编译和最小用例
        ↓
FP32 / FP16 / BF16、2D / 3D / 4D、边界 D 验证
        ↓
性能测量、独立复核、提交包
```

推荐的 Agent 分工如下：

| 研究角色 | 输入 | 产出 | 必须回答的问题 |
| --- | --- | --- | --- |
| 题面与规则 | 题面、提交页、比赛页面 | 规则表、接口表 | 真实上传格式、测试点、额度和 SoC 是什么 |
| CANN API | CANN 9.0.0 文档和模板 | API 证据表 | 当前头文件中的签名、对齐和支持产品是什么 |
| 相近 Kernel | 官方样例和公开源码 | 迁移笔记 | 哪些计算路径可复用，哪些输入/输出语义不同 |
| 数值验证 | 参考实现和覆盖矩阵 | 误差记录 | FP32 累加、BF16 舍入和 epsilon 顺序是否一致 |
| 真机验证 | CANN + NPU | 编译、精度、性能日志 | 代码能否在实际判题条件下运行 |
| 独立复核 | 上述所有产物 | 差异与风险清单 | 是否有未经证实的结论或违规路径 |

这套分工借鉴“并行探索、共享产物、工具验证、独立复核”的公开研究组织方式，但最终验收仍以比赛题面、真实编译和判题结果为准。

## 5. 不能直接迁移的内容

- Navier-Stokes 是连续介质方程的数学构造问题；AddRmsNormBias 是固定张量形状下的 NPU 算子工程问题。
- Lean 形式化证明可以验证数学命题，但不能验证 Ascend C 的 DMA 对齐、UB 容量、指令可用性或 Kernel 性能。
- 多 Agent 数量本身不是质量指标；在本题中，少量拥有明确输入、输出和验收条件的研究任务更有价值。
- OpenAI 论文的“证明纲要”不能直接转化为 Kernel 设计；本题需要逐 API、逐 dtype、逐边界记录可运行证据。

## 6. 当前证据限制

- OpenAI 官方相关正文页面在当前网络环境被 Cloudflare 挑战拦截；本报告只把 RSS、官方 PDF 和能直接读取的公开来源作为已核对证据。
- 官方公开材料未披露完整 Agent 编排细节；任何更细的模型配置、提示词或任务调度说法都保持未确认。
- Clay 页面仍将 Navier-Stokes 题目标为 `Active`；OpenAI 结果的外部正式接受状态未确认。
- OpenAI 页面中有关 Agent 数量、消息量和阶段时长属于官方自述；社区成本估算和外部评价属于二手资料，不能互相替代。
- 本仓库没有真实 CANN 9.0.0 和昇腾 NPU，不能据此声称比赛源码已经编译、精度通过或性能达标。

## 来源

1. OpenAI，`On the Navier-Stokes Millennium Prize Problem`，官方页面：https://openai.com/index/navier-stokes-solution 。RSS 可读取标题和摘要；正文页面当前访问受限。
2. OpenAI，`An OpenAI model has disproved a central conjecture in discrete geometry`，官方页面：https://openai.com/index/model-disproves-discrete-geometry-conjecture 。RSS 可读取标题和摘要；正文页面当前访问受限。
3. OpenAI，`Finite Time Blowup for Navier-Stokes`，官方预印本 PDF：https://cdn.openai.com/pdf/32d9f210-8b73-45e0-91bc-82a30aef8a9a/navier-stokes.pdf 。本地读取 PDF 元数据、首页和正文页，访问日期 2026-09-11。
4. Nature，`OpenAI claims huge maths breakthrough on a famed 'Millennium Problem'`，独立报道：https://www.nature.com/articles/d41586-026-02842-5 。访问日期 2026-09-11；用于 1,000/10,000 Agent 和 50 小时的报道证据。
5. OpenAI，`NavierStokesAndEuler`，官方 GitHub 仓库：https://github.com/openai/NavierStokesAndEuler 。README 的 Lean 4 构建和 Comparator 入口已读取；本机未构建。
6. CNBC，`OpenAI claims to have solved the 90-year-old Navier-Stokes math problem in 88 hours`，独立报道：https://www.cnbc.com/2026/09/09/openai-navier-stokes-math-problem-solved.html 。访问日期 2026-09-11；用于协作 Agent、工具能力和约 88 小时的报道证据。
7. Clay Mathematics Institute，`Navier-Stokes Equation`，官方问题页：https://www.claymath.org/millennium/navier-stokes-equation/ 。访问日期 2026-09-11；页面显示 `Active`，用于外部接受状态限制。
8. Simon Willison，`On Navier-Stokes`，社区分析：https://simonwillison.net/2026/Sep/8/on-navier-stokes/ 。访问日期 2026-09-11；用于公开 token 成本的数量级估算。
9. Hacker News Algolia API，Navier-Stokes 讨论：https://hn.algolia.com/api/v1/search?query=Navier%20Stokes%20proof%20agents&tags=comment 。访问日期 2026-09-11；用于社区争议线索，不作为数学事实证明。
10. Tristan Buckmaster，公开声明 PDF：https://cims.nyu.edu/~tristanb/statement.pdf 。本地缓存于 `../缓存/buckmaster_statement.pdf`，读取日期 2026-09-11；用于外部审阅与优先权边界。
11. OpenAI，`Research acceleration: The view inside OpenAI`，官方页面：https://openai.com/index/research-acceleration-view-inside-openai 。官方 RSS 可确认其主题涉及 coding agents、实验速度、任务复杂度和研究加速；内部方法细节当前未进一步核对。

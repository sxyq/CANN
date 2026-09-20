# F8: 多Agent系统架构模式总结

## Findings

### [1] Anthropic Research 采用 orchestrator-worker 模式：LeadResearcher 制定计划并写入 Memory，再派生多个并行 Subagent 检索，最后由 CitationAgent 统一挂引用
- quote: "Our Research system uses a multi-agent architecture with an orchestrator-worker pattern, where a lead agent coordinates the process while delegating to specialized subagents that operate in parallel."
- url: https://www.anthropic.com/engineering/built-multi-agent-research-system
- source_type: primary
- published: 2025-06-13
- confidence: high

### [2] 多Agent检索相对单Agent在内部 eval 上提升 90.2%（Opus 4 lead + Sonnet 4 subagents 对比单 Opus 4），主要收益来自并行扩大 token 预算
- quote: "a multi-agent system with Claude Opus 4 as the lead agent and Claude Sonnet 4 subagents outperformed single-agent Claude Opus 4 by 90.2% on our internal research eval."
- url: https://www.anthropic.com/engineering/built-multi-agent-research-system
- source_type: primary
- published: 2025-06-13
- confidence: high

### [3] 多Agent成本显著更高：Agent 交互 token 约为普通 chat 的 4 倍，多Agent系统约为 15 倍；BrowseComp 方差的 80% 可由 token 用量单独解释
- quote: "agents typically use about 4× more tokens than chat interactions, and multi-agent systems use about 15× more tokens than chats."
- url: https://www.anthropic.com/engineering/built-multi-agent-research-system
- source_type: primary
- published: 2025-06-13
- confidence: high

### [4] Anthropic 将「任务复杂度 → 子agent数量与工具调用预算」写死进 prompt：简单事实查 1 agent / 3–10 次调用，对比类 2–4 agents / 10–15 次，复杂研究 >10 agents
- quote: "Simple fact-finding requires just 1 agent with 3-10 tool calls, direct comparisons might need 2-4 subagents with 10-15 calls each, and complex research might use more than 10 subagents with clearly divided responsibilities."
- url: https://www.anthropic.com/engineering/built-multi-agent-research-system
- source_type: primary
- published: 2025-06-13
- confidence: high

### [5] 早期失败模式包括：为简单问题一次派生 50 个 subagent、指令过短导致子agent重复劳动或分工缺口；并行化（3–5 个 subagent + 3+ 并行工具调用）把复杂检索时间最多压掉 90%
- quote: "Early agents made errors like spawning 50 subagents for simple queries... We started by allowing the lead agent to give simple, short instructions like 'research the semiconductor shortage,' but found these instructions often were vague enough that subagents misinterpreted the task or performed the exact same searches as other agents."
- url: https://www.anthropic.com/engineering/built-multi-agent-research-system
- source_type: primary
- published: 2025-06-13
- confidence: high

### [6] 结果聚合推荐用「产物落盘 + 轻量引用回传」减少传话失真：subagent 把报告/代码写入外部 artifact，coordinator 只收引用
- quote: "Subagent output to a filesystem to minimize the 'game of telephone.' ... Subagents call tools to store their work in external systems, then pass lightweight references back to the coordinator."
- url: https://www.anthropic.com/engineering/built-multi-agent-research-system
- source_type: primary
- published: 2025-06-13
- confidence: high

### [7] 生产级失败处理：Agent 有状态、错误会复利，不能从头重跑；需断点续跑、确定性 retry、rainbow deploy，且同步 lead 等待 subagent 是当前瓶颈
- quote: "Agents are stateful and errors compound... we built systems that can resume from where the agent was when the errors occurred. We also use the model's intelligence to handle issues gracefully: for instance, letting the agent know when a tool is failing and letting it adapt works surprisingly well. We combine the adaptability of AI agents built on Claude with deterministic safeguards like retry logic and regular checkpoints."
- url: https://www.anthropic.com/engineering/built-multi-agent-research-system
- source_type: primary
- published: 2025-06-13
- confidence: high

### [8] OpenAI Agents SDK 把编排收敛为两种主模式：Agents-as-tools（manager 保有最终答案并汇总）与 Handoffs（triage 路由后 specialist 接管本轮）；代码编排则含 structured output 分类、链式变换、producer–evaluator while 循环、asyncio 并行
- quote: "Agents as tools — A manager agent keeps control of the conversation and calls specialist agents through Agent.as_tool(). ... Handoffs — A triage agent routes the conversation to a specialist, and that specialist becomes the active agent for the rest of the turn."
- url: https://openai.github.io/openai-agents-python/multi_agent/
- source_type: primary
- published: unknown
- confidence: high

### [9] MetaGPT 用 SOP 编码流水线对抗级联幻觉：把标准作业程序写进 prompt 序列，让领域角色 agent 验证中间产物；在协作软件工程基准上比纯 chat 多Agent更连贯
- quote: "MetaGPT encodes Standardized Operating Procedures (SOPs) into prompt sequences for more streamlined workflows, thus allowing agents with human-like domain expertise to verify intermediate results and reduce errors. MetaGPT utilizes an assembly line paradigm to assign diverse roles to various agents."
- url: https://arxiv.org/abs/2308.00352
- source_type: primary
- published: 2023-08-01 (v7: 2024-11-01)
- confidence: high

### [10] CAMEL 用 role-playing + inception prompting 实现无需人工逐步引导的双 Agent 自主协作，并开源了研究多Agent社会行为的库
- quote: "we propose a novel communicative agent framework named role-playing. Our approach involves using inception prompting to guide chat agents toward task completion while maintaining consistency with human intentions."
- url: https://arxiv.org/abs/2303.17760
- source_type: primary
- published: 2023-03-31 (NeurIPS 2023)
- confidence: high

### [11] LangGraph 定位为低层编排运行时：在同一张图里混合确定性步骤与 LLM 决策步骤，提供持久执行、human-in-the-loop、长短期记忆；不抽象 prompt 与架构
- quote: "One of LangGraph's core strengths is the ability to mix deterministic steps with LLM-driven agentic steps in a single graph."
- url: https://docs.langchain.com/oss/python/langgraph/overview
- source_type: primary
- published: unknown
- confidence: high

### [12] Multiagent Debate（Du et al.）：多个 LLM 实例各自给出答案与推理，多轮互相辩论后收敛到共同答案，可直接套在黑盒模型上；在数学与事实性任务上显著提升
- quote: "multiple language model instances propose and debate their individual responses and reasoning processes over multiple rounds to arrive at a common final answer. Our findings indicate that this approach significantly enhances mathematical and strategic reasoning across a number of tasks."
- url: https://arxiv.org/abs/2305.14325
- source_type: primary
- published: 2023-05-23
- confidence: high

### [13] Self-consistency（Wang et al., ICLR 2023）通过采样多条推理路径并对答案边缘化投票，在 GSM8K 上 +17.9%、SVAMP +11.0%、AQuA +12.2% —— 是结果聚合「多数投票」路线的代表算法
- quote: "It first samples a diverse set of reasoning paths instead of only taking the greedy one, and then selects the most consistent answer by marginalizing out the sampled reasoning paths."
- url: https://arxiv.org/abs/2203.11171
- source_type: primary
- published: 2022-03-21 (ICLR 2023)
- confidence: high

### [14] Berkeley 2025 研究（Cemri et al.）对 5 个主流 MAS 框架、150+ 任务标注出 14 种失败模式，归入 3 类：规格/系统设计失败、agent 间错位、任务验证与终止；简单「加强角色描述」和「换编排策略」并不足以根治
- quote: "We analyze five popular MAS frameworks over 150 tasks, involving six expert human annotators. We identify 14 unique failure modes... organized into 3 categories, (i) specification and system design failures, (ii) inter-agent misalignment, and (iii) task verification and termination."
- url: https://arxiv.org/abs/2503.13657
- source_type: primary
- published: 2025-03-17 (v3: 2025-10-26)
- confidence: high

### [15] 该失败研究明确指出：多Agent相对单Agent在主流基准上的性能增益仍然很小，「验证与终止」是独立失败类，说明竞赛/科研场景不能默认多Agent一定更强
- quote: "Despite growing enthusiasm for Multi-Agent Systems (MAS), where multiple LLM agents collaborate to accomplish tasks, their performance gains across popular benchmarks remain minimal compared to single-agent frameworks."
- url: https://arxiv.org/abs/2503.13657
- source_type: primary
- published: 2025-03-17
- confidence: high

### [16] CrewAI 把角色定义产品化为 role/goal/backstory 三元组，并默认给出失败处理参数：max_iter=20、max_retry_limit=2、max_rpm、max_execution_time；allow_delegation 默认为 false
- quote: "role, goal, and backstory are required and shape the agent's behavior... max_iter: Maximum attempts before giving best answer... max_retry_limit: Retries on error... Allow Delegation (allow_delegation) — Allow the agent to delegate tasks to other agents. Default is False."
- url: https://docs.crewai.com/en/concepts/agents
- source_type: primary
- published: unknown
- confidence: high

## Dead ends
- WebSearch 工具在本会话不可用，无法做开放式关键词检索；全部来源改为直接 WebFetch 已知官方文档与 arXiv 页面。
- arXiv 2503.13657 首次 GET 返回 transport error，改用 v1 URL 才拿到摘要。
- docs.langchain.com 的 /oss/python/langgraph/agent-handoffs 返回 404，未能拿到 LangGraph handoff 细节页。
- 未单独抓到 OpenAI「introducing deep research」原文；OpenAI 侧证据来自官方 Agents SDK 文档（多Agent编排页）。
- 未找到直接针对「NPU/算子竞赛场景」的多Agent实证研究；科研场景证据以 Anthropic Research 与学术失败模式研究为主。

## Suggested follow-ups
- 对照 Cemri et al. 的 14 种失败模式，检查 CANN 算子优化工作流里「验证与终止」类（编译失败重试、精度阈值、死胡同检测）是否单独建了规则。
- 深读 AutoGen 论文正文（HTML v2）中 conversation pattern 的具体编程接口，与 OpenAI Agents SDK 的 as_tool/handoffs 做字段级对比。
- 调研「artifact/文件系统作为共享状态」在 coding-agent 竞赛（SWE-bench 等）中的实测效果，验证 Anthropic 建议是否在强约束编译/判题场景同样成立。

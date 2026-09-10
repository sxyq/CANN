# 来源清单（公开资料）

> 规则：`缓存/` 只保存公开资料；本清单为全部调研来源的登记表（URL / 仓库 / 版本 / 访问日期 / 用途 / 证据等级）。
> 证据等级：A=官方文档/官网原文；B=官方仓库源码或官方权威培训；C=社区二手/技术博客。

| # | 名称 | URL / 仓库 | 版本/日期 | 访问日期 | 用途 | 等级 |
| --- | --- | --- | --- | --- | --- | --- |
| 1 | CANNJudge 题面页（AddRmsNormBias） | https://cannjudge.cn/public/op_challenge_xinan_prelim/addrmsnormbias | 2026-09-10 抓取 | 2026-09-10 | 题面全文（公式/约束/精度） | A |
| 2 | CANNJudge 提交页 | https://cannjudge.cn/public/op_challenge_xinan_prelim/addrmsnormbias/submit | — | 2026-09-10 | 需登录，上传格式待确认 | A(不可达) |
| 3 | CANNJudge 比赛列表/排名 | https://cannjudge.cn/contests ; .../ranking | — | 2026-09-10 | 通过率/人数统计 | A |
| 4 | DataCopyPad（API 手册·社区版 800 alpha001） | https://www.hiascend.com/doc_center/source/zh/CANNCommunityEdition/800alpha001/apiref/ascendcopapi/atlasascendc_api_07_0253.html | 8.0 | 2026-09-10 | 双向搬运/非对齐/产品支持矩阵 | A |
| 5 | DataCopyPad（API 手册·商用 8.0.RC2.2） | https://www.hiascend.cn/document/detail/zh/canncommercial/80RC22/apiref/opdevgapi/atlasascendc_api_07_0258.html | 8.0.RC2.2 | 2026-09-10 | 同上交叉确认 | A |
| 6 | 无DataCopyPad的处理方式 | https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/80RC3alpha002/devguide/opdevg/ascendcopdevg/atlas_ascendc_10_0037.html | 8.0.RC3 | 2026-09-10 | 降级方案（GatherMask/atomic/UnPad） | A |
| 7 | ReduceSum（API 手册） | https://www.hiascend.cn/document/detail/zh/canncommercial/800/apiref/ascendcopapi/atlasascendc_api_07_0078.html | 8.0 | 2026-09-10 | workLocal 公式/mask/对齐 | A |
| 8 | 昇腾社区 SIMD API Reduce（含 Reg 向量归约） | https://asc.gitcode.com/api/SIMD-API/基础API/Reg矢量计算/归约计算/Reduce.html | 最新 | 2026-09-10 | 归约语义交叉确认 | A |
| 9 | msopgen 工程结构（官方文档·Aclnn算子工程化开发快速入门） | https://asc.gitcode.com/guide/编程指南/高级编程/Aclnn算子工程化开发/Aclnn算子工程化开发快速入门.html | 最新 | 2026-09-10 | 工程模板结构 | A |
| 10 | msOpGen 创建算子工程（hiascend 文档） | https://www.hiascend.com/doc_center/source/zh/canncommercial/70RC1/operatordev/Ascendcopdevg/atlas_ascendc_10_0023.html | 7.0.RC1 | 2026-09-10 | 原型 json 字段说明 | A |
| 11 | 昇腾CANN训练营第二十期 RMSNorm | https://blog.csdn.net/2401_82857325/article/details/155560947 | 2025-12 | 2026-09-10 | 核心教学实现（Cast/Mul/ReduceSum/rsqrt） | B |
| 12 | CANN训练营第九期 非对齐尾块与 DataCopyPad | https://blog.csdn.net/2401_82857325/article/details/155320399 | 2025-11 | 2026-09-10 | 尾块策略 | B |
| 13 | CANN训练营 RMSNorm（v1 文章同作者补充） | https://blog.csdn.net/2401_82857325/article/details/155447666 | 2025-12 | 2026-09-10 | 同 11 交叉 | B |
| 14 | Ascend C 实战 RMSNorm（动态 shape，含两阶段融合） | https://adg.csdn.net/69730df4437a6b40336b6e5f.html | 2025-12 | 2026-09-10 | 动态 shape/融合思路 | C |
| 15 | ops-transformer RMSNorm 数值精度分析 | https://hwcomputing.csdn.net/6a156b8510ee7a33f2754f4b.html | 2026-05 | 2026-09-10 | FP16 溢出/Kahan | C |
| 16 | ops-transformer（GitCode 官方仓） | https://gitcode.com/cann/ops-transformer | `e7019c299cfc02293b184dcf0ca270b08740cdc1`（本地快照） | 2026-09-10 | AddRmsNorm 系列官方参考；读取 `mc2/matmul_all_reduce_add_rms_norm` 与 `mc2/3rd/rms_norm` | B |
| 17 | ops-nn AddRmsNormQuantV2（量化 RMSNorm 带 beta） | https://gitcode.com/cann/ops-nn （页面含 aclnnAddRmsNormQuantV2 说明，CSDN 转述于 https://blog.csdn.net/gitblog_00178/article/details/141418449） | — | 2026-09-10 | 语义最接近参考（beta 即 bias） | B/C |
| 18 | ai2open/cann-ops-transformer（GitHub fork） | https://github.com/ai2open/cann-ops-transformer/tree/master/mc2/inplace_matmul_all_reduce_add_rms_norm | master | 2026-09-10 | 可读源码入口 | B |
| 19 | cann-learning-hub（昇腾学习资源仓，博客转述） | https://gitcode.com/cann/cann-learning-hub | — | 2026-09-10 | 中级算子开发/AddCustom 样例；本轮未 clone | B/C |
| 20 | asc-devkit StoreUnAlign（非对齐搬出，A3/950 系） | https://blog.csdn.net/gitblog_00696/article/details/158201852 | 2026-06 | 2026-09-10 | 潜在非对齐方案（A2 不支持） | C |
| 21 | msopgen 构建过程（AddCustom 样例） | https://bbs.huaweicloud.com/blogs/476012 | 2026-04 | 2026-09-10 | CMakePresets.json 示例 | C |
| 22 | SGLang Ascend NPU 算子开发指南 | https://docs.sglang.io/docs/hardware-platforms/ascend-npus/ascend_npu_operator_development | 最新 | 2026-09-10 | Ascend C 工程 CMake 集成示例 | C |
| 23 | Ascend/cann-samples（官方仓） | https://github.com/Ascend/cann-samples | `23c981c0918e3183958e94e58ef6989d44983230`（本地快照） | 2026-09-10 | `rms_norm_quant_story` 的 ReduceSum、DataCopyPad、多核样例 | B |
| 24 | OpenAI：Navier-Stokes Millennium Prize Problem | https://openai.com/index/navier-stokes-solution | 官方 RSS，2026-09-08；预印本 PDF 另列 | 2026-09-11 | 官方确认 AI 生成解答、写作说明和 Lean 形式化证明 | A（RSS verified；正文 partial） |
| 25 | OpenAI：离散几何单位距离问题 | https://openai.com/index/model-disproves-discrete-geometry-conjecture | 官方 RSS，2026-05-20 | 2026-09-11 | 官方确认 Erdős unit distance problem/conjecture 相关结果；注意不是单位圆问题 | A（RSS verified；正文 partial） |
| 26 | OpenAI：Navier-Stokes 预印本 | https://cdn.openai.com/pdf/32d9f210-8b73-45e0-91bc-82a30aef8a9a/navier-stokes.pdf | PDF，166 页；本地读取日期 2026-09-11 | 2026-09-11 | 核对摘要、定理 1.1、目录、物理描述和证明纲要 | A |
| 27 | Nature：OpenAI claims huge maths breakthrough on a famed 'Millennium Problem' | https://www.nature.com/articles/d41586-026-02842-5 | 独立报道 | 2026-09-11 | 约 1,000/10,000 Agent、约 50 小时和独立审阅背景 | C |
| 28 | OpenAI：Research acceleration: The view inside OpenAI | https://openai.com/index/research-acceleration-view-inside-openai | 官方 RSS；正文当前受限 | 2026-09-11 | 公开确认 coding agents、实验速度、任务复杂度和研究加速主题 | A（RSS verified；正文 partial） |
| 29 | 用户提供的目标远端仓库 | https://github.com/sxyq/CANN.git | `git ls-remote` 未返回引用；GitHub API 先返回 404，后续触发匿名额度限制（2026-09-11） | 2026-09-11 | 目标远端登记；本地首次提交与远端可达性分开记录 | A（访问状态未确认） |
| 30 | OpenAI/NavierStokesAndEuler | https://github.com/openai/NavierStokesAndEuler | 官方仓库；README 入口已读取，commit 未登记 | 2026-09-11 | Lean 4 形式化和 Comparator 核验入口 | B（README verified；本机未构建） |
| 31 | CNBC：OpenAI claims to have solved the 90-year-old Navier-Stokes math problem in 88 hours | https://www.cnbc.com/2026/09/09/openai-navier-stokes-math-problem-solved.html | 独立报道 | 2026-09-11 | 协作 Agent、代码执行、互联网缓存、约 88 小时 | C |
| 32 | Clay Mathematics Institute：Navier-Stokes Equation | https://www.claymath.org/millennium/navier-stokes-equation/ | 页面显示 `Active`；页面更新时间 2026-09-10 | 2026-09-11 | 核对 Millennium Problem 的外部接受状态 | A |
| 33 | Tristan Buckmaster statement | https://cims.nyu.edu/~tristanb/statement.pdf | 4 页 PDF；已缓存于 `缓存/buckmaster_statement.pdf` | 2026-09-11 | 记录对 OpenAI 解答的外部审阅限制 | C（原始声明已读取） |
| 34 | Simon Willison：On Navier-Stokes | https://simonwillison.net/2026/Sep/8/on-navier-stokes/ | 社区分析 | 2026-09-11 | 公开 token 成本数量级估算和独立讨论线索 | C |
| 35 | Hacker News Navier-Stokes discussion | https://hn.algolia.com/api/v1/search?query=Navier%20Stokes%20proof%20agents&tags=comment | Algolia 公共 API 讨论结果 | 2026-09-11 | 社区对优先权、复现、数据来源和成本的讨论线索 | C |
| 36 | OpenAI Agents SDK：Multi-agent | https://openai.github.io/openai-agents-python/multi_agent/ | 官方文档 | 2026-09-11 | Agents-as-tools、Handoffs、并行与链式编排的通用 API 参考 | A |
| 37 | Anthropic：Building a Powerful Agentic Research System | https://www.anthropic.com/engineering/built-multi-agent-research-system | 官方工程文章 | 2026-09-11 | orchestrator-worker、任务规模、共享产物和失败处理参考 | C |
| 38 | Stanford KernelBench | https://scalingintelligence.stanford.edu/blogs/kernelbench/ | 官方项目文章 | 2026-09-11 | Kernel 编译→正确性→性能的分层评测流程 | B |
| 39 | DeepMind：AlphaEvolve | https://deepmind.google/blog/alphaevolve-a-gemini-powered-coding-agent-for-designing-advanced-algorithms/ | 官方项目文章 | 2026-09-11 | 程序生成、自动评估器和候选演化的工程参考 | B |
| 40 | CANNJudge 提交 Skill | https://gitcode.com/cann/cann-learning-hub/blob/master/skills/cannjudge-submit/README.md | `master`，2026-05-19 页面 | 2026-09-11 | 提交字段、状态和结果查询流程线索 | B（页面记录已读取） |
| 41 | CANN 官方竞赛归档仓 | https://gitcode.com/cann/cann-ops-competitions | `master`，2026-09-11 页面 | 2026-09-11 | AI Agent 参与声明和竞赛任务记录格式 | B（页面记录已读取） |
| 42 | plby/Erdos90 | https://github.com/plby/Erdos90 | `main` / `2062b0e6c9770c81e397bfe41148e90b92ca0567` | 2026-09-11 | Lean 形式化和题目/解答文件结构 | B（源码树已读取；本机未运行） |
| 43 | fbundle/erdos90 | https://github.com/fbundle/erdos90 | `master` / `7197bb6c2d2ef0180bde588b3c350a404278d7c4` | 2026-09-11 | 单位距离形式化的另一实现和公理依赖风险 | C（源码树已读取；本机未运行） |
| 44 | Flamehaven-Labs/openai-erdos-eq22-reproduction | https://github.com/Flamehaven-Labs/openai-erdos-eq22-reproduction | `main` / `000d3101d2e232c7595e763108af60d8ebf2fca6`，Release badge `v0.2.5` | 2026-09-11 | 单位距离公式数值复算、测试和 CI | C（源码树已读取；本机未运行） |
| 45 | kim-em/erdos-unit-distance | https://github.com/kim-em/erdos-unit-distance | `master` / commit 未确认，Lean `4.32.2` | 2026-09-11 | 单位距离 Lean 形式化和依赖版本差异 | C（源码树已读取；本机未运行） |
| 46 | kim-em/erdos-unit-distance-comparator | https://github.com/kim-em/erdos-unit-distance-comparator | `master` / commit 未确认 | 2026-09-11 | 独立 comparator、挑战语句和验证脚本 | C（源码树已读取；本机未运行） |
| 47 | mobiusresearch/navier-stokes-theorem-1-1 | https://github.com/mobiusresearch/navier-stokes-theorem-1-1 | `main` / commit 未确认 | 2026-09-11 | Navier-Stokes 正例、弱化例、未证明例和复核记录 | C（源码树已读取；本机未运行） |
| 48 | swarm-ai-research/navier-stokes-lean-check | https://github.com/swarm-ai-research/navier-stokes-lean-check | `main` / commit 未确认 | 2026-09-11 | 上游语句对照和 Lean 构建复核路径 | C（源码树已读取；本机未运行） |
| 49 | alexyyyander/proofweave | https://github.com/alexyyyander/proofweave | `main` / commit 未确认 | 2026-09-11 | 研究记录、隔离 Lean 重放、正负 fixture 和 CI | C（源码树已读取；本机未运行） |
| 50 | AniketWathore/Ramanujan | https://github.com/AniketWathore/Ramanujan | `main` / commit 未确认 | 2026-09-11 | 多模型并行、数学工具和计算检查设计 | C（源码树已读取；本机未运行） |
| 51 | 47thtechcorner/RayCodes_OpenAI_Navier_Stokes | https://github.com/47thtechcorner/RayCodes_OpenAI_Navier_Stokes | `main` / commit 未确认 | 2026-09-11 | Navier-Stokes 解释性资料线索 | C（仅 README） |
| 52 | az9713/openai-navier-stokes-results | https://github.com/az9713/openai-navier-stokes-results | `main` / commit 未确认 | 2026-09-11 | 交互式说明站点的前端组织参考 | C（仅说明站点源码） |

**未使用/未验证来源**：所有 B/C 级内容均未在本机复现运行；仅作为 API 语义与实现模式的参考。真机编译前需以实际安装的 CANN 9.0.0 头文件与手册为准。

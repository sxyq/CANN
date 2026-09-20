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
| 53 | CANN 9.0.X 官方文档入口 | https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/900/ | 9.0.X | 2026-09-11 | 记录目标版本文档入口；本机未安装对应工具链，具体头文件签名待真机确认 | A（入口可访问，API 细节未在本机验证） |
| 54 | Ascend/msopgen（MindStudio Ops Generator 官方开源） | https://github.com/Ascend/msopgen | master；2025-12-31 开源；2026-02-10 适配 AscendC 新工程 | 2026-09-11 | Agent05：msopgen 生成工程骨架而非算法 | A |
| 55 | msopgen 算子工程创建工具参数说明 | https://developer.huawei.com/consumer/cn/doc/hiai-guides/cannkit-creating-operator-project-msopgen-0000002293225826 | 2026-01-04 更新 | 2026-09-11 | Agent05：命令/JSON 字段（msopgen gen -i -c -lan cpp -out） | A |
| 56 | CANN Kit 快速入门（msopgen 生成 AddCustom 工程结构） | https://device.harmonyos.com/cn/docs/apiref/harmonyos-guides/cannkit-operator-development | 2026-05-18 更新 | 2026-09-11 | Agent05：生成工程目录（op_host/op_kernel）证据 | A |
| 57 | MindStudio 全流程工具链（msOpGen 生成器说明，官方博客） | https://www.hiascend.com/developer/blog/details/02178216137870866383 | 2026-06 | 2026-09-11 | Agent05：msOpGen 定位与 AddCustom 基线模板 | C |
| 58 | tile-ai/tilelang-ascend（昇腾 DSL 编译器，AscendC/PTO 后端） | https://github.com/tile-ai/tilelang-ascend | ascendc_pto 分支；2025-09-29 开源 | 2026-09-11 | Agent05：能否生成 Ascend C 的核心证据 | A |
| 59 | TileLang-Ascend: Ascend NPU Code Generation（deepwiki 源码索引） | https://deepwiki.com/tile-ai/tilelang-ascend/7.2-ascend-npu-code-generation | fe6517 | 2026-09-11 | Agent05：两后端差异、自动同步/内存规划 pass | C |
| 60 | tilelang-ascend elementwise 测试（T.reduce_sum/reduce_ascend_lang） | https://raw.githubusercontent.com/tile-ai/tilelang-ascend/ascendc_pto/testing/python/language/test_tilelang_ascend_language_elementwise.py | ascendc_pto | 2026-09-11 | Agent05：归约 DSL 与 PassConfig 证据 | A |
| 61 | triton-lang/triton-ascend（昇腾官方组织的 Triton 适配） | https://github.com/triton-lang/triton-ascend | 3.2.2（2026-07-31） | 2026-09-11 | Agent05：TTIR→Linalg→AscendNPU IR→.o 编译链 | A |
| 62 | Triton-Ascend 架构设计与核心特性 | https://github.com/triton-lang/triton-ascend/blob/main/docs/en/architecture_design_and_core_features.md | main | 2026-09-11 | Agent05：编译链/目录结构/驱动 | A |
| 63 | Triton-Ascend 中文 README（版本依赖矩阵） | https://github.com/triton-lang/triton-ascend/blob/main/README_zh.md | main | 2026-09-11 | Agent05：CANN 9.1.0/TorchNPU 2.7.1.post8 依赖 | A |
| 64 | Triton-Ascend Quick Start | https://github.com/triton-lang/triton-ascend/blob/main/docs/en/quick_start.md | main | 2026-09-11 | Agent05：运行方式（torch_npu 环境） | A |
| 65 | IREE 官方支持矩阵（无 Ascend） | https://iree.dev/index.html | 当前 | 2026-09-11 | Agent05：确认 IREE 后端列表不含昇腾 | A |
| 66 | IREE 仓库（Ascend 相关 issue 检索） | https://github.com/iree-org/iree ；https://github.com/search?q=repo%3Airee-org%2Firee+ascend&type=issues | main | 2026-09-11 | Agent05：反证 IREE 无昇腾后端 | A/C |
| 67 | TVM 主线 src/target 目录（GitHub API） | https://api.github.com/repos/apache/tvm/contents/src/target | main | 2026-09-11 | Agent05：确认 TVM 主线无 ascend codegen | A |
| 68 | cann/xla-npu（华为 XLA 昇腾后端） | https://gitcode.com/cann/xla-npu （转述 https://blog.csdn.net/gitblog_07156/article/details/151463150 ） | 当前 | 2026-09-11 | Agent05：AFIR/GE/Aclnn 三策略 | B（转述 C） |
| 69 | TorchNPU（Ascend/pytorch，PrivateUse1） | https://github.com/ascend/pytorch | master（TorchNPU 26.0.0） | 2026-09-11 | Agent05：torch.compile 昇腾链路入口 | A |
| 70 | PyTorch Shanghai Meetup（Ascend 多后端案例） | https://pytorch.org/blog/pytorch-shanghai-notes/ | 2024-09-08 | 2026-09-11 | Agent05：昇腾接入 PyTorch 机制背景 | A |
| 71 | npu_inductor_mlir（社区：Inductor→torch-mlir→BiSheng） | https://gitee.com/rmch/npu_inductor_mlir2 | master | 2026-09-11 | Agent05：昇腾 Inductor 链路产物形态（二进制） | C |
| 72 | 昇腾 for PyTorch 训练营笔记（③④⑤ 编译链路） | https://bbs.huaweicloud.com/blogs/482845 | 2026-07-26 | 2026-09-11 | Agent05：torch.compile→Inductor（昇腾）→Triton/MLIR→CANN Runtime | C |
| 73 | mindspore-ai/akg（Auto Kernel Generator） | https://github.com/mindspore-ai/akg | master（AKG-MLIR；支持 Atlas 800T A2/A3） | 2026-09-11 | Agent05：昇腾自动生成 kernel 的官方先例 | A |
| 74 | MindSpore 图算融合加速引擎（AKG polyhedral 说明） | https://www.mindspore.cn/docs/zh-CN/r1.10/design/graph_fusion_engine.html | r1.10 | 2026-09-11 | Agent05：AKG 调度/融合/切分机制 | A |
| 75 | IMPACT 2022 keynote：MindSpore/AKG 架构 | https://acohen.gitlabpages.inria.fr/impact/impact2022/slides/keynote.pdf | 2022 | 2026-09-11 | Agent05：图算融合+AKG 三层融合 | C |
| 76 | hicann/pypto（华为昇腾 PTO 框架；非 AMD） | https://github.com/hicann/pypto | 0.2.0（2026-04-10） | 2026-09-11 | Agent05：事实修正 PyPTO 属昇腾 PTO-ISA 生态 | A |
| 77 | PyPTO 上游视图（hw-native-sys/pypto fork） | https://github.com/tsung-li/pypto | main（PTOCodegen for PTO-ISA MLIR） | 2026-09-11 | Agent05：PTO-ISA MLIR 生成证据 | C |
| 78 | pypto 上手：用 Python 直接调 PTO 虚拟指令集 | https://blog.csdn.net/2502_93572233/article/details/161360732 | 2026-05-25 | 2026-09-11 | Agent05：PTO 在 CANN 五层架构中的位置 | C |
| 79 | HIVM: MLIR Dialect Stack for Huawei Atlas NPU（LLVM 开发者会议 2026-04 教程） | https://www.llvm.org/devmtg/2026-04/slides/tutorial/tutorial_tarasov.pdf | 2026-04-15 | 2026-09-11 | Agent05：AscendNPU-IR 方言栈、Triton-Ascend 关联 | A |
| 80 | FlagTree（Triton 兼容统一多后端编译器，含 ascend 后端） | https://gitee.com/flagos-ai/flagtree | main（2025-06-04 ascend 接入） | 2026-09-11 | Agent05：昇腾 Triton 生态另一入口 | B/C |
| 81 | 昇腾 Triton 算子开发初体验（编译链路实操） | https://www.hiascend.com/developer/blog/details/0269202487813540173 | 2026-01-06 | 2026-09-11 | Agent05："Triton IR→AscendNPU IR" 链路社区佐证 | C |
| 82 | SITS2026：昇腾 NPU 后端算子覆盖（ACL+AclGraph） | https://blog.csdn.net/PoliVein/article/details/160025359 | 2026-04-10 | 2026-09-11 | Agent05：背景——昇腾后端经 ACL/AclGraph 而非开放编译器 | C |
| 83 | CANNJudge 题目 API（AddRmsNormBias） | https://cannjudge.cn/api/problems/6a9a9a99bf41025d6013eb85 | 2026-09-11 抓取 | 2026-09-11 | 题面 desc 全文（含**得分规则原文**：15 测试点、单点公式、均值排名）、testcases 15 条、iterations=5、score_mode=1、code_template=npu_kernel_dev、use_baseline=false | A |
| 84 | CANNJudge 提交结果 API（示例提交） | https://cannjudge.cn/api/submissions/6aa202b62d3dd2c5aef620a8 | 2026-09-11 抓取 | 2026-09-11 | 每点 time(μs)/precision_ratio/status/best_time；提交文件字段 tiling_h/tiling_key_h/tiling_key_cpp/host_cpp/kernel_cpp + files 数组（未登录无权限）；时间单位 μs 确认 | A |
| 85 | CANNJudge AddRmsNormBias 排名页 | https://cannjudge.cn/public/op_challenge_xinan_prelim/addrmsnormbias/ranking | 2026-09-11 抓取 | 2026-09-11 | 15 个测试点列、每点独立时间、分数列、TBest 行（全局最优）；得分公式实测验证 78.79↔78.78 | A |
| 86 | CANNJudge 比赛列表页 | https://cannjudge.cn/contests | 2026-09-11 抓取 | 2026-09-11 | 各赛区报名/提交/时间统计；西南赛区 344 人报名、12065 次提交 | A |
| 87 | CANNJudge 西南赛区详情页 | https://cannjudge.cn/public/op_challenge_xinan_prelim | 2026-09-11 抓取 | 2026-09-11 | 赛区信息、题目列表、GitCode 报名链接（competition.gitcode.com/competition/2094722165106008066） | A |
| 88 | CANNJudge 杭厦赛区详情页 | https://cannjudge.cn/public/op_challenge_hangxia_prelim | 2026-09-11 抓取 | 2026-09-11 | 对照：杭厦赛区题目为 QuantMatmulReluQuant（cube），证明各赛区题目不同 | A |
| 89 | cannbot-skills npu-arch skill（官方仓） | https://gitcode.com/cann/cannbot-skills（CSDN 转述 https://blog.csdn.net/gitblog_01165/article/details/151219465） | master，2026-06 | 2026-09-11 | DAV_2201 ↔ Ascend910B1~B4/910B2C/910_93，NPU_ARCH=2201；UB 192KB（910B 系） | B |
| 90 | cannjudge-submit SKILL.md | https://gitcode.com/cann/cann-learning-hub/blob/master/skills/cannjudge-submit/SKILL.md | master | 2026-09-11 | 提交流程、API、泛化设计要点；**"平台不开放测试用例 API"** | B |
| 91 | cannjudge_cli.py 源码 | https://raw.gitcode.com/cann/cann-learning-hub/raw/master/skills/cannjudge-submit/cannjudge_cli.py | master | 2026-09-11 | 提交字段 tiling_h/tiling_key_h/host_cpp/kernel_cpp；模板结构 op_host/op_kernel（旧式算子工程，与本题直调模板不同） | B |
| 92 | CSDN：2026 CANN 挑战赛报名启动 | https://blog.csdn.net/csdn_codechina/article/details/164369653 | 2026-09-04 | 2026-09-11 | 赛制三阶段时间表（初赛 9/5-10/17、决赛 10/20-11/7）、奖金（区域一等奖 2 万）、晋级机制概述 | C |
| 93 | CSDN：CANN学习中心 as_strided 算子实战 | https://blog.csdn.net/gitblog_01418/article/details/150380401 | 2026-05-20 | 2026-09-11 | 旁证：erf 初赛 15/15 PASS；CANNJudge 提交 4 文件（旧格式）；910B UB 192KB | C |
| 94 | CSDN：CANN 竞赛作品提交规范 | https://blog.csdn.net/gitblog_07213/article/details/151463322 | 2026-05-19 | 2026-09-11 | 提交流程：下载工程→代码编辑页→提交→查排名 | C |
| 95 | GitCode 2026CANN挑战赛西南赛区报名页 | https://competition.gitcode.com/competition/2094722165106008066/intro | — | 2026-09-11 | 页面可访问但**无赛事描述/时间线内容**（空页） | A(空) |
| 96 | CANNJudge 模板下载 API（尝试） | https://cannjudge.cn/api/problems/6a9a9a99bf41025d6013eb85/package | — | 2026-09-11 | 未授权访问失败（返回失败/需登录），本地模板由用户提供 | A(访问受限) |
| 97 | PyTorch layer_norm_kernel.cu（RowwiseMoments/vectorized kernel） | https://github.com/pytorch/pytorch/blob/main/aten/src/ATen/native/cuda/layer_norm_kernel.cu | main，2026-09-11 抓取 | 2026-09-11 | GPU 两遍 Welford、warp shuffle+smem 归约、rms_norm 分支、FP32 累加（Agent 4） | A（源码已读取，未运行） |
| 98 | vLLM layernorm_kernels.cu（rms_norm_kernel / fused_add_rms_norm_kernel） | https://github.com/vllm-project/vllm/blob/main/csrc/libtorch_stable/layernorm_kernels.cu | main，2026-09-11 抓取 | 2026-09-11 | residual 融合 RMSNorm、in-place 存 y、vec8 向量化、batch-invariant 归约顺序（Agent 4） | A（源码已读取，未运行） |
| 99 | flash-attention csrc/layer_norm/ln_fwd_kernels.cuh（ln_fwd_kernel） | https://github.com/Dao-AILab/flash-attention/blob/main/csrc/layer_norm/ln_fwd_kernels.cuh | main，2026-09-11 抓取 | 2026-09-11 | 编译期特化矩阵、save_x 中间 buffer、跨 CTA 协作、persistent grid-stride（Agent 4） | A（源码已读取，未运行） |
| 100 | flash-attention flash_attn/ops/triton/layer_norm.py（_layer_norm_fwd_1pass_kernel） | https://github.com/Dao-AILab/flash-attention/blob/main/flash_attn/ops/triton/layer_norm.py | main，2026-09-11 抓取 | 2026-09-11 | Triton 单遍存 x、residual+RMS+bias 融合、mask+other=0 尾块（Agent 4） | A（源码已读取，未运行） |
| 101 | Triton 官方教程 05-layer-norm | https://github.com/triton-lang/triton/blob/main/python/tutorials/05-layer-norm.py | main，2026-09-11 抓取 | 2026-09-11 | tl.sum 块内归约、mask 尾块、真实 N 除法、atomic 反向参考（Agent 4） | B（官方教程，未运行） |
| 102 | llama.cpp ggml/src/ggml-cuda/norm.cu（rms_norm_f32） | https://github.com/ggml-org/llama.cpp/blob/master/ggml/src/ggml-cuda/norm.cu | master，2026-09-11 抓取 | 2026-09-11 | 最小两遍模板、block_reduce、mul/add 融合、4D grid 映射（Agent 4） | A（源码已读取，未运行） |
| 103 | NVIDIA Apex layer_norm | https://github.com/NVIDIA/apex/tree/master/csrc/layer_norm | master，未下载 | 2026-09-11 | 与 PyTorch 同源（Welford+两遍），仅对照未深入（Agent 4） | A（未读取正文） |
| 104 | Unsloth kernels/utils.py | https://github.com/unslothai/unsloth/blob/main/unsloth/kernels/utils.py | main，2026-09-11 抓取 | 2026-09-11 | 检索后确认不含 rmsnorm，未采用（Agent 4） | A（已检索，未采用） |

## 2026-09-11 多代理方案调研新增来源（第二轮，10 代理）

> 本轮新增 119 条，分组编号保留代理内 ID（`AgentNN-序号`），完整表格与去重说明见 `AgentNN-*.md`（Agent10 报告 §1 为跨报告去重清单）。证据等级 A=官方文档/官方仓库原文；B=官方样例/源码/官方权威培训；C=社区。全部来源**未在真实 NPU 验证**。

### Agent02（官方 Ascend C API，34 条）

| ID | 名称 | URL / 仓库 | 访问日期 | 用途要点 | 等级 |
| --- | --- | --- | --- | --- | --- |
| 02-1 | CANN 9.0.X 社区版文档首页 | https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/900/index/index.html | 2026-09-11 | 9.0 文档入口确认 | A |
| 02-2 | CANN 9.0.X Ascend C API 手册目录（DataCopyPad(ISASI)/Cast/ReduceSum 章节） | https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/900/API/ascendcopapi/atlasascendc_api_07_0003.html | 2026-09-11 | API 结构与 ISASI 标注 | A |
| 02-3 | DataCopyPad（社区版 8.0.0 alpha001 手册） | https://www.hiascend.com/doc_center/source/zh/CANNCommunityEdition/800alpha001/apiref/ascendcopapi/atlasascendc_api_07_0253.html | 2026-09-11 | 字段/对齐/型号矩阵（与 #4 同页） | A |
| 02-4 | DataCopyPad GM→UB（9.x master） | https://asc.gitcode.com/api/SIMD-API/basic_api/memory_vector_compute/data_move/DataCopyPad_GMToUB.html | 2026-09-11 | 9.x 字段/产品/dtype/mode/NOP | A |
| 02-5 | DataCopyPad UB→GM（9.x master） | https://asc.gitcode.com/api/SIMD-API/basic_api/memory_vector_compute/data_move/DataCopyPad_UBToGM.html | 2026-09-11 | 搬出语义与约束 | A |
| 02-6 | ReduceSum（商用 8.0 手册） | https://www.hiascend.cn/document/detail/zh/canncommercial/800/apiref/ascendcopapi/atlasascendc_api_07_0078.html | 2026-09-11 | workLocal 公式/累加方式（与 #7 同页） | A |
| 02-7 | ReduceSum（社区 8.0.RC3.alpha001 手册） | https://www.hiascend.cn/document/detail/zh/CANNCommunityEdition/80RC3alpha001/apiref/opdevgapi/atlasascendc_api_07_0099.html | 2026-09-11 | 交叉确认 | A |
| 02-8 | ReduceSum（asc-devkit 仓库文档转述） | https://blog.csdn.net/gitblog_00089/article/details/151635178 | 2026-09-11 | 9.x 产品矩阵（950/A3/Kirin） | B |
| 02-9 | Cast（9.x master） | https://asc.gitcode.com/api/SIMD-API/basic_api/memory_vector_compute/type_conversion/Cast.html | 2026-09-11 | RoundMode 与各产品 dtype 组合 | A |
| 02-10 | DataCopy GM↔UB（9.x master） | https://asc.gitcode.com/api/SIMD-API/basic_api/memory_vector_compute/data_move/DataCopy_GMAndUB_continuous.html | 2026-09-11 | count 对齐约束 | A |
| 02-11 | Add（9.x master） | https://asc.gitcode.com/api/SIMD-API/basic_api/memory_vector_compute/basic_arithmetic/Add.html | 2026-09-11 | A2 dtype（无 bf16） | A |
| 02-12 | TPipe::InitBuffer（9.x master） | https://asc.gitcode.com/api/SIMD-API/basic_api/resource_management/TPipe/InitBuffer.html | 2026-09-11 | InitBuffer 语义、≤64 buffer | A |
| 02-13 | TPipe-TQue 编程原理（9.x guide） | https://asc.gitcode.com/guide/programming_guide/programming_model/ai_core_simd_programming/tpipe_tque_programming/tpipe_tque_principles.html | 2026-09-11 | EnQue/DeQue/FreeTensor 同步语义 | A |
| 02-14 | 核函数定义与调用（9.x guide） | https://asc.gitcode.com/guide/programming_guide/programming_model/ai_core_simd_programming/kernel_function.html | 2026-09-11 | `__global__ __vector__`、`<<<numBlocks,dynUBufSize,stream>>>` | A |
| 02-15 | 核函数（HarmonyOS CANN Kit 镜像） | https://developer.huawei.com/consumer/cn/doc/harmonyos-guides/cannkit-kernel-function | 2026-09-11 | 核函数定义交叉确认 | A |
| 02-16 | GetBlockIdx / GetBlockNum（9.x） | https://asc.gitcode.com/api/SIMD-API/basic_api/tool_interface/system_resources_and_variables/GetBlockIdx.html | 2026-09-11 | 签名与语义 | A |
| 02-17 | PipeBarrier(ISASI)（9.x） | https://asc.gitcode.com/api/SIMD-API/basic_api/sync_control/intra_core_sync/PipeBarrier_ISASI.html | 2026-09-11 | 不支持 PIPE_S、自动同步 | A |
| 02-18 | SetFlag/WaitFlag（8.0.0 alpha001） | https://www.hiascend.com/doc_center/source/zh/CANNCommunityEdition/800alpha001/apiref/ascendcopapi/atlasascendc_api_07_0257.html | 2026-09-11 | HardEvent 枚举、eventID A2 0-7 | A |
| 02-19 | 无 DataCopyPad 的处理方式 | https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/80RC3alpha002/devguide/opdevg/ascendcopdevg/atlas_ascendc_10_0037.html | 2026-09-11 | Duplicate/mask/GatherMask/UnPad/SetAtomicAdd 降级（与 #6 同页） | A |
| 02-20 | Vector 逻辑架构（UB 大小，9.x） | https://asc.gitcode.com/api/SIMD-API/基础API/Memory矢量计算/Vector逻辑架构/Vector逻辑架构.html | 2026-09-11 | A2/A3 每核 UB=192KB | A |
| 02-21 | 低延迟指令优化归约（9.x guide） | https://asc.gitcode.com/guide/算子实践参考/SIMD算子性能优化/矢量计算/选择低延迟指令-优化归约操作性能.html | 2026-09-11 | ReduceSum 性能定位与二分累加 | A |
| 02-22 | API 流水类型汇总（9.x） | https://asc.gitcode.com/api/appendix/api_pipeline_type_summary.html | 2026-09-11 | ReduceSum/Cast/DataCopyPad 流水类型 | A |
| 02-23 | 昇腾 CANN 9.0.1 版本发布公告 | https://www.hiascend.com/productbulletins/detail/804 | 2026-09-11 | aclnnAddRmsNorm 拆分、废弃接口 | A |
| 02-24 | 昇腾 CANN 9.1.0 版本发布公告 | https://www.hiascend.com/productbulletins/detail/806 | 2026-09-11 | 商用/社区归一化 | A |
| 02-25 | CANN 9.1.0 社区版下载页（910b-ops ↔ Atlas A2） | https://www.hiascend.com/en/software/cann/community/ | 2026-09-11 | 包与产品对应关系 | A |
| 02-26 | CANN 8.3.RC1 Ascend C 算子开发指南 PDF（Kernel 直调） | https://www.hiascend.com/doc_center/source/zh/CANNCommunityEdition/83RC1alpha001/opdevg/Ascendcopdevg/CANN社区版%208.3.RC1.alpha001%20Ascend%20C算子开发指南%2001.pdf | 2026-09-11 | Kernel 直调工程章节 | A |
| 02-27 | CANN 9.0.0 商用版文档下载页 | https://www.hiascend.com/document/detail/zh/canncommercial/download | 2026-09-11 | 9.0 手册 PDF 入口 | A |
| 02-28 | asc-devkit DataCopy 内存访问最佳实践 | https://blog.csdn.net/gitblog_00924/article/details/157925888 | 2026-09-11 | 搬运性能/非对齐佐证 | B |
| 02-29 | CANNBot Ascend C 直调开发指南 | https://blog.csdn.net/gitblog_00909/article/details/151507556 | 2026-09-11 | 直调工程规范 | C |
| 02-30 | SuperKernel 核函数直调额外适配说明 | https://asc.gitcode.com/guide/编程指南/高级编程/SuperKernel/核函数直调算子额外适配说明.html | 2026-09-11 | 普通 Kernel 直调 = `__vector__` 佐证 | A |
| 02-31 | 昇腾 910 系列 UB 容量社区数据 | https://arxiv.org/pdf/2607.20120 ; https://github.com/mov20/ascend-dsl-research/blob/main/pyasc2-design.md | 2026-09-11 | UB 容量矛盾记录 | C |
| 02-32 | 内置数据类型 bfloat16_t（9.x） | https://asc.gitcode.com/api/SIMT-API/SIMD与SIMT混合编程简介/扩展语法/内置数据类型-144.html | 2026-09-11 | bfloat16_t 位宽定义 | A |
| 02-33 | asc_atomic_add（SIMT，9.x） | https://asc.gitcode.com/api/SIMT-API/原子操作/asc_atomic_add.html | 2026-09-11 | SIMT 原子加仅 950 支持 | A |
| 02-34 | HIVM 方言内存优化博客（910B UB 1.5MB 说法） | https://www.hiascend.com/developer/blog/details/02134205828844777045 | 2026-09-11 | C 级矛盾记录 | C |

### Agent03（官方昇腾开源仓库，5 条）

| ID | 名称 | URL / 仓库 | 访问日期 | 用途要点 | 等级 |
| --- | --- | --- | --- | --- | --- |
| 03-1 | ops-nn（CANN 神经网络算子库） | https://gitcode.com/cann/ops-nn（master `9b594837`，本地 /tmp/ops-nn.tmp） | 2026-09-11 | add_rms_norm / rms_norm / add_rms_norm_quant(_v2) 源码（公式与本题完全一致） | A（已读源码） |
| 03-2 | ops-transformer（MC2/MoE 组合算子库） | https://gitcode.com/cann/ops-transformer（master `e7019c29`，本地 /tmp/ops-transformer.eKIcVc） | 2026-09-11 | matmul_all_reduce_add_rms_norm 5 变体 + Host tiling | A（已读源码） |
| 03-3 | cann-samples（算子性能实战样例） | https://gitcode.com/cann/cann-samples（master `23c981c0`，本地 /tmp/cann-samples.vc2a1d） | 2026-09-11 | rms_norm_quant_story 7 步优化 | A/B（已读源码） |
| 03-4 | cann-learning-hub（GitHub 镜像，qwen_ops/01_rmsnorm_baseline 训练营） | https://github.com/hicann/cann-learning-hub（master） | 2026-09-11 | Direct Invocation 模板 + 构建脚本 | B（partial） |
| 03-5 | ascend950_rmsnormquant_optimization 博客 | https://github.com/hicann/cann-learning-hub/blob/master/blogs/operator/ascend950_rmsnormquant_optimization | 2026-09-11 | RmsNormQuant 优化方法（与 cann-samples Story 同源） | B（partial） |

### Agent06（数值精度与验证，11 条）

| ID | 名称 | URL / 仓库 | 访问日期 | 用途要点 | 等级 |
| --- | --- | --- | --- | --- | --- |
| 06-1 | CANN Kit 精度转换指令（Cast/CAST_RINT/CAST_ROUND 舍入规则） | https://device.harmonyos.com/cn/docs/apiref/harmonyos-guides/cannkit-precision-conversion-instruction | 2026-09-11 | CAST_RINT=RNE 确认 | A |
| 06-2 | vec_conv 精度转换（商用 8.0.RC2） | https://www.hiascend.com/doc_center/source/zh/canncommercial/80RC2/apiref/opdevgapi/atlastikapi_07_0083.html | 2026-09-11 | f32→f16 舍入与饱和语义 | A |
| 06-3 | asc-devkit Sqrt API（0 ulp） | https://gitcode.com/cann/asc-devkit/docs/api/reg/Sqrt（CSDN 转述） | 2026-09-11 | 向量 Sqrt 精度 0 ulp | B |
| 06-4 | CANN InplaceRsqrt 算子设计文档 | https://gitcode.com/cann/cann-ops-competitions（CSDN 转述） | 2026-09-11 | 官方 rsqrt = vsqrt+vdiv 实现 | B |
| 06-5 | tilelang-ascend issue #1225（rsqrt 精度不足 rtol=1e-3 失配） | https://github.com/tile-ai/tilelang-ascend/issues/1225 | 2026-09-11 | 裸硬件 rsqrt 精度实证反例 | C |
| 06-6 | AscendOpGenAgent 知识库 PR（A2/CANN 8.5 Cast 支持矩阵） | https://github.com/Just-it/AscendOpGenAgent/pull/146 | 2026-09-11 | fp32→half/bf16 Cast 组合实测 | C |
| 06-7 | ops-transformer RMSNorm 数值精度分析 | https://hwcomputing.csdn.net/6a156b8510ee7a33f2754f4b.html | 2026-09-11 | FP16 平方和溢出/FP32 累加器/Kahan | C |
| 06-8 | 昇腾 AI Core rsqrtf() 硬件指令说明 | https://ascendai.csdn.net/693ad54a0800f3458b818b10.html | 2026-09-11 | rsqrtf 延迟、FP32 中间累加 | C |
| 06-9 | numpy 文档（sum pairwise / cumsum 顺序语义） | https://numpy.org/doc/stable/ | 2026-09-11 | 归约误差模拟依据 | A（CPU 语义） |
| 06-10 | 模板判题与 golden 实现 | /Users/sunyiyang/Downloads/addrmsnormbias_problem_1742_template/scripts/{AddRmsNormBias,verify_result}.py | 2026-09-11 | 判题语义（isclose/tol/equal_nan）与 golden 计算链 | A（判题参考实现） |
| 06-11 | 浮点误差累积理论（顺序/树形/Kahan） | 高德纳 TAOCP Vol.2 / IEEE 754 | 2026-09-11 | 大 D 归约误差界推导 | C（教科书） |

### Agent07（性能/UB/硬件，20 条）

| ID | 名称 | URL / 仓库 | 访问日期 | 用途要点 | 等级 |
| --- | --- | --- | --- | --- | --- |
| 07-1 | asc-devkit Add 性能调优样例（UB 192KB、7 步优化） | https://blog.csdn.net/gitblog_00754/article/details/157048953 | 2026-09-11 | UB 容量/分块/双缓冲/L2 直通 | B |
| 07-2 | asc-devkit DataCopy 内存访问最佳实践 | https://blog.csdn.net/gitblog_00924/article/details/157925888 | 2026-09-11 | 非对齐代价 21.6%/47.5%、512B 对齐 | B |
| 07-3 | SIMD-API Vector 指令理论性能汇总 | https://asc.gitcode.com/api/附录/Vector指令理论性能汇总.html | 2026-09-11 | Add fp32=64、Rsqrt=64、Sqrt=32/cycle | A |
| 07-4 | 选择低延迟指令，优化归约操作性能 | https://asc.gitcode.com/guide/算子实践参考/SIMD算子性能优化/矢量计算/选择低延迟指令-优化归约操作性能.html | 2026-09-11 | 二分累加 172 vs ReduceRepeat 242 cycles | A |
| 07-5 | asc-devkit ReduceSum 归约求和 API | https://blog.csdn.net/gitblog_00089/article/details/151635178 | 2026-09-11 | ReduceSum 语义/参数 | B |
| 07-6 | Ascend C 算子性能优化技巧 05（API 使用优化） | https://www.hiascend.com/developer/techArticles/20241107-1 | 2026-09-11 | scalar 类外化 -17%、TQueBind | A |
| 07-7 | CANN 学习中心 as_strided 实战（GetValue/SetValue 性能黑洞） | https://blog.csdn.net/gitblog_01418/article/details/150380401 | 2026-09-11 | 标量同步代价定性 | C |
| 07-8 | CANN 训练营双缓冲流水线实战（10.2ms→4.1ms） | https://blog.csdn.net/aasd23/article/details/154999873 | 2026-09-11 | 双缓冲收益参考 | C |
| 07-9 | Ascend C 中的"流水线"艺术 | https://blog.csdn.net/aasd23/article/details/154999822 | 2026-09-11 | 双缓冲原理/收益 | C |
| 07-10 | ASCEND TO SCIENCE（arXiv 2607.20120） | https://arxiv.org/pdf/2607.20120 | 2026-09-11 | 910A/B/C UB/L2/HBM 带宽反推 | C |
| 07-11 | Atlas 800T A2 服务器官方彩页 | https://www.hiascend.com/hardware/ai-server?tag=900A2 | 2026-09-11 | 单芯片 HBM 带宽 1.6TB/s | A |
| 07-12 | msOpST 生成/执行测试用例 | https://www.hiascend.cn/document/detail/zh/CANNCommunityEdition/800alpha002/devaids/opdev/optool/atlasopdev_16_0033.html | 2026-09-11 | 单算子 ST+性能测量 | A |
| 07-13 | 算子调试 msProf 及仿真 | https://bbs.huaweicloud.com/blogs/5a9291facf6e43148d16ef69a63dc97c | 2026-09-11 | msprof op 上板方法 | B |
| 07-14 | 算子开发工具指南（msKPP 建模） | https://www.hiascend.com/doc_center/source/zh/CANNCommunityEdition/800alpha001/devaids/opdev/optool/CANN%208.0.0%20alpha001%20算子开发工具指南%2001.pdf | 2026-09-11 | msKPP 用法 | A |
| 07-15 | asc.gitcode.com 性能调优（msOpProf） | https://asc.gitcode.com/guide/编程指南/调试调优/性能调优.html | 2026-09-11 | msopprof 上板/仿真 | A |
| 07-16 | Ascend/mskpp 官方仓库 | https://github.com/Ascend/mskpp | 2026-09-11 | msKPP 入口 | A/B |
| 07-17 | 昇腾 910B 系列四款型号对比 | https://jishuzhan.net/article/2069227057103597570 | 2026-09-11 | 910B B1~B4 谱系（带宽口径未采信） | C |
| 07-18 | GLM-5.2-W8A8 部署报告（Atlas 800I A2 = 8×910B2） | https://firsh.me/blog/0184/attachments/glm52_w8a8_report.pdf | 2026-09-11 | 800I A2 硬件构成佐证 | C |
| 07-19 | GPU 学习笔记：A100 vs 910B 分析 | https://blog.csdn.net/qq_40214669/article/details/143271235 | 2026-09-11 | 核数/微架构社区口径 | C |
| 07-20 | 华为昇腾 910B（helix-peak） | https://kb.helix-peak.com/techentry/huawei-ascend-910b.html | 2026-09-11 | 核数另一社区口径（矛盾记录） | C |

### Agent08（Linux/环境/真机工程，28 条）

| ID | 名称 | URL / 仓库 | 访问日期 | 用途要点 | 等级 |
| --- | --- | --- | --- | --- | --- |
| 08-1 | Linux DO：内网ARM服务器部署Qwen3.5 122B（North_warm，2026-04-06） | https://linux.do/t/topic/1904864 | 2026-09-11 | 8×910B4、驱动25.2.3、docker 设备挂载 | C |
| 08-2 | Linux DO：昇腾910B本地部署DeepSeek-V4-Flash（tan90，2026-04-24） | https://linux.do/t/topic/2047891 | 2026-09-11 | 910B 部署/量化环境 | C |
| 08-3 | Linux DO：【cann】昇腾算子怎么这么难？（qwx，2026-06-16） | https://linux.do/t/topic/2418153 | 2026-09-11 | QuantBatchMatmulV3 竞赛 case 排障 | C |
| 08-4 | Linux DO：想咨询华为昇腾搭建算力中心（duelist，2026-06-23） | https://linux.do/t/topic/2461391 | 2026-09-11 | 昇腾集群选型 | C |
| 08-5 | V2EX：Atlas 300I Duo 驱动安装难 | https://www.v2ex.com/t/1176018 | 2026-09-11 | 驱动/固定内核 | C |
| 08-6 | V2EX：欧拉 host 昇腾只能 docker 跑 Ubuntu | https://www.v2ex.com/t/1087530 | 2026-09-11 | 容器化运行昇腾 | C |
| 08-7 | V2EX：昇腾相关讨论（一） | https://www.v2ex.com/t/1207741 | 2026-09-11 | 环境讨论 | C |
| 08-8 | V2EX：昇腾相关讨论（二） | https://www.v2ex.com/t/1208225 | 2026-09-11 | 环境讨论 | C |
| 08-9 | 昇腾社区《安装CANN软件包》 | https://www.hiascend.com/document/detail/zh/canncommercial/800/softwareinst/instg/instg_0008.html | 2026-09-11 | 安装步骤 | A |
| 08-10 | CANN 8.0.RC2.2《软件安装指南》PDF | https://www.hiascend.com/doc_center/source/zh/canncommercial/80RC22/softwareinst/instg/CANN%208.0.RC2.2%20%E8%BD%AF%E4%BB%B6%E5%AE%89%E8%A3%85%E6%8C%87%E5%8D%97%2001.pdf | 2026-09-11 | 驱动固件/容器/故障处理 | A |
| 08-11 | CANN 8.5.0.alpha002《软件安装指南》 | https://www.hiascend.com/doc_center/source/zh/CANNCommunityEdition/850alpha002/softwareinst/instg/ | 2026-09-11 | 社区版安装 | A |
| 08-12 | 官方《安装NPU驱动固件》 | https://www.hiascend.com/doc_center/source/zh/CANNCommunityEdition/800alpha001/softwareinst/instg/instg_0005.html | 2026-09-11 | 首次驱动>固件、覆盖固件>驱动 | A |
| 08-13 | asc.gitcode.com《AI Core算子编译基本用法》 | https://asc.gitcode.com/guide/编程指南/编译与运行/算子编译/AI-Core算子编译基本用法.html | 2026-09-11 | bisheng/编译参数 | B |
| 08-14 | asc.gitcode.com《AI CPU算子编译》 | https://asc.gitcode.com/guide/编程指南/编译与运行/算子编译/AI-CPU算子编译基本用法.html | 2026-09-11 | AI CPU 编译 | B |
| 08-15 | asc.gitcode.com《SIMD BuiltIn关键字》（__NPU_ARCH__） | https://asc.gitcode.com/guide/编程指南/语言扩展层/SIMD-BuiltIn关键字.html | 2026-09-11 | __NPU_ARCH__ 说明 | B |
| 08-16 | hiascend.com Ascend C 主页（.asc/<<<>>>直调） | https://www.hiascend.com/cann/ascend-c | 2026-09-11 | 直调模式入口 | A |
| 08-17 | CANN release-management（9.0.0 ↔ Ascend HDK 配套表） | https://gitcode.com/cann/release-management | 2026-09-11 | HDK 26.0.RC1/25.5.2/25.5.1 配套 | B |
| 08-18 | ascendhub CANN 镜像详情 | https://www.hiascend.com/developer/ascendhub/detail/17da20d1c2b6493cb38765adeba85884 | 2026-09-11 | 镜像 tag 规范 | A/B |
| 08-19 | CANN 官方 msopgen/msopst 文档 | https://www.hiascend.com/document/detail/zh/canncommercial/80RC2/devaids/auxiliarydevtool/atlasopdev_16_0027.html | 2026-09-11 | 工具用法 | A |
| 08-20 | CANN《算子工程参考示例》 | https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/80RC2alpha003/devaids/auxiliarydevtool/atlasopdev_16_0117.html | 2026-09-11 | ASCEND_CANN_PACKAGE_PATH 配置 | A |
| 08-21 | CANN 故障案例（找不到头文件/库） | https://www.hiascend.cn/document/detail/zh/CANNCommunityEdition/80RC3alpha001/devguide/maintenref/troubleshooting/troubleshooting_0216.html | 2026-09-11 | DDK_PATH/NPU_HOST_LIB 作用 | A |
| 08-22 | 昇腾官方论坛 S2 FAQ（507015/507035/errorStr） | https://www.hiascend.com/forum/thread-02100159784900000007-1-1.html | 2026-09-11 | UB 越界/对齐错误码 | A/B |
| 08-23 | 昇腾官方博客（ACL stream synchronize failed 排查） | https://www.hiascend.com/developer/blog/details/0272186082476802015 | 2026-09-11 | fault kernel_name 排查 | B |
| 08-24 | 昇腾官方博客（bisheng 不支持 --npu-arch 旧版本） | https://www.hiascend.com/developer/blog/details/0289201670005562056 | 2026-09-11 | 8.3.RC1 起支持；910B=2201 | A/B |
| 08-25 | cannbot-skills 官方配置指南 | https://gitcode.com/cann/cannbot-skills | 2026-09-11 | 环境变量/561107 错误 | B |
| 08-26 | CSDN 昇腾910B 初始化准备 | https://blog.csdn.net/qq_39146974/article/details/155023733 | 2026-09-11 | 驱动/固件/CANN 安装卸载 | C |
| 08-27 | CSDN CANN 环境版本对应关系 | https://blog.csdn.net/m0_52182894/article/details/156616648 | 2026-09-11 | HDK↔CANN↔torch_npu 配套 | C |
| 08-28 | CSDN 搭建 CANN 开发环境避坑 | https://blog.csdn.net/qq_41397792/article/details/153776541 | 2026-09-11 | libascendcl/import acl/npu-smi 报错 | C |
| 08-29 | CSDN CANN 9.0 + ops-cv 全流程实战 | https://blog.csdn.net/weixin_52908342/article/details/159955428 | 2026-09-11 | 9.0.0 下载/卸载/--install-path | C |
| 08-30 | CSDN 零基础搭建昇腾NPU环境（CANNLab/Docker） | https://blog.csdn.net/gitblog_00174/article/details/156175508 | 2026-09-11 | ascendhub 镜像与挂载 | C |
| 08-31 | OpenI 实操：昇腾NPU算子开发 | https://blog.csdn.net/qq_41823532/article/details/158773506 | 2026-09-11 | bisheng 旧式编译示例 | C |
| 08-32 | 昇腾FAQ-A01 硬件相关 | https://blog.csdn.net/jieph01/article/details/149277585 | 2026-09-11 | npu-smi dcmi 错误/HwHiAiUser | C |
| 08-33 | 华为云博客 CANN 算子开发一（--npu-arch 对应表） | https://bbs.huaweicloud.cn/blogs/475681 | 2026-09-11 | A2/A3=2201、200I/500A2=3002 | C |
| 08-34 | FlagTree User manual for ascend | https://github.com/flagos-ai/FlagTree/wiki/User-manual-for-ascend | 2026-09-11 | CANN 9.0.0 安装命令 | C |
| 08-35 | asc-devkit（Ascend C 语言核心仓） | https://github.com/hicann/asc-devkit | 2026-09-11 | CMake 编译指南与链接库表 | B |
| 08-36 | MindSpore《CANN常见错误分析》 | https://www.mindspore.cn/tutorials/zh-CN/stable/debug/error_analysis/cann_error_cases.html | 2026-09-11 | E80000 等编译错误码 | B |

### Agent09（竞赛经验与失败案例，21 条）

| ID | 名称 | URL / 仓库 | 访问日期 | 用途要点 | 等级 |
| --- | --- | --- | --- | --- | --- |
| 09-1 | CANN学习中心 as_strided 实战（CE：8.5 vs 9.0 命名空间） | https://blog.csdn.net/gitblog_01418/article/details/150380401 | 2026-09-11 | 编译错误案例 | C |
| 09-2 | 昇腾算子挑战赛 S8 Scale 优化分享（中间 cast 回 bf16 才匹配 golden） | https://developer.huawei.com/home/forum/ascend/thread-02200223658291993472-1-1.html | 2026-09-11 | WA/精度案例 | C |
| 09-3 | XJTUACM S7 经验总结（BF16 CAST_RINT；ReduceSum≤4096） | https://www.hiascend.com/developer/blog/details/02127205558228788013 | 2026-09-11 | 精度/ReduceSum 案例 | B |
| 09-4 | ShwStone/Ascend-S7（S7 源码） | https://github.com/ShwStone/Ascend-S7 | 2026-09-11 | 09-3 佐证 | B |
| 09-5 | DataCopyPad 写出溢出 Bug（burst 32B 覆盖相邻数据） | https://blog.csdn.net/ferriswym/article/details/162206787 | 2026-09-11 | 尾块越界案例 | C |
| 09-6 | tilelang-ascend Issue #890（RMSNorm N=513 尾块错误） | https://github.com/tile-ai/tilelang-ascend/issues/890 | 2026-09-11 | 尾块案例 | C |
| 09-7 | Ascend C 算子开发常见报错解析与精度复盘 | https://blog.csdn.net/2301_80840905/article/details/155034892 | 2026-09-11 | 对齐/死锁/FP16 案例 | C |
| 09-8 | CANN训练营·黑客篇：BlackBox 与 Exception Dump | https://blog.csdn.net/2401_82857325/article/details/155754900 | 2026-09-11 | 栈溢出/踩内存案例 | C |
| 09-9 | Ascend C 核异常与同步机制失效深度排查 | https://blog.csdn.net/weixin_27442001/article/details/159974663 | 2026-09-11 | WaitFlag/超时案例 | C |
| 09-10 | Pdist 自定义算子开发复盘（错误统计/混合精度） | https://hwcomputing.csdn.net/69b6a2570a2f6a37c5979df8.html | 2026-09-11 | 超时统计/精度案例 | C |
| 09-11 | CANNJudge 算子提交 Skill（提交字段/判题状态） | https://gitcode.com/cann/cann-learning-hub/blob/master/skills/cannjudge-submit/README.md（转述 https://blog.csdn.net/gitblog_00410/article/details/143789539） | 2026-09-11 | 上传包/隐藏测试点 | B |
| 09-12 | CANNBot Ascend C 直调开发指南（.asc/__vector__/禁止写死） | https://blog.csdn.net/gitblog_00909/article/details/151507556 | 2026-09-11 | 直调模板规范 | B |
| 09-13 | 错误码 0x10 定位案例（官方） | https://www.hiascend.com/doc_center/source/zh/canncommercial/601/devtools/auxiliarydevtool/atlasaicoreerr_16_0010.html | 2026-09-11 | UB 越界/非法指令 | A |
| 09-14 | npucheck 功能（ErrorRead1-4 内存检测） | https://www.hiascend.com/doc_center/source/zh/CANNCommunityEdition/80RC3alpha003/devaids/auxiliarydevtool/atlasascendebug_16_0133.html | 2026-09-11 | 对齐错误码 | A |
| 09-15 | CANN 与 MindSpore 版本不匹配导致算子编译失败 | https://ask.csdn.net/questions/9266350 | 2026-09-11 | msopgen 编译错误 | C |
| 09-16 | 2025昇腾训练营避坑指南（undefined reference to aclInit） | https://blog.csdn.net/pz890123/article/details/152697389 | 2026-09-11 | 链接/环境错误 | C |
| 09-17 | MindSpore AOT 自定义算子文档（`No rule to make target 'package'`） | https://www.mindspore.cn/docs/zh-CN/r2.4.0/model_train/custom_program/operation/op_custom_ascendc.html | 2026-09-11 | 编译路径错误 | B |
| 09-18 | Atlas 900 A3 AICPU 占满导致执行卡死 | https://www.hiascend.com/developer/techArticles/20260603-10?envFlag=1 | 2026-09-11 | 死锁排查 | A |
| 09-19 | 昇腾AI创新大赛-算子挑战赛官方页（S9 规则） | https://www.hiascend.com/developer/ops | 2026-09-11 | 赛季规则 | A |
| 09-20 | 开发工具链介绍：从编译到调试（__aicore__ 语法错误） | https://cann.csdn.net/69ef0b9b54b52172bc704775.html | 2026-09-11 | 语法错误案例 | C |
| 09-21 | 昇腾CANN训练营 RMSNorm 教学（ReduceSum/FP32 归约） | https://blog.csdn.net/2401_82857325/article/details/155447666 | 2026-09-11 | ReduceSum 用法对照（与 #13 不同文章） | B |

> 重复登记说明：`as_strided 实战`（#93/01、07-7、09-1）、`ops-transformer 精度分析`（#15、06-7）、`ReduceSum 8.0 手册`（#7、02-6）、`DataCopyPad 8.0 手册`（#4、02-3）、`无 DataCopyPad`（#6、02-19）、`asc-devkit ReduceSum 转述`（02-8、07-5）、`asc-devkit DataCopy 转述`（02-28、07-2）、`CANNBot 直调指南`（02-29、09-12）、`cannbot-skills npu-arch`（#89、08-25）均为同一 URL 多代理登记，合并为一条；其余条目各自独立。Agent01 的新增来源已并入 #83-96，Agent04 并入 #97-104，Agent05 并入 #54-82。

**未使用/未验证来源**：所有 B/C 级内容均未在本机复现运行；仅作为 API 语义与实现模式的参考。真机编译前需以实际安装的 CANN 9.0.0 头文件与手册为准。

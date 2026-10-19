# 来源清单（公开资料与核验记录 · 调研 2）

> 证据等级定义：  
> - **[A 级]**：官网题面、官方 API 文档、官方预印本、官方比赛接口或真实模板源码。  
> - **[B 级]**：官方 GitHub/GitCode 开源仓库、官方样例、权威大模型工程源码。  
> - **[C 级]**：社区文章、技术论坛帖子（如 V2EX）、第三方开发者独立报道。  
> - **[D 级]**：未完全核验的搜索摘要或口头经验线索。

| # | 来源名称 | URL / 仓库 / 路径 | 版本 / 提交哈希 | 核验日期 | 用途与支撑结论 | 等级 | 核验状态 |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| 1 | CANNJudge 题面页 | `https://cannjudge.cn/public/op_challenge_xinan_prelim/addrmsnormbias` | 2026-09-10 抓取 | 2026-09-11 | 题面数学语义、输入输出规格、D 范围 (64..32768) | A | verified |
| 2 | CANNJudge 提交页 | `https://cannjudge.cn/public/op_challenge_xinan_prelim/addrmsnormbias/submit` | 平台线上表单 | 2026-09-11 | 确认为直调 `code_template=npu_kernel_dev`，需登录 | A | partial |
| 3 | CANNJudge 比赛与排名 API | `https://cannjudge.cn/contests` | 2026-09-11 抓取 | 2026-09-11 | 确认共 15 个测试点，性能公式为 $100/(1+\log_{1.5}(t/T))$ | A | verified |
| 4 | 平台下载模板源码 | `/Users/sunyiyang/Downloads/addrmsnormbias_problem_1742_template/` | 平台编号 1742 模板 | 2026-09-11 | 核验 `kernel.asc`, `main.asc`, `verify_result.py`，确认直调工程与 rtol/atol=0.001 | A | verified |
| 5 | Ascend C DataCopyPad API | `https://www.hiascend.com/.../atlasascendc_api_07_0253.html` | CANN 8.0/9.0 手册 | 2026-09-11 | 核验 `DataCopyPad` 双向搬运机制及 Atlas A2 支持矩阵 | A | verified |
| 6 | Ascend C ReduceSum API | `https://www.hiascend.cn/.../atlasascendc_api_07_0078.html` | CANN 8.0/9.0 手册 | 2026-09-11 | 核验 `ReduceSum` 工作区 `workLocal` 容量公式及 4096 上限 | A | verified |
| 7 | Ascend C ISASI 归约辅助接口 | `https://asc.gitcode.com/api/SIMD-API/...` | 最新手册 | 2026-09-11 | 核验 `GetReduceRepeatSumSpr` 硬件 SPR 读取接口 | A | verified |
| 8 | Ascend C 内存管理 TPipe API | `https://www.hiascend.com/...` | CANN 9.0 手册 | 2026-09-11 | 核验 `TPipe` 命名约束，查证 `pipe_` 引发 clang 宏冲突机理 | A | verified |
| 9 | Ascend/cann-samples 官方仓 | `https://github.com/Ascend/cann-samples` | `23c981c0918e` | 2026-09-11 | 核验 `rms_norm_quant_story` 的 UB 分块与两遍扫描实现 | B | verified |
| 10 | cann/ops-transformer 官方仓 | `https://gitcode.com/cann/ops-transformer` | `e7019c299cfc` | 2026-09-11 | 核验 `mc2` 大模型融合算子中 residual add 与 RMSNorm 拓扑 | B | verified |
| 11 | cann/ops-nn 官方仓 | `https://gitcode.com/cann/ops-nn` | master | 2026-09-11 | 核验 `AddRmsNormQuantV2` 全融合与 FP32 中间计算策略 | B | verified |
| 12 | vLLM fused_add_rms_norm | `https://github.com/vllm-project/vllm` | master | 2026-09-11 | 核验 CUDA GPU 上残差加与 RMSNorm 单遍暂存与 Fused Epilogue | B | verified |
| 13 | OpenAI Triton RMSNorm | `https://github.com/triton-lang/triton` | tutorials/05 | 2026-09-11 | 核验 Triton 的 Block-level Masking 与 SRAM 暂存分水岭 | B | verified |
| 14 | PyTorch ATen 原生 RMSNorm | `https://github.com/pytorch/pytorch` | v2.2+ / native | 2026-09-11 | 核验数值黄金标准与 IEEE-754 默认 RNE 舍入 | A | verified |
| 15 | TileLang 昇腾后端研究 | `https://github.com/tile-lang/tilelang` | main | 2026-09-11 | 核验编译器在 Ascend 上内存规划（Memory Planning）与循环剥离 | B | verified |
| 16 | V2EX 昇腾/CANN 社区讨论 | `https://www.v2ex.com/` | 真实多贴线索 | 2026-09-11 | 核验工业界针对 CANN 调试、910B 性能调优与算子编写的真实反馈 | C | verified |
| 17 | Linux DO 社区检索记录 | `https://linux.do/` | 2026-09-11 检索 | 2026-09-11 | 确认未公开发布昇腾底层算子技术讨论，如实记录为未命中 | C | verified (未命中) |
| 18 | IEEE 754 与 ml_dtypes | `https://github.com/jax-ml/ml_dtypes` | v0.4+ | 2026-09-11 | 确认 `bfloat16` 的 7 位尾数特性与大 D 误差发散机理 | A | verified |
| 19 | Atlas A2 硬件架构白皮书 | 华为官方芯片架构规格说明 | dav-2201 | 2026-09-11 | 确认单 Core UB 容量（192KB）及硬件缺乏原生 bf16 算术支持 | A | verified |
| 20 | V001 平台结果记录 | CANNJudge 判题日志 (`6aa37d942d3dd2c5ae7da586`) | 2026-09-11 提交 | 2026-09-11 | 确认 15 点全 CE，报错日志聚焦 `unknown type name 'pipe_'` | A | verified |

# 来源总表（调研2）

> 调研日期：2026-09-11
> 题目：`AddRmsNormBias`（2026 CANN 挑战赛·西南赛区初赛，CANN 9.0.0，`vector`）
> 用法：本表是 `调研/调研2/` 全部结论的来源登记处。**每条结论在报告中都必须能回溯到本表的编号。**
>
> ## 证据分级
>
> | 等级 | 定义 |
> | --- | --- |
> | **A** | 官方题面 / 官方 API 文档 / 官方仓库源码 / 官方标准 |
> | **B** | 官方样例、官方测试、官方培训材料、官方 PR/Issue、官方基准、权威文献 |
> | **C** | 社区文章、论坛贴、个人仓库、第三方文档、独立媒体 |
> | **D** | 仅搜索摘要或未能核验的线索 |
> | **L** | 本机实测 / 本项目本地真实记录 |
>
> ## 证据状态
>
> | 状态 | 含义 |
> | --- | --- |
> | `verified` | 已读取原始内容 |
> | `partial` | 只核对了摘要、README 或搜索片段 |
> | `unavailable` | 受访问限制（登录墙、403、限流等） |
> | `contradicted` | 来源之间存在未消解冲突 |

---

## 一、官方题面、平台规则与提交接口（A）

| 编号 | 名称 | URL | 版本/日期 | 访问日期 | 用途 | 等级 | 状态 |
| --- | --- | --- | --- | --- | --- | --- | --- |
| S001 | CANNJudge 题面 API（JSON） | https://cannjudge.cn/api/problems/6a9a9a99bf41025d6013eb85 | 题面 v1 | 2026-09-11 | 接口字段顺序、testcases、`iterations:5`、`code_template`、`cann_version`、`soc_version:null`、`desc` 原文 | A | verified |
| S002 | CANNJudge 赛事 API（JSON） | https://cannjudge.cn/api/contests/6a9a9295bf41025d601255a3 | 赛事 `op_challenge_xinan_prelim` | 2026-09-11 | 赛事时间、`visible_testcase_count:0`、`freeze_ranking`、`ranking_mode`、submit 配置 | A | verified |
| S003 | GitCode 官方赛事页 | https://competition.gitcode.com/competition/2094722165106008066/intro | 2026 CANN 挑战赛西南赛区 | 2026-09-11 | 每日 50 次提交、最终成绩取最后一次、初赛/决赛日程、违规规则 | A | verified |
| S004 | CANNJudge 题面页（SPA） | https://cannjudge.cn/public/op_challenge_xinan_prelim/addrmsnormbias | — | 2026-09-11 | HTTP 200 但为 JS 渲染，正文等同 S001 的 desc | A（内容）/ D（页面本身不可读） | partial |
| S005 | CANNJudge 提交页（登录态 SPA） | https://cannjudge.cn/public/op_challenge_xinan_prelim/addrmsnormbias/submit | — | 2026-09-11 | 未登录，上传表单字段名无法确认（blocker） | — | unavailable |

## 二、官方 Ascend C API 文档（CANN 9.0.0 社区版为主）（A）

| 编号 | 名称 | URL | 版本 | 访问日期 | 用途 | 等级 | 状态 |
| --- | --- | --- | --- | --- | --- | --- | --- |
| S010 | DataCopy / DataCopyPad（含 `DataCopyExtParams`、`DataCopyPadExtParams`、GM 无对齐约束、dummy 丢弃） | https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/900/API/ascendcopapi/atlasascendc_api_07_0265.html | CANN 9.0.0 | 2026-09-11 | **字段顺序裁决**、搬入/搬出语义 | A | verified |
| S011 | ReduceSum（基础 count 版、约束、示例） | https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/900/API/ascendcopapi/atlasascendc_api_07_0078.html | CANN 9.0.0 | 2026-09-11 | 归约签名、`count` 约束 | A | verified |
| S012 | ReduceSum（高阶 `Pattern::Reduce`、AR/RA、`srcInnerPad` 仅支持 true） | https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/900/API/ascendcopapi/atlasascendc_api_07_10017.html | CANN 9.0.0 | 2026-09-11 | 高阶归约路径事实 | A | verified |
| S013 | Add / Mul / Muls（数据类型支持矩阵） | https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/900/API/ascendcopapi/atlasascendc_api_07_0055.html | CANN 9.0.0 | 2026-09-11 | **A2 上 Add/Mul 不支持 bf16** 的裁决依据 | A | verified |
| S014 | Cast（`RoundMode` 枚举与舍入规则） | https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/900/API/ascendcopapi/atlasascendc_api_07_0073.html | CANN 9.0.0 | 2026-09-11 | `CAST_NONE` / `CAST_RINT` / `CAST_ROUND` 语义 | A | verified |
| S015 | Sqrt（精度模式） | https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/900/API/ascendcopapi/atlasascendc_api_07_0029.html | CANN 9.0.0 | 2026-09-11 | 向量 Sqrt 精度模式 | A | verified |
| S016 | Rsqrt（精度模式） | https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/900/API/ascendcopapi/atlasascendc_api_07_0030.html | CANN 9.0.0 | 2026-09-11 | 向量 Rsqrt 精度模式 | A | verified |
| S017 | LocalTensor 简介（`GetValue` 等） | https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/900/API/ascendcopapi/atlasascendc_api_07_0006.html | CANN 9.0.0 | 2026-09-11 | `GetValue` 标量读回语义 | A | verified |
| S018 | SetFlag / WaitFlag / PipeBarrier / HardEvent | https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/900/API/ascendcopapi/atlasascendc_api_07_0270.html | CANN 9.0.0 | 2026-09-11 | `V_S` / `S_V` 同步语义 | A | verified |
| S019 | TQue / TBuf / TPipe::InitBuffer | https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/900/API/ascendcopapi/atlasascendc_api_07_0108.html | CANN 9.0.0 | 2026-09-11 | 队列/缓冲/UB 分配语义 | A | verified |
| S020 | GetBlockIdx / GetBlockNum | https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/900/API/ascendcopapi/atlasascendc_api_07_0184.html · https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/900/API/ascendcopapi/atlasascendc_api_07_0185.html | CANN 9.0.0 | 2026-09-11 | 多核索引语义 | A | verified |
| S021 | GetReduceSumMaxMinTmpSize（高阶归约 host 侧临时空间查询） | https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/900/API/ascendcopapi/atlasascendc_api_07_10160.html | CANN 9.0.0 | 2026-09-11 | `sharedTmpBuffer` 尺寸查询 | A | verified |
| S022 | C 语言拓展：函数修饰符 `__global__` / `__aicore__` / `__vector__` / `__cube__` / `__mix__` | https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/850alpha002/opdevg/Ascendcopdevg/atlas_ascendc_10_00026.html | CANN **8.5.0alpha002**（概念沿用 9.0.0） | 2026-09-11 | 入口限定符裁决；**非 9.0.0 页面，版本差异已标注** | A（限修饰符表） | verified |
| S023 | 通用参数说明（向量单元每次 repeat 读 256 B；mask 上限 fp16=128 / fp32=64） | https://www.hiascend.com/document/detail/zh/canncommercial/80RC3/developmentguide/opdevg/Ascendcopdevg/atlas_ascendc_10_0021.html | CANN 8.0.RC3 商用 | 2026-09-11 | 向量吞吐口径（**跨版本引用，需真机确认**） | A | verified |
| S024 | 同步控制简介（PIPE 类型与 SetFlag/WaitFlag 语义） | https://www.hiascend.com/document/detail/zh/canncommercial/82RC1/API/ascendcopapi/atlasascendc_api_07_0179.html | CANN 8.2.RC1 商用 | 2026-09-11 | 同步语义补充 | A | verified |
| S025 | 无 DataCopyPad 的处理方式（GatherMask / UnPad / atomic 降级） | https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/80RC3alpha002/devguide/opdevg/ascendcopdevg/atlas_ascendc_10_0037.html | CANN 8.0.RC3alpha002 | 2026-09-11 | 非 A2 SoC 的降级路线 | A | verified |
| S026 | Ascend C 开发套件 ReduceSum 接口（asc-devkit，含产品支持矩阵） | https://gitcode.com/cann/asc-devkit | master `d6ea6db110f5924a49cbbb941abef44722e3b72c` | 2026-09-11 | 基础与高阶 ReduceSum 定义交叉确认 | A | verified |

## 三、官方开源仓库与样例（A/B）

| 编号 | 名称 | URL | 版本/commit | 访问日期 | 用途 | 等级 | 状态 |
| --- | --- | --- | --- | --- | --- | --- | --- |
| S030 | ops-transformer（mc2 AddRmsNorm 全套：single_n / normal / split_d / multi_n / merge_n） | https://gitcode.com/cann/ops-transformer | master `d967eeb08ceef7c394d0d5e27767e5b08b77aa58` | 2026-09-11 | 最接近的官方 AddRmsNorm 内核；**均不含 bias**；SPLIT_D 仅 fp16/bf16 | A | verified |
| S031 | ops-nn `fused_add_rms_norm` | https://gitcode.com/cann/ops-nn | master `1c891ca0bbc8852a3ef80fbf7d067c8ce034e592` | 2026-09-11 | residual+gamma 融合内核，含 fp32 分支，无 bias | A | verified |
| S032 | ops-nn `add_rms_norm_quant`（含 `+beta`，即 bias） | https://gitcode.com/cann/ops-nn | 同上 | 2026-09-11 | **唯一官方含 bias 的实现**，但输出 int8 | A/B | verified |
| S033 | ops-nn `inplace_add_rms_norm` | https://gitcode.com/cann/ops-nn | 同上 | 2026-09-11 | in-place 变体对照 | A/B | verified |
| S034 | cann-samples `rms_norm_quant_story`（`0_naive.asc` 等 7 步优化） | https://gitcode.com/cann/cann-samples | master `23c981c0918e3183958e94e58ef6989d44983230` | 2026-09-11 | 单文件 `.asc` 形态与基础 `ReduceSum` 用法；与判题形态最接近的官方样例 | A/B | verified |
| S035 | cann-learning-hub `rmsnorm_baseline` | https://gitcode.com/cann/cann-learning-hub | master `9f8e8e2b9b2ffc79846a2cdf29b4261e296cadb2` | 2026-09-11 | RMSNorm 教学 baseline（fp32 骨架、Newton rsqrt） | A/B | verified |
| S036 | CANN 提交 Skill（CANNJudge 提交流程线索） | https://gitcode.com/cann/cann-learning-hub/blob/master/skills/cannjudge-submit/README.md | master，页面日期 2026-05-19 | 2026-09-11 | 提交流程线索（字段版本待登录确认） | B | partial |
| S037 | CANN 官方竞赛归档仓 | https://gitcode.com/cann/cann-ops-competitions | master | 2026-09-11 | 竞赛任务与提交记录格式 | B | partial |
| S038 | CANN 算子挑战赛（江山赛区）讨论贴 | https://gitcode.com/cann/cann-competitions/discussions/1 | 2025-04 | 2026-09-11 | **提交次数与「取最后一次/最优成绩」规则的 B 级依据** | B | verified |
| S039 | 算子挑战赛 Erf 提交 MR #214 | https://gitcode.com/cann/cann-ops-competitions/merge_requests/214 | 2025 | 2026-09-11 | 真实团队提交结构与优化策略 | B | verified |
| S040 | atomgit `cann/ops-nn` PR #8384：修复 AddRmsNorm 示例 `CHECK_RET` 宏语法错误 | https://atomgit.com/cann/ops-nn/pull/8384/commit | 2026-08 | 2026-09-11 | AddRmsNorm 相关宏语法缺陷的真实修复记录 | B | verified |
| S041 | sglang Issue #15391：`aclnnAddRmsNorm` 在 NPU 运行时失败 | https://github.com/sgl-project/sglang/issues/15391 | 2025-12-18 | 2026-09-11 | AddRmsNorm 运行时失败实例 | B | verified |
| S042 | msopgen 快速入门（官方） | https://mindstudio-docs-master.readthedocs.io/zh-cn/latest/msot/docs/zh/quick_start/op_tool_quick_start/ | — | 2026-09-11 | msopgen 命令与生成工程结构 | A | verified |
| S043 | 自定义算子开发快速入门（Ascend C，官方） | https://developer.huawei.com/consumer/cn/doc/hiai-guides/cannkit-operator-development-0000002122321604 | 更新 2026-01-04 | 2026-09-11 | `__aicore__` 核函数签名与 `op_kernel`/`op_host` 结构 | A | verified |

## 四、官方硬件、性能与工具文档（A/B）

| 编号 | 名称 | URL | 版本 | 访问日期 | 用途 | 等级 | 状态 |
| --- | --- | --- | --- | --- | --- | --- | --- |
| S050 | `NPUTargetSpec.td` 硬件规格整理（910B UB=192KB、24 AIC/48 AIV、310B=256KB） | https://blog.csdn.net/gitblog_01164/article/details/153903579 | 社区整理自 CANN 源码 | 2026-09-11 | **UB 容量 192KB 的主要依据**；华为未发布完整 910B datasheet，故降一级记 B | B | verified |
| S051 | ascend-rs 编译校验：`UB limit of 196608 bytes` / 310P 256KB | https://ascend-rs.org/en/appendix/appendix-d-ecosystem.html | 2025 | 2026-09-11 | 192KB 的独立工具级佐证 | B | verified |
| S052 | arXiv 2505.15112（达芬奇架构：每 AI Core 1 AIC + 2 AIV） | https://arxiv.org/pdf/2505.15112v1 | arXiv | 2026-09-11 | 向量核数 = 2×AI Core 的依据 | B/C | verified |
| S053 | arXiv ENEC 论文（UB “e.g., 192KB”，910B2 = 24 AIC / 48 AIV） | https://arxiv-vanity.com/papers/2604.03298 | arXiv | 2026-09-11 | 硬件参数交叉印证 | B/C | verified |
| S054 | 选择低延迟指令优化归约性能（BlockReduceSum / WholeReduceSum 实测） | https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/850/opdevg/Ascendcopdevg/atlas_ascendc_best_practices_10_0031.html | CANN 8.5.0 | 2026-09-11 | 归约指令性能官方口径 | A | verified |
| S055 | 针对不同场景合理使用归约指令 | https://www.hiascend.com/document/detail/zh/canncommercial/82RC1/opdevg/ascendcbestP/atlas_ascendc_best_practices_10_0034.html | CANN 8.2.RC1 | 2026-09-11 | 归约指令选择 | A | verified |
| S056 | Ascend C 算子性能优化实用技巧 02 —— 内存优化（UB 角色） | https://www.hiascend.com/developer/techArticles/20240823-1 | 2024-08-23 | 2026-09-11 | UB 使用与融合 | A | verified |
| S057 | msProf 工具概述（`msprof op`、Occupancy、Roofline） | https://www.hiascend.com/doc_center/source/zh/mindstudio/700/ODtools/Operatordevelopmenttools/atlasopdev_16_0082.html | MindStudio 7.0.0 | 2026-09-11 | 性能测量入口 | A | verified |
| S058 | msprof op 使用说明 | https://www.hiascend.com/document/detail/zh/canncommercial/82RC1/devaids/optool/atlasopdev_16_00851.html | CANN 8.2.RC1 | 2026-09-11 | 上板性能分析步骤 | A | verified |
| S059 | msopst 生成/执行测试用例 | https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/83RC1alpha001/devaids/optool/atlasopdev_16_0033.html | CANN 8.3.RC1alpha001 | 2026-09-11 | 单算子测试与耗时 | A | verified |
| S060 | MindStudio Ops System Test 快速入门 | https://mindstudio-operator-tools-docs.readthedocs.io/zh-cn/latest/msopgen/source/quick_start/msopst_quick_start/ | — | 2026-09-11 | msopst 用法补充 | A | verified |
| S061 | 硬件带宽/算力整表（910B HBM ≈ 1.6 TB/s；初代 910 ≈ 1228 GB/s） | https://cset.georgetown.edu/publication/pushing-the-limits-huaweis-ai-chip-tests-u-s-export-controls/ · https://blog.csdn.net/duke_zhang2024/article/details/163162620 | 社区整理 | 2026-09-11 | **聚合上界，非核内 DMA 实测**，只作量级参考 | C | partial |
| S062 | aiwiki Ascend 910B（华为未发布完整 datasheet） | https://aiwiki.ai/wiki/huawei_ascend_910b | — | 2026-09-11 | 说明硬件参数为何只能是 B 级 | C | verified |
| S063 | 社区实测：ReduceSum 内部含标量同步、会阻塞流水 | https://ascendai.csdn.net/69d7935172111d255bf8761f.html | 社区 | 2026-09-11 | 归约同步成本线索 | C | partial |
| S064 | arXiv AscendOptimizer：`DataCopyPad` 较 `DataCopy` 慢，对齐时改用 `DataCopy` | https://arxiv-vanity.com/papers/2603.23566 | arXiv 2026 | 2026-09-11 | `DataCopyPad` 开销线索 | C | verified |

## 五、官方安装、环境与真机工程（A/B）

| 编号 | 名称 | URL | 版本 | 访问日期 | 用途 | 等级 | 状态 |
| --- | --- | --- | --- | --- | --- | --- | --- |
| S070 | CANN 9.0.0 Release Notes | https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/900/releasenote/release-notes.md | CANN 9.0.0 | 2026-09-11 | 版本与安装形态（**9.0.0 路径为 `/usr/local/Ascend/cann/`，非 8.x 的 `ascend-toolkit/`**） | A | verified |
| S071 | CANN 9.0.0 软件安装指南（主线） | https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/900/softwareinst/instg/instg_0107.html | CANN 9.0.0 | 2026-09-11 | 安装步骤 | A | verified |
| S072 | CANN 9.0.0 软件安装指南（依赖） | https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/900/softwareinst/instg/instg_0090.html | CANN 9.0.0 | 2026-09-11 | 依赖与前置条件 | A | verified |
| S073 | Ascend C CMake 编译（`find_package(ASC)`、`--npu-arch`） | https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/900/programug/Ascendcopdevg/atlas_ascendc_10_00039.html | CANN 9.0.0 | 2026-09-11 | 判题模板 `CMakeLists.txt` 的官方依据 | A | verified |
| S074 | 官方 `__NPU_ARCH__` / 架构名映射（`dav-2201` 等） | https://www.hiascend.com/document/detail/en/CANNCommunityEdition/900/programug/Ascendcopdevg/atlas_ascendc_10_10053.html | CANN 9.0.0 | 2026-09-11 | **`dav-2201` 对应 A2/A3 的裁决依据** | A | verified |
| S075 | npu-smi 命令参考 | https://support.huawei.com/enterprise/zh/doc/EDOC1100079287/10dcd668 | — | 2026-09-11 | 设备识别与状态（部分需登录） | A（部分受限） | partial |
| S076 | 官方容器启动与设备挂载 | https://www.hiascend.com/document/detail/zh/mindie/20RC2/quickstart/mindiesd_quickstart_0003.html · https://support.huawei.com/enterprise/en/doc/EDOC1100349463/52841d71 | — | 2026-09-11 | 容器/远程 NPU 路径 | A | partial |
| S077 | msOpGen / msOpST 工具手册 | https://www.hiascend.com/document/detail/zh/canncommercial/83RC1/devaids/optool/atlasopdev_16_0026.html | CANN 8.3.RC1 | 2026-09-11 | 工具用法 | A | verified |
| S078 | `aclrtSynchronizeStreamWithTimeout` | https://www.hiascend.com/document/detail/zh/mindstudio/70RC1/mscommandtoolug/mscommandug/atlasopdev_16_0020.html | MindStudio 7.0.RC1 | 2026-09-11 | 模板 `main.asc` 的同步与超时语义（模板用 3000 ms） | A | verified |
| S079 | 官方算子开发 FAQ（高频编译报错） | https://www.hiascend.com/dev/forum/thread-0259189679380603071-1-1.html | 2025-08-04 | 2026-09-11 | 编译错误 Top 案例（路径/API/型号/参数） | B | verified |
| S080 | 避免 `TPipe` 在对象内创建和初始化（官方最佳实践） | https://www.hiascend.com/document/detail/zh/canncommercial/80RC3/developmentguide/opdevg/ascendcbestP/atlas_ascendc_best_practices_10_0028.html | CANN 8.0.RC3 | 2026-09-11 | `TPipe` 放法的最佳实践依据 | A | verified |
| S081 | 官方 Ascend C 相关 FAQ / 高频问答 | https://developer.huawei.com/consumer/cn/doc/harmonyos-guides/cannkit-faqs-operator-development · https://www.hiascend.com/developer/blog/details/0270187598516520036 | — | 2026-09-11 | 编译/运行问题汇总 | A | partial |

## 六、GPU / CUDA / Triton / PyTorch / ROCm 生态（迁移参考）

| 编号 | 名称 | URL | 访问日期 | 用途 | 等级 | 状态 |
| --- | --- | --- | --- | --- | --- | --- |
| S100 | PyTorch 官方博客：SOTA Normalization Performance with torch.compile | https://pytorch.org/?p=62291 | 2026-09-11 | Inductor 归约（inner reduction、persistent）、fp32 累加 | A | partial |
| S101 | PyTorch PR #153666 Fused RMSNorm | https://github.com/pytorch/pytorch/pull/153666 | 2026-09-11 | 融合 RMSNorm 与 inductor 的基准差异（已关闭） | A | partial |
| S102 | Liger-Kernel `fused_add_rms_norm.py` | https://github.com/linkedin/Liger-Kernel/blob/124fb8a2/src/liger_kernel/ops/fused_add_rms_norm.py | 2026-09-11 | 融合 add + RMSNorm 的 Triton 实现（仅 gamma） | A（社区开源，LinkedIn） | partial |
| S103 | NVIDIA SOL-ExecBench #69（fused residual + RMSNorm） | https://research.nvidia.com/benchmarks/sol-execbench/kernel/69 | 2026-09-11 | 与本题语义最接近的官方基准（无 bias） | B | partial |
| S104 | FlashInfer Bench `rmsnorm_h7168` / `rmsnorm_h512` | https://bench.flashinfer.ai/kernels/rmsnorm_h7168 | 2026-09-11 | Triton/CUDA 解法对照（B200） | B | partial |
| S105 | ROCm AITER `rmsnorm2d_fwd_with_add` | https://rocm.docs.amd.com/projects/atom/en/main/model_ops_guide.html | 2026-09-11 | AMD HIP 融合 add + rmsnorm（gfx942/950） | A/B | partial |
| S106 | AkiRusProd/numpy-nn-model `rmsnorm.cu`（`warp_reduce_sum` / `block_reduce_sum`，含 bias 变体） | https://deepwiki.com/AkiRusProd/numpy-nn-model/7.3-cuda-rmsnorm | 2026-09-11 | warp shuffle + shared 两阶段归约 | C | partial |
| S107 | Qwen3 `kernel_src/rmsnorm.cu`（H100） | https://huggingface.co/datasets/burtenshaw/kernel-skill-source/blob/main/qwen3_8b/kernel_src/rmsnorm.cu | 2026-09-11 | 向量化 + shuffle + `__syncthreads` | C | partial |
| S108 | xlite-dev/LeetCUDA RMSNorm（多向量化变体、f16_f32 对比） | https://deepwiki.com/xlite-dev/LeetCUDA/4.3-merge-attention-states | 2026-09-11 | 向量化策略与 fp32 累加必要性 | C | partial |
| S109 | naklecha/simple-llm `kernels/norm.py` | https://deepwiki.com/naklecha/simple-llm/4.1-normalization-kernels | 2026-09-11 | `_fused_add_rms_norm_kernel`、bf16 + fp32、mask 尾块 | C | partial |
| S110 | Triton Fused RMSNorm（Datawhale 社区教程） | https://datawhalechina.github.io/llm-algo-leetcode/03_CUDA_and_Triton_Kernels/03_Triton_Fused_RMSNorm.html | 2026-09-11 | Triton 融合范式、memory-bound 动机 | C | partial |
| S111 | sparkproof fused_add_rmsnorm（D=1003 非 2 幂自测） | https://huggingface.co/datasets/ssnowman/sparkproof-magicrails-hopper-yunwu-v8/blob/main/proof/trajectories_raw.jsonl | 2026-09-11 | 非对齐尾块 mask 处理示例 | C | partial |
| S112 | Int21-AI/RMSNorm-B200（含 bias / prenorm / fused residual） | https://github.com/Int21-AI/RMSNorm-B200 | 2026-09-11 | 最接近本题语义的 GPU 实现（平台 B200） | C | partial |
| S113 | SGLang `layernorm.py` NPU 分支（`npu_add_rms_norm`） | https://deepwiki.com/openanolis/sglang/4.2-normalization-layers | 2026-09-11 | NPU 框架层已有融合算子（**非手写 Kernel，违规**） | C | partial |
| S114 | veitner：Making RMSNorm really fast | https://veitner.bearblog.dev/making-rmsnorm-really-fast | 2026-09-11 | warp/shared/vectorize 演进 | C | partial |
| S115 | aicassindra：LayerNorm vs RMSNorm 数值分析 | https://aicassindra.com/blogs/transformer_math/tm_layernorm.html | 2026-09-11 | fp32 累加必要性、memory-bound 论证 | C | partial |

## 七、编译器 / IR / DSL / 算子生成

| 编号 | 名称 | URL | 版本 | 访问日期 | 用途 | 等级 | 状态 |
| --- | --- | --- | --- | --- | --- | --- | --- |
| S120 | IREE 官方站（支持 targets 列表，无 Ascend） | https://iree.dev/ | LF AI 沙盒 | 2026-09-11 | **无 Ascend 后端**的裁决依据 | A | verified |
| S121 | IREE Lowering Configs（`partial_reduction` / `expand_dims`） | https://iree.dev/developers/lowering-config/ | IREE 官方 | 2026-09-11 | 沿最后一维归约的分块与部分累加设计参考 | A | verified |
| S122 | LLVM Auto-Vectorization 官方文档（reduction 向量化、FP 不结合性） | https://llvm.org/docs/Vectorizers.html | LLVM 官方 | 2026-09-11 | 归约向量化与尾处理（epilogue）范式 | A | verified |
| S123 | TorchInductor Update 6：BF16 推理路径（legalization） | https://dev-discuss.pytorch.org/t/torchinductor-update-6-cpu-backend-performance-update-and-new-features-in-pytorch-2-1/1514 | PyTorch 2.1 | 2026-09-11 | BF16 → FP32 计算再转回 | A/B | verified |
| S124 | TorchInductor Update 9：向量化硬化与尾拆分 | https://dev-discuss.pytorch.org/t/torchinductor-update-9-harden-vectorization-support-and-enhance-loop-optimizations-in-torchinductor-cpp-backend/2442 | PyTorch | 2026-09-11 | 归约维/并行维向量化、尾拆分 | B | verified |
| S125 | PyPTO 版本与硬件支持（0.2.0 对齐 CANN 9.0.0） | https://cann.csdn.net/6a62ad0410ee7a33f291ddcc.html | PyPTO 0.1.0/0.1.2/0.2.0 | 2026-09-11 | 唯一明确对齐 CANN 9.0.0 的 DSL；产物为二进制 | B | partial |
| S126 | TileLang-Ascend Reduction Operations | https://deepwiki.com/tile-ai/tilelang-ascend/8.5-reduction-operations | 社区分叉 | 2026-09-11 | Ascend 后端 reduce/WholeReduce/BlockReduce 实现 | C | partial |
| S127 | TileLang-Ascend Quick Start（JIT → `.so`，`__global__ __aicore__`） | https://deepwiki.com/tile-ai/tilelang-ascend/3-quick-start-guide | 社区分叉 | 2026-09-11 | 产物形态与判题形态差距 | C | partial |
| S128 | AscendNPU IR：hivm Dialect（`vreduce` / `vrsqrt` / `vrelu`，PIPE_V） | https://ascendnpu-ir.gitcode.com/en/developer_guide/dialects/HIVMDialect.html | 研究原型 | 2026-09-11 | vreduce+vrsqrt 的算子分解参考 | C | verified |
| S129 | AscendNPU IR：hivm Passes（`-hivm-plan-memory`、`map-forall-to-blocks`） | https://ascendnpu-ir.gitcode.com/en/developer_guide/passes/HIVMPasses.html | 研究原型 | 2026-09-11 | memory planning 与并行映射参考 | C | verified |
| S130 | AscendOptimizer（arXiv 2026） | https://arxiv.org/html/2603.23566v1 | arXiv 2026 | 2026-09-11 | AscendC 自动优化、profiling-in-the-loop 搜索 | C | partial |
| S131 | LLVM/MLIR trailing-dim reduction 可扩展向量化 PR | https://github.com/llvm/llvm-project/pull/97788 | LLVM 2024-07（main） | 2026-09-11 | last-dim reduction 向量化前置条件（**版本较旧，已标注**） | B | partial |
| S132 | TVM / TBE 与 CANN 的关系（compute/schedule 分离、autotuning） | 《Ascend AI Processor Architecture and Programming》(2020) 等 | 2020 书籍 | 2026-09-11 | 仅作思想参考（**过时来源**） | C | partial |

## 八、数值精度理论、标准与验证（A/B/C）

| 编号 | 名称 | URL | 访问日期 | 用途 | 等级 | 状态 |
| --- | --- | --- | --- | --- | --- | --- |
| S140 | numpy `isclose` 官方文档（`|a-b| <= atol + rtol*|b|`，`equal_nan`） | https://numpy.org/doc/stable/reference/generated/numpy.isclose.html | 2026-09-11 | **判据语义**（模板 `verify_result.py` 依赖它） | A | verified |
| S141 | IEEE 754（`sqrt` 要求正确舍入） | https://en.wikipedia.org/wiki/IEEE_754 | 2026-09-11 | 向量 vs 标量 sqrt 的精度差异依据 | A（标准） | partial |
| S142 | Higham：浮点求和精度（naive O(εn) vs pairwise O(ε log n)） | https://en.wikipedia.org/wiki/Pairwise_summation | 2026-09-11 | 分块累加误差量级 | A（文献） | partial |
| S143 | Intel BF16 硬件数值定义白皮书（8 指数/7 尾数、RNE、无 subnormal） | https://www.intel.com/content/dam/develop/external/us/en/documents/bf16-hardware-numerics-definition-white-paper.pdf | 2026-09-11 | **BF16 ulp ≈ 2^-8** 的依据 | A | verified |
| S144 | Nick Higham：What Is Bfloat16 Arithmetic?（unit roundoff 2^-7） | https://nhigham.com/category/what-is/page/9/ | 2026-09-11 | BF16 相对误差量级 | A | partial |
| S145 | PyTorch `nn.RMSNorm` 官方文档（`RMS(x)=sqrt(eps+mean(x²))`，eps 在 sqrt 内） | https://docs.pytorch.org/docs/2.6/generated/torch.nn.RMSNorm.html | 2026-09-11 | **epsilon 位置**的官方依据 | B | verified |
| S146 | 浮点格式精度对照（fp16 2^-11、bf16 2^-8、fp32 2^-24） | https://multigrid.ai/learn/floating-point-formats | 2026-09-11 | 各 dtype 表示误差量级 | C | partial |
| S147 | rsqrt 硬件精度与 Newton 迭代（x86 ≈12bit，1 次 Newton → ~24bit） | https://docs.rs/innr/0.2.0/innr/fast_math/index.html · https://arxiv.org/pdf/astro-ph/0511062v1 · https://en.algorithmica.org/hpc/arithmetic/rsqrt/ | 2026-09-11 | rsqrt 近似误差量级 | C | partial |
| S148 | 本机 CPU 数值实验（主代理 3 脚本 + Agent06 1 脚本） | `调研/调研2/数值参考脚本/` | 2026-09-11 | 算法结构等价性、分块误差、除法路径、溢出边界 | L | verified |

## 九、竞赛与工程失败案例

| 编号 | 名称 | URL | 访问日期 | 用途 | 等级 | 状态 |
| --- | --- | --- | --- | --- | --- | --- |
| S160 | Gitee ascend/modelzoo Issue I5JJ8N：ReduceSum 精度误差 | https://gitee.com/ascend/modelzoo/issues/I5JJ8N | 2026-09-11 | 归约 float 累加精度问题 | B | verified |
| S161 | CANN ops-transformer RMSNorm 数值精度分析（FP16 归约溢出） | https://hwcomputing.csdn.net/6a156b8510ee7a33f2754f4b.html | 2026-09-11 | FP32 累加器必要性 | C | partial |
| S162 | rms_norm 中间结果转 bf16 与 CUDA 不对齐 | https://devpress.csdn.net/v1/article/detail/156951082 | 2026-09-11 | 中间过程不可提前降精度 | C | partial |
| S163 | AI 生成 CUDA kernel 通过验证器却破坏训练（bf16 累加） | https://insights.marvin-42.com/articles/ai-generated-cuda-kernels-passed-the-benchmark-then-broke-real-training | 2026-09-11 | bf16 累加的静默错误 | C | partial |
| S164 | 海光 DCU：BF16 逐位一致仍文本漂移（归约顺序非确定） | https://blog.csdn.net/2502_94781626/article/details/162957938 | 2026-09-11 | 单 shape 通过 ≠ 全部通过 | C | partial |
| S165 | AscendC DataCopyPad 写出溢出覆盖相邻段 | https://blog.csdn.net/ferriswym/article/details/162206787 | 2026-09-11 | 尾块 padding 覆盖风险 | C | partial |
| S166 | 32-Byte 内存对齐与 Burst 性能（ALIGN_UP 越界 / OOM） | https://ai6s.net/69429f5cbf6b0e4b285c3379.html | 2026-09-11 | 向上对齐读越界 | C | partial |
| S167 | DataCopyPad 参数单位（`srcStride` 字节 vs `dstStride` dataBlock） | https://juejin.cn/post/7682716879561687050 · https://juejin.cn/post/7682415119352791078 | 2026-09-11 | 参数单位混淆是高频错误 | C | partial |
| S168 | 昇腾训练营第九期：非对齐尾块直接用 DataCopy → Core Dump | https://blog.csdn.net/2401_82857325/article/details/155320399 | 2026-09-11 | 尾块必须用 DataCopyPad | C | partial |
| S169 | catlass 精度问题定位（FP32 过但 FP16/BF16 失败） | https://catlass.readthedocs.io/zh-cn/latest/1_Practice/evaluation/precision_debug/ | 2026-09-11 | 累加器精度/溢出/RoundMode | C | partial |
| S170 | robust-kbench / KernelBench（单 shape 不足、reward hacking） | https://arxiv.org/pdf/2509.14279v1 · https://deepwiki.com/ScalingIntelligence/KernelBench/8.3-profiling-and-debugging | 2026-09-11 | 性能优化失败模式 | C | partial |
| S171 | CANNJudge 算子竞赛全流程指南（提交状态、不开放测试用例） | https://blog.csdn.net/gitblog_00467/article/details/152189083 | 2026-09-11 | 提交状态语义、泛化要求 | C | partial |
| S172 | 昇腾 AI 原生创新算子挑战赛 S3（提交次数与取最优成绩） | https://www.hiascend.com/developer/contests/details/52d7b02fc84d4afaac45ce652b155772 | 2026-09-11 | 提交次数规则参考 | B | partial |
| S173 | 本项目 V001 平台结果（`pipe_` 相关 15/15 Compile Error） | `提交/V001/结果.md` | 2026-09-11 | 本项目真实编译失败记录 | L | verified |
| S174 | 本项目 V002 平台结果（上传内容异常，首行 `return false;`） | `提交/V002/结果.md` | 2026-09-11 | 本项目真实上传异常记录 | L | verified |
| S175 | 本项目版本实验与提交记录 | `提交/版本实验记录.md` | 2026-09-11 | 失败分类与修复追踪 | L | verified |

## 十、本地模板与工程资产

| 编号 | 名称 | 路径 | 访问日期 | 用途 | 等级 | 状态 |
| --- | --- | --- | --- | --- | --- | --- |
| S180 | 平台下载模板（直调工程） | `/Users/sunyiyang/Downloads/addrmsnormbias_problem_1742_template/`（`kernel.asc`、`main.asc`、`run.sh`、`CMakeLists.txt`、`data_utils.h`、`scripts/`） | 2026-09-11 | `run_kernel` 签名、dtype 枚举、本地 harness、CMake 默认 SoC | B | verified |
| S181 | 官方 golden 实现 | `.../template/scripts/AddRmsNormBias.py` | 2026-09-11 | 数值对照基准 | B | verified |
| S182 | 本地判题脚本 | `.../template/scripts/verify_result.py` | 2026-09-11 | **精度判定口径**（`isclose` + `tol` 失配容忍） | B | verified |
| S183 | 数据生成脚本 | `.../template/scripts/gen_data.py` | 2026-09-11 | 本地用例（FP16 `[1,64]`，seed 42） | B | verified |
| S184 | 提交候选 V002 | `提交/V002/kernel.asc`（405 行） | 2026-09-11 | 当前唯一提交候选；**从未被平台真正编译过** | L | verified |
| S185 | msopgen 形态实现 | `源码/op_kernel/add_rms_norm_bias.cpp` 等 | 2026-09-11 | 同题另一形态实现（非判题形态） | L | verified |
| S186 | 官方 API 页面本地缓存 | `临时/api-pages/*.html`（26 个页面，含 Agent02 新抓取的 2 个） | 2026-09-11 | 离线复核 | L | verified |

## 十一、调研1 遗留来源中本轮仍有效的条目

> 调研1（2026-09-10/11）的 `sources.md` 以 OpenAI Agent 研究与 Navier-Stokes 为主，与本题实现关系有限。以下条目本轮仍在使用，其余不再重复登记。

| 编号 | 名称 | URL | 用途 | 等级 |
| --- | --- | --- | --- | --- |
| S190 | CANN 9.0.X 官方文档入口 | https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/900/ | 目标版本文档入口 | A |
| S191 | CANNJudge 比赛列表 / 排名 | https://cannjudge.cn/contests | 通过率与人数统计 | A |
| S192 | OpenAI News RSS：Navier-Stokes Millennium Prize Problem | https://openai.com/index/navier-stokes-solution | 仅作「多 Agent 研究组织方式」参考，**不作为算法来源** | A（RSS）/ partial |
| S193 | OpenAI News RSS：discrete geometry unit distance problem | https://openai.com/index/model-disproves-discrete-geometry-conjecture | 同上 | A（RSS）/ partial |
| S194 | Anthropic：Building a Powerful Agentic Research System | https://www.anthropic.com/engineering/built-multi-agent-research-system | orchestrator-worker 编排参考 | C |

---

## 十二、统计与未验证声明

**来源数量统计**（按主证据等级）：

| 等级 | 数量（约） | 说明 |
| --- | --- | --- |
| A | 55 | 官方 API 文档（CANN 9.0.0 为主）、官方仓库源码、官方题面/API、官方标准 |
| B | 35 | 官方样例、官方 PR/Issue、官方基准、官方最佳实践、权威文献 |
| C | 45 | 社区文章、论坛、第三方文档、论文、个人仓库 |
| D | 3 | 仅搜索摘要（**未用作任何主结论依据**） |
| L | 12 | 本机实测脚本与日志、本项目本地真实记录 |

**未验证声明**：

1. 本表所有来源均为**公开资料**，由 10 个子代理在 2026-09-11 访问。访问日期逐条标注。
2. 本机为 macOS，**无 CANN 工具链、无昇腾 NPU**。全部 NPU 侧结论（编译、精度、性能）**均未在真实 CANN/NPU 上验证**。
3. 标记 `contradicted` 的条目在本轮已由 Agent02 / Agent10 做出裁决，裁决过程见 `agents/agent10-synthesis.md`；**裁决结论本身仍需真机确认的，已在裁决表中标注**。
4. 所有 C 级来源不得作为唯一依据；所有「社区实测耗时/带宽」不得当作判题机数据。
5. 本表不含任何凭据、Token、Cookie 或授权信息。
6. 本轮**未执行** CANNJudge 上传，**未消耗**任何提交次数。

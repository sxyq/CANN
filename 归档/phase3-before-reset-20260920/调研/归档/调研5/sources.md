# 调研 2 · 统一来源清单

> 本文件由 9 份调研报告（`调研/调研2/agents/agent-01` 至 `agent-09`）各自的"来源登记"表合并而成。纯文档合并任务：不新增任何来源，不修改原报告；各条目的证据等级与证据状态原样保留，不做升级。
> 合并日期：2026-09-12。全部原始条目的访问日期均为 2026-09-12（个别本地读取项按原表标注）。

## 一、证据分级与证据状态定义

**证据分级**（合并口径，与各报告原文措辞归纳一致）：

| 等级 | 定义 |
| --- | --- |
| A | 官方文档 / 官方仓库（官网题面、官方 API 文档、官方博客/论坛、官方仓库与官方镜像） |
| B | 官方样例 / 源码 / 测试 / 平台自身源码（含官方仓库的 issue/PR、官方样例的媒体转载） |
| C | 社区文章 / 论坛 / 个人仓库 / 媒体通稿 |
| D | 仅搜索摘要未核验 |

**证据状态**：

| 状态 | 定义 |
| --- | --- |
| verified | 已读取原始内容 |
| partial | 仅摘要 / README / 搜索结果级 |
| unavailable | 访问受限 |
| contradicted | 与来源冲突 |

## 二、合并与登记规则

1. **去重键为 URL**：同 URL 被多个代理引用时合并为一行，"采集代理"与"原编号"列并列登记，备注写明；共合并 6 组（见统计）。
2. **等级/状态冲突处理**：两个代理对同一 URL 给出不同等级或状态时，双方标注均保留并移入"等级冲突/复合条目"组（S192–S195），备注注明冲突内容；不做仲裁、不升级。
3. **原编号对照**：按各报告原表编号登记（如 agent01-S03、agent04-#7、agent07-S11、agent08-S23、agent09-S12）。agent02 的两张来源表无编号列，其本地缓存表按行序记为 agent02-缓存01～缓存25，在线表按行序记为 agent02-在线01～在线14。
4. **agent02 等级口径**：其在线表未设逐条等级列，按该报告自述口径（"官方文档页面/官方文档仓库原文，含 gitcode 官方镜像 = A"）将在线01～在线13 记为 A；其中在线14（昇腾官方博客）agent02 在状态栏自注"C 级辅证"，与 agent05 的 A 级登记构成冲突，移入冲突组（S193）双标注保留。其本地缓存表同理按官方文档缓存记 A（状态按原表）。
5. **URL 归一化**：仅将 `hiascend.com` 与 `www.hiascend.com` 的同路径页面视为同页（S038）；同一文档的不同版本路径（如 MindSpore cann_error_cases 的 r2.6.0 与 r2.6.0rc1、ReduceSum 的 900 社区版/80RC2 商用版/800alpha001 版）URL 不同，不合并，备注注明关联。
6. 原报告中找不到的字段写"—"；原表 URL 含省略号或描述性文字时照录并在备注注明。个别条目原表即含两个 URL（agent01-S06、agent06-#6），按一条登记，备注注明。

## 三、统一来源总表（在线来源，共 197 条）

### A 级（81 条）

| 统一编号 | 原编号 | URL | 标题/内容 | 仓库/版本/commit | 访问日期 | 用途 | 证据等级 | 证据状态 | 采集代理 | 备注 |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| S001 | agent01-S01 | https://cannjudge.cn/public/op_challenge_xinan_prelim/addrmsnormbias | CANNJudge AddRmsNormBias 题面页（HTML） | — | 2026-09-12 | 题面全文、元信息、通过率统计 | A | verified | agent01 | — |
| S002 | agent01-S02 | https://cannjudge.cn/api/problems/6a9a9a99bf41025d6013eb85 | 题目详情 API（desc 原文 Markdown、testcases 15 条、iterations/score_mode/kernel_pattern 等） | — | 2026-09-12 | 题面权威原文、测试点与计分元数据 | A | verified | agent01 | — |
| S003 | agent01-S03 | https://cannjudge.cn/public/op_challenge_xinan_prelim/addrmsnormbias/ranking | 提交排名页（15 列测试点、TBest、前 20 名、共 235 条） | — | 2026-09-12 | 计分与 15 测试点互证、TBest 数据 | A | verified | agent01 | — |
| S004 | agent01-S04 | https://cannjudge.cn/api/problems/6a9a9a99bf41025d6013eb85/ranking | 题目排名 API（result: time/precision_ratio/testcase_status 逐点） | — | 2026-09-12 | 判题结果字段结构 | A | verified | agent01 | — |
| S005 | agent01-S05 | https://cannjudge.cn/api/contests/name/op_challenge_xinan_prelim | 赛事元数据 API（时间/封榜/报名/zone_id/edit_logs 11 条） | — | 2026-09-12 | 赛事规则、时间线、配置字段 | A | verified | agent01 | — |
| S006 | agent01-S06 | https://cannjudge.cn/public/op_challenge_xinan_prelim 与 https://cannjudge.cn/contests | 赛事页与全部赛事列表（26 赛事、五大赛区对照） | — | 2026-09-12 | 赛事规模、赛区结构 | A | verified | agent01 | 一条登记含两个 URL（原表如此） |
| S007 | agent01-S07 | https://cannjudge.cn/api/submissions/contest/6a9a9295bf41025d601255a3/stats | 赛事提交统计 API（summary: 380 人/14299 次；逐队 submissionCount，最高 558） | — | 2026-09-12 | 额度反证、参与规模 | A | verified | agent01 | — |
| S008 | agent01-S11 | https://competition.gitcode.com/competition/2094722165106008066/intro（及 /publish、/ranking、/qa） | GitCode 赛事页（未登录渲染为空，无业务数据；API 探测 301） | — | 2026-09-12 | 报名入口存在性确认 | A（页面本身）/内容 | unavailable（内容需登录/异步加载） | agent01 | — |
| S009 | agent02-在线01 | https://asc.gitcode.com/api/SIMD-API/基础API/Memory矢量计算/数据搬运/DataCopy_GMAndUB_continuous.html | DataCopy（GM 与 UB 连续数据搬运） | asc-devkit 文档（gitcode 官方镜像） | 2026-09-12 | DataCopy 三参原型、count·sizeof(T) 32B 对齐向下取整、A2 数据类型 | A | verified | agent02 | 等级按 agent02 报告口径（官方文档/官方镜像=A） |
| S010 | agent02-在线02 | https://asc.gitcode.com/api/SIMD-API/基础API/Memory矢量计算/数据搬运/DataCopyPad_UBToGM.html | DataCopyPad（UB→GM 非对齐数据搬运） | 同上 | 2026-09-12 | 定案 2 最新版原文（dummy 丢弃）、blockLen∈[0,2097151] 且须为 sizeof(T) 整数倍、官方样例路径 | A | verified | agent02 | — |
| S011 | agent02-在线03 | https://asc.gitcode.com/api/SIMD-API/基础API/Memory矢量计算/归约计算/ReduceSum.html | ReduceSum（gitcode 官方镜像） | 同上 | 2026-09-12 | 定案 3 交叉验证：count 受 UB 限制、repeatTime int32 例外、count=0 NOP（A2）、A2 half/float、workLocal 公式 | A | verified | agent02 | — |
| S012 | agent02-在线04 | https://asc.gitcode.com/api/SIMD-API/基础API/Memory矢量计算/归约计算/WholeReduceSum.html | WholeReduceSum | 同上 | 2026-09-12 | repeatTime/mask 约束（16320 推导依据） | A | verified | agent02 | — |
| S013 | agent02-在线05 | https://asc.gitcode.com/api/SIMD-API/基础API/资源管理/TPipe/InitBuffer.html | InitBuffer | 同上 | 2026-09-12 | InitBuffer 两种签名、len 自动 32B 补齐、Buffer 总数 ≤64、double buffer 语义 | A | verified | agent02 | — |
| S014 | agent02-在线06 | https://asc.gitcode.com/guide/编程指南/编程模型/AI-Core-SIMD编程/核函数.html | 核函数 | 同上 | 2026-09-12 | __global__ + 四种执行空间限定符规范、`__global__ __vector__` 官方示例、<<<>>> ABI | A | verified | agent02 | — |
| S015 | agent02-在线07 | https://asc.gitcode.com/guide/编程指南/编译与运行/算子编译/约束说明.html | 约束说明 | 同上 | 2026-09-12 | Kernel 类型标记建议、bfloat16_t Host 端模板限制（A2 列入支持） | A | verified | agent02 | — |
| S016 | agent02-在线08 | https://asc.gitcode.com/api/SIMD-API/基础API/同步控制/核间同步/核间同步能力概述.html | 核间同步能力概述 | 同上 | 2026-09-12 | __vector__=KERNEL_TYPE_AIV_ONLY、group 配置表 | A | verified | agent02 | — |
| S017 | agent02-在线09 | https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/83RC1alpha002/opdevg/Ascendcopdevg/atlas_ascendc_10_00026.html | C++语言拓展（83RC1alpha002） | CANN 8.3.RC1.alpha002 | 2026-09-12 | 修饰符表版本演进中间点 | A | verified（搜索快照全文） | agent02 | — |
| S018 | agent02-在线10 | https://www.hiascend.com/doc_center/source/zh/CANNCommunityEdition/800alpha001/apiref/ascendcopapi/atlasascendc_api_07_0078.html | ReduceSum（8.0.0.alpha001） | CANN 8.0.0.alpha001 | 2026-09-12 | count 约束跨版本一致性 | A | verified | agent02 | — |
| S019 | agent02-在线11 | https://www.hiascend.com/doc_center/source/zh/CANNCommunityEdition/800alpha001/apiref/ascendcopapi/atlasascendc_api_07_0079.html | WholeReduceMax（8.0.0.alpha001） | CANN 8.0.0.alpha001 | 2026-09-12 | repeatTimes∈[0,255] 佐证 | A | verified | agent02 | — |
| S020 | agent02-在线12 | https://gitcode.com/cann/asc-devkit | 官方文档/样例仓库 cann/asc-devkit | gitcode.com/cann/asc-devkit | 2026-09-12 | 文档仓库定位；DataCopyPad 官方样例 examples/01_simd_cpp_api/03_basic_api/00_data_movement/data_copy_pad_gm2ub_ub2gm | A | verified（仓库页；样例未逐行核读） | agent02 | GitHub 镜像另见 S111（agent05-#31） |
| S021 | agent02-在线13 | https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/900/opdevg/Ascendcopdevg/atlas_ascendc_10_00026.html | C++语言拓展（900） | CANN 9.0.0 | 2026-09-12 | 尝试直取 9.0.0 原文 | A | unavailable | agent02 | cookie 墙拦截，未能直取 |
| S022 | agent03-#1 | https://github.com/hicann/ops-nn | ops-nn（CANN 神经网络算子库，GitCode cann/ops-nn 官方 GitHub 镜像） | master（2026-09 快照，经 API） | 2026-09-12 | add_rms_norm/rms_norm/gemma_rms_norm 源码 | A | verified（读取 README 与多个源码原文） | agent03 | — |
| S023 | agent03-#2 | https://github.com/hicann/ops-nn/blob/v9.0.0/norm/add_rms_norm/ | AddRmsNorm v9.0.0 tag 源码 | tag `v9.0.0` | 2026-09-12 | 确认 9.0.0 版本与 master 核心逻辑一致 | A | verified（diff 基类仅格式差异） | agent03 | — |
| S024 | agent03-#3 | https://gitcode.com/cann/ops-nn | ops-nn 官方主仓（GitCode） | 分支 9.0.0 / master | 2026-09-12 | 官方下载入口 `git clone -b 9.0.0`；PR/讨论链接 | A | partial（经 GitHub 镜像 README 内链确认，页面未直接抓通） | agent03 | — |
| S025 | agent03-#4 | https://github.com/hicann/ops-nn/blob/master/norm/add_rms_norm/op_kernel/add_rms_norm.cpp 等 | add_rms_norm 主入口/五种策略/tiling 全套源码 | master | 2026-09-12 | tiling、尾块、归约、多核、流水摘录 | A | verified（全文读取 .cpp/.h×5/tiling×2） | agent03 | — |
| S026 | agent03-#5 | https://github.com/hicann/ops-nn/blob/master/norm/rms_norm/op_kernel/rms_norm_base.h 与 reduce_common.h | RMSNorm 公共基类与归约件 | master | 2026-09-12 | DataCopyCustom/ReduceSum 系列摘录 | A | verified（全文读取） | agent03 | — |
| S027 | agent03-#6 | https://github.com/hicann/cann-samples | CANN-SAMPLES（官方实战样例仓） | master | 2026-09-12 | 样例覆盖与构建 | A | verified（README + 目录 + CMakeLists） | agent03 | README 单页另见 S125（agent08-S43），URL 不同未合并 |
| S028 | agent03-#7 | https://github.com/hicann/cann-samples/blob/master/Samples/2_Performance/rms_norm_quant_story/ | RmsNormQuant 算子性能优化指南（story） | master | 2026-09-12 | 直调 .asc 渐进优化全链路 | A | verified（README 全文 + 0/5 源码全文） | agent03 | — |
| S029 | agent03-#8 | https://github.com/hicann/cann-samples/blob/master/Samples/2_Performance/kv_rms_norm_rope_cache_story/README.md | KvRmsNormRopeCache MemBase→RegBase 样例 | master | 2026-09-12 | MemBase/RegBase 边界与构建命令 | A | verified（README 全文） | agent03 | — |
| S030 | agent03-#9 | https://github.com/Ascend/op-plugin | op-plugin（torch_npu 插件，ops-transformer 现名） | master | 2026-09-12 | AddRmsNormV2→aclnnInplaceAddRmsNorm 调用链 | A | verified（读取桥接源码与目录） | agent03 | — |
| S031 | agent03-#11 | https://github.com/hicann/ops-nn/blob/master/norm/gemma_rms_norm/README.md | GemmaRmsNorm 算子说明 | master | 2026-09-12 | 公式变体对照（gamma+1） | A | verified | agent03 | — |
| S032 | agent03-#12 | https://github.com/hicann/ops-nn/blob/master/docs/QUICKSTART.md | ops-nn 快速入门（构建命令） | master | 2026-09-12 | `build.sh --pkg --soc= --ops=` 构建入口 | A | verified | agent03 | — |
| S033 | agent04-#13 | https://docs.nvidia.com/deeplearning/transformer-engine-releases/release-1.5/user-guide/api/c/rmsnorm.html | TE rmsnorm.h C API 文档（nvte_rmsnorm_fwd 等） | NVIDIA 官方文档（release 1.5） | 2026-09-12 | API 签名与公式交叉验证 | A | verified | agent04 | — |
| S034 | agent04-#20 | https://pytorch.org/blog/towards-free-normalization-fusing-normalization-into-gemm-and-attention-kernels/ | PyTorch 博客：Towards Free Normalization（Meta，B200） | pytorch.org 官方博客（2026-07-10） | 2026-09-12 | norm 与 GEMM/attention 融合的行业动向 | A | partial（正文已读，结论未用于本题） | agent04 | — |
| S035 | agent05-#15 | https://mlir.llvm.org/docs/Dialects/Linalg/ | MLIR 'linalg' Dialect（含 linalg.reduce/softmax） | mlir.llvm.org | 2026-09-12 | 归约 named op 与 indexing map 表达 | A | verified | agent05 | — |
| S036 | agent05-#16 | https://mlir.llvm.org/docs/Tutorials/transform/Ch0/ | Transform 教程 Ch0：结构化 Linalg 操作 | mlir.llvm.org | 2026-09-12 | vector.reduction/contract、padding-peeling 范式 | A | verified | agent05 | — |
| S037 | agent05-#21 | https://www.hiascend.com:6066/developer/techArticles/20260506-1 | TileLang AscendNPU IR 从入门到专家算子开发知识点 | 昇腾官方技术文章（2026-05-08） | 2026-09-12 | AscendNPU IR=MLIR 底座；TileLang 编译依赖（bisheng/g++/TVM） | A | verified（宣传材料，性能声称未复核） | agent05 | — |
| S038 | agent05-#22 / agent08-S23 | https://www.hiascend.com/document/detail/zh/canncommercial/80RC2/devaids/auxiliarydevtool/atlasopdev_16_0027.html | Ascend C 自定义算子开发实践（msOpGen/msOpST） | CANN 商用版 8.0.RC2 文档 | 2026-09-12 | msopgen 工程骨架（op_host/op_kernel/build.sh）与 msopst ST 流程 | A | verified | agent05、agent08 | 去重合并：两代理同页登记（agent08 记 URL 无 www 前缀，视为同页），等级状态一致 |
| S039 | agent05-#24 | https://www.hiascend.cn/document/detail/zh/canncommercial/800/devaids/opdev/optool/atlasopdev_16_0021.html | 创建算子工程（json 原型说明） | CANN 8.0.0 文档 | 2026-09-12 | 算子原型 JSON 字段（op/input_desc/output_desc/attr） | A | verified | agent05 | — |
| S040 | agent05-#25 | https://www.mindspore.cn/tutorials/experts/zh-CN/r2.3.0/operation/op_custom_ascendc.html | Ascend C 自定义算子开发与使用指南 | MindSpore 2.3.0 教程 | 2026-09-12 | custom_compiler 离线编译（msopgen 封装） | A | verified | agent05 | — |
| S041 | agent05-#26 | https://www.hiascend.com/doc_center/source/zh/CANNCommunityEdition/850/opdevg/BishengCompiler/CANN%E7%A4%BE%E5%8C%BA%E7%89%88%208.5.0%20%E6%AF%95%E6%98%87%E7%BC%96%E8%AF%91%E5%99%A8%E7%94%A8%E6%88%B7%E6%8C%87%E5%8D%97%2001.pdf | 毕昇编译器用户指南 8.5.0 | CANN 社区版 8.5.0 | 2026-09-12 | `bisheng -O2 --npu-arch=dav-2201` 官方命令样例 | A | verified（PDF 摘录） | agent05 | — |
| S042 | agent05-#27 | https://www.hiascend.cn/document/detail/zh/CANNCommunityEdition/80RC3alpha001/devguide/opdevg/BishengCompiler/atlas_bisheng_10_0002.html | 毕昇编译器快速上手 | CANN 8.0.RC3 文档 | 2026-09-12 | 旧式选项 --cce-soc-version/--cce-soc-core-type=VecCore、.cce 混合编译 | A | verified | agent05 | — |
| S043 | agent05-#30 | https://www.hiascend.com/productbulletins/detail/803 | 产品公告：昇腾 CANN 9.0.0 版本发布 | hiascend.com（2026-04-30） | 2026-09-12 | CANN 9.0.0 发布时间与特性（判题目标版本） | A | verified | agent05 | — |
| S044 | agent06-#1 | https://docs.pytorch.org/docs/main/generated/torch.nn.modules.normalization.RMSNorm.html | PyTorch RMSNorm 官方文档 | main（含 c712ec6 引用） | 2026-09-12 | epsilon 位置官方定义 sqrt(eps+mean)；opmath eps 默认 | A | verified | agent06 | — |
| S045 | agent06-#8 | https://developer.nvidia.cn/blog/cuda-math-method-cn/ | CUDA C Programming Guide 附录 H 数学方法 | CUDA 11.x+ | 2026-09-12 | rsqrtf 2 ulp、sqrtf 0-1 ulp、rintf 单指令 | A | verified | agent06 | — |
| S046 | agent06-#13 | https://www.mindspore.cn/mindformers/docs/zh-CN/r1.7.0/advanced_development/precision_optimization.html | MindSpore 大模型精度调优指南 | r1.7.0 | 2026-09-12 | layernorm_compute_type（Norm 计算精度独立配置）、rms_norm_eps 对齐检查项 | A | verified | agent06 | — |
| S047 | agent07-S1 | https://asc.gitcode.com/api/SIMD-API/基础API/Memory矢量计算/Vector逻辑架构/Vector逻辑架构.html | Vector逻辑架构（SIMD-API） | asc-devkit 文档（gitcode 镜像） | 2026-09-12 | A2 UB 192KB/16BG×3bank/行 32B；950 256KB 结构 | A | verified | agent07 | — |
| S048 | agent07-S2 | https://asc.gitcode.com/guide/算子实践参考/SIMD算子性能优化/内存访问/避免UB的bank冲突/avoid_bank_conflict_npu_arch_2201.html | 避免bank冲突（NPU架构版本2201） | asc-devkit 指南 | 2026-09-12 | 48 bank/16BG/每拍每BG一行；冲突类型与地址优化法；msOpProf | A | verified | agent07 | — |
| S049 | agent07-S3 | https://asc.gitcode.com/api/附录/Vector指令理论性能汇总.html | Vector指令理论性能汇总 | asc-devkit 文档 | 2026-09-12 | Add/Mul 128(half)/64(float) elem/cycle；Sqrt 32；Rsqrt 128/64；DataCopy UB→UB 256B/cycle | A | verified | agent07 | — |
| S050 | agent07-S4 | https://asc.gitcode.com/guide/算子实践参考/SIMD算子性能优化/矢量计算/选择低延迟指令-优化归约操作性能.html | 选择低延迟指令，优化归约操作性能 | asc-devkit 指南 | 2026-09-12 | ReduceRepeat 延迟 2-5×Add；二分累加 172 vs 242 cycle；ReduceSum 接口最慢 | A | verified | agent07 | — |
| S051 | agent07-S5 | https://asc.gitcode.com/guide/算子实践参考/SIMD算子性能优化/流水编排/使能DoubleBuffer.html | 开启DoubleBuffer | asc-devkit 指南 | 2026-09-12 | MTE2/V/MTE3 队列并行原理；双缓冲反例（小数据/搬运占比低时收益小或反效果） | A | verified | agent07 | — |
| S052 | agent07-S6 | https://asc.gitcode.com/guide/技术附录/概念原理和术语/内存访问原理/Scalar读写数据.html | Scalar读写数据 | asc-devkit 指南 | 2026-09-12 | GetValue/SetValue=PIPE_S；DataCache 64B 行；自动同步语义与手动事件示例 | A | verified | agent07 | — |
| S053 | agent07-S7 | https://asc.gitcode.com/api/SIMD-API/基础API/同步控制/核内同步/关键特性说明.html | 关键特性说明（自动同步） | asc-devkit 文档 | 2026-09-12 | 自动同步范围（SIMD 核内）；V_S 等 HardEvent 语义 | A | verified（部分正文截断，要点已获取） | agent07 | — |
| S054 | agent07-S8 | https://asc.gitcode.com/api/SIMD-API/基础API/工具接口/系统资源与变量/GetBlockNum.html | GetBlockNum | asc-devkit 文档 | 2026-09-12 | numBlocks 语义；AIC:AIV=1:2 时 AIV=2×numBlocks | A | verified | agent07 | — |
| S055 | agent07-S9 | https://developer.huawei.com/consumer/cn/doc/harmonyos-guides/cannkit-kernel-function | 核函数（Ascend C） | HarmonyOS CANN Kit 文档 | 2026-09-12 | blockDim∈[1,65535]、逻辑核概念 | A | verified | agent07 | — |
| S056 | agent07-S10 | https://www.hiascend.com/doc_center/source/zh/canncommercial/80RC2/apiref/opdevgapi/atlasascendc_api_07_0464.html | SetDim | CANN 8.0.RC2 商用版 | 2026-09-12 | 离散架构"20 AIC/40 AIV"示例（910B3 口径佐证） | A | verified | agent07 | — |
| S057 | agent07-S22 | https://www.hiascend.com/doc_center/source/zh/canncommercial/80RC3/devaids/opdev/optool/atlasopdev_16_0092.html | msProf 工具概述 | CANN 8.0.RC3 官方 | 2026-09-12 | msprof op 参数官方口径（kernel-name/launch-count 等） | A | verified | agent07 | — |
| S058 | agent07-S23 | https://www.hiascend.com/doc_center/source/zh/canncommercial/80RC2/apiref/ascendcopapi/atlasascendc_api_07_0078.html | ReduceSum（8.0.RC3 官方 API） | CANN 商用版 | 2026-09-12 | A2 两种相加方式（前 n 个=方式二；高维切分=方式一） | A | verified（Agent 2 已核，此处仅引用） | agent07 | 900 社区版同文档缓存见 L012；800alpha001 版见 S018 |
| S059 | agent07-S24 | https://www.hiascend.com/doc_center/source/zh/canncommercial/80RC2/apiref/opdevgapi/atlasascendc_api_07_0089.html | WholeReduce | CANN 商用版 | 2026-09-12 | WholeReduceSum 每 repeat 二叉树语义 | A | verified（Agent 2 范畴，仅引用） | agent07 | — |
| S060 | agent08-S1 | hiascend.com/doc_center/source/zh/300Vtest/300VG/300V_0039.html | 安装CANN | 华为官方 | 2026-09-12 | run 包安装步骤、~/.bashrc 持久化 | A | verified | agent08 | — |
| S061 | agent08-S2 | hiascend.com/document/detail/zh/canncommercial/800/softwareinst/instg/instg_0008.html | 安装CANN软件包 | 华为官方 | 2026-09-12 | toolkit/kernels/nnrt 安装顺序、默认路径、版本检查 | A | verified | agent08 | — |
| S062 | agent08-S3 | hiascend.com/doc_center/source/zh/CANNCommunityEdition/80RC2alpha002/devguide/opdevg/ascendcopdevg/atlas_ascendc_10_0002.html | 环境准备 | 华为官方 | 2026-09-12 | 三方依赖、python 版本要求 | A | verified | agent08 | — |
| S063 | agent08-S7 | hiascend.com/zh/hardware/firmware-drivers | 固件与驱动（社区版下载页） | 华为官方 | 2026-09-12 | 当前配套默认（9.1.0/HDK 26.1.1）、产品系列入口 | A | verified | agent08 | — |
| S064 | agent08-S8 | hiascend.com/document/detail/zh/canncommercial/80RC1/softwareinst/instg/instg_0015.html | Atlas 800I A2 安装参考 | 华为官方 | 2026-09-12 | 800I A2 软件包组成（firmware/driver/toolkit/kernels/nnrt/nnae/toolbox） | A | verified | agent08 | — |
| S065 | agent08-S9 | hiascend.com/doc_center/source/zh/mindx-dl/501/dockerruntime/dockerruntimeug/...（MindX DL 5.0.1 Ascend Docker Runtime 用户指南 PDF） | Ascend Docker Runtime 用户指南 | 华为官方 | 2026-09-12 | 容器运行时定义、支持产品、安装与 K8s/Containerd 集成 | A | verified | agent08 | 原表 URL 含省略号，照录 |
| S066 | agent08-S12 | hiascend.com/doc_center/source/zh/mindx-dl/50rc1/dluserguide/clusterscheduling/dlug_guide_03_000133.html | 在iSula客户端使用（Ascend Docker Runtime 参数） | 华为官方 | 2026-09-12 | ASCEND_VISIBLE_DEVICES/NODRV/VNPU_SPECS 参数语义 | A | verified | agent08 | — |
| S067 | agent08-S13 | hiascend.com/doc_center/source/zh/mindx-dl/30rc3/dluserguide/toolboxug/toolboxug_000139.html | Ascend Docker Runtime默认挂载内容 | 华为官方 | 2026-09-12 | 各产品默认挂载表（/dev/davinciX、driver lib64、npu-smi 等） | A | verified | agent08 | — |
| S068 | agent08-S14 | hiascend.com/doc_center/source/zh/canncommercial/800/softwareinst/instg/instg_0059.html（6066 端口镜像） | 手动挂载方式启动容器 | 华为官方 | 2026-09-12 | 完整 docker run 挂载命令、Device 独占规则 | A | verified | agent08 | — |
| S069 | agent08-S16 | hiascend.cn/document/detail/zh/CANNCommunityEdition/800alpha002/apiref/appdevgapi/aclcppdevg_03_0057.html | aclrtSynchronizeDeviceWithTimeout | 华为官方 | 2026-09-12 | 超时语义（-1/毫秒/ACL_ERROR_RT_STREAM_SYNC_TIMEOUT） | A | verified | agent08 | — |
| S070 | agent08-S17 | hiascend.com/developer/blog/details/02160212311885656064 | FunASR+Atlas 300I DUO 部署实践分享 | hw31098709 / 2026-04-23 | 2026-09-12 | 驱固安装顺序、容器完整挂载参数、LD_LIBRARY_PATH | A（官方博客） | verified | agent08 | — |
| S071 | agent08-S19 | openi.org.cn（"芯动开源"openMind 专场活动页） | OpenI 启智社区昇腾算力活动 | OpenI 官方 | 2026-09-12 | 昇腾算力 4 积分/卡时、调试任务模式 | A | verified | agent08 | — |
| S072 | agent08-S21 | hiascend.com/doc_center/source/zh/Atlas 200I A2/24.1.RC2/re/npu/npusmi_007.html | 查询基本信息（npu-smi info） | 华为官方 | 2026-09-12 | npu-smi 输出字段逐项解读 | A | verified | agent08 | — |
| S073 | agent08-S22 | hiascend.com/doc_center/source/zh/canncommercial/81RC1/.../毕昇编译器用户指南.pdf | CANN 商用版 8.1.RC1 毕昇编译器用户指南 | 华为官方 | 2026-09-12 | bisheng 定位、ccec_compiler/bin/bisheng 路径 | A | verified | agent08 | 原表 URL 含省略号，照录；8.5.0 社区版另见 S041 |
| S074 | agent08-S24 | developer.huawei.com/consumer/cn/doc/hiai-guides/cannkit-creating-operator-project-msopgen-0000002293225826 | 算子工程创建工具参数说明 | 华为官方 | 2026-09-12 | msopgen 参数表 | A | verified | agent08 | — |
| S075 | agent08-S29 | mindspore.cn/tutorials/zh-CN/r2.6.0/debug/error_analysis/cann_error_cases.html | CANN常见错误分析 | MindSpore 官方 | 2026-09-12 | EZ9999 总纲、日志环境变量 | A | verified | agent08 | 与 S080（agent09-S14）为同文档不同版本路径（r2.6.0 与 r2.6.0rc1），URL 不同未合并 |
| S076 | agent08-S30 | hiascend.com/document/detail/zh/canncommercial/80RC3/.../troubleshooting_0150.html | AI Core算子执行报错 | 华为官方 | 2026-09-12 | MTE DDR 越界实例、plog 路径 | A | verified | agent08 | 原表 URL 含省略号，照录 |
| S077 | agent08-S31 | hiascend.cn/...（CANNCommunityEdition/800alpha001/.../troubleshooting_0004.html） | AI Core Error问题现象描述 | 华为官方 | 2026-09-12 | EZ9999 打屏格式解读、errorStr=Illegal instruction/unaligned UUB | A | verified | agent08 | 原表 URL 含省略号，照录 |
| S078 | agent08-S32 | hiascend.com/doc_center/source/zh/canncommercial/60RC1/troublemanagement/troubleshooting/troubleshooting_0104.html | Memcpy异步拷贝算子执行报错 | 华为官方 | 2026-09-12 | EI9999、0x0002 地址错误三类原因 | A | verified | agent08 | — |
| S079 | agent08-S46 | hiascend.com/developer/blog/details/02176215666023825269 | 昇腾驱动固件安装指南 | Synapse / 2026-06-01 | 2026-09-12 | 驱动/固件分层、OS 内核版本表、锁内核警告 | A（官方博客） | verified | agent08 | — |
| S080 | agent09-S14 | https://www.mindspore.cn/tutorials/experts/zh-CN/r2.6.0rc1/debug/error_analysis/cann_error_cases.html | CANN常见错误分析（MindSpore 官方） | MindSpore 官方文档 | 2026-09-12 | EB0000 UB 超限、E80012 ReduceSum 维度（错误码日志样本） | A | verified | agent09 | 与 S075 同文档不同版本路径，URL 不同未合并 |
| S081 | agent09-S15 | https://www.hiascend.cn/document/detail/zh/CANNCommunityEdition/800alpha002/apiref/ascendcopapi/atlasascendc_api_07_0215.html | GET_TILING_DATA_WITH_STRUCT（官方 API 参考） | 华为 CANN 官方文档／更新 2025-03-24 | 2026-09-12 | "暂不支持 kernel 直调工程"约束（A 级） | A | verified | agent09 | 900 版同页缓存见 L016 |

### B 级（44 条）

| 统一编号 | 原编号 | URL | 标题/内容 | 仓库/版本/commit | 访问日期 | 用途 | 证据等级 | 证据状态 | 采集代理 | 备注 |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| S082 | agent01-S10 | https://cannjudge.cn/app.js 及 js/services/shared.js、js/pages/open.js、js/pages/problem/editor.js、js/pages/problem/detail.js、js/pages/problem/submission.js（前端源码） | CANNJudge 前端模块（API 端点、npu_kernel_dev 文件校验、提交 payload、封榜逻辑、GitCode 提交表单） | — | 2026-09-12 | 提交接口与限额逻辑逆向 | B（平台自身源码） | verified | agent01 | 公开静态 JS 文件，全程未 POST/登录 |
| S083 | agent03-#10 | https://github.com/Ascend/samples | Ascend/samples（旧官方样例仓） | master（2023-11-22 后停更） | 2026-09-12 | 确认旧 msopgen 样例已迁移 | B | verified（目录级检查，operator/ 无归一化样例） | agent03 | — |
| S084 | agent04-#1 | https://github.com/pytorch/pytorch/blob/main/aten/src/ATen/native/cuda/layer_norm_kernel.cu | PyTorch aten CUDA LayerNorm/RMSNorm kernel | pytorch/pytorch main @ `8671f09631c5`（2026-09-11） | 2026-09-12 | RmsNormKernelImpl、Welford 退化、向量化快路径、网格配置 | B | verified（全文读取） | agent04 | — |
| S085 | agent04-#2 | https://github.com/pytorch/pytorch/blob/main/aten/src/ATen/native/cuda/block_reduce.cuh | PyTorch CUDA block/warp 归约工具头 | pytorch/pytorch main @ `8671f09631c5` | 2026-09-12 | warp shuffle/两级块归约标准实现 | B | verified | agent04 | — |
| S086 | agent04-#3 / agent06-#3 | https://github.com/pytorch/pytorch/pull/195638 | [ATen] Support mixed-dtype weight in fused RMSNorm CUDA kernel（Open PR） | pytorch/pytorch PR，commit `e8a736ec16ce` | 2026-09-12 | gamma 混合精度（bf16 激活/fp32 权重）dispatch 与 FP32 累加佐证；bf16 前向 rel-L2 1.661e-3 vs fp64 数值表 | B | verified | agent04、agent06 | 去重合并：两代理同 URL 同等级同状态（B/verified） |
| S087 | agent04-#4 | https://github.com/vllm-project/vllm/blob/main/csrc/libtorch_stable/layernorm_kernels.cu | vLLM RMSNorm/FusedAddRMSNorm CUDA kernel | vllm-project/vllm main @ `0c1e89ceb92b`（2026-09-11） | 2026-09-12 | fused_add_rms_norm 全文、host 启发式、cub BlockReduce | B | verified（全文读取；旧路径 csrc/layernorm_kernels.cu 已迁移） | agent04 | — |
| S088 | agent04-#5 | https://github.com/vllm-project/vllm/blob/main/csrc/libtorch_stable/quantization/vectorization_utils.cuh | vLLM 向量化读写工具（前缀-主体-尾块） | vllm-project/vllm main @ `0c1e89ceb92b` | 2026-09-12 | 尾块三段式处理模式 | B | verified | agent04 | — |
| S089 | agent04-#6 | https://github.com/vllm-project/vllm/blob/main/csrc/libtorch_stable/type_convert.cuh | vLLM half/bf16/float 转换与 packed 类型 | vllm-project/vllm main @ `0c1e89ceb92b` | 2026-09-12 | `_f16Vec`/packed 数学、CUDA<12 与 sm<80 的 bf16 缺失 | B | verified | agent04 | — |
| S090 | agent04-#9 | https://github.com/NVIDIA/TransformerEngine/blob/main/transformer_engine/common/normalization/rmsnorm/rmsnorm_fwd_kernels.cu | TE RMSNorm 前向核（tuned+general） | NVIDIA/TransformerEngine main @ `224f6ecf5e8d`（2026-09-11） | 2026-09-12 | 寄存器驻留、Ktraits 平铺、跨 CTA allreduce、尾块守卫 | B | verified（全文读取） | agent04 | — |
| S091 | agent04-#10 | https://github.com/NVIDIA/TransformerEngine/blob/main/transformer_engine/common/normalization/rmsnorm/rmsnorm_fwd_cuda_kernel.cu | TE RMSNorm 前向 launch 配置 | 同上 | 2026-09-12 | occupancy 网格、cooperative launch、barrier/workspace 尺寸 | B | verified | agent04 | — |
| S092 | agent04-#11 | https://github.com/NVIDIA/TransformerEngine/blob/main/transformer_engine/common/normalization/rmsnorm/rmsnorm_api.cpp | TE RMSNorm C API（fwd/bwd/bwd_add） | 同上 | 2026-09-12 | 确认前向无 residual（fused add 仅在 bwd） | B | verified | agent04 | — |
| S093 | agent04-#12 | https://github.com/NVIDIA/TransformerEngine/blob/main/transformer_engine/common/utils.cuh | TE Stats/Reducer/InterCTASync（Welford+跨CTA） | 同上 | 2026-09-12 | warp_chan_upd_dynamic、InterCTASync 自旋 barrier | B | verified | agent04 | — |
| S094 | agent04-#14 | https://github.com/flashinfer-ai/flashinfer/blob/main/include/flashinfer/norm.cuh | flashinfer norm kernels（RMSNorm/FusedAddRMSNorm/Quant） | flashinfer-ai/flashinfer main @ `c05407ceffb7`（2026-09-11） | 2026-09-12 | smem_x 驻留 y、两级归约、PDL、vec 边界守卫 | B | verified（全文读取） | agent04 | — |
| S095 | agent04-#15 | https://github.com/linkedin/Liger-Kernel/blob/main/src/liger_kernel/ops/rms_norm.py | Liger-Kernel Triton RMSNorm | linkedin/Liger-Kernel main @ `95b01e94027c`（2026-09-10） | 2026-09-12 | Triton 整行单块、mask 尾块、casting 模式 | B | verified（全文读取） | agent04 | — |
| S096 | agent04-#17 | https://github.com/unslothai/unsloth/blob/fd753fed99ed5f10ef8a9b7139588d9de9ddecfb/unsloth/kernels/rms_layernorm.py | unsloth RMS LayerNorm Triton kernel | unslothai/unsloth @ `fd753fed99ed5f10ef8a9b7139588d9de9ddecfb`（Liger 头部指定） | 2026-09-12 | Liger 上游最简形态、HF 精度复刻行为 | B | verified | agent04 | — |
| S097 | agent04-#18 | https://github.com/triton-lang/triton/blob/main/python/tutorials/05-layer-norm.py | Triton 官方教程 05-Layer Normalization | triton-lang/triton main @ `7fe90e2150e6`（2026-09-11） | 2026-09-12 | 分块循环两/三遍扫描（大 D 通用解）、mask 尾块 | B | verified（全文读取） | agent04 | — |
| S098 | agent04-#19 | https://github.com/ROCm/pytorch/pull/3564 | [ROCm] Optimize AMD normalization backward kernel（tiled） | ROCm/pytorch（pytorch fork）PR | 2026-09-12 | ROCm 侧 norm 内核优化动向（M>>N 场景） | B（diff） | partial | agent04 | — |
| S099 | agent05-#4 | https://github.com/apache/tvm-rfcs/blob/main/rfcs/0005-meta-schedule-autotensorir.md | RFC 0005: Meta Schedule | apache/tvm-rfcs | 2026-09-12 | MetaSchedule 搜索空间设计与统一 API | B | verified | agent05 | — |
| S100 | agent05-#5 | https://arxiv.org/html/2406.20037v2/ | Explore as a Storm, Exploit as a Raindrop（Ansor 调优改进） | arXiv 2406.20037v2 | 2026-09-12 | Ansor 草图生成+注释+演化搜索机制 | B | partial（摘要与正文首节） | agent05 | — |
| S101 | agent05-#7 | https://github.com/apache/tvm/pull/19425 | [Backend][Relax] Add NPU BYOC backend example | apache/tvm PR#19425（2026-04 合入） | 2026-09-12 | TVM 主线无真实 Ascend 后端的佐证 + BYOC 四步流程 | B | verified | agent05 | — |
| S102 | agent05-#8 | https://github.com/apache/tvm-vta | VTA Hardware Design Stack | apache/tvm-vta | 2026-09-12 | 片上 buffer 显式 scope 规划的早期实践对照 | B | verified | agent05 | — |
| S103 | agent05-#9 | https://triton-lang.org/main/getting-started/tutorials/06-fused-attention.html | Triton 官方教程：Fused Attention | triton-lang.org | 2026-09-12 | Triton 归约+融合 kernel 组织（旁证） | B | verified | agent05 | — |
| S104 | agent05-#11 | https://github.com/NVIDIA/TensorRT-LLM/blob/main/.claude/skills/kernel-triton-writing/SKILL.md | Triton Kernel Writing（SKILL） | NVIDIA/TensorRT-LLM | 2026-09-12 | Triton 不自动提升 fp16/bf16 累加精度、超越函数 fp32 规则 | B | verified | agent05 | — |
| S105 | agent05-#14 | https://github.com/pytorch/pytorch/pull/190808 | [inductor][NVGEMM] Fuse pointwise epilogues into scaled GEMM | pytorch PR#190808（2026-07） | 2026-09-12 | epilogue fusion 的模板物化机制 | B | verified | agent05 | — |
| S106 | agent05-#17 | https://www.llvm.org/devmtg/2023-10/slides/techtalks/Warzynski-Caballero-VectorizationinMLIR.pdf | Vectorization in MLIR（LLVM Dev 2023） | llvm.org | 2026-09-12 | 渐进式向量化四步与 masked transfer_read | B | verified | agent05 | — |
| S107 | agent05-#18 | https://arxiv.org/html/2602.19762 | Hexagon-MLIR: An AI Compilation Stack for Qualcomm NPUs | arXiv 2602.19762（2026-02） | 2026-09-12 | Triton 行 softmax→linalg→NPU 全链 lowering 实例 | B | verified | agent05 | — |
| S108 | agent05-#19 | https://github.com/tile-ai/tilelang | Tile Language（主仓后端支持表） | tile-ai/tilelang | 2026-09-12 | Ascend 后端列为 Ecosystem 级（A2/A3） | B | verified | agent05 | — |
| S109 | agent05-#20 | https://github.com/tile-ai/tilelang-ascend | TileLang-Ascend（ascendc_pto/npuir 双路线） | tile-ai/tilelang-ascend（2025-09 开源） | 2026-09-12 | 显式 UB/L1/L0C 原语、TL_ASCEND_MEMORY_PLANNING/AUTO_SYNC、softmax/normalization/reduce 示例、CANN≥8.3.RC1 约束 | B | verified | agent05 | 其 issue#1195 另见 S174（agent09-S07） |
| S110 | agent05-#23 | https://github.com/Ascend/msopgen | MindStudio Ops Generator | Ascend/msopgen（2025-12 开源） | 2026-09-12 | msopgen 开源状态与功能表 | B | verified | agent05 | — |
| S111 | agent05-#31 | https://github.com/hicann/asc-devkit | Ascend C（asc-devkit） | hicann/asc-devkit（v9.x） | 2026-09-12 | Ascend C 开发工具包开源仓（版本对应） | B | verified | agent05 | GitCode 主仓另见 S020（agent02-在线12） |
| S112 | agent06-#2 | https://github.com/pytorch/pytorch/issues/189581 | bf16 RMSNorm 小方差归一化常数错误（偏差 1.718x） | 2026-07-10 开 | 2026-09-12 | bf16 低精度累加实证（D=32, std=0.058） | B | verified | agent06 | — |
| S113 | agent06-#4 | https://github.com/pytorch/pytorch/issues/170758 | bf16 LayerNorm 大数值输出全错 | 2025-12-18 开 | 2026-09-12 | 大数值低精度链灾难案例（1e10→-114688） | B | verified | agent06 | — |
| S114 | agent06-#5 | https://github.com/pytorch/helion/pull/1983 | Relax rms_norm tolerance for Pallas bf16 | 7433e64 | 2026-09-12 | bf16 rms_norm 1e-3 容差下 14/8.4M 失配、max 0.03125（1 ulp） | B | verified | agent06 | — |
| S115 | agent06-#9 | https://github.com/openxla/xla/issues/40862 | XLA rsqrt f64 1 ULP off（Blackwell sweep） | 2026-04-14 开 | 2026-09-12 | rsqrt 与 1/sqrt 位级差异实证（26% 样本 1 ULP） | B | verified | agent06 | — |
| S116 | agent07-S11 | https://blog.csdn.net/gitblog_00924/article/details/157925888 | CANN/asc-devkit：DataCopy内存访问最佳实践样例 | CSDN 官方转载 asc-devkit | 2026-09-12 | A2 实测：分块粒度 3 档（548/221/203μs）、非对齐 -21.6%/-47.5%、1.8TB/s 口径、L2 192MB、512B 对齐建议、Block Num=48 | B | verified | agent07 | — |
| S117 | agent07-S12 | https://blog.csdn.net/gitblog_00754/article/details/157048953 | CANN/asc-devkit：Add性能调优样例 | CSDN 官方转载 asc-devkit | 2026-09-12 | A2 实测 Case0-5：多核 306μs、大块 -12.5%、双缓冲 -1.7%、L2 bypass -28.9%；指标字段表；Case6 截断 | B | verified（Case6 数字缺失） | agent07 | — |
| S118 | agent07-S13 | https://blog.csdn.net/gitblog_00445/article/details/151381043 | CANN/asc-devkit：Matmul最佳实践样例 | CSDN 官方转载 asc-devkit | 2026-09-12 | HBM≈1.6TB/s、L2≈5TB/s（A2 口径）；AIC 侧指标字段 | B | verified（摘要为主，关键数字在正文可见） | agent07 | — |
| S119 | agent07-S14 | https://blog.csdn.net/gitblog_00344/article/details/152106814 | CANN/cann-outreach：Atlas A2与A3架构对比 | CSDN 官方转载 cann-outreach | 2026-09-12 | A2=ASCEND910B/DAV_2201；910B2：24 Cube/1.8GHz/UB 192KB/Vector fp16 22T；L1/L0/UB/L2 容量表 | B | verified | agent07 | — |
| S120 | agent07-S15 | http://raw.githubusercontent.com/Project-HAMi/ascend-device-plugin/refs/heads/main/ascend-device-configmap.yaml | HAMi ascend-device-configmap | HAMi 官方仓库 raw | 2026-09-12 | 910B2 aiCore 24、910B3/B4 aiCore 20（SKU 核数） | B | verified | agent07 | — |
| S121 | agent07-S16 | https://bbs.huaweicloud.com/blogs/5a9291facf6e43148d16ef69a63dc97c | CANN学习资源开源仓的算子调试三msProf及仿真 | 华为云社区（cann-learning-hub 配套） | 2026-09-12 | msprof op 参数（warm-up/launch-count 1~5000）；8 个 CSV 清单；1.8TB/s 与 11.06TOPS 理论公式；MTE2/MTE3 共享带宽 | B | verified | agent07 | — |
| S122 | agent07-S17 | https://blog.csdn.net/gitblog_00297/article/details/157004129 | CANN/cannbot-skills：上板性能采集与调优 | CSDN 官方转载 cannbot-skills | 2026-09-12 | msprof 采集命令模板；核间均衡 <10% 判定；Elementwise vec_ratio 期望；"910B: 20~40 核" | B | verified | agent07 | — |
| S123 | agent08-S15 | github.com/PaddlePaddle/PaddleNLP/blob/develop/llm/devices/npu/llama/README.md | 使用 PaddleNLP 在 NPU 下跑通 llama2-13b 模型 | PaddlePaddle 官方 | 2026-09-12 | `lspci \| grep d802` 验证 910B、容器 ASCEND_RT_VISIBLE_DEVICES | B | verified | agent08 | — |
| S124 | agent08-S25 | github.com/MinghuasLab/flash-attention-npu/issues/56 | [INSTALL]: Compile error when installing csrc/flash_attn_npu_v3 | Riht5 / 2026-07-13 | 2026-09-12 | bisheng 完整命令行、dav-c220/2201、ASCPLUGIN 回退行为 | B | verified | agent08 | — |
| S125 | agent08-S43 | github.com/hicann/cann-samples/blob/master/README.md | CANN-SAMPLES | hicann 官方样例仓 | 2026-09-12 | CANN 9.0.0 时间戳 20260422000325096 验证 PASS | B | verified | agent08 | 仓库级登记另见 S027（agent03-#6），URL 不同未合并 |

### C 级（65 条）

| 统一编号 | 原编号 | URL | 标题/内容 | 仓库/版本/commit | 访问日期 | 用途 | 证据等级 | 证据状态 | 采集代理 | 备注 |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| S126 | agent01-S09 | https://blog.csdn.net/csdn_codechina/article/details/164369653 | 最高 2 万元奖金＋华为校招绿卡！2026 CANN 挑战赛报名启动（AtomGit 官方账号，2026-09-04） | — | 2026-09-12 | 三阶段赛制、奖金、报名/决赛时间线 | C（官方社区账号通稿转载） | verified | agent01 | — |
| S127 | agent03-#13 | 搜索引擎结果页（CSDN gitblog 对 ops-nn InplaceAddRmsNorm 的转载、hicann/ops-nn op_list.md 等） | 辅助定位 | — | 2026-09-12 | 仓库/算子发现线索 | C | partial（仅摘要，结论均回到官方源码核实） | agent03 | 原表无具体 URL，照录 |
| S128 | agent04-#7 | https://github.com/vllm-project/vllm/issues/43390 | [Bug]: integer overflow in fused_add_rms_norm | vllm-project/vllm issue（2026-05-22） | 2026-09-12 | int32 索引溢出风险案例（`blockIdx.x*vec_hidden_size+idx`） | C | verified（issue 正文） | agent04 | — |
| S129 | agent04-#8 | https://github.com/vllm-project/vllm/issues/41430 | fused_add_rms_norm 不支持 weight=None 分支 | vllm-project/vllm issue（2026-05-01） | 2026-09-12 | 融合核参数面（has_weight 分支）佐证 | C | partial | agent04 | — |
| S130 | agent05-#1 | https://discuss.tvm.apache.org/t/optimizing-reduction-initialization-in-generated-cuda-code/18613 | Optimizing Reduction Initialization in Generated CUDA Code | TVM Discourse 帖 18613 | 2026-09-12 | 归约 init 的 lowering 形态与 decompose_reduction 取舍 | C | verified | agent05 | — |
| S131 | agent05-#2 | https://discuss.tvm.apache.org/t/how-to-keep-data-in-local-buffer-between-matrix-and-vector-ops-avoid-extra-global-memory-copies/18797 | How to keep data in local buffer between matrix and vector ops | TVM Discourse 帖 18797 | 2026-09-12 | 片上 buffer 避免回落 GM 的调度问题（与 UB 复用同源） | C | verified | agent05 | — |
| S132 | agent05-#3 | https://tvm.d2l.ai/chapter_gpu_schedules/conv.html | d2l-tvm: Convolution（GPU 调度） | d2l-tvm 教程 | 2026-09-12 | shared/local tiling 与 tvm_if_then_else 边界零填充 | C | verified | agent05 | — |
| S133 | agent05-#6 | https://www.cs.cmu.edu/~zhihaoj2/15-779/slides/09-ML-compilers-part-2.pdf | CMU 15-779 Lecture 9: Kernel Autotuning | CMU 课程幻灯 2025-09 | 2026-09-12 | Triton/AutoTVM/Ansor 搜索空间对比表 | C | verified | agent05 | — |
| S134 | agent05-#10 | https://blog.csdn.net/ouliten/article/details/160897982 | Triton 笔记 3：融合 Softmax | CSDN（2026-05） | 2026-09-12 | 行归约+epilogue 模式中文详解（BLOCK_SIZE 2 的幂、mask、精度提升） | C | verified | agent05 | — |
| S135 | agent05-#12 | https://jvoltci.github.io/mosaic/ml-execution/roofline-profiling/torch-compile-fusion/ | Inductor Fusion Heuristics | jvoltci.github.io | 2026-09-12 | Inductor 融合规则表（Pointwise↔Reduction prologue/epilogue；RMSNorm 例子） | C | verified | agent05 | — |
| S136 | agent05-#13 | https://calwoo.github.io/notes/concepts/pytorch-internals/torch-compile/inductor/ | TorchInductor: Deep Dive | calwoo.github.io | 2026-09-12 | ir.Pointwise/ir.Reduction、persistent vs looped reduction codegen | C | verified | agent05 | — |
| S137 | agent05-#29 | https://bbs.huaweicloud.com/blogs/92700f7e560b41e79a12b83c96bf0af8 | CANN 学习资源开源仓：算子开发二 Tiling 和 CMake | 华为云社区（2026-03-26） | 2026-09-12 | find_package(ASC)+LANGUAGES ASC、--npu-arch=dav-2201 CMake 集成、两级并行 tiling 说明 | C | verified | agent05 | — |
| S138 | agent05-#32 | https://blog.csdn.net/xyz3120/article/details/161866077 | 昇腾 950 cv 融合算子体验 | CSDN（2026-06） | 2026-09-12 | CANN 9.0.0 环境 + asc-devkit v9.0.0 源码版本对照、__NPU_ARCH__ 映射旁证 | C | verified | agent05 | — |
| S139 | agent05-#33 | https://blog.csdn.net/gitblog_00820/article/details/151949080 | CANN/asc-devkit AI CPU 算子编译指南 | CSDN（2026-05） | 2026-09-12 | bisheng 编译 .asc/.aicpu 的选项表（--npu-arch 等） | C | verified | agent05 | — |
| S140 | agent05-#34 | https://github.com/endaiHW/tilelang-mlir-ascend | tilelang-mlir-ascend（fork） | endaiHW fork of tile-ai | 2026-09-12 | AscendNPU IR 分支活跃度旁证 | C | verified | agent05 | — |
| S141 | agent06-#7 | https://handwiki.org/wiki/Pairwise_summation | Pairwise summation（Higham 误差界转述） | Higham 1993 | 2026-09-12 | O(n·ε) vs O(log n·ε) vs Kahan O(ε) | C | verified | agent06 | — |
| S142 | agent06-#10 | https://hwcomputing.csdn.net/6a156b8510ee7a33f2754f4b.html | CANN ops-transformer：RMSNorm 算子数值精度分析 | 2026-05-26 | 2026-09-12 | fp16 平方先溢出后 cast 已晚；乘法前 cast FP32 | C | verified | agent06 | — |
| S143 | agent06-#11 | https://blog.csdn.net/2401_82857325/article/details/155560947 | 昇腾 CANN 训练营：RMSNorm Ascend C 开发 | 2025-12-04 | 2026-09-12 | ReduceSum 必须 FP32 累加"黄金法则"；32K 需分段汇总 | C | verified | agent06 | — |
| S144 | agent06-#12 | https://xie.infoq.cn/article/acee34538e3422096a0e9dbee | 国产芯片大模型精度排查（DeepLink，A2） | 2026-01-14 | 2026-09-12 | A2 上 rms_norm 精度问题案例；浮点求和顺序不确定性与确定性开关 | C | verified | agent06 | — |
| S145 | agent06-#14 | https://github.com/anviit/triton-llm-kernels | triton-llm-kernels（RMSNorm ATOL 表） | main | 2026-09-12 | RMSNorm fp16 ATOL 5e-3 理由"fp16 rounding accumulates with D" | C | verified | agent06 | — |
| S146 | agent06-#15 | https://build.nvidia.com/station/kernel-dev-ft/instructions | NVIDIA Station：RMSNorm Triton 内核课 | — | 2026-09-12 | bf16 RMSNorm 放宽容差测试（max diff 1.56e-02 PASSED） | C | verified | agent06 | — |
| S147 | agent07-S18 | https://ascendai.csdn.net/696c53eca16c6648a9832a4b.html | AscendC算子代码阅读指南 | CSDN（昇腾专区） | 2026-09-12 | ub_size=196608B=192KB；910B1~B4 核数表（引 cann/runtime platform_config）；SetFlag/WaitFlag 机制 | C | verified | agent07 | — |
| S148 | agent07-S20 | https://github.com/Aeolion-mu/ops/blob/main/mamba2_fusion_report.md | Mamba-2 2.7B 算子融合实验报告 | 社区仓库（北京昇腾研发部署记录） | 2026-09-12 | 910B4 kernel launch ~280μs 端到端下限（口径参照）；aclnnReduceSum 226μs 等参照 | C | verified | agent07 | — |
| S149 | agent07-S21 | https://blog.csdn.net/jieph01/article/details/164574099 | AscendC DataCopyPad 32 字节对齐报错：尾块搬运方案 | CSDN（昇腾知识图谱检索文） | 2026-09-12 | DataCopyExtParams blockCount/blockLen/stride 单位口径；尾块拆分方案（主体 DataCopy+尾 DataCopyPad） | C | verified | agent07 | — |
| S150 | agent08-S4 | blog.csdn.net/zhangfeng1133/article/details/163783783 | 华为 CANN 9.0.0 9.2.0 Ubuntu x86_64 A2芯片 安装指南 | zhangfeng1133 / 2026-08-22 | 2026-09-12 | apt 一键安装命令、离线 run 包、验证命令 | C | verified | agent08 | — |
| S151 | agent08-S5 | blog.csdn.net/zhangfeng1133/article/details/163774160 | cann 9.0.0 安装同时支持 a2 a3 910 950pr 的仿真环境 | zhangfeng1133 / 2026-08-15 | 2026-09-12 | A2=910b 包名体系、cannsim 仅支持 950PR、CPU 孪生调试 | C | verified | agent08 | — |
| S152 | agent08-S10 | ascendai.csdn.net/695243d7836da32144889114.html | Ascend昇腾设备上启容器时映射NPU，解决无法找到npu、无法使用npu-smi info的情况 | m0_52182894 / 2025-12-29 | 2026-09-12 | 手动挂载设备/工具/库清单、libc_sec.so 报错、HwHiAiUser 组权限 | C | verified | agent08 | — |
| S153 | agent08-S11 | cann.csdn.net/6a473ffe10ee7a33f28753b5.html | 华为 算子开发中 cann 环境变量 有多个，怎么设置 | zhangfeng1133 / 2026-07-03 | 2026-09-12 | 多 set_env.sh 选择、latest 软链、atc 版本查看 | C | verified | agent08 | — |
| S154 | agent08-S18 | blog.csdn.net/qq_41823532/article/details/158773506 | 使用免费的OpenI启智平台开发昇腾NPU算子 | 匿名 / 2026-03-07 | 2026-09-12 | OpenI 支持 910/910B、cce demo、bisheng 两种编译选项 | C | verified | agent08 | — |
| S155 | agent08-S20 | juejin.cn/post/7631852793092128768 | 实测：1199元 Atlas 200I DK A2 跑 ResNet50，代码零修改迁移华为云 910B | 墨睿思MORES / 2026-04-23 | 2026-09-12 | ModelArts 910B4 按需约 30 元/小时 | C | verified | agent08 | — |
| S156 | agent08-S27 | blog.csdn.net/xyz3120/article/details/149570493 | Ascendc helloworld编译问题 | xyz3120 / 2025-07-24 | 2026-09-12 | No CMAKE_CXX_COMPILER、cstdint not found | C | verified | agent08 | — |
| S157 | agent08-S28 | blog.csdn.net/futao1229/article/details/144127705 | 昇腾Ascend C算子开发测试时卡死问题分析与解决 | futao1229 / 2024-11 | 2026-09-12 | SetFlag/WaitFlag 未配对致核异常与状态残留 | C | verified | agent08 | — |
| S158 | agent08-S33 | blog.csdn.net/jieph01/article/details/149277585 | 昇腾FAQ-A01-硬件相关 | jieph01 / 2025-07-11 | 2026-09-12 | dcmi -8005、驱动包环境不匹配、固驱升级顺序 | C | verified | agent08 | — |
| S159 | agent08-S34 | blog.csdn.net/2502_94138550/article/details/155002287 | 保姆级避坑！Ascend C 算子开发环境搭建实操指南 | 匿名 / 2025-11-20 | 2026-09-12 | compiler 组件缺失、Docker 镜像 ascendhub 拉取 | C | verified | agent08 | — |
| S160 | agent08-S35 | blog.csdn.net/qq_46207024/article/details/156979140 | 环境配置最佳实践：CANN版本兼容性与依赖管理 | 匿名 / 2026-01-15 | 2026-09-12 | 先定驱动再定 CANN、版本强耦合 | C | verified | agent08 | — |
| S161 | agent08-S36 | v2ex.com/t/1176018 | 国产显卡有没有好的安装驱动"姿势" | couture / 2025-12-01 | 2026-09-12 | Atlas 300I Duo 内核兼容坑 | C | verified | agent08 | — |
| S162 | agent08-S37 | v2ex.com/t/1146404 | 居然有个叫摩尔线程的国产 GPU，孤陋寡闻了 | yagamil / 2025-07-20 | 2026-09-12 | 国产卡生态对照（含昇腾 CANN） | C | verified | agent08 | — |
| S163 | agent08-S38 | v2ex.com/t/1231122 | 昇腾 怎么感觉是虚假宣传吗？ | DeYiAo / 2026-07-31 | 2026-09-12 | 910B 适配周期、底层重编译讨论 | C | verified | agent08 | — |
| S164 | agent08-S39 | v2ex.com/t/1204827 | 传梁文锋内部发声，DeepSeek V4 将于 4 月下旬发布 | xiangqiankan / 2026-04-10 | 2026-09-12 | DeepSeek↔昇腾适配舆论（不作技术依据） | C | verified | agent08 | — |
| S165 | agent08-S40 | v2ex.com/t/1218631 | 需要购买国产显卡本地部署大模型，哪家的比较好 | Flagship9945 / 2026-06-08 | 2026-09-12 | Atlas 部署/微调社区体验 | C | verified | agent08 | — |
| S166 | agent08-S41 | github.com/richardokonicha/TurboQuant/blob/main/docs/backend/CANN.md | llama.cpp for CANN | 社区（fork） | 2026-09-12 | 设备号表 d802/d803/d500→产品映射 | C | verified | agent08 | — |
| S167 | agent08-S42 | blog.csdn.net/singgel/article/details/153818511 | GPU 进阶 华为昇腾 910B GPU 相关 | singgel / 2025-10-24 | 2026-09-12 | npu-smi 23.0.rc2 实机输出样例、EulerOS/hccn_tool | C | verified | agent08 | — |
| S168 | agent08-S45 | hwcomputing.csdn.net/69b236b60a2f6a37c596c1c5.html | 昇腾-mindie环境搭建 | kingcjh97 / 2026-03-12 | 2026-09-12 | version.info 检查法、lspci d802/d803 判型 | C | verified | agent08 | — |
| S169 | agent09-S01 | https://www.hiascend.com/app-forum/topic-detail/0272153064709468009 | 昇腾AI原生创新算子挑战赛S1——CNASP坤坤爱曼巴队经验分享 | b1ankcat／2024-06-06 | 2026-09-12 | 提交前 build 检查教训（两题 0 分）、通用模板策略 | C | verified | agent09 | 结论性资料 |
| S170 | agent09-S02 | https://blog.csdn.net/gitblog_00702/article/details/151517425 | CANN复旦赛算子实现（ClipByValue/Lerp/Addcmul） | 潘正浩（MrWine）／2026-06-07（原作 GitCode cann/cann-ops-competitions） | 2026-09-12 | WA 案例：绕过输出队列、公式改写、int32 中转；过度开核阈值 | C（转载；原仓 B） | verified | agent09 | — |
| S171 | agent09-S03 | https://blog.csdn.net/gitblog_01418/article/details/150380401 | CANN学习中心：as_strided算子实战（CANNJudge 开放题库） | cann-learning-hub 实践记录／2026-05-20 | 2026-09-12 | CE（命名空间）、UB 192KB、标量黑洞、erf 4 次提交、踩坑全景 | C（转载；原仓 B） | verified | agent09 | — |
| S172 | agent09-S04 | https://blog.csdn.net/ferriswym/article/details/162206787 | 【昇腾/AscendC开发】AscendC DataCopyPad 写出溢出 Bug 详解 | ferriswym／2026-06-22 | 2026-09-12 | 32B burst padding 溢出覆盖相邻段（尾块风险核心同构案例） | C | verified | agent09 | — |
| S173 | agent09-S05 | https://github.com/hw-native-sys/simpler/issues/517 | [Bug] Build fails with CANN 8.5.1: BLK macro collision in tensor.h | Hzfengsy／2026-04-10 | 2026-09-12 | 宏冲突 CE 完整日志（V001 同类外部佐证） | C | verified | agent09 | — |
| S174 | agent09-S07 | https://github.com/tile-ai/tilelang-ascend/issues/1195 | T.copy(gm[a:b,i], ub) 与预期不一致 | fengz72／2026-06-16 | 2026-09-12 | GM→UB 非连续搬运 pad 0（CANN 9.0.0） | C | verified | agent09 | 仓库级登记另见 S109（agent05-#20） |
| S175 | agent09-S08 | https://github.com/vllm-project/vllm-ascend/issues/1682 | gather_v3 断言 Index out of range | JackeyGuo／2025-07-09 | 2026-09-12 | 运行期越界断言样本 | C | verified | agent09 | — |
| S176 | agent09-S09 | https://github.com/Just-it/AscendOpGenAgent/issues/83 | EmbeddingDenseBackward 越界索引导致精度测试不稳定 | zhshgmail／2026-04-14 | 2026-09-12 | 测试数据缺陷致"正确实现"误判 WA | C | verified | agent09 | — |
| S177 | agent09-S10 | https://github.com/Just-it/AscendOpGenAgent/pull/132 | cumsum修复+修复提前退出（含 NPU 精度问题与 Workaround） | chopper0126／2026-04-27 | 2026-09-12 | fp16 并行扫描非确定性（0.1–0.5% 波动/大 tensor 10% 偏离） | C | verified | agent09 | — |
| S178 | agent09-S11 | https://github.com/Just-it/AscendOpGenAgent/pull/149 | 对齐CPU标杆，优化cumsum算子精度 | jianhuang163／2026-04-29 | 2026-09-12 | 参考实现并行归约与串行 kernel 的偏差处理 | C | verified | agent09 | — |
| S179 | agent09-S13 | https://blog.csdn.net/2301_80840905/article/details/155034892 | 避坑指南：Ascend C算子开发常见报错解析与精度优化复盘 | CSDN 作者／2025-11-19 | 2026-09-12 | PipeBarrier 缺失、FreeTensor 死锁、32B 对齐、Tiling 结构体错位、fp16 累加 | C | verified | agent09 | — |
| S180 | agent09-S16 | https://www.hiascend.com/developer/blog/details/02192216483616286018 | Ascend C 算子开发典型问题：Segmentation fault 根因分析 | 昇腾社区博客（匿名）／2026-06-10 | 2026-09-12 | 段错误五大类分类学（主机/GM/LM/DMA/指令对齐） | C | partial | agent09 | — |
| S181 | agent09-S17 | https://blog.csdn.net/gitblog_00561/article/details/145252761 | CANN/asc-tools 同步事件管理完整指南 | asc-tools 文档转载／2026-05-30 | 2026-09-12 | ErrorSync1–4 分类；npuchk 排查流程；cpu_debug 工具 | C（转载；原仓 B） | verified | agent09 | — |
| S182 | agent09-S18 | https://blog.csdn.net/gitblog_00127/article/details/151604873 | CANN Erf算子优化作品（东南大学，江山赛区预选赛） | 关注塔菲喵队／2026-07-02（原作 cann/cann-ops-competitions） | 2026-09-12 | Direct/Medium/Large 分路径 15/15 Pass 正面样板 | C（转载；原仓 B） | verified | agent09 | — |
| S183 | agent09-S19 | https://jsj.nwpu.edu.cn/info/1598/23245.htm | 从小白到大赛金奖，西工大学子勇闯算子开发探索之路 | 西工大计算机学院／2024-10-12 | 2026-09-12 | S2 金奖经验（结论性） | C | verified | agent09 | 结论性资料 |
| S184 | agent09-S20 | https://we.yesky.com/blog/326258 | 自学1个月拿金奖！北交大学子分享昇腾算子挑战赛秘籍 | 天极网／2024-10-14 | 2026-09-12 | S2 策略：基础解法→性能优化两阶段；实时榜单 | C | verified | agent09 | 结论性资料 |
| S185 | agent09-S21 | https://blog.csdn.net/pz890123/article/details/152697389 | 2025昇腾训练营避坑指南 | CSDN 作者／2026-03-02 | 2026-09-12 | __aicore__ 未定义等入门 CE（结论性） | C | partial | agent09 | — |
| S186 | agent09-S22 | https://www.hiascend.com/app-forum/topic-detail/0269202456643546167 | 【CANN训练营】Erf算子Ascend C开发 | callmedayao-2022／2026-01-21 | 2026-09-12 | 满核原则/UB 充分利用/tilingKey 定制（正面方法论） | C | verified | agent09 | — |
| S187 | agent09-S23 | https://blog.csdn.net/gitblog_00619/article/details/141845484 | CANN/cannbot-skills：rms_norm.asc 直调转自定义算子示例 | cannbot-skills 文档／2026-05-09 | 2026-09-12 | RMSNorm 直调源码形态（3 kernel 类、BF16→FP32 范式） | C（转载；原仓 B） | verified | agent09 | — |
| S188 | agent09-S24 | https://blog.csdn.net/ferriswym/article/details/162241341 | 直调模式 VS 算子框架模式：入口点选择指南 | ferriswym／2026-06-23 | 2026-09-12 | 直调不支持 MIX（hang）等能力边界 | C | verified | agent09 | — |
| S189 | agent09-S25 | https://github.com/mouliangyu/PTOAS/issues/380 | run_ci.sh fails in CANN 9.0.0 env | learning-chip／2026-05-19 | 2026-09-12 | CANN 9.0.0 bisheng 后端崩溃日志 | C | verified | agent09 | — |
| S190 | agent09-S26 | https://blog.csdn.net/gitblog_07213/article/details/151463322 | CANN竞赛作品提交规范 | cann-competitions 转载／2026-05-19 | 2026-09-12 | 作品目录/提交流程规范（复赛阶段参照） | C（转载；原仓 B） | verified | agent09 | — |

### D 级（1 条）

| 统一编号 | 原编号 | URL | 标题/内容 | 仓库/版本/commit | 访问日期 | 用途 | 证据等级 | 证据状态 | 采集代理 | 备注 |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| S191 | agent08-S6 | blog.csdn.net/gitblog_00732/article/details/160916855 | CANN发布管理9.1.0版本说明 | gitblog 转载 / 2026-05 | 2026-09-12 | CANN↔HDK 配套矩阵、ops↔toolkit 配套（原仓 gitcode.com/cann/release-management） | D（原仓 A） | verified | agent08 | 原表标注：转载页记 D，原仓为 A 级 |

### 等级冲突 / 复合等级条目（6 条）

| 统一编号 | 原编号 | URL | 标题/内容 | 仓库/版本/commit | 访问日期 | 用途 | 证据等级 | 证据状态 | 采集代理 | 备注 |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| S192 | agent01-S08 / agent09-S12 | https://blog.csdn.net/gitblog_00410/article/details/143789539（原仓库 https://gitcode.com/cann/cann-learning-hub） | CANN/cann-learning-hub：CANNJudge 算子提交 Skill（提交 API、四字段、状态枚举、平台不开放测试用例；Histogram min/max=0 陷阱；泛化五要素） | GitCode cann/cann-learning-hub（经 CSDN 转载） | 2026-09-12 | 提交接口旁证、泛化必要性；属性默认值特殊语义案例 | B（agent01：官方仓库文档的媒体转载）/ C（agent09：转载；原仓 B） | partial（agent01）/ verified（agent09） | agent01、agent09 | **冲突**：等级（B vs C）与状态（partial vs verified）均不一致，双方标注原样保留，不仲裁 |
| S193 | agent02-在线14 / agent05-#28 | https://www.hiascend.com/developer/blog/details/0289201670005562056 | 案例：CANN 版本问题导致 bisheng 报 unsupported option '--npu-arch'（昇腾官方博文，2025-12-21） | 昇腾官方博客 | 2026-09-12 | --npu-arch 需 CANN≥8.3.RC1 的实证（8.2.RC1 失败）；910B/A2 ↔ 2201/dav-2201 佐证、bisheng 用法 | C 辅证（agent02 自注）/ A（agent05） | verified（双方一致） | agent02、agent05 | **冲突**：等级不一致（C vs A），双方标注原样保留 |
| S194 | agent07-S19 / agent08-S44 | https://github.com/hicann/cann-learning-hub/blob/master/quick_start/cann_basics/02_what_is_npu.ipynb | 02_what_is_npu.ipynb | hicann/cann-learning-hub | 2026-09-12 | 910B Vector=128 FP16 元素/周期；npu-smi Name=Ascend910B3 字段含义 | C（agent07）/ B（agent08） | partial（agent07，snippet 可见）/ verified（agent08） | agent07、agent08 | **冲突**：等级与状态均不一致，双方标注原样保留 |
| S195 | agent08-S26 / agent09-S06 | https://www.hiascend.com/app-forum/topic-detail/0279188908116457252 | 自定义算子使用npu测试报错：TriangularMode is not a function（LOWER 宏冲突） | xxxyyy0011／2025-07-26 | 2026-09-12 | 内建宏与用户标识符冲突（LOWER 宏）；CPU 通过 NPU 失败 | A（agent08：官方论坛）/ C（agent09） | verified（双方一致） | agent08、agent09 | **冲突**：等级不一致（A vs C），双方标注原样保留 |
| S196 | agent04-#16 | https://github.com/linkedin/Liger-Kernel/pull/1000 | [NPU]: avoid pointer mutation in rms_norm kernel | linkedin/Liger-Kernel PR（2026-01 合入） | 2026-09-12 | Triton→NPU 后端移植旁证 | B/C（原表复合标注） | partial | agent04 | 单代理复合等级条目（原表如此） |
| S197 | agent06-#6 | https://arxiv.org/abs/1910.07467 ；https://github.com/bzhangGo/rmsnorm | Root Mean Square Layer Normalization（原论文+官方仓库） | NeurIPS 2019 / master | 2026-09-12 | 原论文公式无 eps；rmsnorm 语义出处 | A/B（原表复合标注） | verified（论文正文 partial，官方 README verified） | agent06 | 单代理复合等级条目；一条登记含两个 URL（原表如此） |

## 四、本地来源表（共 33 条）

> 说明：agent02 的本地 API 缓存均位于 `/Users/sunyiyang/Desktop/Project/cann/临时/api-pages/`，正文从页面内嵌数据完整解码核读；"对应 URL"栏中 `...` 代表基路径 `hiascend.com/document/detail/zh/CANNCommunityEdition/900/API/ascendcopapi/`（850alpha002 等特殊版本另注）。缓存条目等级按 agent02 报告口径记 A（官方文档页面缓存）。

| 编号 | 原编号 | 本地路径/文件 | 标题/内容 | 用途 | 对应在线 URL（如有） | 等级 | 状态 | 采集代理 | 备注 |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| L001 | agent01-S12 | /Users/sunyiyang/Downloads/addrmsnormbias_problem_1742_template/（kernel.asc、main.asc、run.sh、CMakeLists.txt、data_utils.h、scripts/*.py） | 官方下载模板（工程包），8 文件全读 | 接口签名、dtype 枚举、本地判定逻辑核对 | 工程包来源：CANNJudge `/api/problems/{problemId}/package` | B | verified | agent01 | 与 L019、L030 指向同一模板目录的不同文件子集 |
| L002 | agent01-S13 | 文档/competition-rules.md、文档/problem-add-rms-norm-bias.md | 项目既有规则/题面分析备忘 | 与网页核实结果互证、差异修正 | — | 内部文档 | verified | agent01 | — |
| L003 | agent02-缓存01 | 临时/api-pages/atlasascendc_api_07_0007.html | GlobalTensor简介 | SetGlobalBuffer/GetValue/GetSize 签名 | hiascend.com/document/detail/zh/CANNCommunityEdition/900/API/ascendcopapi/atlasascendc_api_07_0007.html | A | verified | agent02 | — |
| L004 | agent02-缓存02 | 临时/api-pages/atlasascendc_api_07_0006.html | LocalTensor简介 | LocalTensor 成员、kernel_operator.h 引用 | .../atlasascendc_api_07_0006.html | A | verified | agent02 | — |
| L005 | agent02-缓存03 | 临时/api-pages/atlasascendc_api_07_0265.html | DataCopyPad(ISASI) | 定案 1/2 全部证据（表 4/6/7、UB→GM dummy 丢弃、A2 通路与 mode 支持度、示例） | .../atlasascendc_api_07_0265.html | A | verified | agent02 | — |
| L006 | agent02-缓存04 | 临时/api-pages/atlasascendc_api_07_0078.html | ReduceSum | 定案 3（count 约束、方式一/二、workLocal 公式、A2 类型 half/float） | .../atlasascendc_api_07_0078.html | A | verified | agent02 | 在线版见 S011/S018/S058（不同版本路径） |
| L007 | agent02-缓存05 | 临时/api-pages/agent02_GetReduceSumMaxMinTmpSize_900.html | GetReduceSumMaxMinTmpSize | Host 侧 tmpSize 公式接口 | https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/900/API/ascendcopapi/atlasascendc_api_07_10160.html | A | verified | agent02 | — |
| L008 | agent02-缓存06 | 临时/api-pages/atlasascendc_api_07_0073.html | Cast | RoundMode 枚举、A2 Cast 支持矩阵（fp32→bf16 5 种模式） | .../atlasascendc_api_07_0073.html | A | verified | agent02 | — |
| L009 | agent02-缓存07 | 临时/api-pages/atlasascendc_api_07_0035.html | Add | A2 类型支持（无 bf16） | .../atlasascendc_api_07_0035.html | A | verified | agent02 | — |
| L010 | agent02-缓存08 | 临时/api-pages/atlasascendc_api_07_0037.html | Mul | A2 类型支持（无 bf16） | .../atlasascendc_api_07_0037.html | A | verified | agent02 | — |
| L011 | agent02-缓存09 | 临时/api-pages/atlasascendc_api_07_0055.html | Muls | A2 类型支持（无 bf16） | .../atlasascendc_api_07_0055.html | A | verified | agent02 | — |
| L012 | agent02-缓存10 | 临时/api-pages/atlasascendc_api_07_0029.html | Sqrt | 向量 Sqrt 原型、A2 half/float | .../atlasascendc_api_07_0029.html | A | verified | agent02 | — |
| L013 | agent02-缓存11 | 临时/api-pages/atlasascendc_api_07_0030.html | Rsqrt | 向量 Rsqrt 原型、A2 half/float | .../atlasascendc_api_07_0030.html | A | verified | agent02 | — |
| L014 | agent02-缓存12 | 临时/api-pages/atlasascendc_api_07_0101.html | DataCopy简介 | DataCopy 功能总览（导航页） | .../atlasascendc_api_07_0102.html（页面内嵌 route 0102） | A | verified | agent02 | 缓存文件名 0101，对应 route 0102（原表如此） |
| L015 | agent02-缓存13 | 临时/api-pages/atlasascendc_api_07_0108.html | TPipe简介 | TPipe 职责（InitBuffer/AllocEventID/ReleaseEventID） | .../atlasascendc_api_07_00148.html（页面内嵌 route 00148） | A | verified | agent02 | 缓存文件名 0108，对应 route 00148（原表如此） |
| L016 | agent02-缓存14 | 临时/api-pages/atlasascendc_api_07_0136.html | TQue简介 | TQue 模板参数、VECIN buffer≤8、AllocTensor/EnQue/DeQue/FreeTensor | .../atlasascendc_api_07_0136.html | A | verified | agent02 | — |
| L017 | agent02-缓存15 | 临时/api-pages/atlasascendc_api_07_0160.html | TBuf简介 | TBuf/Get/InitBuffer 差异 | .../atlasascendc_api_07_0160.html | A | verified | agent02 | — |
| L018 | agent02-缓存16 | 临时/api-pages/atlasascendc_api_07_0270.html | SetFlag/WaitFlag(ISASI) | HardEvent 枚举全量、A2 eventID 0-7、成对约束、AllocEventID/FetchEventID 要求 | .../atlasascendc_api_07_0270.html | A | verified | agent02 | — |
| L019 | agent02-缓存17 | 临时/api-pages/atlasascendc_api_07_0271.html | PipeBarrier(ISASI) | PIPE_S 禁令、PIPE_ALL、自动同步注记 | .../atlasascendc_api_07_0271.html | A | verified | agent02 | — |
| L020 | agent02-缓存18 | 临时/api-pages/atlasascendc_api_07_0184.html | GetBlockNum | int64_t 签名 | .../atlasascendc_api_07_0184.html | A | verified | agent02 | — |
| L021 | agent02-缓存19 | 临时/api-pages/atlasascendc_api_07_0185.html | GetBlockIdx | int64_t 签名 | .../atlasascendc_api_07_0185.html | A | verified | agent02 | — |
| L022 | agent02-缓存20 | 临时/api-pages/atlasascendc_api_07_0215.html | GET_TILING_DATA_WITH_STRUCT | "暂不支持 Kernel 直调工程"约束 | .../atlasascendc_api_07_0215.html | A | verified | agent02 | 800alpha002 版在线登记见 S081（agent09-S15） |
| L023 | agent02-缓存21 | 临时/api-pages/atlasascendc_api_07_00003.html | REGISTER_TILING_DEFAULT | 对照条目（A2 支持） | —（页面标题版本 8.2.RC1.alpha001 线） | A | verified | agent02 | — |
| L024 | agent02-缓存22 | 临时/api-pages/atlasascendc_api_07_00174.html | GetSysWorkSpacePtr | 对照条目（200I/500 A2 不支持） | .../atlasascendc_api_07_00174.html | A | verified | agent02 | — |
| L025 | agent02-缓存23 | 临时/api-pages/atlasascendc_api_07_00007.html | MulDstAdd | 对照条目（A2 不支持） | .../atlasascendc_api_07_0007.html（内嵌） | A | verified | agent02 | — |
| L026 | agent02-缓存24 | 临时/api-pages/agent02_vector_qualifier_850a002.html | C++语言拓展 | 修饰符表（850 时代口径）、__NPU_ARCH__ 2201 映射 | hiascend.com/document/detail/zh/CANNCommunityEdition/850alpha002/opdevg/Ascendcopdevg/atlas_ascendc_10_00026.html | A | verified | agent02 | 900 版尝试直取见 S021（unavailable） |
| L027 | agent02-缓存25 | 临时/api-pages/guide_tutorial.html / guide_index.html | 什么是Ascend C（编程指南导航） | 目录确认（无增量正文） | zh/CANNCommunityEdition/900 | A | partial | agent02 | — |
| L028 | agent06-#16 | /Users/sunyiyang/Downloads/addrmsnormbias_problem_1742_template/scripts/{AddRmsNormBias.py, verify_result.py, gen_data.py} | 官方模板 golden/判定/数据生成脚本 | golden 语义（FP32 全中间）、判定参数（1e-3/1e-3/0.1%，equal_nan） | — | A | verified | agent06 | 模板目录同 L001 |
| L029 | agent06-#17 | /tmp/agent06/exp.py、exp2.py、exp3.py | CPU 参考实验脚本（numpy 2.0.2 / ml_dtypes 0.5.4 / torch 2.4.1） | 数值精度报告第 3 节全部实验数字 | — | 本地 | verified | agent06 | — |
| L030 | agent08-S47 | /Users/sunyiyang/Downloads/addrmsnormbias_problem_1742_template/（run.sh、CMakeLists.txt） | 判题直调模板（只读核对） | run.sh 流程、find_package(ASC)、--npu-arch=dav-2201 | — | B | verified | agent08 | 模板目录同 L001 |
| L031 | agent08-S48 | /Users/sunyiyang/Desktop/Project/cann/文档/source-build.md | 源码编译说明 | 版本敏感点、SoC 核对红线、V001 pipe_ 冲突史 | — | B（内部） | verified | agent08 | — |
| L032 | agent09-L01 | 提交/V001/结果.md | V001 平台结果（15/15 CE；提交编号 6aa37d942d3dd2c5ae7da586） | 本地失败案例 1（pipe_ 标识符与平台宏冲突） | — | 内部文档 | 已确认 | agent09 | — |
| L033 | agent09-L02 | 提交/V002/结果.md、提交/版本实验记录.md、提交/README.md | V002 上传异常记录与版本实验（提交编号 6aa388bd2d3dd2c5ae81372f，2026-09-11） | 本地失败案例 2（上传内容异常） | — | 内部文档 | 已确认（根因未定论） | agent09 | — |

## 五、统计

### 在线来源（统一总表）

- 原始登记：9 份报告共 **203 条**在线来源记录。
- 去重合并：**6 组**同 URL 记录（12 条 → 6 条），统一登记 **197 条**。

**按证据等级**（冲突/复合条目单列，其余按原表等级）：

| 等级 | 条数 |
| --- | --- |
| A | 81 |
| B | 44 |
| C | 65 |
| D | 1 |
| 跨代理等级/状态冲突（双标注保留） | 4 |
| 单代理复合等级（A/B、B/C） | 2 |
| **合计** | **197** |

**按证据状态**：

| 状态 | 条数 |
| --- | --- |
| verified | 183（含 2 条跨代理一致合并条目 S038、S086；2 条冲突条目双方均 verified：S193、S195） |
| partial | 9 |
| unavailable | 2（S008、S021） |
| 跨代理状态不一致（partial/verified 并记） | 2（S192、S194） |
| 单代理复合状态 | 1（S197） |
| **合计** | **197** |

**其他统计项**：

- 去重合并数：**6**（S038、S086、S192、S193、S194、S195；其中 S038、S086 为等级状态一致的合并，其余 4 条存在等级或状态差异）。
- 冲突条目数（跨代理等级或状态冲突）：**4**（S192、S193、S194、S195；其中 S192、S194 等级与状态均冲突，S193、S195 仅等级冲突）。
- contradicted 状态条目数：**0**（各报告来源登记表中无状态为 contradicted 的条目；agent07 报告正文提及 Vector 整卡算力 22T 与 23.5T 两说存疑，但该争议未登记为独立来源条目）。

### 本地来源

共 **33 条**：官方模板相关 3 条（L001、L028、L030，同一模板目录的不同文件子集）、项目内部文档 4 条（L002、L031、L032、L033）、本地 API 缓存（临时/api-pages/）25 条（L003–L027，其中 verified 24 条、partial 1 条）、/tmp CPU 实验脚本 1 条（L029）。

### 汇总

| 类别 | 条数 |
| --- | --- |
| 在线来源（去重后） | 197 |
| 本地来源 | 33 |
| 登记条目总计 | 230 |

# 来源总表（调研2 · 2026-09-12）

> 本表汇总 2026-09-12 十代理深度调研的全部来源，共 **235 条**（S001–S291 区间内实际使用编号）。
> 子代理报告位于 `agents/`，本表由其来源清单合并生成，编号沿用各代理分段，**未重新编号**。

## 1. 证据等级定义

| 等级 | 含义 |
| --- | --- |
| **A** | 官方题面 / 官方 API 文档 / 官方仓库 / 平台下发的模板文件（一手权威） |
| **B** | 官方样例、源码、测试、官方文档转引、原作者发布的资料 |
| **C** | 社区文章、论坛帖子、个人仓库、独立媒体报道 |
| **D** | 仅搜索摘要、未能核验原文的线索、或明确标注为「无法确认」的项 |

## 2. 状态定义

| 状态 | 含义 |
| --- | --- |
| `verified` | 已读取原始内容（正文、源码、JSON、文件） |
| `partial` | 只核对了摘要、README、标题或抓取到的部分正文 |
| `unavailable` | 受访问限制（登录墙、反爬、404、接口禁用） |
| `contradicted` | 与其他来源冲突，已在 `agents/agent10-synthesis.md` 中裁决 |

## 3. 分段索引

| 代理 | 主题 | 编号段 | 条目数 | 平台 |
| --- | --- | --- | --- | --- |
| Agent 1 | 题面、规则与提交接口 | S001–S030 | 15 | cannjudge.cn、GitCode 赛事页/公告、CANNJudge 公开 API |
| Agent 2 | 官方 Ascend C API（CANN 9.0.0/9.0.X） | S031–S060 | 30 | hiascend.com、hiascend.cn、asc.gitcode.com、本地缓存 `临时/api-pages/` |
| Agent 3 | 官方开源仓库 | S061–S090 | 25 | GitCode CANN 官方仓、GitHub Ascend 组织 |
| Agent 4 | GPU / CUDA / Triton / PyTorch 迁移 | S091–S120 | 15 | NVIDIA 论坛、CUDA Samples、PyTorch、Triton、GPU MODE、ROCm |
| Agent 5 | 编译器 / IR / 算子自动生成 | S121–S150 | 30 | LLVM/MLIR Discourse、TVM、OpenXLA、IREE、TorchInductor、TileLang、PyPTO、msopgen |
| Agent 6 | 数值精度与验证方法 | S151–S180 | 16 | PyTorch Issue/PR、NumPy、MindSpore、bf16/fp16 论文、本机 CPU 实测 |
| Agent 7 | 性能 / UB / 硬件架构 | S181–S210 | 23 | Ascend 硬件与 Profiling 文档、社区性能案例、Nsight / ROCm / HPC 资料 |
| Agent 8 | Linux / 真机工程环境 | S211–S240 | 30 | Linux DO、V2EX、Stack Overflow、Ask Ubuntu、Server Fault、HPCwire、Phoronix |
| Agent 9 | 竞赛经验与失败案例 | S241–S270 | 30 | GitHub Issue/PR、GitCode、Kaggle/AIcrowd/Codeforces、CANN 社区、历史算子竞赛页 |
| Agent 10 | 证据审阅与方案合并 | S271–S300 | 21 | 本地模板、提交快照、项目文档、A1–A9 报告复核 |

## 4. 使用规则

1. **引用时写编号**，例如「（S273）」；同一来源被多代理引用时保留各自编号并在 `agent10-synthesis.md` §1.1 标注去重关系。
2. **A/B 级来源可直接支撑实现决策**；C 级只能作为佐证或反例线索；D 级不得作为结论依据，只能作为「待核实」。
3. 缓存到 `缓存/` 或 `临时/` 的来源，必须在 URL 处同时注明本地路径与访问日期。
4. 本轮**未登录任何需要凭据的站点**；CANNJudge 提交页、个人结果页等受登录保护的页面一律记为 `unavailable` 或在分级中标注。

## 5. 已知证据缺口（对应来源空白）

- 判题端 15 个测试点的具体 shape/dtype/epsilon 配置 —— 平台未开放（`unavailable`）。
- 判题 SoC 型号 —— 题目 API 无该字段（`unavailable`）。
- 判题端精度判定是否含「失配比例容差」—— 平台未开放。
- CANN 9.0.0 **正式版** `Mul`/`Div` 在 Atlas A2 上对 bf16 的支持表 —— 当前仅有 9.0.0-beta.2 明文 + 同族推断（B 级）。
- `头文件`中 `DataCopyPadExtParams<T>` 的真实结构体声明 —— 未读安装包，依文档表格推断。

---

# 来源明细

## Agent 1 — 题面、规则与提交接口

- **编号段**：S001–S030
- **平台**：cannjudge.cn、GitCode 赛事页/公告、CANNJudge 公开 API
- **条目数**：15

- [S001] CANNJudge 题目 API（AddRmsNormBias）| https://cannjudge.cn/api/problems/6a9a9a99bf41025d6013eb85 | 平台 | 访问 2026-09-12 | 等级 A | 状态 verified | 用途：接口/测试点/评分/环境原始字段 | 可支持结论：接口表、15 测试点、计分公式、iterations=5、code_template、cann_version=9.0.0、use_baseline=False、ranking_submission_mode=latest、start/end_time、testcase 配置不暴露
- [S002] CANNJudge 公开题面页（AddRmsNormBias）| https://cannjudge.cn/public/op_challenge_xinan_prelim/addrmsnormbias | 平台 | 访问 2026-09-12 | 等级 A | 状态 verified | 用途：题面全文+公开统计 | 可支持结论：通过率61%/通过240/尝试391、vector CANN:9.0.0、题面 §3.3–§七 全文（含 §3.7 Inf/NaN、§四 确定性）
- [S003] CANNJudge 赛事 API（西南赛区初赛）| https://cannjudge.cn/api/contests/6a9a9295bf41025d601255a3 | 平台 | 访问 2026-09-12 | 等级 A | 状态 verified | 用途：赛事级配置与规则字段 | 可支持结论：title=2026年CANN挑战赛_西南赛区（初赛）、zone_id=2094722165106008066、visible_testcase_count=0、freeze_ranking=True(30)、signup_url、submit_enabled=False、scoring_rule=default、start/end_time
- [S004] GitCode 赛事规则/报名页（西南赛区）| https://competition.gitcode.com/competition/2094722165106008066/intro | GitCode（官方外部报名页，由 S003 signup_url 指向）| 访问 2026-09-12 | 等级 A | 状态 verified | 用途：提交额度/违规/赛程/晋级 | 可支持结论：每日最多50次、取最后一次成绩、Host CPU代算/空kernel占位违规、前32强晋级、10/17封榜18:00截止、参赛对象与组队
- [S005] 平台下发模板 kernel.asc | /Users/sunyiyang/Downloads/addrmsnormbias_problem_1742_template/kernel.asc | 平台模板 | 访问 2026-09-12 | 等级 A | 状态 verified | 用途：run_kernel 签名与直调形态 | 可支持结论：单文件直调、run_kernel 参数顺序与 epsilon 末位、核函数启动方式
- [S006] 平台下发模板 main.asc | .../main.asc | 平台模板 | 访问 2026-09-12 | 等级 A | 状态 verified | 用途：元信息结构与核数获取 | 可支持结论：TensorGroupInfo/TensorInfo 运行时传入、ACL_DEV_ATTR_VECTOR_CORE_NUM 取核数、epsilon=1e-5、单用例[1,64]fp16
- [S007] 平台下发模板 CMakeLists.txt | .../CMakeLists.txt | 平台模板 | 访问 2026-09-12 | 等级 A | 状态 verified | 用途：编译环境与 SoC 默认 | 可支持结论：find_package(ASC)、默认 SOC_ARCH=dav-2201（可被 NPU_ARCH 覆盖）、链接库
- [S008] 平台下发模板 run.sh | .../run.sh | 平台模板 | 访问 2026-09-12 | 等级 A | 状态 verified | 用途：本地执行与超时 | 可支持结论：timeout 120、单 case 校验
- [S009] 平台下发 golden 脚本 AddRmsNormBias.py | .../scripts/AddRmsNormBias.py | 平台模板 | 访问 2026-09-12 | 等级 A | 状态 verified | 用途：golden 计算口径 | 可支持结论：FP32 内部计算→整体 cast 回原 dtype、`y/rms*gamma` 除法语义、epsilon 默认 1e-5
- [S010] 平台下发 verify_result.py | .../scripts/verify_result.py | 平台模板 | 访问 2026-09-12 | 等级 A | 状态 verified | 用途：本地判定口径 | 可支持结论：rtol=atol=1e-3、tol=1e-3、仅 1 个 fp16 用例（与官方 fp32 1e-4 更严形成差异）
- [S011] CSDN 公告《2026 CANN 挑战赛报名启动》| https://blog.csdn.net/csdn_codechina/article/details/164369653 | 社区/官方发布 | 访问 2026-09-12 | 等级 C | 状态 verified | 用途：赛制三阶段与日程佐证 | 可支持结论：线上初赛/区域决赛/冠军挑战赛；9/5 发布、10/17 18:00 截止、10/16 23:59 报名截止
- [S012] 西南交通大学本科生院 赛事通知 | https://bksy.swjtu.edu.cn/info/1431/94591.htm | 高校通知 | 访问 2026-09-12 | 等级 C | 状态 verified | 用途：赛程与奖项佐证 | 可支持结论：报名至10/16 23:59、初赛9/5、作品10/17、区域决赛11/7；西南赛区面向高校学生
- [S013] 上海工程技术大学 赛事通知 | https://www.sues.edu.cn/91/d0/c26790a299472/page.htm | 高校通知 | 访问 2026-09-12 | 等级 C | 状态 verified | 用途：赛程佐证 | 可支持结论：报名7/29–10/16、初赛9/5–10/19、决赛10/20–11/7；聚焦算子性能优化
- [S014] 测试点详情端点（批量探测）| https://cannjudge.cn/api/testcases/11456 等 | 平台 | 访问 2026-09-12 | 等级 A（接口行为）| 状态 unavailable(403 Forbidden) | 用途：验证测试点配置是否公开 | 可支持结论：各测试点 shape/dtype/epsilon 不公开 → [当前无法确认]
- [S015] CANNJudge 提交页 | https://cannjudge.cn/public/op_challenge_xinan_prelim/addrmsnormbias/submit | 平台 | 访问 2026-09-12 | 等级 A（页面存在）| 状态 partial(需登录，未尝试) | 用途：上传字段/大小限制 | 可支持结论：提交表单字段与编辑器限制未公开 → [页面可访问但内容不完整]

## Agent 2 — 官方 Ascend C API（CANN 9.0.0/9.0.X）

- **编号段**：S031–S060
- **平台**：hiascend.com、hiascend.cn、asc.gitcode.com、本地缓存 临时/api-pages/
- **条目数**：30

- [S031] LocalTensor 简介（GetValue/SetValue/GetSize）| file:///.../临时/api-pages/atlasascendc_api_07_0006.html（对应 hiascend 9.0.X）| 平台 hiascend | 访问日期 2026-09-12 | 等级 A | 状态 verified | 用途：LocalTensor 单点读写与尺寸 API | 可支持结论：LocalTensor 提供 GetValue/SetValue/GetSize（GetLength 已弃用）。
- [S032] GlobalTensor 简介（SetGlobalBuffer）| file:///.../临时/api-pages/atlasascendc_api_07_0007.html | 平台 hiascend | 2026-09-12 | A | verified | 用途：GM 绑定 | 可支持结论：`SetGlobalBuffer(__gm__ T*, uint64_t)` 与无长度重载。
- [S033] DataCopyPad(ISASI) 9.0.0 | file:///.../临时/api-pages/atlasascendc_api_07_0265.html | hiascend | 2026-09-12 | A | verified | 用途：非对齐搬运与 padding 表、UB→GM 无 padParams、Q1/Q2 | 可支持结论：padding 仅 GM→UB；表 6 顺序=选项 A；UB→GM 自动丢弃 dummy。
- [S034] ReduceSum 9.0.0 | file:///.../临时/api-pages/atlasascendc_api_07_0078.html | hiascend | 2026-09-12 | A | verified | 用途：归约 count/sharedTmpBuffer/A2 支持 | 可支持结论：count 无硬上限数字；A2 仅 half/float；sharedTmpBuffer 公式给出。
- [S035] Cast 9.0.0 | file:///.../临时/api-pages/atlasascendc_api_07_0073.html | hiascend | 2026-09-12 | A | verified | 用途：roundMode 与 bf16 互转（Q5） | 可支持结论：fp32→bf16 支持多 roundMode；bf16/fp16→fp32 无损。
- [S036] Sqrt 9.0.0 | file:///.../临时/api-pages/atlasascendc_api_07_0029.html | hiascend | 2026-09-12 | A | verified | 用途：开方 API | 可支持结论：`Sqrt(dst,src,count)` 前 n 个数据计算。
- [S037] Rsqrt 9.0.0 | file:///.../临时/api-pages/atlasascendc_api_07_0030.html | hiascend | 2026-09-12 | A | verified | 用途：开方取倒数（推荐） | 可支持结论：`Rsqrt(dst,src,count)`。
- [S038] Muls 9.0.0 | file:///.../临时/api-pages/atlasascendc_api_07_0055.html | hiascend | 2026-09-12 | A | verified | 用途：矢量×标量、A2 不支持 bf16（Q4） | 可支持结论：A2 支持 half/int16_t/float/int32_t，无 bf16。
- [S039] MulDstAdd 9.0.0 | file:///.../临时/api-pages/atlasascendc_api_07_00007.html | hiascend | 2026-09-12 | A | verified | 用途：融合乘加（SimD RegTensor） | 可支持结论：仅 Atlas 350 支持（A2 ×），不能用于 A2。
- [S040] TQue 简介 9.0.0 | file:///.../临时/api-pages/atlasascendc_api_07_0136.html | hiascend | 2026-09-12 | A | verified | 用途：队列 AllocTensor/EnQue/DeQue/FreeTensor | 可支持结论：三段式流水 API 完整；队列深度推荐 1。
- [S041] 核函数限定符 / 语言扩展层（__vector__/__aicore__/GetBlockIdx/GetBlockNum/HardEvent V_MTE3）| file:///.../临时/api-pages/agent02_vector_qualifier_850a002.html | hiascend | 2026-09-12 | A | verified | 用途：限定符表、多核变量、同步示例 | 可支持结论：`__vector__` 仅 Vector 核、耦合架构不生效；`__global__` 须配 `__aicore__`；GetBlockIdx/GetBlockNum 可用。
- [S042] GetReduceSumMaxMinTmpSize 9.0.0 | file:///.../临时/api-pages/agent02_GetReduceSumMaxMinTmpSize_900.html | hiascend | 2026-09-12 | A | verified | 用途：host 侧 sharedTmpBuffer 大小（Q3） | 可支持结论：最大临时空间=最小临时空间；host 侧 `ge::Shape` 接口。
- [S043] 什么是 Ascend C（编程指南）9.0.0 | file:///.../临时/api-pages/guide_index.html | hiascend | 2026-09-12 | A | partial | 用途：支持产品、SIMD/SIMT 模型 | 可支持结论：支持 Atlas A2 等；dav-2201 支持完整 Memory 矢量 UB 编程。
- [S044] SetFlag/WaitFlag(ISASI)（含 HardEvent 枚举、PipeBarrier）9.0.0 | file:///.../临时/api-pages/atlasascendc_api_07_0270.html | hiascend | 2026-09-12 | A | verified | 用途：HardEvent 枚举、eventID 范围、A2 支持 | 可支持结论：`enum HardEvent{MTE2_V,V_MTE2,MTE3_V,V_MTE3,S_V,V_S,...}`；A2 eventID 0–7；SetFlag/WaitFlag 须成对。
- [S045] DataCopyPad（华为开发者联盟/鸿蒙）| https://developer.huawei.com/consumer/cn/doc/harmonyos-guides/cannkit-datacopypad | huawei | 2026-09-12 | B | verified | 用途：Q1 表 5 顺序佐证 | 可支持结论：顺序 isPad/leftPadding/rightPadding/paddingValue。
- [S046] 如何高效处理 Ascend C 非对齐数据（技术文章）| https://www.hiascend.com/developer/techArticles/20250627-1 | hiascend | 2026-09-12 | C | verified | 用途：Q2 UB→GM dummy 丢弃 | 可支持结论：搬出时框架补 dummy、写 GM 时丢弃。
- [S047] AscendC DataCopyPad 32 字节对齐（掘金/社区解析）| https://juejin.cn/post/7682716879561687050 | 第三方 | 2026-09-12 | C | verified | 用途：Q1 结构体顺序解析、Q2 dummy 丢弃细节 | 可支持结论：结构体字段顺序=选项 A；UB→GM 丢弃 dummy。
- [S048] Add 自定义算子开发概述（CANN 9.0.0-beta.2）| https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/900beta2/opdevg/Ascendcopdevg/atlas_ascendc_10_0032.html | hiascend | 2026-09-12 | A(β2) | verified | 用途：Q4 Add 不支持 bf16（A2） | 可支持结论：A2 上 Add 不支持 bfloat16_t，须 Cast 到 float。
- [S049] Add（基础算术）8.1RC1 | https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/81RC1alpha002/apiref/ascendcopapi/atlasascendc_api_07_0035.html | hiascend | 2026-09-12 | B | verified | 用途：Add 签名与 A2 产品表 | 可支持结论：`Add(dst,src0,src1,calCount)`；A2 half/int16_t/float/int32_t（无 bf16）。
- [S050] DataCopy 基础数据搬运（8.5.0 + 华为云博客）| https://www.hiascend.com/document/detail/en/canncommercial/latest/API/ascendcopapi/atlasascendc_api_07_0038.html 与 https://bbs.huaweicloud.com/blogs/436918 | hiascend/华为云 | 2026-09-12 | B | partial | 用途：DataCopy 基础签名 | 可支持结论：`DataCopy(dst,src,calCount)` 与 DataCopyParams 重载；GM↔UB A2 支持。
- [S051] PipeBarrier(ISASI)（PIPE_V/PIPE_S/PIPE_MTE2/PIPE_MTE3）| https://developer.huawei.com/consumer/cn/doc/hiai-References/cannkit-pipebarrier-0000002479486693 | 华为 | 2026-09-12 | B | verified | 用途：流水类型定义 | 可支持结论：PIPE_MTE2=GM→UB，PIPE_MTE3=UB→GM，PIPE_V=矢量，PIPE_S=标量；`PipeBarrier<PIPE_S>()` 报错。
- [S052] Div（基础算术）| https://developer.huawei.com/consumer/cn/doc/hiai-References/cannkit-vector-calculation-binocular-div-0000002158595293 与 8.5.0 atlasascendc_api_07_0038 | 华为/hiascend | 2026-09-12 | B | verified | 用途：Div 签名 | 可支持结论：`Div(dst,src0,src1,calCount)`；支持 half/float（A2 无 bf16）。
- [S053] Divs（双目标量）| http://www.hqwc.cn/a/422816.html（引自 CANN 文档）| 第三方 | 2026-09-12 | C | verified | 用途：Divs 签名 | 可支持结论：`Divs(dst,src0,src1,count)`。
- [S054] GetBlockDim 不存在说明 | https://wenku.csdn.net/answer/6pw24npu8h17 与 8.0.0 API 列表 | CSDN/hiascend | 2026-09-12 | C/B | verified | 用途：澄清 GetBlockDim 非标准 API | 可支持结论：仅 GetBlockNum/GetBlockIdx 标准；GetBlockDim 不存在。
- [S055] 9.0.X 基础 API 列表（LocalTensor/GlobalTensor 方法清单）| https://www.hiascend.com/document/detail/en/CANNCommunityEdition/900/API/ascendcopapi/atlasascendc_api_07_10432.html | hiascend | 2026-09-12 | A | verified | 用途：LocalTensor/GlobalTensor 方法清单 | 可支持结论：LocalTensor 含 SetValue/GetValue/GetSize；GlobalTensor 含 SetGlobalBuffer/GetValue/SetValue/GetSize。
- [S056] Built-in Data Types 9.0.0 | https://www.hiascend.com/document/detail/en/CANNCommunityEdition/latest/API/ascendcopapi/atlas_ascendc_10_0019.html | hiascend | 2026-09-12 | A | verified | 用途：bf16 是否为内置类型 | 可支持结论：A2 支持 bfloat16_t 作为内置类型（但计算算子另有限制，见 Q4）。
- [S057] Add 样例（asc-devkit，产品与 CANN 版本）| https://gitcode.com/cann/asc-devkit/blob/master/examples/01_simd_cpp_api/00_introduction/01_add/add/README.md | asc.gitcode | 2026-09-12 | B | verified | 用途：A2 支持 ≥ CANN 9.0.0 | 可支持结论：Atlas A2 训练/推理 ≥ CANN 9.0.0 支持 Add 样例。
- [S058] Ascend C API 列表（TPipe/TBuf/TQue/Sync）8.5.0 | https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/850alpha001/API/ascendcopapi/atlasascendc_api_07_10251.html | hiascend | 2026-09-12 | B | verified | 用途：TPipe/TBuf 在 API 列表中 | 可支持结论：TPipe.InitBuffer、TBuf、TQue、PipeBarrier、SetFlag/WaitFlag、GetBlockNum/GetBlockIdx 均为标准 API。
- [S059] Memory 矢量计算编程（静态/动态 Tensor，`__global__ __vector__` 示例，dav-2201）| https://asc.gitcode.com/guide/编程指南/编程模型/AI-Core-SIMD编程/基于Tensor的CPP编程/Memory矢量计算编程.html | asc.gitcode | 2026-09-12 | B | verified | 用途：纯矢量核 `__global__ __vector__` 写法、dav-2201 支持 | 可支持结论：dav-2201 支持完整 Memory 矢量 UB 编程；示例用 `__global__ __vector__`。
- [S060] DataCopyPad（英文 8.5.0，Table 5 顺序）| https://www.hiascend.com/document/detail/en/canncommercial/latest/API/ascendcopapi/atlasascendc_api_07_0265.html | hiascend | 2026-09-12 | B | verified | 用途：Q1 英文表顺序佐证 | 可支持结论：isPad/leftPadding/rightPadding/paddingValue 顺序与选项 A 一致。

## Agent 3 — 官方开源仓库（ops-transformer / ops-nn / cann-samples）

- **编号段**：S061–S090
- **平台**：GitCode CANN 官方仓、GitHub Ascend 组织
- **条目数**：25

- [S061] 仓库 gitcode.com/cann/ops-transformer（官方镜像，含 AddRmsNorm 源码） | https://gitcode.com/cann/ops-transformer | 分支 9.0.0 commit efca5d19e52b6c6fa3da44d962e92d91357fe983 | 访问 2026-09-12 | 等级 A | 状态 verified | 用途：定位 AddRmsNorm 真实源码 | 可支持结论：官方 AddRmsNorm 存在于 mc2/matmul_all_reduce_add_rms_norm/op_kernel/
- [S062] 文件 ops-transformer mc2/matmul_all_reduce_add_rms_norm/op_kernel/add_rms_norm.h（KernelAddRmsNorm 基线实现） | https://raw.gitcode.com/cann/ops-transformer/raw/9.0.0/mc2/matmul_all_reduce_add_rms_norm/op_kernel/add_rms_norm.h | 9.0.0 efca5d19 | 访问 2026-09-12 | 等级 B | 状态 verified | 用途：逐行摘录多核切分/残差相加/RMSNorm 数值链 | 可支持结论：多核按行、残差 Add、fp32 升精度、1/rms 经 GetValue(0) 读回、仅 *gamma 无 bias
- [S063] 文件 ops-transformer .../op_kernel/add_rms_norm_split_d.h（大 D 两遍法） | https://raw.gitcode.com/cann/ops-transformer/raw/9.0.0/mc2/matmul_all_reduce_add_rms_norm/op_kernel/add_rms_norm_split_d.h | 9.0.0 efca5d19 | 访问 2026-09-12 | 等级 B | 状态 verified | 用途：大 D/非对齐 D 的 split_d 两遍处理 | 可支持结论：D 切 ubFactor 块、Former 累加平方和、Latter 乘回，支持 D 达 32768 与尾块
- [S064] 文件 ops-transformer .../op_kernel/add_rms_norm_single_n.h（一行一核整 D 进 UB） | https://raw.gitcode.com/cann/ops-transformer/raw/9.0.0/mc2/matmul_all_reduce_add_rms_norm/op_kernel/add_rms_norm_single_n.h | 9.0.0 efca5d19 | 访问 2026-09-12 | 等级 B | 状态 verified | 用途：单行单核分块基线 | 可支持结论：SINGLE_N_BUFFER_SIZE=(192-1)*1024 字节，整 D 进 UB，D 受限时回退
- [S065] 文件 ops-transformer .../op_kernel/reduce_common.h（归约原语） | https://raw.gitcode.com/cann/ops-transformer/raw/9.0.0/mc2/matmul_all_reduce_add_rms_norm/op_kernel/reduce_common.h | 9.0.0 efca5d19 | 访问 2026-09-12 | 等级 B | 状态 verified | 用途：WholeReduceSum / 级联加 ReduceSumHalfInterval | 可支持结论：大 D 归约用 findPowerTwo 级联 Add + WholeReduceSum
- [S066] 文件 ops-transformer .../op_kernel/rms_norm_base.h（ReduceSumCustom/DataCopyCustom） | https://raw.gitcode.com/cann/ops-transformer/raw/9.0.0/mc2/matmul_all_reduce_add_rms_norm/op_kernel/rms_norm_base.h | 9.0.0 efca5d19 | 访问 2026-09-12 | 等级 B | 状态 verified | 用途：归约与搬运封装、非对齐尾块兜底 | 可支持结论：dav-2201 走 DataCopyPad（需 32B 对齐）；非 220 用 GetValue/SetValue 处理尾部
- [S067] 文件 ops-transformer .../op_kernel/add_rms_norm_kernel.h（分发器） | https://raw.gitcode.com/cann/ops-transformer/raw/9.0.0/mc2/matmul_all_reduce_add_rms_norm/op_kernel/add_rms_norm_kernel.h | 9.0.0 efca5d19 | 访问 2026-09-12 | 等级 B | 状态 verified | 用途：keyTile 选 4 策略、numBlocks 来源 | 可支持结论：策略由 Host tiling key 决定（10/11/12/13/14 +BF16）
- [S068] 文件 ops-transformer .../op_kernel/matmul_all_reduce_add_rms_norm_tiling_data.h（AddRMSNormTilingData 结构体） | https://raw.gitcode.com/cann/ops-transformer/raw/9.0.0/mc2/matmul_all_reduce_add_rms_norm/op_kernel/matmul_all_reduce_add_rms_norm_tiling_data.h | 9.0.0 efca5d19 | 访问 2026-09-12 | 等级 B | 状态 verified | 用途：确认 tiling 结构体字段 | 可支持结论：含 num_row/num_col/block_factor/row_factor/ub_factor/epsilon/avg_factor（本题没有，需运行时推导）
- [S069] 仓库 gitcode.com/cann/ops-nn（独立 AddRmsNorm 算子来源） | https://gitcode.com/cann/ops-nn | 分支 9.0.0 commit fcebf031d193d641d2d1472a539bcc387b1e5f09 | 访问 2026-09-12 | 等级 A | 状态 verified | 用途：独立 add_rms_norm 算子 | 可支持结论：norm/add_rms_norm 为纯净单算子实现
- [S070] 文件 ops-nn norm/add_rms_norm/op_kernel/add_rms_norm.h（KernelAddRmsNorm<T,MODE>） | https://raw.gitcode.com/cann/ops-nn/raw/9.0.0/norm/add_rms_norm/op_kernel/add_rms_norm.h | 9.0.0 fcebf031 | 访问 2026-09-12 | 等级 B | 状态 verified | 用途：逐行摘录（多核 GetBlockNum/残差/数值链/fp32 分支） | 可支持结论：支持 fp32（有 else 分支直接 Add）；无 bias；多核按行+尾核余数
- [S071] 文件 ops-nn norm/add_rms_norm/op_kernel/add_rms_norm_split_d.h | https://raw.gitcode.com/cann/ops-nn/raw/9.0.0/norm/add_rms_norm/op_kernel/add_rms_norm_split_d.h | 9.0.0 fcebf031 | 访问 2026-09-12 | 等级 B | 状态 verified | 用途：独立算子的 split_d 大 D 处理 | 可支持结论：与 ops-transformer 同构的两遍法
- [S072] 文件 ops-nn norm/add_rms_norm/op_kernel/add_rms_norm.cpp（extern "C" 核入口） | https://raw.gitcode.com/cann/ops-nn/raw/9.0.0/norm/add_rms_norm/op_kernel/add_rms_norm.cpp | 9.0.0 fcebf031 | 访问 2026-09-12 | 等级 B | 状态 verified | 用途：确认 __global__ 入口形态 | 可支持结论：标准 msopgen 单算子入口，含 tiling 结构体注册
- [S073] 仓库 gitcode.com/cann/cann-samples（直调 .asc 样例来源） | https://gitcode.com/cann/cann-samples | 分支 master commit 23c981c0918e3183958e94e58ef6989d44983230 | 访问 2026-09-12 | 等级 A | 状态 verified | 用途：直调单文件 .asc 样例 | 可支持结论：含 rms_norm_quant_story 与 reduce 原语 .asc
- [S074] 文件 cann-samples Samples/2_Performance/rms_norm_quant_story/src/2_multi_core.asc | https://raw.gitcode.com/cann/cann-samples/raw/master/Samples/2_Performance/rms_norm_quant_story/src/2_multi_core.asc | master 23c981c | 访问 2026-09-12 | 等级 B | 状态 verified | 用途：直调形态 + 多核 + RMSNorm 数值链（量化） | 可支持结论：__global__ __aicore__ __vector__ + <<<blockNum,0,stream>>>；量化版（scale/offset/int8），无 residual、无 bias
- [S075] 文件 cann-samples .../rms_norm_quant_story/src/4_double_buffer.asc | https://raw.gitcode.com/cann/cann-samples/raw/master/Samples/2_Performance/rms_norm_quant_story/src/4_double_buffer.asc | master 23c981c | 访问 2026-09-12 | 等级 B | 状态 verified | 用途：双缓冲 + VF 按 64 元素分块处理非对齐 D | 可支持结论：UpdateMask 掩码尾块法，天然处理 D 非 32 倍数
- [S076] 文件 cann-samples .../rms_norm_quant_story/Story.md（教学文档） | https://raw.gitcode.com/cann/cann-samples/raw/master/Samples/2_Performance/rms_norm_quant_story/Story.md | master 23c981c | 访问 2026-09-12 | 等级 B | 状态 verified | 用途：naive→multi_core→double_buffer→binary_sum 优化脉络 | 可支持结论：官方给出的 AddRmsNorm 多核/双缓冲优化路线图
- [S077] 文件 cann-samples Samples/2_Performance/simd_vf_story/reduce/src/reduce_sum_ra_baseline.asc | https://raw.gitcode.com/cann/cann-samples/raw/master/Samples/2_Performance/simd_vf_story/reduce/src/reduce_sum_ra_baseline.asc | master 23c981c | 访问 2026-09-12 | 等级 B | 状态 verified | 用途：ReduceSum 原语 + 非对齐尾块（D1=250 %8≠0） | 可支持结论：VF + UpdateMask 沿最后一维归约的标准写法，可套用于本题
- [S078] 仓库 gitcode.com/cann/cann-learning-hub（旧名 Ascend/samples） | https://gitcode.com/cann/cann-learning-hub | 分支 master commit ff0e08e27655888c1bc8f6befd1956f39ca79346 | 访问 2026-09-12 | 等级 A | 状态 partial | 用途：候选补充单算子样例库 | 可支持结论：确认可达，未深挖（本轮已从 S061-S077 取得足量逐行源码）
- [S079] 仓库 GitHub Ascend/samples（cann-learning-hub 旧名，GitHub 侧存在） | https://github.com/Ascend/samples | master | 访问 2026-09-12 | 等级 A | 状态 partial | 用途：对照 GitHub 侧可达性 | 可支持结论：GitHub 上仅 samples 存在，ops-transformer/ops-nn/cann-samples/cann-learning-hub 在 GitHub 均 404
- [S080] 目录 ops-nn norm/rms_norm（纯 RMSNorm，无 residual，对照用） | https://raw.gitcode.com/cann/ops-nn/raw/9.0.0/norm/rms_norm/op_kernel/ | 9.0.0 fcebf031 | 访问 2026-09-12 | 等级 B | 状态 verified | 用途：对照「残差融合」差异 | 可支持结论：纯 RMSNorm 无 Add(residual)，本题需 residual 故主取 add_rms_norm
- [S081] 目录 ops-nn norm/add_rms_norm_quant（量化版，甄别非等价） | https://raw.gitcode.com/cann/ops-nn/raw/9.0.0/norm/add_rms_norm_quant/op_kernel/ | 9.0.0 fcebf031 | 访问 2026-09-12 | 等级 B | 状态 contradicted | 用途：显式甄别「量化≠本题 bias」 | 可支持结论：带 quant_scale/quant_offset、输出 int8，末尾 offset 是量化偏移非本题 bias，不可作等价实现
- [S082] 目录 ops-transformer mc2/3rd/add_rms_norm（薄封装/引用） | https://raw.gitcode.com/cann/ops-transformer/raw/9.0.0/mc2/3rd/add_rms_norm/ | 9.0.0 efca5d19 | 访问 2026-09-12 | 等级 B | 状态 verified | 用途：确认独立 add_rms_norm 引用位置 | 可支持结论：仅为 tiling 头引用，算子体在 matmul_all_reduce_add_rms_norm/op_kernel
- [S083] 文件 ops-transformer .../op_kernel/add_rms_norm_multi_n.h（多行合并策略） | https://raw.gitcode.com/cann/ops-transformer/raw/9.0.0/mc2/matmul_all_reduce_add_rms_norm/op_kernel/add_rms_norm_multi_n.h | 9.0.0 efca5d19 | 访问 2026-09-12 | 等级 B | 状态 verified | 用途：多 N 合并分块策略参考 | 可支持结论：当行数多/UB 富余时可多行一批提升利用率
- [S084] 文件 ops-transformer .../op_kernel/add_rms_norm_merge_n.h（多行合并策略） | https://raw.gitcode.com/cann/ops-transformer/raw/9.0.0/mc2/matmul_all_reduce_add_rms_norm/op_kernel/add_rms_norm_merge_n.h | 9.0.0 efca5d19 | 访问 2026-09-12 | 等级 B | 状态 verified | 用途：多 N 合并分块策略参考 | 可支持结论：与 multi_n 同族，提供多行合并的另一种 UB 布局
- [S085] 文件 cann-samples .../simd_vf_story/reduce/src/reduce_sum_ar_baseline.asc（axis=1 归约原语） | https://raw.gitcode.com/cann/cann-samples/raw/master/Samples/2_Performance/simd_vf_story/reduce/src/reduce_sum_ar_baseline.asc | master 23c981c | 访问 2026-09-12 | 等级 B | 状态 verified | 用途：沿最后一维（axis=1）归约标准写法 | 可支持结论：与本题「沿最后一维 D 归约」直接对应，可作为归约骨架

## Agent 4 — GPU / CUDA / Triton / PyTorch 迁移分析

- **编号段**：S091–S120
- **平台**：NVIDIA 论坛、CUDA Samples、PyTorch、Triton、GPU MODE、ROCm
- **条目数**：15

- [S091] PyTorch `aten/src/ATen/native/cuda/layer_norm_kernel.cu`（含 RMSNorm 模板路径） | https://raw.githubusercontent.com/pytorch/pytorch/main/aten/src/ATen/native/cuda/layer_norm_kernel.cu | PyTorch (GitHub) | 访问日期 2026-09-12 | 等级 B | 状态 verified | 用途：GPU 原生 fused RMSNorm 的归约/向量化/FP32 累加/尾部处理 | 可支持的结论：表 1/2/3/4/6/13/15 行；warp shuffle + shared memory 块归约 + float4/half4 向量化 + acc_type fp32 + N%4==0 向量主循环 |
- [S092] Liger-Kernel `src/liger_kernel/ops/rms_norm.py` | https://raw.githubusercontent.com/linkedin/Liger-Kernel/main/src/liger_kernel/ops/rms_norm.py | Liger-Kernel (GitHub) | 访问日期 2026-09-12 | 等级 B | 状态 verified | 用途：Triton RMSNorm 的 tl.sum 归约、masked load、casting_mode、weight+offset 融合、calculate_settings | 可支持的结论：表 5/9/13/14 行；residual 不在核内融合、FP32 累加、masked other=0 |
- [S093] Triton 官方 tutorial 05 `python/tutorials/05-layer-norm.py` | https://raw.githubusercontent.com/triton-lang/triton/main/python/tutorials/05-layer-norm.py | Triton (GitHub) | 访问日期 2026-09-12 | 等级 B | 状态 verified | 用途：Triton 层归一化前向的两遍扫描、masked load、program_id 行映射、MAX_FUSED_SIZE 限制、num_warps 启发式 | 可支持的结论：表 1/5/6/9/15 行；单核分块循环+tl.sum、特征维≥64KB 不支持单遍 |
- [S094] Triton Tutorials 索引页 | https://triton-lang.org/main/getting-started/tutorials/index.html | Triton 官方文档 | 访问日期 2026-09-12 | 等级 B | 状态 verified | 用途：确认官方教程含 05 Layer Norm 与 09 Persistent Matmul | 可支持的结论：表 8 行（persistent matmul 作为 persistent kernel 思想来源） |
- [S095] lmdeploy `lmdeploy/pytorch/kernels/cuda/rms_norm.py`（`add_rms_norm_kernel`） | https://github.com/InternLM/lmdeploy/blob/5efcf6e79bf952a832cf838adda474debfab1189/lmdeploy/pytorch/kernels/cuda/rms_norm.py | lmdeploy (GitHub) | 访问日期 2026-09-12 | 等级 C | 状态 partial | 用途：Triton 实现且显式融合 residual 的 add_rms_norm；BLOCK_N=next_power_of_2；反向 dW 用 (sm_count, n_cols) 部分求和 | 可支持的结论：表 5/7/14 行；残差融合形态正对应本题 AddRmsNormBias |
- [S096] RMSNorm-B200（Int21-AI，Blackwell PTX 手写 RMSNorm） | https://github.com/Int21-AI/RMSNorm-B200 | GitHub 社区 | 访问日期 2026-09-12 | 等级 C | 状态 verified（搜索摘要） | 用途：证明 Blackwell 上仍有「手写 CUDA/PTX」与「Triton」并存路线；支持 residual 融合、affine weight/bias | 可支持的结论：表 14 行；residual+affine 融合是跨代共性 |
- [S097] Fused-LayerNorm-CUDA-Operator（JonSnow1807） | https://github.com/JonSnow1807/Fused-LayerNorm-CUDA-Operator | GitHub 社区 | 访问日期 2026-09-12 | 等级 C | 状态 verified（搜索摘要） | 用途：{LN,RMS}×{plain,fused-add}×epilogue 通用核模板、fp8 输出、确定性反向 | 可支持的结论：表 14 行；残差+epilogue 融合成熟范式 |
- [S098] FlashAttention `csrc/layer_norm`（vllm 镜像 `e5da6e4`）setup.py | https://github.com/vllm-project/flash-attention/tree/e5da6e4dcd436a782da8ef73c03cdc95f60e9442/csrc/layer_norm | FlashAttention (GitHub) | 访问日期 2026-09-12 | 等级 C | 状态 verified | 用途：按 hidden size 特化的 `ln_fwd_256.cu … ln_fwd_8192.cu` + `ln_parallel_*` | 可支持的结论：第 2.5 节；「按 D 静态特化 kernel」对应表 9 行思想 |
- [S099] FlashAttention `csrc/layer_norm` README | 同上仓库 README | FlashAttention (GitHub) | 访问日期 2026-09-12 | 等级 C | 状态 verified | 用途：「Implement RMSNorm as an option」「fused dropout+residual+LayerNorm」「switched to a Triton-based implementation」 | 可支持的结论：第 2.5 节；residual 融合 + Triton 化趋势 |
- [S100] ROCm `rocm-examples` Normalization（rmsnorm2d / add_rmsnorm2d_quant） | https://github.com/rocm/rocm-examples | ROCm (GitHub) | 访问日期 2026-09-12 | 等级 C | 状态 partial | 用途：AMD 侧 RMSNorm2D 与 add+RMSNorm2D+量化示例，底层 CK Tile/HIP tile 模型 | 可支持的结论：第 2.6 节；跨厂商均有 residual 融合实现（非 CUDA 独有） |
- [S101] NVIDIA CUDA C++ Programming Guide（warp shuffle / shared memory bank conflict 概念） | https://docs.nvidia.com/cuda/cuda-c-programming-guide/ | NVIDIA 官方文档 | 访问日期 2026-09-12 | 等级 A | 状态 D（本会话未直接抓取，通用 CUDA 知识） | 用途：warp shuffle 原语与 shared memory bank conflict 定义 | 可支持的结论：表 1/2 行 GPU 侧机制定义 |
- [S102] CUDA `atomicAdd` 用于反向 gamma/beta 梯度跨行汇总（通用范式） | — | CUDA 通用知识 | 访问日期 2026-09-12 | 等级 D | 状态 D | 用途：说明 GPU 反向梯度 device 原子加 | 可支持的结论：表 7 行「不可直接迁移部分」 |
- [S103] CUDA `__restrict__` 指针别名提示（通用 C++/CUDA 语义） | — | CUDA 通用知识 | 访问日期 2026-09-12 | 等级 D | 状态 D | 用途：说明 `__restrict__` 是编译器别名假设 | 可支持的结论：表 11 行 |
- [S104] PTX `fma` / `ex2` / `rcp` 与 `--use_fast_math` 近似语义（指令级） | — | NVIDIA PTX 通用知识 | 访问日期 2026-09-12 | 等级 D | 状态 D | 用途：FFMA 融合乘加与近似超越函数 | 可支持的结论：表 12 行 |
- [S105] NVIDIA Nsight Systems / Nsight Compute（GPU profiler） | https://developer.nvidia.com/nsight-systems / https://developer.nvidia.com/nsight-compute | NVIDIA 官方工具 | 访问日期 2026-09-12 | 等级 B | 状态 D（本会话未直接抓取文档页） | 用途：GPU 时间线/算子级性能分析 | 可支持的结论：表 10 行「GPU 侧 profiler 方法论」 |

## Agent 5 — 编译器 / IR / 算子自动生成

- **编号段**：S121–S150
- **平台**：LLVM/MLIR Discourse、TVM、OpenXLA、IREE、TorchInductor、TileLang、PyPTO、msopgen
- **条目数**：30

- [S121] Triton-Ascend 官方仓库（triton-lang/triton-ascend）| https://github.com/triton-lang/triton-ascend | 平台 GitHub | 访问日期 2026-09-12 | 等级 B | 状态 verified | 用途：确认 Triton-Ascend 存在、CANN 9.0.0 兼容性、需 torch_npu 运行时 | 可支持的结论：问题1/3 为否，归 A 类
- [S122] Triton-Ascend 安装指南 | https://triton-ascend.readthedocs.io/en/v3.2.2/installation_guide.html | 平台 官方文档 | 访问日期 2026-09-12 | 等级 B | 状态 verified | 用途：确认 pip 安装、依赖 torch_npu + CANN、Linux 环境 | 可支持的结论：产物经 Python 运行时启动，非单文件 kernel.asc
- [S123] Triton-Ascend 概览（GitCode 镜像/Ascend）| https://gitcode.com/Ascend/triton-ascend | 平台 社区镜像 | 访问日期 2026-09-12 | 等级 C | 状态 verified | 用途：确认 3.2.1 配 CANN 9.0.0、Atlas A2/A3/950 支持 | 可支持的结论：硬件可达但形态不符
- [S124] msopgen 快速入门（华为开发者文档）| https://developer.huawei.com/consumer/cn/doc/harmonyos-guides/cannkit-operator-development | 平台 华为官方 | 访问日期 2026-09-12 | 等级 A | 状态 verified | 用途：确认 msopgen 生成完整算子工程、核函数 extern "C" __global__ __aicore__ 形态 + aclnn 启动 | 可支持的结论：问题1/3 为否，归 A 类
- [S125] 创建算子工程（CANN 9.0.0 社区版）| https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/900beta2/devaids/optool/atlasopdev_16_0021.html | 平台 华为官方 | 访问日期 2026-09-12 | 等级 A | 状态 verified | 用途：确认 msopgen gen 参数、生成 op_host/op_kernel/framework | 可支持的结论：生成完整工程而非单文件直调
- [S126] PyPTO 仓库（hw-native-sys/pypto）| https://github.com/hw-native-sys/pypto | 平台 GitHub | 访问日期 2026-09-12 | 等级 C | 状态 verified | 用途：确认 PTO 范式、JIT、MPMD 调度、生成 Ascend C 代码 | 可支持的结论：需 PyPTO 运行时，非单文件 kernel.asc
- [S127] PyPTO 解析（昇腾开源生态专区）| https://ascendai.csdn.net/69dcb64072111d255bf8a440.html | 平台 CSDN | 访问日期 2026-09-12 | 等级 C | 状态 verified | 用途：确认 Tile 编程模型、自动内存管理、动态 shape | 可支持的结论：FP32/尾块策略无官方定论
- [S128] TileLang 官方仓库（tile-ai/tilelang）| https://github.com/tile-ai/tilelang | 平台 GitHub | 访问日期 2026-09-12 | 等级 B | 状态 verified | 用途：确认基于 TVM、目标 CUDA/HIP、AscendC 为 2025-09 preview | 可支持的结论：问题1/3 为否，归 A 类
- [S129] TileLang ICLR2026 论文笔记 | https://en.papernotes.org/ICLR2026/llm_efficiency/tilelang_bridge_programmability_and_performance_in_modern_neural_kernels | 平台 论文笔记 | 访问日期 2026-09-12 | 等级 C | 状态 verified | 用途：确认 FTG、accum_dtype、layout inference | 可支持的结论：支持指定 FP32 累加，但需 Python 运行时
- [S130] TVM rfactor CUDA codegen 讨论 | https://discuss.tvm.apache.org/t/cuda-codegen-could-it-generate-warp-shuffle-instructions/14641 | 平台 Apache TVM Discuss | 访问日期 2026-09-12 | 等级 C | 状态 verified | 用途：确认 rfactor 树形/跨线程归约 + set_store_predicate | 可支持的结论：归约 lowering 改变累加顺序，需 TVM runtime
- [S131] TVM TensorIR RFactor 提交（apache/tvm）| https://github.com/apache/tvm/commit/dc7bbb63838b0780b21c68030efdb60e73a5b224 | 平台 GitHub | 访问日期 2026-09-12 | 等级 B | 状态 verified | 用途：确认 rfactor 产出 B_rf 中间缓冲、部分归约+写回 | 可支持的结论：树形归约改变顺序，精度受影响
- [S132] IREE 官方文档（iree.dev）| https://iree.dev/ | 平台 官方文档 | 访问日期 2026-09-12 | 等级 B | 状态 verified | 用途：确认 AOT 生成 .vmfb + 运行时、支持后端 | 可支持的结论：需 IREE runtime，无 Ascend 后端
- [S133] IREE 架构/wiki | https://hivebook.wiki/wiki/iree-mlir-based-ml-compiler-and-runtime | 平台 社区 wiki | 访问日期 2026-09-12 | 等级 C | 状态 verified | 用途：确认 Flow/Stream/HAL 分层、.vmfb 工件 | 可支持的结论：产物形态非单文件 kernel.asc
- [S134] OpenXLA 官网 | http://openxla.org/ | 平台 官方 | 访问日期 2026-09-12 | 等级 A | 状态 verified | 用途：确认 XLA 编译器、目标后端列表 | 可支持的结论：目标无 Ascend NPU，问题3 为否
- [S135] XLA 支持设备（Wikiwand）| https://www.wikiwand.com/en/articles/Accelerated_Linear_Algebra | 平台 维基 | 访问日期 2026-09-12 | 等级 C | 状态 verified | 用途：确认支持 NVIDIA/AMD/Intel/Apple/TPU/Trainium/IPU/CPU，无 Huawei | 可支持的结论：无 Ascend 后端
- [S136] TorchInductor triton.py 源码 | https://github.com/pytorch/pytorch/blob/1cae60a87e5bdda8bcf55724a862eeed98a9747e/torch/_inductor/codegen/triton.py | 平台 GitHub | 访问日期 2026-09-12 | 等级 B | 状态 verified | 用途：确认 triton_acc_type、reduction 代码生成、rmask/mask | 可支持的结论：需 torch.compile 运行时，问题1/3 为否
- [S137] TorchInductor Persistent reductions 提交 | https://github.com/pytorch/pytorch/commit/a8fdfb4ba8a804c67d744a763fd9fa1f72d28590 | 平台 GitHub | 访问日期 2026-09-12 | 等级 B | 状态 verified | 用途：确认 persistent vs looped reduction 两种策略 | 可支持的结论：归约策略改变累加顺序
- [S138] TorchInductor Reduction Kernels 博客 | https://karthick.ai/blog/2025/Learn-By-Doing-Torchinductor-Reduction | 平台 博客 | 访问日期 2026-09-12 | 等级 C | 状态 verified | 用途：确认 should_use_persistent_reduction 启发式、inner 归约融合 | 可支持的结论：融合边界与归约顺序启发
- [S139] MLIR 'linalg' Dialect 官方文档 | https://mlir.llvm.org/docs/Dialects/Linalg | 平台 官方文档 | 访问日期 2026-09-12 | 等级 B | 状态 verified | 用途：确认 Tiling / Promotion / Fusion / Vectorization 变换 | 可支持的结论：基础设施，无直调单文件出口
- [S140] MLIR Ch0：Structured Linalg / vector.reduction | https://mlir.llvm.org/docs/Tutorials/transform/Ch0/ | 平台 官方文档 | 访问日期 2026-09-12 | 等级 B | 状态 verified | 用途：确认 vector.reduction 可 lowering 为 scf.for 顺序 loop | 可支持的结论：归约 lowering 可顺序可硬件，顺序 loop 保序
- [S141] MLIR [Linalg] Scalable Vectorization of Reduction PR #97788 | https://github.com/llvm/llvm-project/pull/97788 | 平台 GitHub | 访问日期 2026-09-12 | 等级 B | 状态 verified | 用途：确认 trailing-dim 归约 scalable vectorization + masking | 可支持的结论：尾块用 vector masking
- [S142] Triton tl.sum API 文档 | https://triton-lang.cn/main/python-api/generated/triton.language.sum.html | 平台 官方文档 | 访问日期 2026-09-12 | 等级 B | 状态 verified | 用途：确认浮点默认提升到至少 fp32 累加、可指定 dtype | 可支持的结论：Triton 默认 FP32 累加，树形归约改顺序
- [S143] Triton 归约深度 vs 数值误差 gist | https://gist.github.com/EthanZhong02/fef62df5728ad5af60dd035bb91e6e05 | 平台 GitHub Gist | 访问日期 2026-09-12 | 等级 C | 状态 verified | 用途：量化 BLOCK_SIZE / 累加 dtype 对误差影响 | 可支持的结论：FP32 累加显著降低误差，树形归约改顺序
- [S144] Triton Exercises（归约/边界）| https://lweitkamp.github.io/triton_exercises/print.html | 平台 教程 | 访问日期 2026-09-12 | 等级 C | 状态 verified | 用途：确认 mask/boundary_check 处理非对齐尾块、fp32 累加约定 | 可支持的结论：尾块 mask 策略启发
- [S145] MLIR Linalg Dialect 中文解析 | https://www.lei.chat/zh/posts/mlir-linalg-dialect-and-patterns/ | 平台 博客 | 访问日期 2026-09-12 | 等级 C | 状态 verified | 用途：确认 Linalg 分层、Promotion to fast memory 思想 | 可支持的结论：UB 复用/memory planning 启发
- [S146] TVM reduction 教程（tvm-fork）| https://gitlab.engr.illinois.edu/yifanz16/tvm-fork/-/blob/b236f10908d22eef2d83dd80183fd0e9affcd67d/tutorials/language/reduction.py | 平台 GitLab | 访问日期 2026-09-12 | 等级 C | 状态 verified | 用途：确认 te.sum / rfactor 基础归约调度 | 可支持的结论：归约 lowering 基础
- [S147] PyPTO 深度解析（昇腾开源生态专区）| https://ascendai.csdn.net/69dcb64072111d255bf8a440.html | 平台 CSDN | 访问日期 2026-09-12 | 等级 C | 状态 verified | 用途：确认三层 IR、PTO-ISA、融合指令 | 可支持的结论：需 PyPTO 运行时
- [S148] TileLang-MUSA（摩尔线程，跨平台佐证）| https://ima.qq.com/wiki/?shareId=e1480c65f3c34052d1c165c9b57fe17d7af11352a365513ba43e59f1d4d3dbb0 | 平台 社区 | 访问日期 2026-09-12 | 等级 C | 状态 verified | 用途：佐证 TileLang 跨平台能力、需后端运行时 | 可支持的结论：进一步说明 DSL 需各自后端运行时
- [S149] IREE Developer overview | https://iree.dev/developers/general/developer-overview/ | 平台 官方文档 | 访问日期 2026-09-12 | 等级 B | 状态 verified | 用途：确认 iree-compile / .vmfb / runtime 工作流 | 可支持的结论：产物形态非单文件 kernel.asc
- [S150] Triton-Ascend 上手指南（上海交大 xflops）| https://xflops.sjtu.edu.cn/hpc-start-guide/ascend/triton/ | 平台 高校指南 | 访问日期 2026-09-12 | 等级 C | 状态 verified | 用途：确认 Triton-Ascend 安装组合（CANN 9.1.0 + torch_npu）、Python 启动 | 可支持的结论：判题纯 C++ 直调场景无法使用

## Agent 6 — 数值精度与验证方法

- **编号段**：S151–S180
- **平台**：PyTorch Issue/PR、NumPy、MindSpore、bf16/fp16 论文、CPU 实测
- **条目数**：16

- [S151] PyTorch `torch.nn.RMSNorm` 官方文档 | https://docs.pytorch.org/docs/stable/generated/torch.nn.RMSNorm.html | PyTorch | 访问日期 2026-09-12 | 等级 B | 状态 verified | 用途：确认标准公式 `RMS(x)=sqrt(eps+mean(x²))`，epsilon 在 sqrt 内 | 可支持结论：Q2/Q7 公式定义
- [S152] RMSNorm Deep Dive — Math + Kernels (Belgavi AI Lab) | http://aicassindra.com/blogs/transformer_math/tm_rmsnorm_deep.html | 技术博客/研究 | 访问日期 2026-09-12 | 等级 C | 状态 verified | 用途：FP32 累加必要性、epsilon 量纲、fp16 单元素溢出阈值 |x|>256、bf16 8-bit 尾数静默停滞 | 可支持结论：Q3/Q4/Q5/Q6
- [S153] LayerNorm vs RMSNorm architecture (Belgavi AI Lab) | http://aicassindra.com/blogs/transformer_math/tm_layernorm.html | 技术博客/研究 | 访问日期 2026-09-12 | 等级 C | 状态 verified | 用途：混合精度下归一化统计量必须 FP32、bf16 长累加精度丢失 | 可支持结论：Q5/Q6
- [S154] RMSNorm: Efficient Normalization for Modern LLMs | https://mbrenndoerfer.com/writing/rmsnorm-efficient-normalization-modern-llms | 技术博客/研究 | 访问日期 2026-09-12 | 等级 C | 状态 verified | 用途：LLaMA 风格 RMSNorm 全程 float32 计算后 cast 回原精度 | 可支持结论：第 5 节策略
- [S155] Normalization (DeepWiki, fused kernel 分析) | https://deepwiki.com/kiki632/Smurfs/5-normalization | 技术文档 | 访问日期 2026-09-12 | 等级 C | 状态 verified | 用途：FP32 累加、epsilon 在 rsqrt 前加入、rsqrtf 硬件指令 | 可支持结论：Q2/Q5/Q7
- [S156] NumPy `numpy.isclose` 文档 | https://numpy.org/doc/stable/reference/generated/numpy.isclose.html | NumPy 官方 | 访问日期 2026-09-12 | 等级 B | 状态 verified | 用途：`absolute(a-b) <= atol + rtol*abs(b)`、非对称（b 为参考）、`equal_nan` 语义、atol 对近零值作用 | 可支持结论：第 3 节比较口径、特殊值 NaN 处理
- [S157] LLM-algo-4：RMSNorm fp16 溢出陷阱与 Upcasting | https://blog.csdn.net/weixin_43424450/article/details/162249124 | 技术博客 | 访问日期 2026-09-12 | 等级 D | 状态 verified | 用途：fp16 max=65504、平方安全上限 255、升精度模式（UPcasting） | 可支持结论：Q4
- [S158] Triton 替换 RMSNorm 溢出踩坑（fp16 累加→inf→0） | https://www.cnblogs.com/lihuacheng/p/19642788 | 技术博客 | 访问日期 2026-09-12 | 等级 D | 状态 verified | 用途：fp16 全程累加使 `mean=inf`→`1/inf=0`→输出全 0 | 可支持结论：Q6 实证（fp16 低精度灾难）
- [S159] Nano-vLLM 源码解读：RMSNorm 为何先 `.float()` | https://blog.csdn.net/qq_37755661/article/details/161624866 | 技术博客 | 访问日期 2026-09-12 | 等级 D | 状态 verified | 用途：fp16 累加 `var→inf`，fp32 累加≈900（对照实证） | 可支持结论：Q6 实证
- [S160] MindSpore `mindspore.ops.rms_norm` 官方文档 | https://mindspore.cn/docs/zh-CN/master/api_python/ops/mindspore.ops.rms_norm.html | MindSpore 官方 | 访问日期 2026-09-12 | 等级 B | 状态 verified | 用途：公式 `y = x_i / sqrt(mean(x²)+ε) * γ_i`，支持 fp16/fp32/bf16，epsilon 默认 1e-6 | 可支持结论：Q2/Q7 多框架一致
- [S161] MindSpore `test_ops_rms_norm.py` 测试代码 | https://gitee.com/mindspore/mindspore/blob/v2.7.0/tests/st/ops/test_ops_rms_norm.py | MindSpore 测试 | 访问日期 2026-09-12 | 等级 B | 状态 verified | 用途：前向精度阈值 `loss = 1e-4 (fp32) / 1e-3 (fp16)`——与题面 Agent 1 口径一致 | 可支持结论：判题阈值佐证（fp32=1e-4, fp16=1e-3）
- [S162] MindSpeed-Core-MS `norm.py`（RMSNorm 实现） | https://atomgit.com/Ascend/MindSpeed-Core-MS/blob/r0.1.0/mindspeed_ms/legacy/model/norm.py | 框架源码 | 访问日期 2026-09-12 | 等级 C | 状态 verified | 用途：`x.float()` 升精度 + `mint.rsqrt(mean(square(x))+eps)` 实践 | 可支持结论：第 5 节策略（升精度+epsilon 内）
- [S163] Bfloat16 floating-point format (Wikipedia) | https://en.wikipedia.org/wiki/Bfloat16_floating-point_format | 百科/标准 | 访问日期 2026-09-12 | 等级 B | 状态 verified | 用途：bf16 仅 8-bit 尾数、与 fp32 转换可截断或 RNE、依赖硬件 | 可支持结论：Q5/Q6 + 第 6 节 cast 舍入局限
- [S164] `BF16RoundingMode` 文档（IEEE 754 舍入模式） | https://docs.rs/torsh-core/latest/torsh_core/dtype/bfloat16/enum.BF16RoundingMode.html | 技术文档 | 访问日期 2026-09-12 | 等级 C | 状态 verified | 用途：RNE(默认)/TowardZero/等舍入模式对数值稳定性与可复现性的影响 | 可支持结论：第 6 节 cast 舍入未验证项
- [S165] Floating Point：torch bf16 cast 用 RNE 舍入（实测） | https://tensor.khalilli.ai/blog/floating-point/ | 技术博客/研究 | 访问日期 2026-09-12 | 等级 C | 状态 verified | 用途：torch 的 fp32→bf16 cast 用最近偶舍入，逐位验证 | 可支持结论：第 5/6 节 cast 舍入
- [S166] Automatic Verification of Floating-Point Accumulation Networks | https://link.springer.com/content/pdf/10.1007/978-3-031-98682-6_12.pdf | 论文(Springer) | 访问日期 2026-09-12 | 等级 B | 状态 verified | 用途：RNE 为 IEEE 754 默认舍入，unit roundoff u=2^-p，单操作相对误差界 | 可支持结论：第 6 节舍入误差理论上界

## Agent 7 — 性能 / UB / 硬件架构

- **编号段**：S181–S210
- **平台**：Ascend 硬件与 Profiling 文档、社区性能案例、Nsight/ROCm/HPC 资料
- **条目数**：23

- [S181] CANN/cannbot-skills NPU硬件架构总览（含 NPUTargetSpec.td 规格表：910B1/910B2 UB=192KB、24 AIC/48 AIV、dav-c220；310B UB=256KB）| https://blog.csdn.net/gitblog_01164/article/details/153903579 | CSDN | 访问日期 2026-09-12 | 等级 B | 状态 verified | 用途：复核 UB/核数 | 可支持的结论：910B2 UB=192KB、48 AIV
- [S182] ENEC: Lossless AI Model Compression on Ascend NPUs（arxiv，确认 910B2 = 24 AI Cores / 24 AIC / 48 AIV，UB ~192KB）| https://arxiv.org/html/2604.03298 | arxiv | 访问日期 2026-09-12 | 等级 B | 状态 verified | 用途：复核核数与 UB | 可支持的结论：48 向量核、UB 192KB
- [S183] 昇腾 vs 英伟达/AMD 主流 AI 芯片关键参数对比表（910B HBM 带宽 1.6 TB/s、FP16 320 TFLOPS）| https://developer.huawei.com/consumer/cn/blog/topic/03202360837318320 | 华为开发者联盟 | 访问日期 2026-09-12 | 等级 B | 状态 verified | 用途：HBM 带宽/算力 | 可支持的结论：BW≈1.6TB/s、FP16≈320TFLOPS
- [S184] Huawei Ascend 910B AI Accelerator Card（经销商页：64GB HBM2e、1.2 TB/s）| https://omnixonglobal.com/products/huawei-ascend-910b-ai-accelerator-card | Omnixon | 访问日期 2026-09-12 | 等级 C | 状态 partial（与 S183 带宽不一致，疑为 910B1/早期） | 用途：交叉核对带宽 | 可支持的结论：带宽有 1.2TB/s 说法，取 1.6TB/s 为标称需谨慎
- [S185] Ascend C Exp API 官方文档（向量单元每次读 256B；fp16 mask≤128、fp32 mask≤64）| https://www.hiascend.com/document/detail/en/CANNCommunityEdition/900/API/ascendcopapi/atlasascendc_api_07_0025.html | hiascend 官方 | 访问日期 2026-09-12 | 等级 A | 状态 verified | 用途：向量 repeat 宽度 | 可支持的结论：256B/repeat、128/64 元素上限
- [S186] Ascend C Duplicate API 官方文档（256B = 8 block × 32B）| https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/83RC1alpha001/API/ascendcopapi/atlasascendc_api_07_0088.html | hiascend 官方 | 访问日期 2026-09-12 | 等级 A | 状态 verified | 用途：datablock=32B | 可支持的结论：搬运粒度 32B
- [S187] 2024CANN训练营：Ascend C API 接口（repeat=8 block×32B）| https://bbs.huaweicloud.com/blogs/436918 | 华为云社区 | 访问日期 2026-09-12 | 等级 B | 状态 verified | 用途：repeat 机制 | 可支持的结论：256B/repeat
- [S188] 性能探针：Ascend C 算子性能分析与 Profiling 工具链实战（msprof op 命令、OpBasicInfo/PipeUtilization、AscendCTimeStamp）| https://ai6s.net/6942197fbf6b0e4b285c1821.html | 社区教程 | 访问日期 2026-09-12 | 等级 C | 状态 verified | 用途：测量方法 | 可支持的结论：msprof op 取 Task Duration、aiv/mte2/mte3 占比
- [S189] 使用 msprof 分析 Ascend C kernel 执行耗时（OpBasicInfo.csv Task Duration；4096 逻辑核/40 物理核 → 总时间=aiv_time×波次）| https://www.toutiao.com/article/7644776436096385563 | 头条博客 | 访问日期 2026-09-12 | 等级 C | 状态 verified | 用途：核数映射 | 可支持的结论：逻辑核>物理核时排队，Block Dim 为逻辑核数
- [S190] CANN msprof 环境准备官方文档（msprof --output、op_summary/task_time 产物）| https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/80RC3alpha003/devaids/auxiliarydevtool/atlasprofiling_16_0004.html | hiascend 官方 | 访问日期 2026-09-12 | 等级 A | 状态 verified | 用途：测量工具 | 可支持的结论：msprof 产出文件结构
- [S191] Ascend C 算子性能优化实践指南（BlockReduceSum+WholeReduceSum 比两次 WholeReduceSum 快 39%；100 循环 52us vs 85us）| https://ai6s.net/6a4dd516662f9a54cb8adcb4.html | 社区教程 | 访问日期 2026-09-12 | 等级 C | 状态 verified | 用途：reduce 优化 | 可支持的结论：reduce 组合有显著收益
- [S192] 昇腾 Ascend C 算子开发-ReduceSum 导致的性能问题（华为专家确认 ReduceSum 含 scalar 同步等待、阻塞流水）| https://ascendai.csdn.net/69d7935172111d255bf8761f.html | 昇腾开源生态 | 访问日期 2026-09-12 | 等级 B/C | 状态 verified | 用途：标量同步代价 | 可支持的结论：WholeReduceSum 阻塞流水，改用 BlockReduceSum+WholeReduceSum
- [S193] 针对不同场景合理使用归约指令（官方最佳实践：shape=256 fp32，BlockReduceSum+WholeReduceSum=8.44us vs 两次 WholeReduceSum=13us）| https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/82RC1/opdevg/ascendcbestP/atlas_ascendc_best_practices_10_0034.html | hiascend 官方 | 访问日期 2026-09-12 | 等级 A | 状态 verified | 用途：reduce 指令组合 | 可支持的结论：Block+Whole 优于两次 Whole
- [S194] Ascend C 算子性能优化实战：从跑通到打满带宽的四步走（32B 对齐硬要求、DataCopyPad 处理尾部、512B cacheline 对齐、Double Buffer ~1.6×）| https://developer.huawei.com/home/forum/ascend/thread-02200221215465808268-1-1.html | 华为开发者联盟 | 访问日期 2026-09-12 | 等级 B | 状态 verified | 用途：对齐/双缓冲 | 可支持的结论：32B 对齐风险真实、DataCopyPad 对策、双缓冲加速比
- [S195] 内存金字塔：Ascend C 多级存储与高效访存（UB 由 32–64 个 Bank 组成，每 Bank 256bit=32B；bank conflict）| https://ascendai.csdn.net/69d4d5d472111d255bf7e316.html | 社区 | 访问日期 2026-09-12 | 等级 C | 状态 verified | 用途：UB bank | 可支持的结论：UB 32B bank 粒度
- [S196] 昇腾 CANN 训练营：UB Bank Conflict 深度解析（dstStride padding 打破 bank 冲突）| https://blog.csdn.net/2401_82857325/article/details/155499501 | CSDN | 访问日期 2026-09-12 | 等级 C | 状态 verified | 用途：bank conflict 规避 | 可支持的结论：padding stride 可缓解冲突
- [S197] [AI][昇腾950]数据搬运（GM ~1.6–1.8TB/s，mte2 ratio 常 >95% 显 memory-bound；UB 192KB for A2/A3）| https://hwcomputing.csdn.net/6a5ec2d9662f9a54cb927882.html | 社区 | 访问日期 2026-09-12 | 等级 C | 状态 verified | 用途：带宽/瓶颈 | 可支持的结论：本类算子 memory-bound、UB=192KB
- [S198] 昇腾 CANN 训练营：32-Byte 内存对齐与 Burst 性能哲学（aclrtMalloc size+32 安全余量；fp16 tileLength 须为 16 倍数）| https://blog.csdn.net/2401_82857325/article/details/156026480 | CSDN | 访问日期 2026-09-12 | 等级 C | 状态 verified | 用途：对齐对策 | 可支持的结论：Host 侧多留 32B、fp16 行须 16 对齐
- [S199] Ascend C 算子性能优化实用技巧（减少 PipeBarrier 使用、仅在跨流水依赖处同步）| https://www.hiascend.cn/developer/techArticles/20241107-1?envFlag=1 | 昇腾社区 | 访问日期 2026-09-12 | 等级 B | 状态 verified | 用途：同步优化 | 可支持的结论：细粒度同步替代 PIPE_ALL
- [S200] Ascend 950 架构总览（DaVinci AIC/AIV 解耦、HBM/UB 层级；社区整理，含未发布规格）| (无 URL，社区长文) | 社区 | 访问日期 2026-09-12 | 等级 C | 状态 partial（含前瞻规格，谨慎引用） | 用途：架构背景 | 可支持的结论：AIC/AIV 分工、memory-bound 普遍性
- [S201] 华为昇腾计算官网（产品页，无具体带宽数字）| https://e.huawei.com/cn/products/computing/ascend | 华为官网 | 访问日期 2026-09-12 | 等级 A | 状态 partial（无本任务所需具体数） | 用途：平台背景 | 可支持的结论：无数值结论
- [S202] Ascend Hardware Background（arxiv 2505.15112：DaVinci 架构、AIV 支持 reduce/gather）| https://arxiv.org/pdf/2505.15112v1 | arxiv | 访问日期 2026-09-12 | 等级 B | 状态 verified | 用途：架构背景 | 可支持的结论：AIV 支持 reduction
- [S203] 华为开发者联盟 昇腾 vs N卡 对比（复用 S183 链接，FP16 320TFLOPS、HBM 1.6TB/s）| https://developer.huawei.com/consumer/cn/blog/topic/03202360837318320 | 华为开发者联盟 | 访问日期 2026-09-12 | 等级 B | 状态 verified | 用途：算力/带宽交叉验证 | 可支持的结论：与 S183 一致

## Agent 8 — Linux / 真机工程环境

- **编号段**：S211–S240
- **平台**：Linux DO、V2EX、Stack Overflow、Ask Ubuntu、Server Fault、HPCwire、Phoronix
- **条目数**：30

- [S211] 昇腾社区·快速安装CANN（含驱动/固件/CANN 7.0 示例） | https://www.hiascend.com/doc_center/source/zh/canncommercial/700/quickstart/quickstart/quickstart_18_0002.html | 平台 hiascend(官方) | 访问日期 2026-09-12 | 等级 A | 状态 verified | 用途：步骤1/2 驱动固件与CANN安装、npu-smi info自检、set_env.sh | 可支持的结论：驱动/固件+CANN安装顺序、npu-smi自检、环境变量激活方式为官方标准流程
- [S212] hiascend·CANN社区版软件安装指南（选择安装场景/软件包清单） | https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/800alpha001/softwareinst/instg/instg_0001.html | 平台 hiascend(官方) | 访问日期 2026-09-12 | 等级 A | 状态 verified | 用途：软件包清单（driver/firmware/toolkit/nnae/nnrt/kernels）、社区版vs商用版 | 可支持的结论：CANN分社区版/商用版；toolkit含算子编译与runtime；kernels包用途
- [S213] hiascend·NPU Driver and Firmware Installation（英文，PM/容器/VM 场景） | https://www.hiascend.com/document/detail/en/canncommercial/800/softwareinst/instg/instg_0005.html | 平台 hiascend(官方) | 访问日期 2026-09-12 | 等级 A | 状态 verified | 用途：驱动/固件安装命令、`--check`、`--full --install-for-all`、HwHiAiUser 运行用户、重启用 npu-smi info 验证 | 可支持的结论：驱动固件必须用run包；首次/覆盖安装顺序不同；安装后reboot并npu-smi验证
- [S214] hiascend·编译选项（毕昇编译器 bisheng/ccec） | https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/83RC1alpha003/opdevg/BishengCompiler/atlas_bisheng_10_0010.html | 平台 hiascend(官方) | 访问日期 2026-09-12 | 等级 A | 状态 verified | 用途：bisheng/ccec位置（`${INSTALL_DIR}/compiler/ccec_compiler`）、set_env.sh激活、--npu-arch概念 | 可支持的结论：毕昇编译器随CANN发布；通过set_env.sh或PATH激活；ccec_compiler/bin含bisheng
- [S215] hiascend·快速安装FAQ（9.x 合一包、whitelist、apt/yum在线安装） | https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/920beta2/softwareinst/instg/instg_0050.html | 平台 hiascend(官方) | 访问日期 2026-09-12 | 等级 A | 状态 verified | 用途：9.x版本在线/离线安装、whitelist=driver/toolkit、apt-get/conda安装 | 可支持的结论：CANN 9.x支持合一包与组件白名单安装；在线安装需OS源支持
- [S216] 华为开发者论坛·昇腾 NPU 环境下 PyTorch 项目迁移与 torch_npu 配置踩坑 | https://developer.huawei.com/home/forum/ascend/thread-02178214133377043079-1-1.html | 平台 华为开发者论坛 | 访问日期 2026-09-12 | 等级 C | 状态 verified | 用途：CANN与torch_npu环境组成、版本必须匹配、set_env.sh、NPU可用性验证、常见报错(libascendcl.so等) | 可支持的结论：驱动/固件/CANN/PyTorch/torch_npu版本严格配套；CANN环境变量未source会致动态库找不到
- [S217] 掘金·linux.do：一个被严重低估的中文技术社区（linux.do 背景介绍） | https://juejin.cn/post/7627405638134972462 | 平台 掘金(社区) | 访问日期 2026-09-12 | 等级 C | 状态 verified | 用途：解释 linux.do 是什么、内容以AI/大模型为主、基于Discourse、2024-01上线 | 可支持的结论：linux.do并非Linux发行版论坛，主聊AI大模型，昇腾算子开发类帖子极少见
- [S218] DeepWiki·Linux Do Wiki·Services Documentation（linux.do 社区构成） | https://deepwiki.com/chenyme/Linux-Do-Wiki/3-services-documentation | 平台 DeepWiki(社区索引) | 访问日期 2026-09-12 | 等级 C | 状态 verified | 用途：linux.do社区服务/AI服务/知识库构成，无昇腾开发板块 | 可支持的结论：linux.do公开资料中无昇腾/CANN环境搭建类板块，佐证"未命中"
- [S219] CSDN·CANNBot Ascend C 直调开发指南（环境变量/CMake/find_package(ASC)） | https://blog.csdn.net/gitblog_00909/article/details/151507556 | 平台 CSDN(社区) | 访问日期 2026-09-12 | 等级 C | 状态 verified | 用途：ASCEND_HOME_PATH(非ASCEND_HOME)正确名、bisheng路径、find_package(ASC)自动发现、直调工程配置 | 可支持的结论：环境变量正确名为ASCEND_HOME_PATH；CMake经find_package(ASC)发现bisheng；直调开发需host侧算tiling
- [S220] CANN开发者社区·昇腾平台环境搭建（apt 在线装 toolkit、set_env.sh） | https://cann.csdn.net/6a38e45c10ee7a33f280baa8.html | 平台 CSDN(社区) | 访问日期 2026-09-12 | 等级 C | 状态 verified | 用途：apt-cache search ascend-cann-toolkit、source set_env.sh、echo $ASCEND_HOME_PATH | 可支持的结论：CANN社区版可apt在线安装(默认/usr/local/Ascend)；set_env.sh激活后ASCEND_HOME_PATH生效
- [S221] ai6s.net·昇腾 910B 服务器初始化（驱动/固件/CANN/容器挂载设备清单） | https://ai6s.net/691d5df70e4c466a32e92168.html | 平台 ai6s.net(社区) | 访问日期 2026-09-12 | 等级 C | 状态 verified | 用途：Ascend Docker Runtime安装、docker run挂载/dev/davinci*与driver目录、ASCEND_RT_VISIBLE_DEVICES | 可支持的结论：容器用昇腾NPU必须装Ascend Docker Runtime并正确挂载设备与driver；容器内编号从0起
- [S222] 51CTO·基于Aarch64 openEuler 的 910B 离线部署（驱动固件安装实操） | https://blog.51cto.com/u_16099278/14255405 | 平台 51CTO(社区) | 访问日期 2026-09-12 | 等级 C | 状态 verified | 用途：--check校验、--full --install-username=root、npu-smi info、驱动DKMS模式 | 可支持的结论：非HwHiAiUser用户安装驱动需显式--install-username/--install-usergroup；校验包完整性用--check
- [S223] ai6s.net·Ascend C 入门实战：从零构建昇腾 AI 加速算子（环境/编程模型/编译） | https://ai6s.net/693f91f40800f3458b825754.html | 平台 ai6s.net(社区) | 访问日期 2026-09-12 | 等级 C | 状态 verified | 用途：kernel_operator.h作用、GM/UB三级存储、AllocTensor/FreeTensor、UB溢出、CMake示例 | 可支持的结论：Ascend C基于C++17扩展；UB容量有限需控制AllocTensor大小；E40021为算子编译失败类错误
- [S224] CSDN·快速安装/升级/卸载 CANN/Ascend 配套软件包（社区版/商用版、run包参数） | https://blog.csdn.net/m0_37605642/article/details/137511287 | 平台 CSDN(社区) | 访问日期 2026-09-12 | 等级 C | 状态 verified | 用途：run包--check/--install/--install-path/--full/--uninstall、社区版不受限 | 可支持的结论：Ascend配套软分社区版/商用版；CANN含runtime/compiler/opp/toolkit；安装路径可自定义
- [S225] ascend.readthedocs.io·快速安装昇腾环境（系统要求/驱动自检） | https://ascend.readthedocs.io/zh-cn/latest/sources/ascend/quick_install.html | 平台 昇腾开源readthedocs(官方镜像) | 访问日期 2026-09-12 | 等级 A | 状态 verified | 用途：lspci确认卡、uname/cat os-release、npu-smi info验证、创建HwHiAiUser | 可支持的结论：开源安装文档的系统要求与驱动安装后npu-smi验证流程
- [S226] CSDN·hwcomputing·昇腾 910B NPU 大模型部署实践（vLLM/容器设备映射/驱动挂载不全报错） | https://hwcomputing.csdn.net/69a055dd0a2f6a37c593d81b.html | 平台 CSDN(社区) | 访问日期 2026-09-12 | 等级 C | 状态 verified | 用途：容器内ASCEND_RT_VISIBLE_DEVICES设置、aclInit 107001 Invalid device ID、fwkacllib必须挂载 | 可支持的结论：容器内NPU设备ID从0起；驱动库挂载不全致aclInit失败；bridge模式-p与--network=host冲突
- [S227] ai6s.net·小模型在昇腾NPU上的推理部署（torch_npu 版本配套表） | https://ai6s.net/698ed5d454b52172bc5b75e9.html | 平台 ai6s.net(社区) | 访问日期 2026-09-12 | 等级 C | 状态 verified | 用途：CANN/PyTorch/torch_npu三者版本必须匹配、Docker容器示例 | 可支持的结论：torch_npu依赖CANN，版本严格配套；推荐用官方镜像避免环境错配
- [S228] 百度百科·华为昇腾NPU（架构/产品/软件生态概述） | https://baike.baidu.com/item/%E5%8D%8E%E4%B8%BA%E6%98%87%E8%85%BENPU/67703028 | 平台 百度百科 | 访问日期 2026-09-12 | 等级 C | 状态 verified | 用途：达芬奇架构Cube/Vector/Scalar、CANN开源(2025-08)、950系列 | 可支持的结论：昇腾NPU采用达芬奇架构；CANN于2025年开源开放（背景信息）
- [S229] V2EX·中国的算力缺口这么大嘛?看到 2025 华为昇腾出货 81 万块… | https://www.v2ex.com/t/1207741 | 平台 V2EX(社区，真实命中) | 访问日期 2026-09-12 | 等级 C | 状态 partial | 用途：反映昇腾软件生态短板与写算子难度（社区真实讨论） | 可支持的结论：社区侧对"昇腾写算子费劲、CUDA/Triton生态好用"有广泛共鸣；**仅1条真实V2EX命中，未达≥3条验收**
- [S230] 百度百科·华为昇腾（产品系列/950PR-DT/970路线） | https://baike.baidu.com/item/%E5%8D%8E%E4%B8%BA%E6%98%87%E8%85%BE/68598325 | 平台 百度百科 | 访问日期 2026-09-12 | 等级 C | 状态 verified | 用途：昇腾型号参数、CANN开源、Atlas系列形态 | 可支持的结论：昇腾950PR/DT分别面向推理prefill/decode；CANN已开源开放（背景）
- [S231] hiascend·昇腾社区官方论坛首页（论坛性质，非V2EX/linux.do） | https://www.hiascend.com/forum | 平台 hiascend(官方) | 访问日期 2026-09-12 | 等级 A | 状态 verified | 用途：说明"昇腾论坛"主要指hiascend官方论坛，与V2EX/linux.do区分 | 可支持的结论：昇腾相关讨论主阵地是hiascend官方论坛，非V2EX/linux.do
- [S232] hiascend·打破CUDA枷锁：昇腾如何重塑大模型时代的国产AI算力版图（论坛帖，非V2EX） | https://www.hiascend.com/forum/thread-02178215697926099297-1-1.html | 平台 hiascend(官方论坛) | 访问日期 2026-09-12 | 等级 C | 状态 verified | 用途：达芬奇架构vs CUDA、CANN生态 | 可支持的结论：作为"非V2EX帖子"的证据，说明检索到的昇腾讨论多集中在hiascend官方论坛
- [S233] CSDN·昇腾社区·AI推理的NPU加速:cann-recipes-harmony-infer实战（鸿蒙NPU，非Linux服务端） | https://harmonyosdev.csdn.net/6a11d44a10ee7a33f274ae10.html | 平台 CSDN(社区) | 访问日期 2026-09-12 | 等级 C | 状态 verified | 用途：说明鸿蒙NPU推理与Linux/Android NPU推理接口不兼容（不同环境） | 可支持的结论：鸿蒙NPU推理用NAPI/零拷贝，与Linux服务端CANN/ccec路线不同，本题不采用
- [S234] Tom's Hardware·Huawei Ascend NPU roadmap(950/960/970, FP8/FP4, UnifiedBus) | https://www.tomshardware.com/tech-industry/artificial-intelligence/huawei-ascend-npu-roadmap-examined-company-targets-4-zettaflops-fp4-performance-by-2028-amid-manufacturing-constraints | 平台 Tom's Hardware(媒体) | 访问日期 2026-09-12 | 等级 C | 状态 verified | 用途：950系列SIMD+SIMT架构、FP8/MXFP4/HiF4、UnifiedBus互联 | 可支持的结论：950起走SIMD+SIMT（类GPGPU），算子开发更高效，可匹配CUDA算子（背景，非实操）
- [S235] 21世纪经济报道·昇腾破局 国产算力不再低调（DeepSeek/CloudMatrix 384） | https://www.21jingji.com/article/20250624/4308cca8d1655447800e83b326361246.html | 平台 21财经(媒体) | 访问日期 2026-09-12 | 等级 C | 状态 verified | 用途：昇腾超节点、系统工程、CANN开源背景 | 可支持的结论：昇腾通过超节点/系统工程补单芯片差距；生态仍落后CUDA（背景）
- [S236] 华尔街见闻·华为AI芯片来时的路（《昇腾崛起》整理，CANN灵魂论） | https://wallstreetcn.com/articles/3779151 | 平台 华尔街见闻(媒体) | 访问日期 2026-09-12 | 等级 C | 状态 verified | 用途：CANN被称为"昇腾的灵魂"、Ascend C由Tik演进、全下沉调度 | 可支持的结论：CANN/毕昇是算子开发核心；Ascend C于2022定名（背景，解释为何写算子是生态重点）
- [S237] 海昆鹏·基于鲲鹏CPU推理服务器部署DeepSeek 70B（CANN 8.1.RC1、kernels、nnal） | https://www.hikunpeng.com/developer/techArticles/20250602-1 | 平台 海昆鹏(鲲鹏社区) | 访问日期 2026-09-12 | 等级 C | 状态 verified | 用途：CANN toolkit/kernels/nnal安装、set_env.sh、torch_npu版本、绑核优化 | 可支持的结论：CANN安装含toolkit+kernels+nnal；set_env.sh激活；版本配套
- [S238] CSDN·Ascend 910B NPU 部署文档（驱动/固件安装实操，社区版） | https://ascendai.csdn.net/69d4c82c72111d255bf7ca27.html | 平台 CSDN(社区) | 访问日期 2026-09-12 | 等级 C | 状态 verified | 用途：HwHiAiUser创建、driver/firmware下载与安装、npu-smi info | 可支持的结论：910B部署前必须先装驱动固件并npu-smi验证（与步骤1一致）
- [S239] 百度百科·华为集群计算（CANN开源、灵衢UnifiedBus、950/960/970路线） | https://baike.baidu.com/item/%E5%8D%8E%E4%B8%BA%E9%9B%86%E7%BE%A4%E8%AE%A1%E7%AE%97/58964236 | 平台 百度百科 | 访问日期 2026-09-12 | 等级 C | 状态 verified | 用途：CANN开源(2025-12)、灵衢协议、集群计算方案 | 可支持的结论：CANN开源开放已成定局，社区版下载不受限（支撑步骤2版本选择）
- [S240] Agent 8 检索受限声明（linux.do/V2EX 站内搜索接口 fetch failed） | 站内检索 https://linux.do/search.json?q=Ascend 与 https://www.v2ex.com/search?q=昇腾 均 fetch failed | 平台 检索工具(受限) | 访问日期 2026-09-12 | 等级 D | 状态 unavailable | 用途：记录 linux.do/V2EX 站内搜索不可达，佐证"未命中/受限"结论 | 可支持的结论：linux.do/V2EX 真实帖子须以可访问页面为准；当前无法直连核验，故仅以搜索引擎命中为准，未编造

## Agent 9 — 竞赛经验与失败案例

- **编号段**：S241–S270
- **平台**：GitHub Issue/PR、GitCode、Kaggle/AIcrowd/Codeforces、CANN 社区、历史算子竞赛页
- **条目数**：30

- [S241] 昇腾AI创新大赛-算子挑战赛FAQ(编译报错解决方案) | https://www.hiascend.com/dev/forum/thread-0259189679380603071-1-1.html | 平台 hiascend 社区 | 访问日期 2026-09-12 | 等级 C | 状态 verified | 用途：Compile Error 五类案例 | 可支持的结论：C2（API 签名/产品系列/路径类编译错）
- [S242] [算子挑战赛-S2赛季] 如何查看作品运行状态码 | https://developer.huawei.com/home/forum/ascend/thread-0272156485005269337-1-1.html | 平台 华为开发者联盟 | 访问日期 2026-09-12 | 等级 A | 状态 verified | 用途：判题状态码语义 | 可支持的结论：C15（Install failed/Wrong answer/Run failed/Fail,target,result/-1/Pass）
- [S243] 揭开算子精度调试黑箱(MindStudio/msSanitizer) | https://www.hiascend.com/dev/forum/thread-0297191474168807357-1-1.html | 平台 hiascend 社区 | 访问日期 2026-09-12 | 等级 C | 状态 verified | 用途：非 32B 对齐精度异常 | 可支持的结论：C10（UB 非对齐→精度崩）
- [S244] 算子开发常见问题(cannkit-faqs) | https://developer.huawei.com/consumer/cn/doc/hiai-GUIdes/cannkit-faqs-operator-development-0000002300578238 | 平台 华为官方文档 | 访问日期 2026-09-12 | 等级 A | 状态 verified | 用途：编译/精度/运行时错误 | 可支持的结论：C13（Cannot find compile result file / json 不匹配）、精度比对 md5 不一致
- [S245] Kernel编译时报错"error: out of jump/jumpc imm range" | https://asc.gitcode.com/guide/%E7%BC%96%E7%A8%8B%E6%8C%87%E5%8D%97/%E9%99%84%E5%BD%95/FAQ/Kernel%E7%BC%96%E8%AF%91%E6%97%B6%E6%8A%A5%E9%94%99-error-out-of-jump-jumpc-imm-range.html | 平台 gitcode 官方指南 | 访问日期 2026-09-12 | 等级 A | 状态 verified | 用途：大 kernel 编译失败 | 可支持的结论：C3（kernel 过大→跳转超 int16 范围）
- [S246] 从零构建:Ascend C算子工程项目创建与结构全解 | https://blog.csdn.net/zxylovezxylovezxy/article/details/155717943 | 平台 CSDN | 访问日期 2026-09-12 | 等级 C | 状态 partial | 用途：kernel 内误用 C++ 标准库 | 可支持的结论：C4（std::vector 在 kernel 内编译错）
- [S247] ascendc算子常见问题FAQ(GitCode cann-recipes-infer #1) | https://gitcode.com/cann/cann-recipes-infer/discussions/1 | 平台 GitCode 官方 recipe 仓 | 访问日期 2026-09-12 | 等级 B | 状态 verified | 用途：A2 编译需 -c ascend910b | 可支持的结论：A2 编译/环境差异（Compile Error 相关）
- [S248] AscendC DataCopyPad 写出溢出 Bug 详解 | https://hwcomputing.csdn.net/6a38f47710ee7a33f280c253.html | 平台 鲲鹏昇腾社区 | 访问日期 2026-09-12 | 等级 C | 状态 verified | 用途：DMA 非对齐多写覆盖相邻段 | 可支持的结论：C8（静默数据损坏）
- [S249] ReduceSum API 文档(官方) | https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/82RC1/API/ascendcopapi/atlasascendc_api_07_10017.html | 平台 hiascend 官方 | 访问日期 2026-09-12 | 等级 A | 状态 verified | 用途：ReduceSum 溢出/对齐约束 | 可支持的结论：C11（内部不处理溢出、srcInnerPad 仅支持 true）
- [S250] 昇腾 AscendC ReduceSum 算子实现详解 | https://hwcomputing.csdn.net/6a9e70de48977663a5dda4b0.html | 平台 鲲鹏昇腾社区 | 访问日期 2026-09-12 | 等级 C | 状态 verified | 用途：reducePattern/work 空间/对齐 | 可支持的结论：C11 补充（work 空间、32B 对齐尾块）
- [S251] CANNJudge 算子竞赛全流程指南 | https://blog.csdn.net/gitblog_00467/article/details/152189083 | 平台 CSDN(聚合帖) | 访问日期 2026-09-12 | 等级 C | 状态 partial | 用途：提交状态/泛化清单 | 可支持的结论：仅过程性事实（状态名/计分）被官方 S242 与已知规则佐证；失败 anecdote 不采信
- [S252] CANN算子竞赛代码提交脚本说明 | https://blog.csdn.net/gitblog_00223/article/details/153107013 | 平台 CSDN(聚合帖) | 访问日期 2026-09-12 | 等级 C | 状态 partial | 用途：提交流程/路径穿越检查 | 可支持的结论：提交脚本过程事实；无 primary 链接，失败断言不采信
- [S253] 大文件分片上传后文件损坏:大小不一致与解压失败 | https://juejin.cn/post/7533048851199049778 | 平台 掘金 | 访问日期 2026-09-12 | 等级 C | 状态 verified | 用途：上传截断/损坏工程复盘 | 可支持的结论：C7（传输层丢包/MTU→文件损坏，对应 V002）
- [S254] 如何解决超过1GB的文件压缩包上传失败 | https://www.cnblogs.com/hwrex/p/18643473 | 平台 博客园 | 访问日期 2026-09-12 | 等级 D | 状态 partial | 用途：大文件上传失败背景 | 可支持的结论：仅作上传失败背景，无 CANN 证据
- [S255] CANN预选赛算子测试 | https://blog.csdn.net/gitblog_00244/article/details/160917058 | 平台 CSDN(聚合帖) | 访问日期 2026-09-12 | 等级 C | 状态 partial | 用途：赛事规则/计分公式 | 可支持的结论：计分公式与「全过才计分」被本题已知规则佐证；失败 anecdote 不采信
- [S256] 首届智能算子测试大赛收官 | https://blog.csdn.net/bq990914/article/details/160583930 | 平台 CSDN | 访问日期 2026-09-12 | 等级 C | 状态 verified | 用途：隐藏测试点/失分分布 | 可支持的结论：C16（边界≈35%、数值稳定性≈28%）
- [S257] Competitive Programming 3 (Steven Halim) | https://raw.githubusercontent.com/prasadgujar/CompetitiveProgramming/refs/heads/master/book/Competitive%20Programming%203.pdf | 平台 GitHub(raw) | 访问日期 2026-09-12 | 等级 B | 状态 verified | 用途：竞赛秘密用例/边界铁律 | 可支持的结论：C17（全过才计分、N=0/1、32 位溢出、TLE）
- [S258] 基本流程(CANN 9.0.0-beta.1 官方) | https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/900beta1/opdevg/Ascendcopdevg/atlas_ascendc_10_0064.html | 平台 hiascend 官方 | 访问日期 2026-09-12 | 等级 A | 状态 verified | 用途：GetValue/SetValue 多核一致性 | 可支持的结论：C18（DataCache 需 CleanAndInvalid、Scalar 与 MTE 依赖）
- [S259] 非对齐问题背景与 DataCopyPad(官方) | https://www.hiascend.com/document/detail/zh/canncommercial/80RC2/developmentguide/opdevg/Ascendcopdevg/atlas_ascendc_10_0021.html | 平台 hiascend 官方 | 访问日期 2026-09-12 | 等级 A | 状态 verified | 用途：非对齐尾块写 -1 覆盖 | 可支持的结论：C12（DataCopy 搬 11 half 实际写 16，尾部覆盖）
- [S260] 深入解析华为CANN算子开发:从Tiling到Kernel | https://bbs.huaweicloud.com/blogs/469425 | 平台 华为云社区 | 访问日期 2026-09-12 | 等级 C | 状态 verified | 用途：BF16 在 A2 上 Add 不支持 | 可支持的结论：C14（需 Cast→float→Cast 绕行）
- [S261] CANNBot Ascend C直调开发指南 | https://blog.csdn.net/gitblog_00909/article/details/151507556 | 平台 CSDN(聚合帖) | 访问日期 2026-09-12 | 等级 C | 状态 partial | 用途：.asc 后缀/入口属性/禁止前向声明 | 可支持的结论：直调工程规范（与 V001 保留字/入口相关）
- [S262] Lab 3.5 昇腾算子开发与优化(fused_add_rmsnorm) | https://hpc101.zjusct.io/lab/Lab3.5-AscendC-Op/ | 平台 高校实验 | 访问日期 2026-09-12 | 等级 B | 状态 verified | 用途：本题算子原型的实验题背景 | 可支持的结论：确认 AddRmsNorm 是真实考题形态（背景，非失败案例）
- [S263] [BUG] Ascend C compilation fails because paged attention uses reserved block_idx | https://github.com/InfiniTensor/InfiniCore/issues/1519 | 平台 GitHub | 访问日期 2026-09-12 | 等级 B | 状态 verified | 用途：保留字冲突导致编译失败 | 可支持的结论：C1（block_idx 内建变量被当局部变量→编译拒绝）
- [S264] [Bug] T.tile.round produces incorrect results for float16 (PTO backend) | https://github.com/tile-ai/tilelang-ascend/issues/1637 | 平台 GitHub | 访问日期 2026-09-12 | 等级 B | 状态 verified | 用途：float16 舍入静默错误 | 可支持的结论：C9（编译运行正常但输出不符、无异常）
- [S265] 出包错误日志(EinSpi/scripts #69) | https://github.com/EinSpi/scripts/issues/69 | 平台 GitHub | 访问日期 2026-09-12 | 等级 B | 状态 partial | 用途：ascend 出包编译日志 | 可支持的结论：编译期 warning/错误日志形态（辅助 C13）
- [S266] 算法竞赛避坑指南:从本地AC到线上WA | https://www.hqwc.cn/a/1048594.html | 平台 通用技术站 | 访问日期 2026-09-12 | 等级 D | 状态 partial | 用途：通用竞赛避坑 | 可支持的结论：无 CANN 专属证据，**未采信**（仅通用方法论旁证）
- [S267] CANN/cann-learning-hub: Ascend C API 最佳实践速查 | https://blog.csdn.net/gitblog_00034/article/details/160918316 | 平台 CSDN(聚合帖) | 访问日期 2026-09-12 | 等级 C | 状态 partial | 用途：SetValue/GetValue 效率低、32B 对齐规则 | 可支持的结论：对齐规则与 C10/C18 互证；具体断言无 primary 链接
- [S268] 深度解析 32-Byte 内存对齐与 Burst 性能哲学 | https://blog.csdn.net/2401_82857325/article/details/156026480 | 平台 CSDN | 访问日期 2026-09-12 | 等级 C | 状态 verified | 用途：ALIGN_UP 越界 OOM/尾部风险 | 可支持的结论：非对齐尾块越界风险（补充 C12）
- [S269] Ascend C算子开发进阶:非对齐尾块处理与DataCopyPad实战 | https://blog.csdn.net/2401_82857325/article/details/155320399 | 平台 CSDN | 访问日期 2026-09-12 | 等级 C | 状态 partial | 用途：DataCopyPad 尾块处理示例 | 可支持的结论：尾块处理形态（与 C8/C12 一致）
- [S270] ops-mathExpandAdapt0313 mul_addn_align_bf16.h | https://atomgit.com/luwenxiang1998/ops-mathExpandAdapt0313/blob/master/math/mul_addn/op_kernel/mul_addn_align_bf16.h | 平台 AtomGit | 访问日期 2026-09-12 | 等级 C | 状态 verified | 用途：BF16 对齐分支实现样例 | 可支持的结论：C14 佐证（bf16 对齐/分支处理）

## Agent 10 — 证据审阅与方案合并（含本地一手材料）

- **编号段**：S271–S300
- **平台**：本地模板、提交快照、项目文档、A1–A9 报告复核
- **条目数**：21

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

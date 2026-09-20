# Agent 9 报告：竞赛经验与失败案例调研（AddRmsNormBias）

- 日期：2026-09-11
- 任务：收集真实失败案例（编译错误 / 答案错误 / 运行错误 / 超时 / 精度 / 尾块越界 / ReduceSum / 上传包 / 提交次数 / 隐藏测试点），避免重蹈覆辙。
- 证据纪律：每条案例标注证据等级（A=官方文档/官网原文，B=官方仓库/官方博客，C=社区帖子/Issue/技术博客）与"是否在本机验证"（**一律：未验证**，本机 macOS 无 CANN 无 NPU）。
- 引用纪律：只采用有实质内容（现象+根因+细节）的案例；"结论一句话、无细节"的经验贴不采用（见文末"未采用来源说明"）。

---

## 一、案例清单（按失败类别）

### 类别 1：Compile Error（编译错误）

**案例 1.1｜CANNJudge 判题环境与本地版本不一致 → 命名空间编译失败**
- 链接：https://blog.csdn.net/gitblog_01418/article/details/150380401 （CANN学习中心 as_strided 实战，作者用 CANNBot 在 CANNJudge 上实战 S2 真题）
- 现象：首次提交即 **Compile Error**。
- 根因：CANNJudge 判题环境使用 CANN 8.5，Ascend C 类型位于 `AscendC::` 命名空间下；而本地 CANN 9.0 默认 `using namespace AscendC`。同一份代码在本地编译通过、判题环境编译失败。
- 教训：**本地编译通过 ≠ 判题环境编译通过**。不要依赖隐式命名空间；跨环境代码显式写 `AscendC::` 前缀，或以判题环境（题面指定的 CANN 9.0.0）为准。
- 证据等级：C；本机未验证。

**案例 1.2｜`__aicore__` 修饰符错误使用**
- 链接：https://cann.csdn.net/69ef0b9b54b52172bc704775.html （CANN 开发者社区《开发工具链介绍：从编译到调试》）
- 现象：`error: '__aicore__' attribute only applies to function types`。
- 根因：把 `__aicore__` 用在类声明上（`class __aicore__ MyClass {}`），该属性只允许修饰函数。
- 教训：`__aicore__` 只修饰成员函数/核函数；类定义本身不带该属性。
- 证据等级：C；本机未验证。

**案例 1.3｜msopgen / CANN 工具链版本 ABI 不匹配 → 工程生成/编译失败**
- 链接：https://ask.csdn.net/questions/9266350 （CANN 与 MindSpore 版本不匹配导致算子编译失败）
- 现象：`msopgen` 执行失败提示 `Failed to load op_proto ... Invalid argument`；TBE 编译报 `incompatible proto version 3.1 vs expected 4.0`；链接报 `undefined symbol: aclGetRecentErrMsg` / `version mismatch for libascendcl.so.2`。
- 根因：msopgen（CANN 8.0 起 OpProto 从 Protobuf v3.11 升 v3.21）与 MindSpore/TBE 插件 ABI 语义差异；驱动版本＜CANN 版本倒挂。
- 教训：工具链必须整体版本一致（msopgen/aot/CANN/driver 配套）；`msopgen --version` 与 `aot --version` 版本混杂即报错源。
- 证据等级：C（社区问答，含机制分析）；本机未验证。

**案例 1.4｜Kernel 文件后缀必须是 `.asc`**
- 链接：https://blog.csdn.net/gitblog_00909/article/details/151507556 （CANNBot Ascend C 直调开发指南，cannbot-skills 官方仓库转述）
- 现象：Kernel 代码文件用 `.cpp/.cc` 时 ASC 编译器不识别；`<<<>>>` 内核启动语法仅在 `.asc` 文件中有效。
- 根因：直调（direct invoke）模式编译器只识别 `.asc` 后缀。
- 教训：CANNJudge 直调模板的核函数文件必须命名为 `*.asc`；文件内代码顺序固定（include → Kernel 类 → 核函数入口 → Host KernelCall → main）。
- 证据等级：B（官方仓库 cann/cannbot-skills）；本机未验证。

**案例 1.5｜链接库路径错误 → `undefined reference to aclInit`**
- 链接：https://blog.csdn.net/pz890123/article/details/152697389 （2025昇腾训练营避坑指南）
- 现象：`CMakeFiles/add_custom_test.dir/test_add_custom.cpp.o: In function 'main': undefined reference to 'aclInit'`。
- 根因：链接库路径不对或 CMake 未链接 acllib；另见官方 MindSpore 文档同类：`gmake: *** No rule to make target 'package'. Stop.` 是因为未传 `--cann_package_path`（https://www.mindspore.cn/docs/zh-CN/r2.4.0/model_train/custom_program/operation/op_custom_ascendc.html，A/B 级）。
- 教训：编译前核对 CANN 包路径与 `LD_LIBRARY_PATH`；CANN 版本≥8.0.RC2.alpha003、驱动版本≥CANN 版本。
- 证据等级：C（B 级佐证：MindSpore 官方文档）；本机未验证。

### 类别 2：Wrong Answer（答案错误）

**案例 2.1｜BF16 尾数不足 + 舍入模式 → 精度不达标（关键案例）**
- 链接：https://www.hiascend.com/developer/blog/details/02127205558228788013 （XJTUACM 算子挑战赛 S7 经验总结，官方博客；源码仓 https://github.com/ShwStone/Ascend-S7）
- 现象：BF16 尾数只有 7 位，无法直接达到题面千分之一精度要求。
- 根因：输出转换必须**还原 CPU 参考实现的舍入机制**；实测 torch 的舍入是 `RoundMode::CAST_RINT`（四舍六入五成双）。
- 教训：**FP16/BF16 输出 Cast 的 RoundMode 必须与参考实现一致**（half→fp32 用 CAST_NONE；fp32→half/bf16 用 CAST_RINT）。这正是我们首版实现已采用的模式，真机需对照题面参考实现复核。
- 证据等级：B（官方博客 + 公开源码）；本机未验证。

**案例 2.2｜中间结果 Cast 回低精度反而更"匹配 golden"（反向陷阱）**
- 链接：https://developer.huawei.com/home/forum/ascend/thread-02200223658291993472-1-1.html （昇腾AI创新大赛算子挑战赛 S8 赛季 Scale 算子优化分享，本赛题成绩最优）
- 现象：作者把乘法中间结果全程留在 fp32（数学上更精确），结果反而对不上 golden；把中间结果 Cast 回 bf16 后通过。
- 根因：**判题精度验收比的是"和参考实现一致"，不是"和数学真值一致"**；参考实现每一步的中间精度都是固定的。
- 教训：必须按参考实现逐步复现其中间 Cast 时机；"更高精度"可能是 WA 源。对本题：参考实现（PyTorch 组合）中 y^2 归约、rsqrt、gamma/bias 乘加的中间精度需在真机逐个核对。
- 证据等级：C（官方论坛）；本机未验证。

**案例 2.3｜FP16 大数吃小数（归约精度）**
- 链接：https://blog.csdn.net/2301_80840905/article/details/155034892 （避坑指南：Ascend C 算子开发常见报错解析与精度优化复盘）
- 现象：FP16 对差异大的数做 ReduceSum，小数被"吃掉"，结果偏差严重。
- 根因：FP16 有效位数少，归约累加需升格。
- 教训：中间计算升格 FP32（Cast → fp32 运算 → Cast 回输出），与本题"归约必须 FP32"的约束一致。
- 证据等级：C；本机未验证。
- 佐证：https://hwcomputing.csdn.net/6a156b8510ee7a33f2754f4b.html （CANN ops-transformer RMSNorm 数值精度分析）：FP32 朴素求和 10⁷ 项后也会丢约 1 ULP；Kahan 求和把归约误差从 O(n·ε) 降到 O(ε)。本题 D ≤ 32768，FP32 朴素归约理论可承受，但真机需按题面误差阈值核对是否需 Kahan。

**案例 2.4｜half 中间函数上溢（特殊值边界）**
- 链接：https://www.hiascend.com/developer/blog/details/02127205558228788013 （XJTUACM S7：Softplus 题）
- 现象：half 精度 `Exp` 结果最大只能到 65506，更大输入算出的结果偏小（上溢饱和）。
- 根因：半精度中间函数上溢饱和。
- 教训：半精度链路中会产生大中间值的函数需要"与输入取最大"等兜底。本题平方和可能很大（D 大、|y| 大），归约升格 FP32 是必要防溢出手段。
- 证据等级：B；本机未验证。

**案例 2.5｜参考算子默认值/特殊语义理解错误**
- 链接：https://blog.csdn.net/gitblog_00410/article/details/143789539 （cann-learning-hub CANNJudge 算子提交 Skill；gitcode.com/cann/cann-learning-hub）
- 现象：`torch.histc(min=0, max=0)` 被实现成范围 [0,0]，正确语义是"min==0 && max==0 → 自动计算数据范围"。WA。
- 根因：没有查阅参考算子官方文档的默认值特殊语义。
- 教训：本题的 `epsilon`、`gamma/bias` 广播、非 32 倍数 D 的参考行为都必须在题面/参考实现确认后再写；平台不开放测试用例 API，必须泛化设计（shape/dtype/属性/对齐/边界）。
- 证据等级：B（官方仓库）；本机未验证。

### 类别 3：Runtime Error / 崩溃

**案例 3.1｜DataCopy 地址非对齐直接挂掉（最常见）**
- 链接：https://blog.csdn.net/2301_80840905/article/details/155034892
- 现象：运行到 `DataCopy` 指令时直接 Core Dump / `Aicore Error` / Segmentation Fault。
- 根因：DMA 搬运对 GM/Local 首地址要求 32 字节对齐（部分指令 512 字节），非对齐地址硬件直接拒绝执行。
- 教训：Host Tiling 保证块大小是 32B 倍数；无法对齐时用支持非对齐的 API（DataCopyPad）或 UB 拼接。对应官方 npucheck 错误码：`ErrorRead4 = 读取地址非 32 字节对齐`（https://www.hiascend.com/doc_center/source/zh/CANNCommunityEdition/80RC3alpha003/devaids/auxiliarydevtool/atlasascendebug_16_0133.html，A 级）。
- 证据等级：C（A 级佐证：官方 npucheck 文档）；本机未验证。

**案例 3.2｜Tiling 结构体字节对齐错位 → 随机乱码/越界**
- 链接：https://blog.csdn.net/2301_80840905/article/details/155034892
- 现象：结果随机乱码或越界访问。
- 根因：Host 与 Device 两侧各自定义 Tiling 结构体，编译器对齐设置不同，Host 的 `int64` 被 Device 当两个 `int32` 读，参数全错位。
- 教训：Tiling 结构体加 `__attribute__((packed))`，或成员按从大到小排序（先 int64 再 int32）；Host/Device 共用同一份头文件。
- 证据等级：C；本机未验证。

**案例 3.3｜忘记 FreeTensor → 流水线死锁**
- 链接：https://blog.csdn.net/2301_80840905/article/details/155034892
- 现象：跑前几块数据正常，之后程序卡死。
- 根因：队列深度有限，`DeQue` 拿数据后未配对 `FreeTensor`，下游不还 buffer，上游塞不进去，流水线堵死。
- 教训：`DeQue` 必须配对 `FreeTensor`（有借有还）；核内每个分支都要保证释放。
- 证据等级：C；本机未验证。

**案例 3.4｜缺少同步屏障 → 结果全 0 / 上一轮残留**
- 链接：https://blog.csdn.net/2301_80840905/article/details/155034892
- 现象：输出全 0 或残留上一轮值。
- 根因：NPU 异步执行，搬入指令发出后立即计算，数据还在路上。
- 教训：CopyIn 之后、Compute 之前必须有 EnQue/DeQue 依赖或 PipeBarrier 同步。
- 证据等级：C；本机未验证。

**案例 3.5｜UB 地址不对齐 → AICORE 错误码 0x10（指令非法）**
- 链接：https://www.hiascend.com/doc_center/source/zh/canncommercial/601/devtools/auxiliarydevtool/atlasaicoreerr_16_0010.html （官方定位案例指南：错误码 0x10）
- 现象：`AICERROR code: 0x10`（指令非法），一般因 UB 地址不对齐或 scalar 指令不合法。
- 根因（官方案例）：一个 TIK 算子只支持 cloud 版，执行后把编译版本从 mini 切到 cloud，导致后续 mini 环境算子 UB 越界报非法指令。
- 教训：AICORE 错误 0x10 优先查 UB 对齐与指令合法性；注意编译 arch/形态（mini/cloud）一致性。
- 证据等级：A；本机未验证。

**案例 3.6｜Kernel 栈上大数组 → PC 跳飞/非法指令**
- 链接：https://blog.csdn.net/2401_82857325/article/details/155754900 （昇腾CANN训练营·黑客篇：BlackBox 与 Exception Dump 定位 NPU 死机）
- 现象：PC 指针跳到奇怪位置，报非法指令。
- 根因：Kernel 内声明 `float temp[1024]` 这类大局部数组；AI Core 栈空间极小（几 KB）。
- 教训：大块内存必须从 UB（TPipe/TBuf）或 GM 申请，严禁栈上开大数组。
- 证据等级：C；本机未验证。

**案例 3.7｜CopyOut 写越界踩内存（Memory Stomp）**
- 链接：https://blog.csdn.net/2401_82857325/article/details/155754900
- 现象：算子 A 跑完后算子 B 挂掉，或 Host 收到乱码——写越界默默改写了相邻内存（下一个算子输入/页表）。
- 根因：`DataCopy` 的 len 超出 `AllocTensor` 大小。
- 教训：检查 Tiling，DataCopy len 不超出分配大小；若总是同一 Core 挂，优先查 Tiling 边缘（尾块）处理。
- 证据等级：C；本机未验证。

**案例 3.8｜SetFlag/WaitFlag 配对陷阱 → 超时/死锁**
- 链接：https://blog.csdn.net/weixin_27442001/article/details/159974663 （昇腾 Ascend C 算子开发中核异常与同步机制失效的深度排查）
- 现象：`halCqReportRecv failed` + 错误码 507014（aicore 执行超时）；部分分支下必现、部分概率出现。
- 根因：条件分支提前 `return` 导致 `WaitFlag` 未被调用，核永远等不到信号；跨核同步顺序错乱（WaitFlag 早于 SetFlag）。
- 教训：SetFlag/WaitFlag 必须覆盖所有分支；先定位首个异常算子（核异常有传染性，一个算子越界可能污染整个 NPU，连官方样例都受牵连）。
- 证据等级：C；本机未验证。

### 类别 4：Timeout（超时）

**案例 4.1｜算子执行超时统计（Pdist 赛题复盘）**
- 链接：https://hwcomputing.csdn.net/69b6a2570a2f6a37c5979df8.html （基于 Ascend C 的 Pdist 自定义算子开发复盘）
- 现象：该文对赛题期错误做了量化统计：算子执行超时占比 2.4%（6 例），脚本 >30s 主动退出。
- 根因与排查：`for_range` 存在死循环；超大张量 + 单核未并行；tile 过大导致 UB 溢出降级。
- 教训：检查循环出口条件；小 shape 主动降核（核数不是越多越好）；UB 预算必须覆盖最大 tile 的临时空间。
- 证据等级：C；本机未验证。

**案例 4.2｜核异常导致的"伪超时"**
- 链接：https://blog.csdn.net/weixin_27442001/article/details/159974663
- 现象：AICore 超时（507014），根因其实是同步缺失/核异常，而非计算慢。
- 教训：超时先查同步与越界，再查性能。
- 证据等级：C；本机未验证。

**案例 4.3｜通信死锁（外围参考）**
- 链接：https://www.hiascend.com/developer/techArticles/20260603-10?envFlag=1 （官方技术文章：Atlas 900 A3 AICPU 占满导致执行卡死）
- 现象：HCCL `notify/wait` 超时，AICPU 与通信算子形成循环依赖死锁。
- 根因：资源竞争 + 循环依赖（AICPU 等通信完成、通信等 AICPU 执行）。
- 教训（与本算子相关部分）：死锁排查按 plog → stream/notify → 事件依赖链 顺序定位卡点。本算子无通信，但同源经验适用于核间同步死锁排查。
- 证据等级：A；本机未验证。

### 类别 5：精度不稳定

见类别 2 案例 2.1/2.2/2.3/2.4，另有：

**案例 5.1｜Pdist 混合精度：float16 全流程精度偏差明显**
- 链接：https://hwcomputing.csdn.net/69b6a2570a2f6a37c5979df8.html
- 现象：float16 全流程计算精度偏差明显（误差需 ≤1e-4），改"输入输出 fp16 + 中间 fp32"后通过。
- 根因：fp16 中间运算（幂/开方）精度损失超标。
- 教训：输入输出保目标类型、中间升格 fp32——与本题官方训练营"黄金法则"一致；最后结果转回 fp16 时用 CAST_RINT。
- 证据等级：C；本机未验证。

### 类别 6：尾块越界（D 非 32 倍数）

**案例 6.1｜DataCopyPad 写出 padding 覆盖相邻数据（关键案例，直击本题）**
- 链接：https://blog.csdn.net/ferriswym/article/details/162206787 （【昇腾/AscendC开发】AscendC DataCopyPad 写出溢出 Bug 详解）
- 现象：Segment Reduce 算子 `feat_dim=4`（每行 16 字节）用 DataCopyPad 从 UB 写 GM，每段写出 16B 数据但 DMA 实际搬运 32B（burst 最小单位），padding 覆盖了相邻段前 4 字节。seg 2 是空段不写出，导致它的前 4B 残留被 padding 污染，最终结果错误。
- 根因：**DataCopyPad UB→GM 方向，搬运单位 burst 必须 32 字节对齐**；有效数据不足 32B 倍数时 DMA 多写 padding，覆盖相邻数据。
- 教训：**行间连续布局 + 尾块 DataCopyPad 写出 = 覆盖下一行开头**。本题 D 非 32 倍数时（尤其 D 较小、行间距仅几字节），上一行尾块写出若多写 padding 会踩下一行；必须验证 DataCopyPad 在写方向的精确语义（blockLen 是否为精确字节数），必要时改用"按行 32B 对齐步长写 + 标量/逐元素补齐真实尾元素"或先验证手册。
- 证据等级：C；本机未验证。

**案例 6.2｜框架自动尾块处理失效（N=513 错误）**
- 链接：https://github.com/tile-ai/tilelang-ascend/issues/890 （[Bug] RMSNorm 在 N=513 时结果错误，自动尾块处理未生效或存在缺陷）
- 现象：TileLang Ascend 实现 RMSNorm，N=512 正确、N=513 输出明显错误；框架宣称内置自动尾块，实际未生效。
- 根因：非对齐长度（513）自动尾块处理有缺陷。
- 教训：**不能依赖框架/模板宣称的自动尾块**；D 非 32 倍数必须自己构造最小复现验证尾块边界（本机用 CPU 参考实现做差分验证，真机跑 N=64+1 这类形状）。
- 证据等级：C（GitHub Issue）；本机未验证。

### 类别 7：ReduceSum 错误

**案例 7.1｜ReduceSum(count) 内部实际能力上限 4096 元素（关键案例）**
- 链接：https://www.hiascend.com/developer/blog/details/02127205558228788013 （XJTUACM S7）
- 现象：文档宣称 ReduceSum 可对任意长度求和，实际内部至多调用两次 `WholeReduceSum`；一次 `WholeReduceSum` 对 256B 数据（64 个 float）求和，所以 **ReduceSum(count 版本) 至多只能对 4096 个元素求和**。作者被此 bug 卡了很久，最终实现分段求和（WholeReduceSum 255 repeat 一轮 + 256 元素缓冲区累加）。
- 根因：count 版本 ReduceSum 的底层实现能力限制；workLocal/dst 对齐与 mask 语义随硬件形态（A2/训练/推理）不同。
- 教训：本题 D ∈ [64, 32768]。**D > 4096 时不能依赖单次 ReduceSum(count=len)**；要么按 ≤4096 分块多次 ReduceSum 再标量累加（我们首版 tileLen=2048/4096 元素，count ≤ 4096，理论安全但需真机确认），要么手工 WholeReduceSum 分段。workLocal 最小空间按官方公式计算（iter1OutputCount = count/64 向上取整到 8 的倍数），不能拍脑袋给 1024。
- 证据等级：B（官方博客 + 公开源码）；本机未验证。
- 佐证（A 级）：官方 ReduceSum API 手册 https://www.hiascend.cn/document/detail/zh/canncommercial/800/apiref/ascendcopapi/atlasascendc_api_07_0078.html —— workLocal 空间计算公式、dst 起始地址 2/4 字节对齐、src 32 字节对齐、数据类型必须一致、最大处理数据量不能超过 UB 限制。

### 类别 8：上传包错误

**案例 8.1｜CANNJudge 提交字段与工程结构（官方 Skill 文档）**
- 链接：https://blog.csdn.net/gitblog_00410/article/details/143789539 ；原文 https://gitcode.com/cann/cann-learning-hub/blob/master/skills/cannjudge-submit/README.md
- 现象/事实：提交 API `POST /api/submissions/submit` 的 body 字段为 `problemId / userId / tiling_h / tiling_key_h / host_cpp / kernel_cpp`（文件不存在则提交空字符串）；判题状态：`Running / Accepted / Wrong Answer / Compile Error / Runtime Error`；用例结果含 `testcase_status / precision_ratio（1.0=完全匹配）/ time(ms)`。
- 工程模板结构：`code/op_host/{op}.cpp`、`code/op_kernel/{op}_tiling.h`、`code/op_kernel/tiling_key_{op}.h`、`code/op_kernel/{op}.cpp`。
- 教训：**上传包必须与模板字段一一对应**（tiling_h/tiling_key_h/host_cpp/kernel_cpp 四个内容字段）；遗漏 tiling_key 或命名不匹配（`tiling_key_{op}.h`）会直接判错或 Compile Error；上传的是源码文本而非本地路径。**注意**：题面（本赛）为"直调模板：kernel.asc 定义 run_kernel(...) + __global__ __vector__"——与本 Skill 的 msopgen 四文件模板是两种格式，提交前必须按题面给的模板骨架修改，不能混用。
- 证据等级：B（官方仓库）；本机未验证。

### 类别 9：提交次数管理

**案例 9.1｜未找到 CANNJudge 官方每日提交额度数字（明确标注：无法确认）**
- 检索了 CANNJudge 提交页、cann-learning-hub skill、历届赛季规则帖，未找到 CANNJudge 的每日提交次数上限明文。
- 已知事实（S9 赛季官方规则页 https://www.hiascend.com/developer/ops ）：一支队伍 1-3 人，一人只能参加一支队伍，多账号组队取消资格；获奖作品需在 GitCode 开源合入 PR；S9 使用社区版 CANN 8.5.0（本地 9.0 与之存在版本差，见案例 1.1）。
- 教训：提交额度以提交页实际提示为准；不要盲目重提交——每轮判题都要带回可归因的差异（改了哪个文件、预期改善哪项），否则同一错误重复消耗额度。
- 证据等级：A（规则页可访问）但"每日额度"具体数字**未找到**；本机未验证。

### 类别 10：隐藏测试点与边界

**案例 10.1｜平台不开放测试用例 API，必须泛化设计**
- 链接：https://blog.csdn.net/gitblog_00410/article/details/143789539 （cann-learning-hub skill）
- 事实：CANNJudge 不提供测试用例 API；判题用多种 shape/dtype/属性组合；泛化要素：shape 泛化、dtype 泛化、属性泛化（边界值/默认值/特殊值）、对齐泛化（DataCopyPad 处理尾部）、边界泛化（空输入/单元素/极端值）。
- 教训：**只测样例就提交 = 必翻车**。提交前自建覆盖矩阵：D ∈ {64, 65, 127, 128, 4096, 4097, 8192, 32768}、outer ∈ {1, 2, 核数-1, 核数, 核数+1}、dtype ∈ {fp16, bf16, fp32}、epsilon ∈ {1e-5, 1e-6}、数值 ∈ {0, ±1, 极大值, 极小值, NaN/Inf 视题面}。
- 证据等级：B；本机未验证。

---

## 二、"常见失败 → 根因 → 排查方法"表

| 常见失败 | 根因 | 排查/预防方法 | 案例 |
| --- | --- | --- | --- |
| Compile Error（本地过、判题挂） | 判题环境 CANN 版本/命名空间与本地不同 | 以题面版本为准；显式 `AscendC::` 前缀；`.asc` 后缀 | 1.1, 1.4 |
| Compile Error（工具链） | msopgen/aot/CANN/driver 版本不配套 | `msopgen --version`、`aot --version`、`npu-smi info` 四维核对 | 1.3, 1.5 |
| Runtime 崩溃 | DMA 地址非 32B 对齐 | Tiling 块大小 32B 倍数；非对齐走 DataCopyPad；npucheck 的 ErrorRead4 | 3.1 |
| Runtime 崩溃/乱码 | Tiling 结构体对齐错位 | packed 或成员从大到小；Host/Device 共头文件 | 3.2 |
| 死锁/卡死 | 忘记 FreeTensor / 缺同步 / WaitFlag 分支遗漏 | DeQue↔FreeTensor 配对；CopyIn 后同步；Set/Wait 覆盖所有分支 | 3.3, 3.4, 3.8 |
| 非法指令 0x10 | UB 地址不对齐 / 编译 arch 形态混用 | 查 .o 反汇编定位指令；核对 mini/cloud 形态 | 3.5 |
| 输出全 0 / 随机值 | 数据未就位（异步）、GlobalTensor.SetValue | EnQue/DeQue 或 PipeBarrier；改 LocalTensor + DataCopyPad | 3.4 |
| WA（精度） | BF16 舍入模式、中间精度与 golden 不一致 | 输出 Cast 用 CAST_RINT；逐步复现参考实现的中间 Cast | 2.1, 2.2 |
| WA（归约） | FP16 大数吃小数、ReduceSum 能力上限 | 归约升格 FP32；D>4096 分段归约；按公式算 workLocal | 2.3, 7.1 |
| WA（语义） | 参考算子默认值/广播特殊语义 | 先查题面与参考实现文档再编码 | 2.5, 10.1 |
| Timeout | 死循环 / 单核 / tile 过大 / 同步缺失 | 检查循环出口；多核均分；UB 预算复核；先排除同步/越界 | 4.1, 4.2 |
| 尾块越界 | DataCopyPad 写出 padding 覆盖相邻行 | 验证写方向 burst 语义；行间布局下避免依赖"只写有效字节" | 6.1, 6.2 |
| 上传失败 | 字段不全/命名不匹配/格式混用 | 按题面模板字段逐一对齐（tiling_h/tiling_key_h/host_cpp/kernel_cpp 或直调 kernel.asc） | 8.1 |
| 隐藏用例翻车 | 只测样例、未覆盖极端 shape/dtype/特殊值 | 自建覆盖矩阵 + CPU 参考差分 | 10.1, 2.4 |

---

## 三、对本题（AddRmsNormBias）的预防清单

对照首版实现（`源码/op_kernel/add_rms_norm_bias.cpp`、`源码/op_host/add_rms_norm_bias.cpp`）逐点列出风险与对策：

| # | 首版实现风险点 | 对应案例 | 预防动作 |
| --- | --- | --- | --- |
| P1 | 入口为 `extern "C" __global__ __aicore__ void add_rms_norm_bias(...)`，与题面"直调模板 kernel.asc 定义 run_kernel(...) + __global__ __vector__"不一致 | 1.4、8.1 | 提交前按题面模板改入口：文件 `*.asc`、`__global__ __vector__`、入口名 `run_kernel`；确认参数形态（GM_ADDR 直传 vs tilingGm）。矩阵/向量类型决定 `__aicore__` vs `__vector__`，纯向量必须 `__vector__` |
| P2 | kernel 开头 `using namespace AscendC;` | 1.1 | 判题环境若为 8.5/9.0 命名空间差异，改为显式 `AscendC::` 前缀或按题面模板风格；提交前用判题环境版本核对 |
| P3 | 尾块搬出 `DataCopyPad(dst[elemOff], src, {1, len*sizeof(T), 0,0,0})`，D 非 32 倍数时行间连续布局可能被 padding 覆盖 | 6.1、6.2 | 真机第一步就验证小 D（如 64、65、100、127、128）尾块；确认 DataCopyPad 写方向 blockLen 是否为精确字节；若多写 padding，改为"按 32B 对齐行步长 + 尾元素单独处理"（或 UnPad 流程），并与 CPU 参考差分比对整张输出 |
| P4 | `ReduceSum(sum, yF, work, len)`，fp32 tileLen=2048（count ≤ 2048） | 7.1 | count ≤ 4096 理论安全，但真机必须验证 A2 训练/推理形态的 count 上限与 workLocal 语义；D=32768 时每行 16 次归约 + 标量累加，核对每次 workLocal 空间按官方公式 ≥ RoundUp(count/64, 8) |
| P5 | 每块一次 `sum.GetValue(0)` + V_S/S_V 同步（标量同步过多） | 4.1 | v1 正确性优先可接受；性能阶段改为"块内 ReduceSum 结果先存在 UB，整行归约完成后一次标量读"或每行一次同步，避免每 2K 元素同步一次 |
| P6 | BUFFER_NUM=1 无双缓冲 | 4.1 | 正确性通过后再开 BUFFER_NUM=2，注意队列入队/出队/释放配对（案例 3.3） |
| P7 | Tiling 结构体 {uint32, uint32, float, uint32} 由 host 二进制拷贝、kernel 按 struct 读 | 3.2 | 保持 host/kernel 共用同一头文件定义（当前 host 用的是 tiling.h 的 `set_*` 包装，kernel 用的是裸 struct——真机核对两处布局一致）；若直调模板改为参数直传，则删除 GET_TILING_DATA 机制 |
| P8 | 归约 FP32、输出 CAST_RINT 已就位 | 2.1、2.3、5.1 | 保持；但参考实现若在 y/rms 乘除前有中间 Cast，需按案例 2.2 逐步复现；epsilon 位置（sqrt(mean+eps) vs sqrt(mean)+eps）按题面公式核对 |
| P9 | `sqrtf` 由标量单元执行 + 每行一次除 D | 2.2 | 核对参考实现用 rsqrt 还是 sqrt+除、FP32 还是半精度；若参考用 rsqrt，逐位对齐 |
| P10 | 多核均分（块差 ≤1）、空核 return | 4.1、3.8 | 保留；确认 GetBlockIdx/GetBlockNum 在直调模板下的可用性；空核提前 return 前确认无 WaitFlag 残留（案例 3.8） |
| P11 | 输出 dtype 编码 1/2/3 自定义 | 8.1 | 直调模板下若用 TilingKey 分流（tiling_key_h），改为模板机制，别用自定义魔法数 |
| P12 | 上传包字段 | 8.1 | 提交前按题面模板核对文件清单与字段；不包含本机路径/凭据；记录每次提交的差异与结果 |
| P13 | 未覆盖极端 shape | 10.1 | 提交前 CPU 参考差分矩阵：D ∈ {64,65,127,128,4096,4097,8192,32768}，outer ∈ {1, 2, 核数-1, 核数, 核数+1}，dtype × {fp16,bf16,fp32}，epsilon × {1e-5,1e-6}，特殊值（0、±1、1e-8、1e8、-1e8） |
| P14 | 提交次数无计划 | 9.1 | 每轮提交只改一处假设；记录提交 ID/状态/precision_ratio/time；不盲目重试同一错误 |

---

## 四、本轮新增来源清单（需主 Agent 合并入 sources.md）

| # | 名称 | URL | 类型 | 访问日期 | 用途 | 等级 |
| --- | --- | --- | --- | --- | --- | --- |
| N1 | CANN学习中心 as_strided 实战（CANNJudge Compile Error：8.5 vs 9.0 命名空间） | https://blog.csdn.net/gitblog_01418/article/details/150380401 | 社区 | 2026-09-11 | 编译错误案例 | C |
| N2 | 昇腾算子挑战赛 S8 Scale 优化分享（中间 Cast 回 bf16 才匹配 golden） | https://developer.huawei.com/home/forum/ascend/thread-02200223658291993472-1-1.html | 官方论坛 | 2026-09-11 | WA/精度案例 | C |
| N3 | XJTUACM 算子挑战赛 S7 经验总结（BF16 CAST_RINT；ReduceSum≤4096；half 上溢） | https://www.hiascend.com/developer/blog/details/02127205558228788013 | 官方博客 | 2026-09-11 | 精度/ReduceSum 案例 | B |
| N4 | ShwStone/Ascend-S7（S7 源码） | https://github.com/ShwStone/Ascend-S7 | 官方博客关联源码 | 2026-09-11 | N3 佐证 | B |
| N5 | DataCopyPad 写出溢出 Bug 详解（burst 32B 覆盖相邻数据） | https://blog.csdn.net/ferriswym/article/details/162206787 | 社区 | 2026-09-11 | 尾块越界案例 | C |
| N6 | tilelang-ascend Issue #890（RMSNorm N=513 尾块错误） | https://github.com/tile-ai/tilelang-ascend/issues/890 | GitHub Issue | 2026-09-11 | 尾块案例 | C |
| N7 | 避坑指南：Ascend C 算子开发常见报错解析与精度优化复盘 | https://blog.csdn.net/2301_80840905/article/details/155034892 | 社区 | 2026-09-11 | 对齐/死锁/FP16 案例 | C |
| N8 | 昇腾CANN训练营·黑客篇：BlackBox 与 Exception Dump | https://blog.csdn.net/2401_82857325/article/details/155754900 | 社区 | 2026-09-11 | 栈溢出/踩内存案例 | C |
| N9 | 昇腾 Ascend C 算子开发中核异常与同步机制失效的深度排查 | https://blog.csdn.net/weixin_27442001/article/details/159974663 | 社区 | 2026-09-11 | WaitFlag/超时案例 | C |
| N10 | 基于 Ascend C 的 Pdist 自定义算子开发复盘（错误统计/混合精度） | https://hwcomputing.csdn.net/69b6a2570a2f6a37c5979df8.html | 社区 | 2026-09-11 | 超时统计/精度案例 | C |
| N11 | CANN 学习中心 CANNJudge 算子提交 Skill（提交字段/判题状态/泛化要求） | https://gitcode.com/cann/cann-learning-hub/blob/master/skills/cannjudge-submit/README.md（转述：https://blog.csdn.net/gitblog_00410/article/details/143789539） | 官方仓库 | 2026-09-11 | 上传包/隐藏测试点 | B |
| N12 | CANNBot Ascend C 直调开发指南（.asc 后缀/__vector__/禁止写死/DataCopy 黑名单） | https://blog.csdn.net/gitblog_00909/article/details/151507556 | 官方仓库转述 | 2026-09-11 | 直调模板规范 | B |
| N13 | 错误码 0x10 定位案例（官方） | https://www.hiascend.com/doc_center/source/zh/canncommercial/601/devtools/auxiliarydevtool/atlasaicoreerr_16_0010.html | 官方文档 | 2026-09-11 | UB 越界/非法指令 | A |
| N14 | npucheck 功能（ErrorRead1-4 内存检测） | https://www.hiascend.com/doc_center/source/zh/CANNCommunityEdition/80RC3alpha003/devaids/auxiliarydevtool/atlasascendebug_16_0133.html | 官方文档 | 2026-09-11 | 对齐错误码 | A |
| N15 | CANN 与 MindSpore 版本不匹配导致算子编译失败 | https://ask.csdn.net/questions/9266350 | 社区问答 | 2026-09-11 | msopgen 编译错误 | C |
| N16 | 2025昇腾训练营避坑指南（undefined reference to aclInit；环境版本） | https://blog.csdn.net/pz890123/article/details/152697389 | 社区 | 2026-09-11 | 链接/环境错误 | C |
| N17 | MindSpore AOT 自定义算子文档（`No rule to make target 'package'`） | https://www.mindspore.cn/docs/zh-CN/r2.4.0/model_train/custom_program/operation/op_custom_ascendc.html | 官方文档 | 2026-09-11 | 编译路径错误 | B |
| N18 | Atlas 900 A3 AICPU 占满导致执行卡死（官方技术文章） | https://www.hiascend.com/developer/techArticles/20260603-10?envFlag=1 | 官方技术文章 | 2026-09-11 | 死锁排查方法 | A |
| N19 | 昇腾AI创新大赛-算子挑战赛官方页（S9 规则：CANN 8.5.0/一人一队） | https://www.hiascend.com/developer/ops | 官方页面 | 2026-09-11 | 赛季规则 | A |
| N20 | 开发工具链介绍：从编译到调试（__aicore__ 语法错误） | https://cann.csdn.net/69ef0b9b54b52172bc704775.html | 社区 | 2026-09-11 | 语法错误案例 | C |
| N21 | 昇腾CANN训练营 RMSNorm 教学（ReduceSum/FP32 归约） | https://blog.csdn.net/2401_82857325/article/details/155447666 | 社区 | 2026-09-11 | ReduceSum 用法对照 | B(训练营) |

---

## 五、未采用来源说明（引用纪律）

以下来源因"只有结论、无现象/根因细节"或与本任务无关而未列入案例，仅登记以备查：

- 多篇标题为"CANN 算子挑战赛 复盘/踩坑"的短文只给出"要注意精度""要小心 Tiling"一类无细节结论，无具体报错或数据，未采用。
- 微博/非技术社区关于"每日提交次数"的帖子（https://m.weibo.cn/detail/5339082433495524）与 CANNJudge 无关，未采用；科大讯飞"测试用例生成挑战赛"的每日 3 次提交限制（https://challenge.xfyun.cn/h5/detail?type=Competition-TestGen&ch=ds22-xf-cb03）是另一赛事，仅作为"竞赛普遍有每日额度"的旁证，不能当作 CANNJudge 规则。
- 官方"多流并发死锁检测"（https://github.com/Ascend/torchair/blob/master/docs/zh/npugraph_ex/dfx/deadlock_check.md）是 torch.compile 图模式特性，与本算子直调无关，仅列入交叉参考不展开。

---

## 六、结论

- 满足验收：真实案例 22 个（均带链接与现象/根因），覆盖失败类别 9/10（"提交次数管理"仅有官方规则页，未找到 CANNJudge 每日额度明文，已如实标注不可确认）。
- 对本项目最重要的三条经验：(1) 判题环境与本地版本/命名空间差异是 Compile Error 头号来源（案例 1.1）；(2) DataCopyPad 写方向 padding 覆盖相邻行是 D 非 32 倍数时最隐蔽的尾块炸弹（案例 6.1）；(3) ReduceSum(count) 实际上限 4096、BF16 输出必须 CAST_RINT（案例 7.1、2.1）。
- 所有案例均为社区/官方文档证据，**本机（macOS，无 CANN/NPU）均未验证**；落地必须以真实 CANN 9.0.0 判题环境编译与判题结果为准。

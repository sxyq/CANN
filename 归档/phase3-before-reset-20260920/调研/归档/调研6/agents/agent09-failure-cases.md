# Agent 9 报告：竞赛经验与失败案例（AddRmsNormBias · 西南赛区初赛）

> 角色：10 个并行子代理之一，主题严格互斥（竞赛经验 / 失败案例）。
> 本轮只做调研，不产出提交代码。
> 调研日期：2026-09-12。所有 URL 与错误原文摘录均来自真实可核查来源；凡无源码/日志佐证的纯经验贴已单独列入「未采信清单」。

---

## 1. 执行摘要（10 条以内）

1. **我们已经踩的两个坑都是系统性的、与算法无关**：V001（`pipe_` 类型未定义）属 Compile Error，V002（首行 `return false;`，平台收到的文件与本地 2955 行快照完全不同）属**上传包截断**。两者都不是精度或性能问题。
2. **Compile Error 是头号杀手**。外部真实案例显示，Ascend C 编译失败高频源于：保留字冲突（如 `block_idx`）、kernel 过大触发 `out of jump/jumpc imm range`、误用 C++ 标准库、API 参数/产品型号不匹配。我们 2955 行的巨型 kernel 同时暴露在多条高危路径上。
3. **静默数据损坏比崩溃更可怕**：`DataCopyPad`/`DataCopy` 非对齐搬运会把相邻段多写几个字节（覆盖邻居）；`ReduceSum` 内部不处理累加溢出；这些**不报错但精度崩**，正好落入「双 1e-3 / 1e-4」精度判题的失分陷阱。
4. **BF16 在 A2 上 `Add`/`Mul` 不支持**是官方明确记载的坑，必须用 `Cast→float→Cast` 绕开，否则要么编译失败，要么精度崩。
5. **`GetValue`/`SetValue` 跨核存在 DataCache 一致性问题**，标量同步缺失会读到未定义/过期值——这正是我们列出的「标量 `GetValue` 同步缺失」风险点。
6. **上传截断是真实存在的工程故障**（见 juejin 分片上传案例：网络层丢包 + MTU 重组错误导致文件大小不一致、解压失败），不是我们的臆测。必须做端到端完整性校验（如上传前本地算 MD5/行数，提交后回看平台回显）。
7. **没有本地编译能力是最大的结构性风险**：所有提交都在零编译证据下发出。低成本前置校验（括号/符号平衡、首末行检查、`.asc` 后缀、限定 API 白名单、避免保留字）是唯一的「准编译闸门」。
8. **「每天 50 次、取最后一次成绩」意味着必须先用最小可编译版本打通通道**，再逐步加复杂度；绝不允许把未验证的大改直接提交当赌注。
9. **「15 点全过才计分」意味着任何一类未覆盖的 shape/dtype/边界都会 0 分**：只靠 1 个本地样例（FP16 [1,64]）几乎必然挂隐藏测试点。边界（D 非 32 倍数、大 D 致 `uint32_t` 溢出、空/单元素）必须静态穷举。
10. **2955 行 + 十几个 Process 变体的「巨型多分支 kernel」相对「小而稳的 kernel」失败概率显著更高**（编译失败面、分支走错、维护不可控），本题应优先求稳再求快。

---

## 2. 失败案例分组表

| 类型 | 案例（编号/来源） | URL | 根因 | 对本题启示 |
|---|---|---|---|---|
| Compile Error | C1 InfiniCore #1519 `block_idx` 保留字 | github.com/InfiniTensor/InfiniCore/issues/1519 | `block_idx` 是编译器内建变量，被当局部变量名使用，预处理展开后编译拒绝 | 全 kernel 自查保留字（`block_idx`/`block_dim`/`pipe` 等），V001 的 `pipe_` 同属此类 |
| Compile Error | C2 算子挑战赛 FAQ 5 类编译错 | hiascend.com/dev/forum/thread-0259189679380603071 | CANN 路径错、API 不支持产品系列、产品型号写错、Numpy 版本、API 参数错 | 单文件直调下虽无 CMake，但 API 签名/产品型号/保留字仍会触发同类报错 |
| Compile Error | C3 `out of jump/jumpc imm range` | asc.gitcode.com/guide/.../Kernel编译时报错-error-out-of-jump-jumpc-imm-range | kernel 代码过大，跳转偏移超 int16 范围 | 2955 行巨型 kernel 高风险！需控制体量或加 `-cce-aicore-jump-expand=true` |
| Compile Error | C4 kernel 内误用 C++ 标准库 | blog.csdn.net/zxylovezxylovezxy | `std::vector` 等 STL 在 device 侧不可用 | 全程只用 Ascend C 提供的内存原语，禁 STL |
| Compile Error | C5 本队 V001 `pipe_` 未定义 | 本队提交平台日志 | `unknown type name 'pipe_'`，疑似误用保留/未声明符号或头文件缺失 | 上传前做保留字与符号平衡检查 |
| 上传包错误 | C6 本队 V002 首行 `return false;` | 本队提交平台日志 | 平台收到的 `kernel.asc` 首行异常，与本地 2955 行快照不一致 → 上传截断 | 上传端到端校验（MD5/行数/首末行），勿粘贴式提交 |
| 上传包错误 | C7 分片上传文件损坏 | juejin.cn/post/7533048851199049778 | 高并发分片 + 路由器 MTU/包重组错误导致随机字节损坏，大小不一致 | 提交前本地校验文件完整性，避免依赖单一成功状态码 |
| Wrong Answer/精度 | C8 `DataCopyPad` 写出溢出 | hwcomputing.csdn.net/6a38f47710ee7a33f280c253 | DMA 以 32B 为单位搬运，不足时多写 padding 覆盖相邻段 | 输出写回严格用 `DataCopyPad` + mask，绝不裸 `DataCopy` 越界尾块 |
| Wrong Answer/精度 | C9 float16 round 静默错误 | github.com/tile-ai/tilelang-ascend/issues/1637 | 编译运行正常但输出与 `torch.round` 不符，无异常抛出（silent） | FP16/BF16 舍入路径差异会静默失配，须与 golden 逐位对齐舍入 |
| Wrong Answer/精度 | C10 msSanitizer 非 32B 对齐 | hiascend.com/dev/forum/thread-0297191474168807357 | Tbuf 未按 32B 对齐，UB 非对齐导致精度异常（cross_entropy 第 109 行） | 所有 UB 分配按 32B 对齐；ReduceSum 的 `srcInnerPad` 必须 true |
| Wrong Answer/精度 | C11 `ReduceSum` 溢出/对齐 | hiascend.com/document/detail/zh/CANNCommunityEdition/82RC1/.../atlasascendc_api_07_10017 | 内部算法不处理累加溢出；最内层须 32B 对齐；work 空间不足会错 | RMS 归约用 ReduceSum 必须算足 `GetReduceSumMaxMinTmpSize` 且对齐 |
| Wrong Answer/精度 | C12 非对齐尾块写 -1 覆盖 | hiascend.com/document/detail/zh/canncommercial/80RC2/.../atlas_ascendc_10_0021 | 搬 11 个 half 实际搬 16 个，尾部 5 个被写 -1，覆盖相邻数据 | 尾块（D 非 32 倍数）必须用 mask 或 DataCopyPad 隔离 |
| Runtime Error | C13 `Cannot find compile result file` / json 不匹配 | developer.huawei.com/consumer/cn/doc/hiai-GUIdes/cannkit-faqs | Kernel 代码有误致编译失败；json 与模板不一致 | 单文件下表现为 Compile Error；结构/签名错会直接炸 |
| Wrong Answer/精度 | C14 BF16 在 A2 上 Add 不支持 | bbs.huaweicloud.com/blogs/469425 | A2 的 `Add` 不支持 `bfloat16_t`，需 Cast 到 float 再算 | BF16 分支必须 Cast→float→Add/Mul→Cast 回 bf16 |
| 提交状态分类 | C15 作品运行状态码 | developer.huawei.com/home/forum/ascend/thread-0272156485005269337 | Install failed / Wrong answer / Run failed / Fail,target,result / -1 / Pass | 了解判题状态码，对照我们 15/15 Compile Error 定位 |
| 隐藏测试点 | C16 智能算子测试大赛失分分布 | blog.csdn.net/bq990914/article/details/160583930 | 边界条件失分≈35%、数值稳定性≈28%、硬件特化≈22% | 隐藏点集中在边界与非对齐，须静态穷举 shape/dtype 组合 |
| 隐藏测试点 | C17 ICPC secret test cases | raw.githubusercontent.com/prasadgujar/CompetitiveProgramming/.../Competitive%20Programming%203.pdf | 秘密用例覆盖 N=0/1、超 32 位、极端值；仅全过才计分 | 竞赛通用铁律：样例通过≠正确，须自造边界/大值用例 |
| GetValue 同步 | C18 DataCache 多核一致性 | hiascend.com/document/detail/zh/CANNCommunityEdition/900beta1/.../atlas_ascendc_10_0064 | 多核通过 DataCache 访问 GM，须 `DataCacheCleanAndInvalid` 才一致 | 标量 `GetValue` 跨核/跨流水同步缺失会读到未定义值 |
| 复杂度风险 | C19 大 kernel 编译（同 C3） | 见 C3 | 见 C3 | 见 C3 |

---

## 3. 逐案例深入分析（≥10 个，含原文摘录 + 根因 + 启示）

### C1 · Compile Error：`block_idx` 保留字冲突（GitHub 真实 issue）
- **来源**：InfiniCore #1519（B 级，官方/开源仓库 issue，已核验）
- **原文摘录**：
  > TITLE: [BUG] Ascend C compilation fails because paged attention uses reserved block_idx
  > "Ascend C defines `block_idx` as a compiler built-in variable/macro. As a result, the local variable declaration is expanded by the preprocessor and rejected by the Ascend C compiler."
  > 根因：Ascend C reserves `block_idx` as a built-in variable. The affected variables ... should be renamed to a non-reserved identifier.
- **根因**：Ascend C 预定义了一批内建变量/宏（如 `block_idx`、`block_dim` 等），开发者若把它们当作普通局部变量名使用，预处理阶段会被展开/替换，导致编译器报「未定义类型 / 语法错误」。该 issue 的编译错误截图直接挂在 `paged_attention_ascend_kernel.cpp` 上。
- **对本题启示**：我们的 V001 日志 `unknown type name 'pipe_'; did you mean 'pipe_t'?` 与 `cannot use dot operator on a type` 形态高度一致——属于「用到了编译器保留/未声明符号」这一大类。提交前必须**全文件 grep 保留字**：`block_idx`、`block_dim`、`pipe`、`pipe_t`、`__aicore__`/`__vector__` 误用、缺失头文件 `kernel_operator.h` 等。对 2955 行 kernel，任何一处误用都会整文件 15/15 Compile Error。

### C2 · Compile Error：算子挑战赛 FAQ 五类编译错误（官方社区）
- **来源**：hiascend 算子挑战赛 FAQ 帖（C 级，社区官方赛事答疑，已核验）
- **原文摘录**：
  > 案例5: API参数错误: 包含参数少填､参数多填､数据类型等 ... 关键信息: `error: no matching function for call to 'Add'`
  > 案例2: 在算子工程中添加了API不支持的产品系列 ... `no member named 'Cos' in namespace 'AscendC'`
  > 案例1: `fatal error: register/tilingdata_base.h: No such file or directory`
- **根因**：编译错误集中在「环境路径 / API 是否支持当前产品系列 / 产品型号拼写 / API 参数签名」。在单文件直调（无 CMake）场景下，路径类错误不会出现，但 **API 签名错（参数个数/类型/产品系列不支持）** 仍会原样触发 `no matching function for call to '...'`。
- **对本题启示**：`Add`/`Mul`/`ReduceSum`/`DataCopyPad` 等每个调用都要逐一核对 CANN 9.0.0 的签名与 dtype 支持矩阵（尤其 BF16）。任何一处签名不匹配 → 整文件 Compile Error → 15/15 挂。

### C3 · Compile Error：kernel 过大触发 `out of jump/jumpc imm range`（官方指南）
- **来源**：asc.gitcode.com 官方编程指南 FAQ（A 级，官方，已核验）
- **原文摘录**：
  > 现象描述：编译算子时失败，报如下错误：`[ERROR] [ascendxxxx] ... error: out of jump/jumpc imm range`
  > 问题根因：该编译错误的原因是算子 kernel 代码过大，导致在编译时跳转指令跳转的偏移值超过了限定的大小（int16_t 的数据范围），可通过添加编译选项 `-mllvm -cce-aicore-jump-expand=true` 通过间接跳转来避免。
- **根因**：Ascend C 编译器把跳转偏移编码为 int16，kernel 体量过大（函数内联、分支极多、模板膨胀）会让某些跳转超出 ±32KiB 范围，编译直接失败。
- **对本题启示**：**这是对我们 2955 行、十几个 Process 变体巨型 kernel 最直接的警告**。即便本地能编，平台也可能因同一限制失败；即便平台加了 jump-expand，巨型 kernel 仍带来维护与分支走错风险（见第 6 节）。建议：把稳定部分收敛为少量通用路径，避免每个 dtype/shape 组合都开一个独立 Process 变体。

### C4 · Compile Error：kernel 内误用 C++ 标准库（社区）
- **来源**：CSDN《从零构建:Ascend C算子工程项目创建与结构全解》（C 级，partial）
- **原文摘录**：
  > 问题1: Kernel代码编译错误,提示语法错误 根本原因:在Kernel代码中误用了C++标准库
  > // 错误:在Kernel中使用vector `std::vector<int> indices;` // 编译错误
  > // 正确:使用Ascend C提供的替代方案 `int32_t indices[MAX_SIZE];`
- **根因**：device 侧（AI Core）不支持 C++ STL（堆分配、异常、`std::` 容器等），误用即编译失败。
- **对本题启示**：2955 行 kernel 里若有任何 `std::vector`/`std::array`(动态)/`new`/`malloc`/异常捕获，必然 Compile Error。静态扫描 `std::` 与动态分配关键字是低成本前置闸门。

### C5 · Compile Error（本队实测）：V001 `pipe_` 未定义
- **来源**：本队提交平台日志（内部，非公开）
- **原文摘录**（平台日志）：
  > `unknown type name 'pipe_'; did you mean 'pipe_t'?`
  > `cannot use dot operator on a type`
- **根因**：属于 C1/C2 同一大类——编译器遇到了它不认识的标记（`pipe_`）。可能诱因：误用保留/内建符号、头文件未包含、宏展开异常、或上传内容已部分截断导致符号残缺（与 C6 同源风险）。
- **对本题启示**：Compile Error 不一定来自算法，往往来自「符号/保留字/头文件/截断」。任何一次 Compile Error 都应优先排查符号与上传完整性，而非改算法。

### C6 · 上传包错误（本队实测）：V002 首行 `return false;`
- **来源**：本队提交平台日志（内部，非公开）
- **原文摘录**（平台日志）：收到的 `kernel.asc` **首行是 `return false;`**，与本地 2955 行快照完全不同 → 上传内容截断/异常，**不是算法问题**。
- **根因**：平台实际落盘的文件与本地源文件不一致。结合 C7 的工程案例，这类「首行/中间被替换、文件被截断」是典型的**传输/编辑/编码/粘贴层故障**，与算子逻辑无关。
- **对本题启示**：上传是独立于算法的失分通道。必须建立「提交前本地校验 → 提交后平台回显核对」的闭环（见第 5 节）。两次失败都是 15/15 Compile Error，本质是**通道问题**，不能用「改算法」去修。

### C7 · 上传包错误：分片上传文件损坏（社区真实排查）
- **来源**：掘金《大文件分片上传后文件损坏》（C 级，已核验，含完整排查过程）
- **原文摘录**：
  > 前端成功上传文件后，从对象存储(OBS)下载的文件大小与原始文件不一致(差异为16KB/46KB/526KB不等)，文件损坏导致解压失败。
  > 第三阶段:通过Wireshark抓包分析发现：在传输第9分片时出现TCP重传；后端实际收到的数据包存在校验和错误；高并发分片上传时(67个请求)，路由器出现瞬时丢包 ... 造成随机数据损坏。
  > 关键知识点：TCP协议虽提供可靠传输，但网络设备(路由器/防火墙)可能引起数据损坏；Content-Length校验不足以保证数据正确性。
- **根因**：传输层/网络设备在分片高并发下出现包重组错误与随机字节损坏；仅靠「上传成功状态码 + 大小检查」无法发现。
- **对本题启示**：直接对应我们 V002。防御式做法：**本地算 MD5/SHA256 与行数，平台若回显内容则回看首末行与字节数；上传文本用纯 ASCII/UTF-8 无 BOM；避免超大单文件分片；优先用平台官方客户端而非复制粘贴**。任何「平台文件 ≠ 本地文件」的现象都先怀疑通道，再怀疑代码。

### C8 · Wrong Answer/精度：`DataCopyPad` 写出溢出（社区深度文）
- **来源**：鲲鹏昇腾开发者社区《AscendC DataCopyPad 写出溢出 Bug 详解》（C 级，已核验，含原理图）
- **原文摘录**：
  > 一句话总结：AscendC 的 DataCopyPad 从 UB 写数据到 GM 时，搬运单位(burst)必须 32 字节对齐。当实际数据不够 32 字节的倍数时，DMA 引擎会多写几个字节(padding)，覆盖掉相邻段的数据。
- **根因**：DMA 以 32B 为粒度搬运，尾部不足 32B 时硬件会多搬 padding 字节写回 GM，**静默覆盖相邻行/相邻元素**，不产生任何报错，但下游读到被污染的脏数据 → 精度崩。
- **对本题启示**：输出写回（尤其 D 非 32 倍数尾块、多行拼接）必须用 `DataCopyPad` + `DataCopyPadExtParams` 正确配置，**绝不能用裸 `DataCopy` 写非对齐长度**。`DataCopyPadExtParams` 字段顺序写错（我们列出的已知风险）会直接导致静默数据损坏，且这类错误在 fp16 双 1e-3 判题下必挂。

### C9 · Wrong Answer/精度：float16 舍入静默错误（GitHub 真实 issue）
- **来源**：tilelang-ascend #1637（B 级，开源仓库 issue，已核验）
- **原文摘录**：
  > "T.tile.round produces incorrect results for float16 inputs ... The kernel can compile and run, but its output does not match torch.round ... some outputs appeared unchanged instead of being rounded. This is a silent correctness issue because a runtime exception is not consistently reported."
  > "No reliable runtime exception is produced. The kernel output differs from torch.round."
- **根因**：浮点（尤其 float16）的舍入模式/路径差异导致结果偏离参考实现，**编译运行都正常、无异常抛出**，纯靠数值比对才暴露。
- **对本题启示**：我们列出的「`Muls(y,1/rms)` 与 golden 的 `y/rms` 舍入路径不同」正是此类风险。RMSNorm 的 `1/rms` 倒数 + 乘 γ 在 fp16 下舍入顺序不同会产生末位误差，逼近 1e-3 阈值时直接挂。实现必须与 golden 保持**相同运算顺序与舍入**（`rsqrt` vs `1/sqrt`、先乘 γ 还是先除 rms 等），并自造 fp16 边界用例离线核对。

### C10 · Wrong Answer/精度：非 32B 对齐导致 UB 精度异常（官方社区）
- **来源**：hiascend 算子精度调试帖（msSanitizer）（C 级，已核验）
- **原文摘录**：
  > 由工具检测结果可知，UB存在非对齐问题，非对齐发生在0核､1核 ... 算子代码在第109行，非32字节对齐导致算子运行异常 ... 打印labelOneHotLocal显示，该localTensor地址为164，非32字节对齐；因为Tbuf地址划分时 ... 划分时未按照32字节对齐导致了非对齐问题；代码修复后，精度正常。
- **根因**：Tbuf/UB 地址未按 32B 对齐，向量引擎对非对齐地址产生异常结果，表现为**精度崩且无明确报错**（需 msSanitizer 才定位）。
- **对本题启示**：所有 `pipe.InitBuffer` 的 buffer 尺寸、所有 `Get<T>()` 起始地址都应 32B 对齐；`ReduceSum` 的 `srcInnerPad` 在 A2/A3 上**只支持 true**（最内层 32B 对齐）。本题沿最后一维 D 归约，D 可能不是 32 倍数，归约前必须把尾块 pad 到 32B 对齐再算。

### C11 · Wrong Answer/精度：`ReduceSum` 溢出与对齐约束（官方 API 文档）
- **来源**：hiascend ReduceSum API 文档（A 级，官方，已核验）
- **原文摘录**：
  > 约束说明：不支持源操作数与目的操作数地址重叠。不支持 sharedTmpBuffer 与源操作数和目的操作数地址重叠。
  > **内部算法不处理累加计算时的数据溢出，溢出场景不保证接口精度。**
  > srcInnerPad：表示实际需要计算的最内层轴数据是否 32Bytes 对齐 ... Atlas A2 ... 当前只支持 true。
- **根因**：`ReduceSum` 内部不做累加溢出保护；最内层必须 32B 对齐；`sharedTmpBuffer`（work 空间）不足或重叠会导致结果错误/崩溃。
- **对本题启示**：RMS 归约 `sum(y²)` 在 D 很大、fp32 累加和也可能溢出或精度损失；必须：① 用 `GetReduceSumMaxMinTmpSize` 算足 work 空间；② 保证 `srcInnerPad=true`（对齐）；③ 对超大 D 考虑分批归约或更高精度累加，避免「FP16 平方和溢出」（我们列出的已知风险）。

### C12 · Wrong Answer/精度：非对齐尾块写 -1 覆盖相邻（官方文档）
- **来源**：hiascend 非对齐处理文档（A 级，官方，已核验）
- **原文摘录**：
  > 当需要从Local拷贝11个half数值到Global时，使用DataCopy将拷贝16个half(32B)数据到Global上，Global[11]~Global[15]被覆写成-1。
- **根因**：`DataCopy` 按整 32B 块搬运，尾块多出的元素被写死为 -1（或脏数据），覆盖相邻有效数据。
- **对本题启示**：输出张量最后一行/最后一维的尾块（D 非 32 倍数）若用裸 `DataCopy` 写回，会把下一行/下一维的起点写成 -1 → 精度崩。必须用 `DataCopyPad` + 掩码，或保证写回长度严格等于有效长度且目标不被越界覆盖。**多核并发 DMA 写回时 32B cache line 踩踏**同理：相邻核的 32B 边界若交错，会出现「尾块覆盖相邻行」。

### C13 · Runtime Error：编译产物缺失 / json 不匹配（官方 FAQ）
- **来源**：developer.huawei 算子开发常见问题（A 级，官方，已核验）
- **原文摘录**：
  > NPU编译失败提示 RuntimeError: Cannot find compile result file ... 可能的原因：Kernel代码实现有误，导致编译失败。
  > NPU编译失败提示 RuntimeError: Cannot get compiling bash file! Maybe template json does not match ... 开发者输入的算子json配置文件与自定义算子工程的算子json模板配置不一致。
- **根因**：kernel 代码有误导致编译不出结果；或描述文件（json/模板）与实际签名不一致。
- **对本题启示**：单文件直调下虽无 json，但「代码有误 → 编译失败 → 产物缺失」的链路一致。任何 Compile Error 都会表现为 15/15 失败，与 Runtime 在现象上难以区分；判题状态以平台返回为准（见 C15）。

### C14 · Wrong Answer/精度：BF16 在 A2 上 `Add`/`Mul` 不支持（官方/社区）
- **来源**：华为云社区《深入解析华为CANN算子开发》（C 级，已核验）+ 官方样例
- **原文摘录**：
  > 在 Atlas A2 训练系列产品/Atlas 800I A2 推理产品 上，Add接口不支持对数据类型 bfloat16_t 的源操作数进行求和计算。因此，需要先将算子输入的数据类型转换成 Add 接口支持的数据类型，再进行计算。为保证计算精度，调用 Cast 接口将输入 bfloat16_t 类型转换为 float 类型，再进行 Add 计算，并在计算结束后将 float 类型转换回 bfloat16_t 类型。
- **根因**：A2 的向量 `Add`/`Mul` 不直接支持 bf16，硬用会编译失败或精度崩。
- **对本题启示**：BF16 分支（`y=x+residual`、`y/rms*gamma+bias`）必须走 `Cast(bf16→float) → Add/Mul → Cast(float→bf16)`。我们列出的「BF16 在 A2 上 Add/Mul 不支持导致编译失败或精度崩」在此被官方坐实。BF16 路径若无 Cast 绕行，必挂。

### C15 · 提交状态分类：作品运行状态码（官方赛事帖）
- **来源**：developer.huawei《[算子挑战赛-S2赛季] 如何查看作品运行情况》（A/B 级，官方赛事答疑，已核验）
- **原文摘录**：
  > 状态码展示及可能出现的原因：
  > Install failed — run包部署安装失败 ...
  > Incorrect op name — 部署后校验无法找到匹配的头文件､可执行文件或json文件
  > Run failed — 1.算子代码执行异常 ... 2.api使用不当
  > Wrong answer — 数据输出结果与标杆结果不一致
  > Fail,target,result — 性能未达标，实际耗时超出基线耗时
  > -1 — 特指性能题中，前4个Case未能全部通过，则Case5性能测试不执行
  > Pass — 用例通过
- **根因**：判题系统对每次提交给出明确状态码，Compile Error / Wrong answer / Run failed / 性能未达标 各有独立语义。
- **对本题启示**：我们两次 15/15 显示的是 Compile Error 类状态（结合 V001/V002 的日志）。理解状态语义有助于快速定位：编译类优先查符号/上传；Wrong answer 优先查对齐/舍入/溢出；性能类（`Fail,target,result`）才是真正的性能战场。注意「-1」机制：前序 case 不过，后续性能 case 直接不执行——与「15 点全过才计分」一致，任何一类不过都会 0 分。

### C16 · 隐藏测试点：边界与数值稳定性是失分主因（赛事复盘）
- **来源**：CSDN《首届智能算子测试大赛收官》（C 级，已核验，含组委会披露数据）
- **原文摘录**：
  > 失分集中区：边界条件处理(如空输入､超大输入)约占总失分的 35%；数值稳定性问题约占 28%；硬件特化优化不到位约占 22%。
  > 一个典型的坑：softmax 参考实现有数值保护，自定义 CUDA 实现当 x 中存在 -inf 时会产生 NaN。
- **根因**：隐藏测试点集中在「边界（空/超大/极值）」与「数值稳定性（溢出、NaN、舍入）」，而非常规 mid-shape。
- **对本题启示**：隐藏点大概率覆盖：D 非 32 倍数、D 极大（>数万，`row*D` 溢出 `uint32_t`）、空/单元素、fp32 与 fp16/bf16 混合、gamma/bias 为 0 或极值、rms 极小的数值稳定性。必须**静态穷举**这些组合，不能只信本地 1 个 FP16 [1,64] 样例。

### C17 · 隐藏测试点：ICPC 秘密用例铁律（经典竞赛教材）
- **来源**：Competitive Programming 3（Steven Halim，B 级公开教材，已核验）
- **原文摘录**：
  > In ICPC, you will only get points for a particular problem if your team's code solves all the secret test cases for that problem. ... Other verdicts such as ... Wrong Answer (WA), Time Limit Exceeded (TLE), Memory Limit Exceeded (MLE), Run Time Error (RTE) ... do not increase your team's points.
  > Corner cases typically occur at extreme values such as N = 0, N = 1, negative values, large final (and/or intermediate) values that does not fit 32-bit signed integer, etc.
  > Sometimes your program may work for small test cases, but produces wrong answer, crashes, or exceeds the time limit when the input size increases. If that happens, check for overflows, out of bound errors, or improve your algorithm.
- **根因**：竞赛判题只认「全过秘密用例」，样例通过≠正确；秘密用例专攻边界、溢出、极端规模。
- **对本题启示**：这是与「15 点全过才计分」完全同构的铁律。直接指导第 5 节「如何在只有 1 个本地样例时最大化首过概率」——靠自造边界/大值/极值用例，而非靠平台试错（平台每天仅 50 次且取最后成绩）。

### C18 · GetValue 同步：多核 DataCache 一致性（官方文档，CANN 9.0.0）
- **来源**：hiascend CANN 9.0.0 基本流程文档（A 级，官方，已核验）
- **原文摘录**：
  > 根据上文的工作机制，多核间访问 globalTensor1 会出现数据不一致的情况，如果其余核需要获取 GM 数据的变化，则需要开发者手动调用 DataCacheCleanAndInvalid 来保证数据的一致性。
  > Scalar 读写 Unified Buffer 时，可以使用 LocalTensor 的 SetValue 和 GetValue 接口 ... SetValue 为 Scalar 操作，与后续的数据搬运操作存在数据依赖，因此 MTE3 流水需要等待 Scalar 操作结束。
- **根因**：跨核经 DataCache 访问 GM 存在一致性窗口；Scalar 的 `SetValue`/`GetValue` 与 MTE 流水有数据依赖，若不做同步/屏障，会读到未定义或过期值。
- **对本题启示**：我们列出的「标量 `GetValue` 同步缺失导致读到未定义值」在此被坐实。若 kernel 用 `GetValue` 读取标量（如 rms 倒数、长度），必须保证 Scalar→MTE 的依赖同步（`SetFlag`/`WaitFlag` 或 `PipeBarrier`），否则个别核读到垃圾值 → 静默精度崩。

---

## 4. 未采信清单（只有结论、无源码/日志/证据）

以下来源**未作为结论依据**，仅作背景参考；其具体失败断言因缺乏可核查的源码/日志/原始贴而被排除。

- **[未采信] hqwc.cn《算法竞赛避坑指南:从本地AC到线上WA》**：通篇为通用竞赛建议（CE/WA/TLE 分类、diff 比对等），**无任何 CANN/Ascend C 专属证据、无日志、无源码**，且未给出可核查的原始判题记录。仅可作为「竞赛通用方法论」的旁证，不能支撑本题任何具体结论。
- **[未采信] 若干 CSDN `gitblog_*` 自动聚合帖**（含「CANNJudge 算子竞赛全流程指南」「CANN算子竞赛代码提交脚本说明」「CANN预选赛算子测试」）：这些帖子由聚合/生成式账号发布，**缺乏指向 primary source（具体 issue/PR/commit）的永久链接**，无法直接核验其引用的失败案例是否真实发生；其「提交状态名」「计分公式」等过程性事实，仅因被官方帖（C15）与本题已知规则**独立佐证**后才被有限采用，其自身的失败 anecdote 一律不采信。
- **[未采信] 纯「经验总结」类回复**：凡在论坛/问答中只写「我当初就是 XXX 才过的 / 要注意 YYY」而无复现步骤、无报错文本、无代码差异的，均不采信（例如部分「算子竞赛经验」短回复）。

> 说明：本报告的结论性论断（Compile Error 大类、对齐/溢出/舍入/上传截断/保留字/bf16 不支持/多核一致性）均有 A/B/C 级**带原文摘录**的来源支撑，未使用上述未采信项作为依据。

---

## 5. 专项分析（针对我们的处境）

### 5.1 上传内容截断（V002 同款问题）
**有没有别人遇到过「平台收到的文件与本地不一致」？**
有。C7（掘金分片上传）是工程级的完整复盘：高并发分片 + 路由器 MTU/包重组错误 → 收到的文件大小与原始不一致、随机字节损坏、解压失败，且「成功状态码 + 大小检查」都发现不了。我们的 V002（首行变成 `return false;`）形态不同（属于「首部被替换/截断」而非随机损坏），但同属「传输/编辑层使平台落盘文件 ≠ 本地源文件」这一大类。

**可能原因（按可能性排序）**：
1. 复制粘贴提交：从编辑器整段复制进网页文本框时，富文本/换行/编码被改造，或只粘了片段（最可能解释 V002 首行 `return false;`）。
2. 文件大小/分片限制：超大单文件（2955 行）被上传组件截断或分片重组出错。
3. 特殊字符/编码：中文注释、BOM、不可见字符（零宽空格、CR/LF 混用）在传输中被吞掉或改写，破坏结构。
4. 编辑器自动改写：IDE/网页在保存/粘贴时插入了模板片段。

**怎么防（低成本、必须做）**：
- **提交前**：本地 `wc -l`、`md5`/`sha256`、确认首行是 `#include "kernel_operator.h"` 或 `extern "C"`，末行完整；统一保存为 **UTF-8 无 BOM、LF 换行**。
- **提交时**：优先用平台官方上传/客户端，避免网页文本框粘贴；如必须粘贴，先粘贴到空白文件核对行数再提交。
- **提交后**：若平台回显内容/预览，立即回看首末行与字节数是否吻合；一旦 15/15 Compile Error 且日志怪异（如 `unknown type`、首行异常），**先怀疑通道，再怀疑代码**。

### 5.2 无法本地编译就提交（零编译证据下的低成本前置校验）
我们没有 CANN/NPU，从未本地编译过。在「准编译闸门」缺位时，可用以下**纯静态、零依赖**手段替代部分编译检查：

1. **括号/符号平衡**：脚本统计 `(){}[]<>` 配对（注意模板 `<>` 与比较 `<` 的歧义，仅作粗筛）；统计 `extern "C"`、`__global__`、`__aicore__`/`__vector__` 出现次数成对。
2. **首末行与结构检查**：首行应为 `#include "kernel_operator.h"`；必须存在 `extern "C" void run_kernel(...)` 且 `<<<blockNum,nullptr,stream>>>` 形态正确（CANNBot 直调指南强调 `.asc` 后缀、入口属性、禁止前向声明）。
3. **保留字/禁用符号扫描**：grep `block_idx`、`block_dim`、`pipe`、`pipe_t`、`std::`、`new `、`malloc`、`throw`、`vector<`、`class std` 等，命中即高风险（对应 C1/C4）。
4. **API 白名单核对**：只允许出现已知支持的 API（`DataCopy`/`DataCopyPad`/`Cast`/`Add`/`Mul`/`ReduceSum`/`rsqrt`/`GetValue` 等），对 BF16 路径强制检查 `Cast` 绕行（C14）。
5. **限制文件复杂度**：函数嵌套深度、单函数长度、模板实例化数量设上限；对 2955 行巨型 kernel 做「能否拆成 ≤N 行的小 kernel」评估（见第 6 节）。
6. **对齐/溢出静态审查清单**：所有 UB 分配 32B 对齐；所有 GM 写回非对齐尾块用 `DataCopyPad`+mask；`row*D` 用 `uint64_t` 防溢出；`ReduceSum` work 空间算足且 `srcInnerPad=true`。

> 这些不能替代编译，但能把「V001/V002 类符号/截断错误」在发出前拦下大半。

### 5.3 提交额度管理（每天 50 次、取最后一次成绩）
**这意味着什么策略**：
- 「取最后一次成绩」= 前面的提交只用于**探路/验证通道**，最终成绩只看最后一次。因此**不要把未验证的大改当作最后一次**。
- 推荐节奏：
  1. **第 1 次**：只提交一个**最小可编译、功能最简**的 kernel（如仅 FP16 单路径、无分支），目的只有一个——**验证上传通道与编译通道通不通**（直接针对 V001/V002 复现风险）。
  2. **中间若干次**：在最小版本基础上，分批增加 dtype/shape 分支与优化，**每次只改一类**，便于从编译/判题状态反推是哪一类引入问题。
  3. **最后一次（当天）**：提交经过自测、结构收敛、复杂度可控的版本。
- 「每天 50 次」不是鼓励乱提交：每次提交都消耗当天的「最后一次」机会成本，**任何一次都应是经过本地静态校验 + 离线数值自测的**，严禁「先交了看报错」式的盲提交。

### 5.4 如何在只有 1 个本地样例（FP16 [1,64]）时最大化首次通过概率
- **认知前提**：15 点全过才计分（C15/C17 同构铁律），1 个样例通过 ≈ 0 信息量。必须**离线自造隐藏点用例**（即使没有 NPU，也能用 CPU/numpy 参考实现做数值对照，验证逻辑等价性）。
- **必造用例清单**（对应 C16/C17 失分集中区）：
  1. dtype 全矩阵：fp16 / bf16 / fp32 各至少 1 例（bf16 必须 Cast 绕行，C14）。
  2. 维度：2D/3D/4D 各至少 1 例（沿最后一维 D 归约）。
  3. **D 非 32 倍数**：如 D=64 之外再测 D=70、D=100、D=3001（触发尾块 mask/DataCopyPad，C8/C12）。
  4. **D 极大**：D=32768 量级，验证 `row*D` 不溢出 `uint32_t`（用 `uint64_t`），验证 `ReduceSum` work 空间足够（C11）。
  5. 边界：单元素、极小值 eps、gamma=0、bias=0、rms 极小的数值稳定性（防 NaN/溢出，C9/C16）。
  6. 多核：blockNum>1 验证多核 DMA 写回无 32B cache line 踩踏（C8/C12）。
- **离线数值自测**：在 macOS 用 numpy 实现 golden（双 1e-4 / 1e-3 阈值），把同一套输入跑 CPU 参考与 kernel 的「逻辑等价 Python 模型」比对，至少确认**运算顺序与舍入路径**一致（C9）。没有 NPU 也能验证算法层，把「逻辑错」在提交前消灭。

---

## 6. 复杂度风险评估（2955 行巨型多分支 kernel）

### 评估对象
当前 `提交/V002/kernel.asc`：**2955 行**，内含大量专用分支（`SmallFp32Batched` / `WideFp16Cached` / `ProcessFp32FullRowOutputPipelined` 等**十几个 Process 变体**）。

### 相对「小而稳 kernel」的失败风险对比

| 风险维度 | 巨型多分支 kernel（当前） | 小而稳 kernel | 结论 |
|---|---|---|---|
| 编译失败概率 | 高（C3 `out of jump/jumpc imm range` 直接命中；体量越大越易触发 int16 跳转上限；保留字/符号扫描更难） | 低（跳转范围小、分支少） | 巨型显著更高 |
| 分支走错 | 高（十几个变体，判题 shape/dtype 落入错误分支 → 静默错或崩） | 低（路径少、易全覆盖自测） | 巨型显著更高 |
| 维护/定位难度 | 高（2955 行，单点改动牵一发动全身；V001/V002 类故障难定位） | 低（易读易改） | 巨型显著更高 |
| 上传截断概率 | 高（文件越大，C7 类截断/损坏概率上升） | 低 | 巨型更高 |
| 性能上限 | 可能更高（专用分支可针对优化） | 较低（通用路径） | 巨型唯一优势，但**前提是先编译通过且分支正确** |

### 佐证
- **编译维度（证据）**：C3 官方指南明确「kernel 代码过大 → `out of jump/jumpc imm range`」，这是 2955 行 kernel 的**直接、官方级**警告。
- **分支走错（论证而非证据）**：软件工程常识 + 竞赛案例（C16 边界失分 35%）表明，分支越多，未覆盖/走错分支导致隐藏点失败的概率越高；本题 15 点全过才计分，任一分支错即 0 分。
- **维护/上传（证据+常识）**：C7 证明大文件传输更易损坏；2955 行使符号扫描、diff 复核、人工定位成本陡增，与「无本地编译」叠加后风险放大。

### 建议（论证而非证据，基于上述 + C3/C7/C16）
1. **先稳后快**：当前阶段目标是「15/15 先全过」，不是拿性能满分。优先把十几个变体**收敛为少量通用、参数化的路径**（按 dtype/对齐/是否多核 用 `if constexpr` 而非完全独立的 Process 类），砍掉未经验证的专用分支。
2. **控制体量**：每个 Process 变体尽量短；如体量仍大，主动加 `-mllvm -cce-aicore-jump-expand=true`（C3 方案）作为兜底，但更要减少无谓分支。
3. **分支可测**：每个变体都能被离线自造用例覆盖（见 5.4），避免「某个变体永远不被测到却在某测试点被选中」。
4. **小 kernel 通关后再叠加优化**：用「最小可编译版本」先拿 Pass（C15），再逐步引入专用分支与流水线，每次增量提交（见 5.3）。

---

## 7. 给本题的「提交前必做检查清单」

> 以下清单不依赖本地编译，全部可在 macOS 上零依赖完成；目标是把 V001/V002 类故障与已知精度陷阱在发出前拦下。

**A. 上传完整性（防 V002）**
- [ ] 文件保存为 **UTF-8 无 BOM、LF 换行**；无中文/特殊不可见字符问题。
- [ ] 提交前 `wc -l` / `md5` 记录；首行是 `#include "kernel_operator.h"`，末行完整闭合。
- [ ] 提交后若平台有内容回显，立即核对首末行与字节数；用官方客户端而非粘贴。

**B. 编译类静态扫描（防 V001/C1/C3/C4）**
- [ ] grep 保留字/禁用符号：`block_idx` `block_dim` `pipe` `pipe_t` `std::` `new ` `malloc` `throw` `vector<` → 零命中。
- [ ] `extern "C"` / `__global__` / `__aicore__`(或`__vector__`) 配对正确；入口为 `run_kernel` 且 `<<<blockNum,nullptr,stream>>>` 形态正确。
- [ ] 圆括号/花括号/方括号/尖括号粗筛平衡（模板 `<>` 除外）。
- [ ] 文件行数/体量评估：若接近触发 `out of jump/jumpc imm range`（C3），考虑收敛分支或加 jump-expand 编译选项。

**C. 精度/对齐/溢出（防 C8/C9/C10/C11/C12/C14/C18）**
- [ ] 所有 UB 分配 32B 对齐；`ReduceSum` 的 `srcInnerPad=true`，work 空间用 `GetReduceSumMaxMinTmpSize` 算足。
- [ ] 所有 GM 写回非对齐尾块用 `DataCopyPad` + `DataCopyPadExtParams` 正确配置 + mask；禁止裸 `DataCopy` 写非对齐长度。
- [ ] BF16 路径确认 `Cast(bf16→float)→Add/Mul→Cast(float→bf16)` 绕行（C14）。
- [ ] `row*D` 偏移用 `uint64_t`，防 `uint32_t` 溢出（D 极大时）。
- [ ] FP16/BF16 的 `1/rms` 与 golden 保持**相同运算顺序与舍入**（C9）；离线 numpy 对照验证。
- [ ] 标量 `GetValue`/`SetValue` 确认 Scalar→MTE 依赖已同步（C18）。

**D. 隐藏点覆盖（防 C16/C17，离线自造用例）**
- [ ] dtype：fp16/bf16/fp32 各覆盖；维度：2D/3D/4D 各覆盖。
- [ ] D 非 32 倍数（70/100/3001 等）；D 极大（32768 级）。
- [ ] 边界：单元素、eps 极小、gamma=0、bias=0、rms 极小（数值稳定性）。
- [ ] 多核：blockNum>1 验证无 32B cache line 踩踏。

**E. 提交策略（防 5.3）**
- [ ] 当天首次提交 = 最小可编译版本，仅验证通道；最后一次 = 经静态校验 + 离线自测的收敛版本。
- [ ] 任何 15/15 Compile Error + 怪异日志，先怀疑上传/符号，不盲目改算法。

---

## 8. 来源清单（S241–S270）

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

---

### 附：本队实测失败（内部来源，不计入 S 编号）
- **V001（Compile Error）**：平台日志 `unknown type name 'pipe_'; did you mean 'pipe_t'?`、`cannot use dot operator on a type` → 属保留字/符号类编译错（与 C1/C2/C5 同类）。
- **V002（上传包错误）**：平台收到的 `kernel.asc` 首行为 `return false;`，与本地 2955 行快照完全不同 → 上传截断/异常（与 C6/C7 同类），非算法问题。

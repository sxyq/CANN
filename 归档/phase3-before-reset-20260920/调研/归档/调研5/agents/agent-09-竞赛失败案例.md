# Agent 09 调研报告：竞赛经验与失败案例库（AddRmsNormBias）

> 子代理：Agent 9（竞赛经验和失败案例）｜访问日期：2026-09-12（北京时间）
> 方法：本地失败记录复盘（`提交/V001/结果.md`、`提交/V002/结果.md`、`提交/版本实验记录.md`、`提交/README.md`）+ 网络检索（昇腾社区论坛、CSDN/知乎生态、GitHub Issue、官方 API 文档、往届赛事报道）。
> 约束遵守：纯调研，未调用 CANNJudge 任何接口，未修改任何本地文件，未在本项目目录创建除本报告外的文件。
> 证据等级（依 AGENTS.md）：A=官方 API 文档/官网；B=官方 GitCode/GitHub 仓库、官方样例、官方培训材料；C=社区文章、论坛、个人仓库、媒体报道。
> 证据状态：verified=已读取原始内容；partial=仅核对摘要/搜索结果；unavailable=受访问限制；contradicted=来源冲突。
> 特别标注：**"结论性资料"= 只有结论、没有日志/代码佐证的 C 级经验贴，不得作为独立证据，只作策略参考。**

---

## 1. 调研范围与覆盖

| 平台/入口 | 检索内容 | 覆盖结果 |
| --- | --- | --- |
| 昇腾社区论坛（hiascend.com/app-forum） | 算子挑战赛经验帖、宏冲突编译错误帖 | 命中 S1 获奖队伍复盘、TriangularMode::LOWER 宏冲突案例 |
| CSDN（含昇腾生态专区 hwcomputing/ascendai/cann） | 竞赛实战、避坑指南、DataCopyPad/同步/段错误专题 | 命中复旦赛作品、as_strided CANNJudge 实战、Erf 优化作品、DataCopyPad 溢出 Bug、多篇避坑指南 |
| CSDN gitblog 转载（源自 GitCode cann/ 官方仓） | cann-learning-hub、cannbot-skills、asc-tools、cann-ops-competitions | 命中 CANNJudge 提交 Skill（含 Histogram 陷阱）、rms_norm.asc 直调示例、同步事件管理、提交规范 |
| GitHub Issue | simpler、tilelang-ascend、vllm-ascend、PTOAS、AscendOpGenAgent | 命中 BLK 宏冲突、GM→UB 列切片 pad 0、gather_v3 越界断言、CANN 9.0.0 编译器崩溃、越界索引精度不稳定、cumsum 非确定性 |
| 官方 API 文档（hiascend.cn / asc.gitcode.com） | GET_TILING_DATA_WITH_STRUCT、毕昇编译器预定义宏 | 确认"暂不支持 kernel 直调工程"约束（A 级） |
| MindSpore 官方文档 | CANN 常见错误分析（E/EB 错误码） | 命中 E80012 ReduceSum 维度、EB0000 UB 超限 |
| 高校/媒体报道 | S1/S2 赛季获奖选手采访 | 西工大 wanna be free、北交大 Tangefly（结论性资料） |
| 本地项目文件 | V001/V002 平台失败记录 | 完整复盘（见第 2 节） |

**外部案例总数：22 条**（含可核查链接），按 CE/WA/RE/TLE/上传/泛化/提交策略七类编目，超出验收标准（≥8 条）。

---

## 2. 本项目 V001/V002 失败案例复盘

### 2.1 案例 L-01：V001——`pipe_` 标识符与平台宏冲突（15/15 Compile Error）

- **证据位置**：`提交/V001/结果.md`；提交编号 `6aa37d942d3dd2c5ae7da586`。
- **现象**：平台 15 个测试点全部 `Compile Error`，无有效分数。平台日志：
  ```text
  unknown type name 'pipe_'; did you mean 'pipe_t'?
  cannot use dot operator on a type
  ```
  错误集中在 `kernel.asc` 的 `pipe_` 成员声明与 `InitBuffer` 调用处。
- **根因**：Kernel 类成员命名 `TPipe pipe_;`，`pipe_` 撞上判题平台编译环境中的预定义宏（`pipe_t` 是 Ascend C 管道枚举类型名，编译器因此给出 "did you mean 'pipe_t'" 提示）。本机（macOS，无 CANN）无法复现，平台侧才触发。
- **外部佐证**（同类失效模式，详见 3.1 节）：CANN 设备编译器头文件 `__clang_cce_vector_intrinsics.h` 中定义了 `#define BLK BLK_Type()`、`#define LOWER Lower_Type()` 等大量短名宏，任何同名标识符（成员名、枚举值、常量）都会被宏展开破坏，报出与原代码 seemingly 无关的语法错误；且这些宏只在设备侧（NPU 编译）路径生效——CPU 仿真通过不能排除此类问题。
- **对本题的预防措施**：
  1. 避免以下划线结尾或短小的常见词作标识符；已将 `pipe_` 统一改为 `tpipe`（V002 落盘）。
  2. 提交前静态扫描：`grep -nE '\b(BLK|LOWER|UPPER|pipe_|T)\b' kernel.asc` 类检查（注意 `T` 作模板参数名也是高危项）。
  3. 判定规则：平台报"unknown type name X; did you mean Y"且 X 是自己声明的成员名时，第一反应应是宏冲突而非笔误。
- **状态**：已确认（平台日志 + 本地文件双证据）。

### 2.2 案例 L-02：V002——上传内容异常（平台收到的文件与本地不一致）

- **证据位置**：`提交/V002/结果.md`；提交编号 `6aa388bd2d3dd2c5ae81372f`（2026-09-11 12:51:09）。
- **现象**：平台日志显示收到的 `kernel.asc` 第 1 行为 `return false;`，随后连锁报错：`expected unqualified-id`、`extraneous closing brace`、`IsSingleTensorGroup` / `IsPositiveShape` / `MAX_ELEMENTS` / `MAX_DIM` 未定义。15/15 `Compile Error`。
- **关键事实**：当时本地快照（405 行）首行为 `#include <cmath>`，包含完整 `run_kernel` 入口和上述全部符号定义。**平台收到的内容与本地文件不一致**，故登记为"上传内容异常"，不计为算法编译失败。
- **根因推断**（未定论，见第 6 节）：在线编辑器粘贴不完整/截断（首行恰是某函数体内的 `return false;`，疑似粘贴起点错位或编辑器残留旧内容），属"上传通道内容损坏"而非代码缺陷。
- **外部对照**：CNASP 坤坤爱曼巴队（S1 铜奖）在昇腾论坛复盘："最后改了指针名字没检查（能否 build success），导致两道题 0 分"——同样是**提交动作本身引入的失败**，与本案例互为印证：最后一步的小改动/操作失察可以直接烧掉一次提交额度。
- **对本题的预防措施**（对应 V002 结果.md 中已有流程，进一步固化）：
  1. 上传后在编辑器中回读核对：首行、末行、总行数、`run_kernel` 入口存在性。
  2. 严禁混用快照：405 行版本与 2955 行版本不可拼接，单次提交只用一份完整快照。
  3. 把"上传后回读"写入提交 checklist，与"提交前本地静态核对"并列为双闸门。
- **状态**：现象已确认；根因（编辑器粘贴机制）无法从外部复核，保持"上传内容异常"定性。

---

## 3. 外部失败案例库（按失败模式分类）

每条含：来源链接、证据等级/状态、现象 → 根因 → 对本题的预防措施。

### 3.1 CE 类：Compile Error

**C-CE-01｜BLK 宏冲突：CANN 8.5.1 设备编译器破坏第三方仓构建**
- 来源：https://github.com/hw-native-sys/simpler/issues/517（2026-04-10，含完整编译日志）｜C 级，verified。
- 现象：CANN 8.5.1 下 `tensor.h:300: error: illegal initializer (only variables can be initialized)`，全部 aicore 编译单元失败；CANN 8.5.0 的 CI 正常。
- 根因：8.5.1 的 `__clang_cce_vector_intrinsics.h:555` 新增 `#define BLK BLK_Type()`，与源码中 `constexpr uint64_t BLK = 64` 冲突，宏展开后声明变成非法代码。
- 对本题：**宏冲突是随 CANN 小版本漂移的**——本地/模板默认版本编译通过 ≠ 判题平台版本通过；提交前应避免任何与 intrinsics 头文件常用短名（BLK、LOWER 等）重合的标识符。与 V001 的 `pipe_` 同类。

**C-CE-02｜LOWER 宏冲突：CPU 模式通过、NPU 模式编译失败**
- 来源：https://www.hiascend.com/app-forum/topic-detail/0279188908116457252（2025-07-26，官方论坛，含完整编译器输出）｜C 级，verified。
- 现象：MatmulPreluInvocation 样例 `./run.sh -r cpu` 通过，NPU 运行时报 `error: called object type 'AscendC::TriangularMode' is not a function or function pointer` 与 `expected '= constant-expression' or end of enumerator definition`。
- 根因：`__clang_cce_vector_intrinsics.h:44` 定义 `#define LOWER Lower_Type()`，把官方 matmul 库自身枚举 `TriangularMode::LOWER` 展开破坏。该头文件只在设备侧编译路径引入，CPU 仿真路径不包含，因此 CPU 通过无法拦截。
- 对本题：**"CPU/仿真通过 ≠ 平台编译通过"是结构性风险**；本题本机无 NPU，任何本地验证都替代不了平台首提。平台日志中"与本地代码无关的奇怪语法错误"应优先怀疑宏展开。

**C-CE-03｜CANNJudge 命名空间差异：本地 CANN 9.0 通过、平台 Compile Error**
- 来源：https://blog.csdn.net/gitblog_01418/article/details/150380401（as_strided 实战，2026-05-20，原作为 GitCode cann/cann-learning-hub 官方学习中心仓实践记录）｜C 级（CSDN 转载），verified。
- 现象：as_strided 首次提交 CANNJudge 遭 Compile Error；本地 CANN 9.0 编译通过。
- 根因：判题环境使用不同 CANN 版本时，Ascend C 类型可能位于 `AscendC::` 命名空间下而未默认引入。修复：在 .cpp 头部显式 `using namespace AscendC;`。
- 对本题：题面标称 CANN 9.0.0，但 as_strided 案例证明平台侧版本行为可能与本地默认不同；若 V002 后续提交出现大量"未定义类型"类报错，优先补命名空间显式引入。

**C-CE-04｜GET_TILING_DATA_WITH_STRUCT 明确不支持直调工程（官方约束）**
- 来源：https://www.hiascend.cn/document/detail/zh/CANNCommunityEdition/800alpha002/apiref/ascendcopapi/atlasascendc_api_07_0215.html（官方 API 文档）｜**A 级**，verified。
- 现象/约束：官方文档"约束说明"明确写出：`GET_TILING_DATA_WITH_STRUCT` **"暂不支持 kernel 直调工程"**。
- 对本题：本题即直调单文件 `kernel.asc`（`code_template=npu_kernel_dev`），tiling 参数走 `run_kernel` 直接入参，**不得**在 kernel.asc 中使用 `GET_TILING_DATA_WITH_STRUCT`/`GET_TILING_DATA` 宏式获取 tiling；使用即 CE。这是任务书点名的风险点，已获 A 级文档确认。

**C-CE-05｜CANN 9.0.0 bisheng 编译器后端崩溃（Intrinsic 类型错误）**
- 来源：https://github.com/mouliangyu/PTOAS/issues/380（2026-05-19，含完整编译器栈）｜C 级，verified。
- 现象：CANN 9.0.0 环境下 bisheng 编译 `Intrinsic has incorrect argument type! … fatal error: error in backend: Broken module found`。
- 对本题：说明目标 CANN 版本的编译器本身存在已知缺陷面；若平台日志出现 backend/bisheng 级报错，应先怀疑编译器/版本组合，改写代码绕开触发模式（如特殊 intrinsic 组合），而非反复重交。

**C-CE-06｜入门级 CE 集合（结论性资料）**
- 来源：https://blog.csdn.net/pz890123/article/details/152697389（2025 昇腾训练营避坑指南，2026-03-02）；https://blog.csdn.net/qq_41397792/article/details/153880915（开发工具链，2025-10-25）｜C 级，partial（摘要级核对）。
- 内容：`kernel_operator.h: No such file or directory`（环境变量未 source）、`__aicore__ does not name a type`（头文件/编译选项错误）等。属结论性资料，仅作 checklist 参考，不独立支撑结论。

### 3.2 WA 类：Wrong Answer

**C-WA-01｜DataCopyPad 写出溢出：32 字节 burst 对齐 padding 覆盖相邻段（与本题尾块风险同构）**
- 来源：https://blog.csdn.net/ferriswym/article/details/162206787（2026-06-22，含内存布局推导与代码）｜C 级，verified。
- 现象：Segment Reduce 算子，`feat_dim=4`（每段 16B）时输出段间出现垃圾值；更致命的是**空段（无数据可写）位置永远保留相邻段写出时溢出的 padding 值**，因为空段自己不触发写出，无人覆盖回正确值。
- 根因：UB→GM 的 `DataCopyPad` 按 32B burst 对齐搬运；有效数据 16B 时 DMA 实写 32B，多出的 16B 覆盖相邻段前 4 个元素。逐段顺序写出时后段覆盖前段溢出、最终正确；一旦存在"不写出的位置"，溢出垃圾即残留。
- 对本题：AddRmsNormBias 中 `D × sizeof(T)` 非 32B 倍数时（fp16 D=33、bf16 D=67、fp32 D=1/33 等），逐行 `DataCopyPad` 写回同样存在 padding 越界写相邻行头部的风险；若多核按行并发写回且相邻行分属不同核，后写核的 padding 会踩掉先写核的结果，且结果**依赖写回顺序（不确定）**。预防：尾块行写回后由"本行所有者"显式回读校验，或采用整块对齐搬运 + 精确长度控制；至少把"D 非 32B 倍数 + 多核写回"列为真机第一验证用例（本项目 `提交/版本实验记录.md` 已规划 V003 对齐行块调度，方向一致）。

**C-WA-02｜复旦赛 ClipByValue：绕过输出队列的"快路径"导致 WA；int32 整数比较不稳定**
- 来源：https://blog.csdn.net/gitblog_00702/article/details/151517425（CANN 复旦赛作品，作者潘正浩/MrWine，2026-06-07；原作为 GitCode cann/cann-ops-competitions 竞赛提交作品）｜C 级（CSDN 转载），verified。
- 现象：为提速尝试"直接原地计算或绕过输出队列"的方案，部分测试点 Wrong Answer；int32 路径直接用整数 min/max 也出现不稳定。
- 根因：绕开 TQue 流水破坏了搬运/计算的同步语义（异步执行下读到未就位/已释放数据）；int32 语义对齐 torch.clamp 需要 float 中转。
- 对本题：任何"跳过队列直写"的性能捷径在判题平台上都可能变成 WA；V002 已含批处理/缓存行快路径，真机验证时须用非对齐多行用例覆盖这些快路径。

**C-WA-03｜复旦赛 Lerp：等价公式改写引入舍入误差**
- 来源：同上（C-WA-02）｜C 级，verified。
- 现象：把 `weight=0.5` 的 `start + 0.5*(end-start)` 改写为 `(start+end)*0.5`（数学等价），fp16 下部分测试点 WA/精度偏差；`weight=0/1` 直接拷贝输出的捷径同样 WA。最终保留与 PyTorch 完全一致的计算顺序。
- 对本题：**"数学等价"不等于"数值等价"**。AddRmsNormBias 的 `y/sqrt(mean(y²)+ε)*γ + b` 若被改写（如 `y*rsqrt(...)` 或先乘 γ 再除 rms），在 fp16/bf16 容差（1e-3）内也可能越界。保持与题面参考实现逐步一致的运算顺序；rsqrt 类替换必须真机验证后再启用。

**C-WA-04｜GM→UB 列切片（非连续内存）搬运结果错误（CANN 9.0.0）**
- 来源：https://github.com/tile-ai/tilelang-ascend/issues/1195（2026-06-16，含最小复现代码；CANN 9.0.0、A3 设备）｜C 级，verified。
- 现象：`T.copy(gm[a:b, i], ub)` 形式（按列切片）搬运后，ub 中除第一个元素外全部被 pad 成 0。
- 根因：DMA 按连续地址拷贝，非连续（跨 stride）源被当作连续段搬运，编译期无检查。
- 对本题：`gamma`/`bias` 是按最后一维广播的一维向量（连续，无此风险）；但若为省事对 `x`/`residual` 做任何跨 stride 切片搬运（如 4D 展平时的错误寻址），将得到看似合法的错值。行基址计算（V002 已用 uint64_t）必须保证每个搬运源是连续段。

**C-WA-05｜归约/扫描类算子的非确定性参考值：fp16 dim=0 并行扫描**
- 来源：https://github.com/Just-it/AscendOpGenAgent/pull/132（2026-04-27，含 workaround 代码与数值分析）；https://github.com/Just-it/AscendOpGenAgent/pull/149（2026-04-29，CPU 标杆对齐）｜C 级，verified。
- 现象：NPU `torch.cumsum` 对 fp16 2D tensor 沿 dim=0（strided scan）结果非确定：小 tensor 多次运行有 ~0.1–0.5% 随机波动，大 tensor（8192×16384）系统性偏离 ~10%；dim=1（contiguous scan）稳定。
- 对本题：说明"最后一维归约"的**实现方式（并行部分和 vs 串行）会改变舍入路径**。AddRmsNormBias 判题为固定 15 个测试点（题面明确结果确定），但参考实现是 PyTorch 的 `mean(dim=-1)`；若平台参考在 NPU 上以分块并行方式计算 mean(y²)，我们的 FP32 顺序累加与之存在系统性偏差的可能。缓解：FP32 累加已是最高性价比方案（与题面 1e-4/1e-3 容差匹配），不必为对齐参考而模仿其并行顺序；但若真机出现稳定小幅超差，应怀疑参考实现的并行归约路径。

**C-WA-06｜属性默认值的特殊语义误读（Histogram min=0/max=0）**
- 来源：https://blog.csdn.net/gitblog_00410/article/details/143789539（CANNJudge 算子提交 Skill，2026-05-09；原作为 GitCode cann/cann-learning-hub 官方学习中心仓）｜C 级（CSDN 转载），verified。
- 现象：把 `min=0.0, max=0.0` 当有效范围实现，判题 WA；`torch.histc` 文档中该默认值实际表示"自动计算数据范围"。
- 对本题：本题 epsilon 是入参而非属性，但同理**不得假设 epsilon 固定值**（如写死 1e-6）；必须按 `run_kernel` 实际传入值计算。版本实验记录中"epsilon 假设固定值"已列为泛化风险，此案例提供外部佐证。

**C-WA-07｜测试数据生成缺陷导致"正确实现"被判 WA（越界索引）**
- 来源：https://github.com/Just-it/AscendOpGenAgent/issues/83（2026-04-14，含 case 定义与影响分析）｜C 级，verified。
- 现象：`_make_tensor` 用 `randint(0, 8001)` 生成索引可含越界值 8000；CANN 参考算子对越界索引写入未定义内存产生"损坏输出"，正确实现（跳过越界）反而在精度比对中 FAIL，且**不稳定**（取决于随机数是否恰好越界）。
- 对本题：非直接同类（本题无索引输入），但提示：若平台某测试点"时而过时而不过"，需考虑测试数据/参考侧的不稳定因素，而非只怀疑自己的 kernel；此时应向平台反馈而非盲目改代码。

### 3.3 RE 类：Runtime Error

**C-RE-01｜UB 容量按经验假设错误：ascend910b（DAV_2201）实际 192KB 而非 248KB**
- 来源：https://blog.csdn.net/gitblog_01418/article/details/150380401（as_strided 实战，同 C-CE-03）｜C 级（CSDN 转载），verified。
- 现象：设计稿按 248KB（253952B）规划 UB，代码审查阶段查 CANN 9.0.0 源码 `kernel_utils_constants.h` 发现 `__NPU_ARCH__ == 2201`（DAV_2201，即 Ascend910B 系）实际 `TOTAL_UB_SIZE = 192KB (196608)`；若按 248KB 规划，`InitBuffer` 运行时 buffer 溢出崩溃。A5 系列 248KB、A3 系列 192KB，`GetUBSizeInBytes()` 不支持 A3。
- 对本题：**本题目标 SoC 即 dav-2201——本案例直接命中本项目硬件**。V002 的 UB 规划（FP16/BF16 tile 4096 元素、FP32 tile 2048 元素、峰值 80~112KB）在 192KB 安全线内，方向正确；后续 V003 单遍暂存（D≤4096）同样以 192KB 为准，禁止引用任何 248KB 数字。
- 状态备注：192KB 数值出自该实践记录引用的 CANN 9.0.0 源码常量，属 B 级引用链，建议真机侧再以编译器头文件复核一次（已列入第 6 节）。

**C-RE-02｜gather 类算子越界断言（运行期索引校验失败）**
- 来源：https://github.com/vllm-project/vllm-ascend/issues/1682（2025-07-09，含设备侧断言日志）｜C 级，verified。
- 现象：`gather_v3_base.h:137: Assertion '(0 <= val && val < this->gxSize_)' Index 2938 out of range[0 2938)`，推理任务中断。
- 对本题：本题无 gather，但 `gamma`/`bias` 按 `i ∈ [0,D)` 寻址——D 用 32 位变量且 outer×D 超过 2^31，或尾块计算 off-by-one，同样会以越界/断言形式爆 RE。V002 已将行基址提升为 uint64_t；尾块处 `i < D` 边界条件需静态核对。
- 状态：已关闭的社区 issue；根因属框架内置算子，仅作越界模式参考。

**C-RE-03｜同步缺失双案例：PipeBarrier 缺失→结果为 0/残留值；FreeTensor 缺失→流水线死锁**
- 来源：https://blog.csdn.net/2301_80840905/article/details/155034892（避坑指南：常见报错解析与精度优化复盘，2025-11-19）｜C 级，verified。
- 现象 1：`DataCopy` 发出后立刻发计算指令，结果全 0 或为上一轮残留——NPU 异步执行，数据"还在路上"。
- 现象 2：`DeQue` 后忘 `FreeTensor`，队列深度有限，流水线跑几块后卡死。
- 对本题：V002 在 `ReduceSum` 后增加了 `V_S`/`S_V` 同步并从目标缓冲读回（版本实验记录 A 节），正是对"归约结果未就位即读"的防御；任何后续性能版本删减同步前必须有真机对照数据。

**C-RE-04｜同步事件四类错误：set/wait 不配对、eventID 重复使用**
- 来源：https://blog.csdn.net/gitblog_00561/article/details/145252761（asc-tools 同步事件管理指南，2026-05-30；原作为 GitCode cann/asc-tools 官方调试工具仓）｜C 级（CSDN 转载），verified。
- 内容：npu_check 工具定义 ErrorSync1（写入缺 barrier/set-wait）、ErrorSync2（读取缺同步）、ErrorSync3（set/wait 不配对、eventID 不匹配）、ErrorSync4（eventID 重复使用）。
- 对本题：V002 的 `V_S`/`S_V` 事件必须严格配对且 eventID 唯一；若真机出现偶发挂死，优先用 asc-tools 的 npuchk 日志按这四类排查。工具链（cpu_debug/npu_check）同时证明：无真机条件下可先把 kernel 编成 CPU 域可执行体做逻辑级检查（但不能覆盖宏冲突类 CE，见 C-CE-02）。

**C-RE-05｜官方错误码样本：UB 超限与 ReduceSum 维度上限**
- 来源：https://www.mindspore.cn/tutorials/experts/zh-CN/r2.6.0rc1/debug/error_analysis/cann_error_cases.html（MindSpore 官方文档《CANN 常见错误分析》）｜**A 级**（官方文档），verified。
- 内容：`EB0000 Check failed: need_nbits <= info->max_num_bits (4194304 vs. 2097152): data_ub exceed bound of memory: local.UB`（UB 超限）；`E80012 ReduceSum 输入维度 [0,8]`。
- 对本题：UB 规划错误在编译期即可被拦（好消息：这类不会烧提交次数，本地/首次平台编译即暴露）；本题展平为 outer×D 后归约维度恒为 1，无 E80012 风险。

**C-RE-06｜Tiling 结构体跨端字节对齐错位（Host/Device 理解不一致）**
- 来源：https://blog.csdn.net/2301_80840905/article/details/155034892（同 C-RE-03）｜C 级，verified。
- 现象：Host/Device 两侧结构体编译设置不同，`int64` 被当两个 `int32` 读，参数全错位，输出乱码或越界。
- 对本题：直调模式下 `run_kernel` 参数为平铺标量（非结构体），天然规避此坑；但若候选把多参数打包成 struct 传指针，需两侧 `__attribute__((packed))` 或按从大到小排序。当前 V002 为平铺传参，无此风险。

### 3.4 TLE 类：超时/性能失效

**C-TLE-01｜标量操作黑洞：GetValue/SetValue 与 __aicore__ 内除法/取模**
- 来源：as_strided 实战（同 C-CE-03）踩坑全景图 #4/#5；性能验收显示 `aiv_scalar_ratio 31.5%` 被标注为地址计算开销｜C 级（CSDN 转载），verified。
- 现象：逐元素 `SetValue` 构建偏移表、kernel 内 div/mod 计算索引，吞吐极低；改 Host 预计算 + 向量搬运后 Case 3 从疑似回退路径拉回。erf 首考因同类问题（无 Host 预计算）首次提交 WA（precision=0.1875）且踩坑 6.5 小时、共 4 次提交。
- 对本题：AddRmsNormBias 两遍扫描中若用标量循环做 `x+residual` 或逐元素平方累加，必 TLE；`rstd = 1/sqrt(...)` 可用标量（每行一次）但行内必须全向量化。`ReduceSum` + `GetValue(0)` 读回是每行一次的必要标量开销，控制在 O(outer) 而非 O(outer×D)。

**C-TLE-02｜过度开核：轻量算子每核数据量过小，调度开销反噬**
- 来源：复旦赛作品（同 C-WA-02）："对 float32 大输入限制每个 core 至少处理约 3 个 32B block，减少轻量算子过度开核带来的调度开销"｜C 级（CSDN 转载），verified。
- 对本题：AddRmsNormBias 当 outer（展平后行数）小于核数时，多余核闲置；反之 outer 很小但强行满核会把每核行数压到 1–2 行，DMA 启动开销占比飙升。分核策略需对"outer < 核数 / outer ≈ 核数 / outer >> 核数"三档分别设阈值（版本实验记录 C 节已列多核用例，方向一致）。

**C-TLE-03｜正面样板：Erf 分规模路径设计拿到 15/15 Pass（含性能分）**
- 来源：https://blog.csdn.net/gitblog_00127/article/details/151604873（CANN Erf 算子优化作品，东南大学"关注塔菲喵"队，2026 CANN 算子挑战赛江山赛区预选赛，2026-07-02；原作为 GitCode cann/cann-ops-competitions 提交作品）｜C 级（CSDN 转载），verified。
- 要点：按输入规模分 Direct（单 tile 单核静态 LocalTensor）/Medium（分档多核 + 对齐 blockDim 搜索）/Large（TQue 双缓冲）三路径；对齐路径用 `DataCopy`、非对齐保留 `DataCopyPad`；poly9 Horner 链减少除法。CANNJudge 15/15 Pass，输出错误占比 0.00%。
- 对本题：这是与本题判题形态（15 测试点、精度门限 + 耗时计分）最接近的公开成功路径；"对齐快路径 + 非对齐兜底路径"双轨制可直接迁移到 D 为/非 32B 倍数两分支。

**C-TLE-04｜直调模式能力边界：不支持 MIX（Cube+Vector 并行，会挂起）**
- 来源：https://blog.csdn.net/ferriswym/article/details/162241341（直调模式 VS 算子框架模式，2026-06-23）｜C 级，verified。
- 对本题：本题是 vector 算子、直调提交，不涉及 MIX；但提醒候选中不得引入任何 Cube 类 API（Matmul 等），直调下会 hang 而非报错，平台表现可能为超时/挂起，浪费提交次数。

### 3.5 上传/平台类

**C-UP-01｜临场改名未重验，两题 0 分（S1 铜奖队伍亲历）**
- 来源：https://www.hiascend.com/app-forum/topic-detail/0272153064709468009（昇腾 AI 原生创新算子挑战赛 S1——CNASP 坤坤爱曼巴队经验分享，b1ankcat，2024-06-06）｜C 级，verified。**结论性资料**（第一人称复盘，无日志）。
- 原文关键句："提交前一定要检查一下能不能 build success，我们最后改了指针名字没检查，导致两道题 0 分。"
- 对本题：与 L-01（V001 改名类失败）、L-02（上传通道失察）构成三点成面——**任何提交前的最后修改（哪怕只改一个名字）都必须重走完整静态核对**；本题 15/15 全过才计分，一次 CE 提交的期望损失是一次提交额度 + 一个版本周期。

**C-UP-02｜CANNJudge 判题结果与提交节奏的量化样本**
- 来源：as_strided 实战（同 C-CE-03）对比表：erf 初赛 4 次提交（首次 WA precision=0.1875，最终 15/15 PASS，踩坑 6.5 小时）；as_strided 2 次提交（1 次 CE + 1 次 5/5 Pass，踩坑约 2 小时）｜C 级（CSDN 转载），verified。
- 对本题：设计审查（串讲）前移问题是把 4 次提交压到 2 次的主因；本项目无真机，"平台首提"本身即最贵的验证手段，应把 V002 提交当作一次完整的 A/B 实验（编译入口 + 上传完整性 + 尾块语义一次验证），并预留 V003 应对 WA/RE 回归。

**C-UP-03｜提交物格式规范（目录结构/文件树）**
- 来源：https://blog.csdn.net/gitblog_07213/article/details/151463322（CANN 竞赛作品提交规范，2026-05-19；原作关联 GitCode cann/cann-competitions）｜C 级（CSDN 转载），verified。
- 要点：作品需 code/report 分目录、README 完整、从平台"下载工程"获得模板后在其结构内工作。本题初赛为直调单文件 `kernel.asc` 在线编辑器上传（Agent 1 已核实），该规范主要适用于复赛/作品仓阶段；当前只需守住单文件完整性。

### 3.6 泛化/隐藏测试点类

**C-GE-01｜平台不开放测试用例，必须泛化设计（官方学习中心 Skill 明示）**
- 来源：CANNJudge 算子提交 Skill（同 C-WA-06）｜C 级（CSDN 转载），verified。
- 要点：平台不提供测试用例 API；泛化五要素 = shape 泛化（动态维度，不硬编码）+ dtype 泛化（模板分发）+ 属性泛化（边界/默认/特殊值）+ 对齐泛化（DataCopyPad 处理尾块）+ 边界泛化（空输入、单元素、极端值）。
- 对本题：直接支撑版本实验记录中的用例矩阵（D=1/31/32/33/67/127/128/129/1000/32768、2D/3D/4D、三 dtype）；其中 D=1（单元素行、32B 严重不对齐）与 D=32768（超单 tile）是最易漏的两个极端。

**C-GE-02｜广播语义分发样板（Addcmul 五模式）**
- 来源：复旦赛作品（同 C-WA-02）｜C 级（CSDN 转载），verified。
- 要点：按"无广播/标量广播/最后一维广播/后缀广播/通用广播"分模板路径，规则场景走连续 DataCopy，复杂场景走 offset 计算。
- 对本题：本题 gamma/bias 均为最后一维广播（最简单档），但提醒：若平台测试点含 `outer=1` 的退化 shape（等价标量广播路径），分支覆盖须显式测试。

**C-GE-03｜RMSNorm 直调源码的公开形态（迁移参照，非判题证据）**
- 来源：https://blog.csdn.net/gitblog_00619/article/details/141845484（cannbot-skills：rms_norm.asc → 自定义算子示例，2026-05-09；原作为 GitCode cann/cannbot-skills 官方仓）｜C 级（CSDN 转载），verified。
- 要点：公开 `rms_norm.asc` 含 3 个 kernel 类（Float/Half/Bf16），BF16 内部 Cast 到 FP32 计算再 Cast 回（与本项目 V002 策略一致）；host 侧为规避 CANN 版本间 tiling 库头路径差异，自行计算 tiling 字段。
- 对本题：佐证"BF16→FP32 中间计算→Cast 回"是被公开采用的 RMSNorm 家族实现范式；同时提示 CANN 版本间 host 侧库差异是真实痛点（直调模式恰好绕开 host tiling 库，是本题判题形态的先天优势）。

### 3.7 提交次数管理与策略（结论性资料为主）

| 来源 | 要点 | 等级/状态 |
| --- | --- | --- |
| S1 CNASP 队（C-UP-01） | 先花时间做一个通用切分模板，后续一维算子快速复用；提交前必须 build success 检查 | C 级，verified，结论性 |
| S2 北交大 Tangefly（https://we.yesky.com/blog/326258，2024-10-14） | 稳扎稳打：前半程先做基础解法保 AC，后半程集中性能优化；实时榜单按用例显示通过/耗时，据此定策略；预赛 10 题每题 5 用例（4 精度+1 性能） | C 级媒体报道，partial（正文已读，无日志），结论性 |
| S2 西工大 wanna be free（https://jsj.nwpu.edu.cn/info/1598/23245.htm，2024-10-12） | 决赛实时榜单 19:40 被反超、临场再优化未果（金牌仍入账）；建议官方文档写更细——"查阅文档时有些细节没写清，需要自己去调试去猜测" | C 级媒体报道，verified，结论性 |
| erf/as_strided（C-UP-02） | 有判题记录的量化节奏：审查前移可将 4 次提交压到 2 次 | C 级（CSDN 转载），verified，含判题表格 |

**对本题的迁移**：初赛窗口 2026-09-05 至 10-17，计分公式 `100/(1+log₁.₅(t/T))`、15 点全过才计分。综合上述经验与本项目两次失败：建议节奏为「V002 一次完整验证提交（目标：进精度阶段）→ V003 正确性基线（全 shape 兜底）→ 之后每次提交绑定一个可回滚的性能假设」；每次提交前执行固定 checklist（首末行/行数/run_kernel 入口/括号平衡/高危标识符扫描），任何临场改动重走 checklist。

---

## 4. 往届 CANN 算子赛经验汇总（区分证据强度）

### 4.1 有日志/判题记录/代码级证据（可作独立证据）

| # | 经验 | 出处 | 证据形态 |
| --- | --- | --- | --- |
| 1 | CANNJudge 首提 CE 的命名空间成因与修复 | as_strided 实战 | 判题结果表（5/5、precision=1.0、逐用例耗时） |
| 2 | erf 初赛首次 WA precision=0.1875、4 次提交至 15/15 | as_strided 实战（对比表） | 判题记录复述 |
| 3 | dav-2201 UB=192KB（源码常量级依据） | as_strided 实战（引 CANN 9.0.0 kernel_utils_constants.h） | 源码常量引用 |
| 4 | 绕过输出队列→WA、公式改写→精度偏差、int32 需 float 中转 | 复旦赛作品 | 作品文档中的调试记录（无原始日志，但含具体方案与结论对） |
| 5 | DataCopyPad 32B burst 溢出覆盖相邻段 | DataCopyPad 溢出 Bug 详解 | 代码 + 内存布局推导 |
| 6 | BLK/LOWER 宏冲突的完整编译日志 | simpler #517、hiascend 论坛 | 编译器原始输出 |
| 7 | GET_TILING_DATA_WITH_STRUCT 不支持直调 | hiascend 官方 API 文档 | A 级官方约束原文 |
| 8 | UB 超限/ReduceSum 维度错误码 | MindSpore 官方错误分析 | A 级官方错误日志样本 |
| 9 | Erf 分规模路径 15/15 Pass | 江山赛区 Erf 优化作品 | CANNJudge 结果（15/15、错误占比 0.00%） |
| 10 | 同步事件四类错误（ErrorSync1–4） | asc-tools 官方工具仓文档（转载） | 工具检测规则定义 |

### 4.2 无日志结论性资料（仅作策略参考，不作独立证据）

| # | 经验 | 出处 |
| --- | --- | --- |
| 1 | 改指针名未检查 → 两题 0 分；先做通用模板再复用 | S1 CNASP 队论坛复盘 |
| 2 | 前半程基础解法保 AC、后半程性能优化；用实时榜单定策略 | S2 Tangefly 媒体采访 |
| 3 | 决赛榜单临场被反超；官方文档细节不清需自己调试猜测 | S2 wanna be free 媒体报道 |
| 4 | fp16 大数吃小数、PipeBarrier/FreeTensor 缺失现象 | CSDN 避坑指南（现象-原因-解法三段式，无原始日志） |
| 5 | 段错误五大类分类学（主机侧/GM/LM/DMA/指令对齐） | hiascend 开发者博客（示例代码为构造样例） |

---

## 5. 对本题（AddRmsNormBias 直调模式）的风险启示清单

按"对本题致命度"排序，均映射到直调 `kernel.asc` 单文件提交形态：

| # | 风险 | 外部/本地依据 | 预防措施（落地动作） |
| --- | --- | --- | --- |
| R1 | **标识符与平台宏冲突 → 15/15 CE**（本地已发生一次） | L-01；C-CE-01/02 | 已改 `tpipe`；提交前扫描 `pipe_`、`BLK`、`LOWER`、裸 `T` 等短名；接受"平台日志出现与本地无关的语法错误 = 先怀疑宏" |
| R2 | **上传内容异常 → 无效提交**（本地已发生一次） | L-02；C-UP-01 | 上传后编辑器回读首行/末行/行数/`run_kernel`；单一快照原则；临场任何改动重走 checklist |
| R3 | **尾块 32B 写出溢出污染相邻行**（D 非 32B 倍数时） | C-WA-01（同构案例）；版本实验记录 C 节风险 2 | 真机第一验证用例锁定非对齐多行（D=33/67，fp16/bf16）；多核写回时验证相邻行边界；必要时对齐行块调度 |
| R4 | **UB 按 192KB 规划**（dav-2201 硬约束） | C-RE-01（直接命中本题 SoC） | 所有 tile/暂存预算以 196608B 为上界；不引用 248KB 数字；真机复核头文件常量 |
| R5 | **数值链改写导致精度超差**（数学等价 ≠ 数值等价） | C-WA-03；C-WA-05 | 保持 `mean(y²)+ε → sqrt → 除 → ×γ → +bias` 与参考一致的顺序；`rsqrt`/重排类优化须真机误差表支撑 |
| R6 | **归约同步缺失 → 结果为 0/残留** | C-RE-03/04；V002 修复记录 | `ReduceSum` 后 `V_S`/`S_V` 严格配对、eventID 唯一；性能版本删同步前必须有真机对照 |
| R7 | **直调 API 边界：GET_TILING_DATA_WITH_STRUCT 禁用；Cube/MIX API 禁用** | C-CE-04（A 级）；C-TLE-04 | kernel.asc 只走 `run_kernel` 入参；代码中不引入 matmul/cube 头 |
| R8 | **泛化缺口：D=1、D 超 tile、outer<核数、ε 写死** | C-GE-01/02；C-WA-06 | 用例矩阵含 D=1/32768、outer∈{1, 核数, 8×核数}；ε 严格取入参 |
| R9 | **标量化/过度开核 → TLE 丢性能分** | C-TLE-01/02 | 行内全向量化；分核三档阈值；每行 DMA 次数 O(D/tile) 而非 O(D) |
| R10 | **BF16 舍入模式**（Cast 回转 RoundMode） | C-GE-03（BF16→FP32→BF16 范式）；dynamic quant 知识库（AscendOpGenAgent PR #146，bfloat16 RINT 支持表） | 输出 Cast 显式指定 RoundMode（V002 用 CAST_RINT），真机分 dtype 误差表验证 1e-3 |
| R11 | **判题环境版本漂移**（本地/模板通过 ≠ 平台通过） | C-CE-01/03 | 首提结果按"环境差异→代码缺陷"两分支归因；出现成片"未定义"报错先试 `using namespace AscendC;` |

---

## 6. 未确认事项

1. **`pipe_` 冲突宏的准确定义位置**：V001 日志只有报错文本；推断为平台设备编译链（bisheng/ccec 的 `__clang_cce_vector_intrinsics.h` 家族）预定义宏，与 BLK/LOWER 同类，但未在本机 CANN 源码中直接定位（本机无 CANN）。建议真机环境 `grep -rn "define pipe_" $ASCEND_TOOLKIT_HOME` 复核。
2. **V002 上传内容异常的确切机制**：无法从外部复现在线编辑器的粘贴/缓存行为；保持"上传内容异常"定性，待下次完整上传以新提交编号验证。
3. **dav-2201 UB=192KB**：出自 as_strided 实践记录引用的 CANN 9.0.0 源码常量（B 级引用链），本次未直接读取头文件原文；真机侧应以 `kernel_utils_constants.h` 为最终依据。
4. **西南赛区判题环境与本地 CANN 9.0.0 的差异面**：as_strided 案例发生在公开题库（平台 CANN 8.5 时期记录），与本题平台（标称 CANN 9.0.0）不完全可比；"本地通过 ≠ 平台通过"作为风险成立，但具体差异项待 V002 平台日志检验。
5. **外部案例的硬件/版本迁移有效性**：C-WA-04（A3 设备）、C-CE-01（CANN 8.5.1）、C-RE-02（CANN 8.1.RC1）等与本题（dav-2201、CANN 9.0.0）环境不同，失效模式成立但触发阈值可能不同。
6. **媒体报道类经验（Tangefly/wanna be free）无任何日志**，已全部标注为结论性资料；其策略价值依赖与有日志案例（erf/as_strided）的一致性，不单独支撑结论。
7. Kaggle/AIcrowd/Codeforces/Topcoder/LeetCode Discuss/GPU MODE 通用竞赛失败模式（CE 上限、封榜策略等）与 CANNJudge 直调判题形态差异较大，本次检索未发现与 Ascend C 判题直接相关的上述平台案例，未纳入案例库。

---

## 7. 来源登记表

| # | URL | 标题 | 作者/日期 | 访问日期 | 用途 | 证据等级 | 证据状态 |
| --- | --- | --- | --- | --- | --- | --- | --- |
| S01 | https://www.hiascend.com/app-forum/topic-detail/0272153064709468009 | 昇腾AI原生创新算子挑战赛S1——CNASP坤坤爱曼巴队经验分享 | b1ankcat／2024-06-06 | 2026-09-12 | 提交前 build 检查教训（两题 0 分）、通用模板策略 | C | verified |
| S02 | https://blog.csdn.net/gitblog_00702/article/details/151517425 | CANN复旦赛算子实现（ClipByValue/Lerp/Addcmul） | 潘正浩（MrWine）／2026-06-07（原作 GitCode cann/cann-ops-competitions） | 2026-09-12 | WA 案例：绕过输出队列、公式改写、int32 中转；过度开核阈值 | C（转载；原仓 B） | verified |
| S03 | https://blog.csdn.net/gitblog_01418/article/details/150380401 | CANN学习中心：as_strided算子实战（CANNJudge 开放题库） | cann-learning-hub 实践记录／2026-05-20 | 2026-09-12 | CE（命名空间）、UB 192KB、标量黑洞、erf 4 次提交、踩坑全景 | C（转载；原仓 B） | verified |
| S04 | https://blog.csdn.net/ferriswym/article/details/162206787 | 【昇腾/AscendC开发】AscendC DataCopyPad 写出溢出 Bug 详解 | ferriswym／2026-06-22 | 2026-09-12 | 32B burst padding 溢出覆盖相邻段（尾块风险核心同构案例） | C | verified |
| S05 | https://github.com/hw-native-sys/simpler/issues/517 | [Bug] Build fails with CANN 8.5.1: BLK macro collision in tensor.h | Hzfengsy／2026-04-10 | 2026-09-12 | 宏冲突 CE 完整日志（V001 同类外部佐证） | C | verified |
| S06 | https://www.hiascend.com/app-forum/topic-detail/0279188908116457252 | 自定义算子npu测试报错 TriangularMode::LOWER（已解决） | xxxyyy0011／2025-07-26 | 2026-09-12 | LOWER 宏冲突；CPU 通过 NPU 失败 | C | verified |
| S07 | https://github.com/tile-ai/tilelang-ascend/issues/1195 | T.copy(gm[a:b,i], ub) 与预期不一致 | fengz72／2026-06-16 | 2026-09-12 | GM→UB 非连续搬运 pad 0（CANN 9.0.0） | C | verified |
| S08 | https://github.com/vllm-project/vllm-ascend/issues/1682 | gather_v3 断言 Index out of range | JackeyGuo／2025-07-09 | 2026-09-12 | 运行期越界断言样本 | C | verified |
| S09 | https://github.com/Just-it/AscendOpGenAgent/issues/83 | EmbeddingDenseBackward 越界索引导致精度测试不稳定 | zhshgmail／2026-04-14 | 2026-09-12 | 测试数据缺陷致"正确实现"误判 WA | C | verified |
| S10 | https://github.com/Just-it/AscendOpGenAgent/pull/132 | cumsum修复+修复提前退出（含 NPU 精度问题与 Workaround） | chopper0126／2026-04-27 | 2026-09-12 | fp16 并行扫描非确定性（0.1–0.5% 波动/大 tensor 10% 偏离） | C | verified |
| S11 | https://github.com/Just-it/AscendOpGenAgent/pull/149 | 对齐CPU标杆，优化cumsum算子精度 | jianhuang163／2026-04-29 | 2026-09-12 | 参考实现并行归约与串行 kernel 的偏差处理 | C | verified |
| S12 | https://blog.csdn.net/gitblog_00410/article/details/143789539 | CANN/cann-learning-hub：CANNJudge算子提交Skill | cann-learning-hub Skill 文档／2026-05-09 | 2026-09-12 | Histogram min/max=0 语义陷阱；泛化五要素；平台无测试用例 API | C（转载；原仓 B） | verified |
| S13 | https://blog.csdn.net/2301_80840905/article/details/155034892 | 避坑指南：Ascend C算子开发常见报错解析与精度优化复盘 | CSDN 作者／2025-11-19 | 2026-09-12 | PipeBarrier 缺失、FreeTensor 死锁、32B 对齐、Tiling 结构体错位、fp16 累加 | C | verified |
| S14 | https://www.mindspore.cn/tutorials/experts/zh-CN/r2.6.0rc1/debug/error_analysis/cann_error_cases.html | CANN常见错误分析（MindSpore 官方） | MindSpore 官方文档 | 2026-09-12 | EB0000 UB 超限、E80012 ReduceSum 维度（错误码日志样本） | A | verified |
| S15 | https://www.hiascend.cn/document/detail/zh/CANNCommunityEdition/800alpha002/apiref/ascendcopapi/atlasascendc_api_07_0215.html | GET_TILING_DATA_WITH_STRUCT（官方 API 参考） | 华为 CANN 官方文档／更新 2025-03-24 | 2026-09-12 | "暂不支持 kernel 直调工程"约束（A 级） | A | verified |
| S16 | https://www.hiascend.com/developer/blog/details/02192216483616286018 | Ascend C 算子开发典型问题：Segmentation fault 根因分析 | 昇腾社区博客（匿名）／2026-06-10 | 2026-09-12 | 段错误五大类分类学（主机/GM/LM/DMA/指令对齐） | C | partial |
| S17 | https://blog.csdn.net/gitblog_00561/article/details/145252761 | CANN/asc-tools 同步事件管理完整指南 | asc-tools 文档转载／2026-05-30 | 2026-09-12 | ErrorSync1–4 分类；npuchk 排查流程；cpu_debug 工具 | C（转载；原仓 B） | verified |
| S18 | https://blog.csdn.net/gitblog_00127/article/details/151604873 | CANN Erf算子优化作品（东南大学，江山赛区预选赛） | 关注塔菲喵队／2026-07-02（原作 cann/cann-ops-competitions） | 2026-09-12 | Direct/Medium/Large 分路径 15/15 Pass 正面样板 | C（转载；原仓 B） | verified |
| S19 | https://jsj.nwpu.edu.cn/info/1598/23245.htm | 从小白到大赛金奖，西工大学子勇闯算子开发探索之路 | 西工大计算机学院／2024-10-12 | 2026-09-12 | S2 金奖经验（结论性） | C | verified |
| S20 | https://we.yesky.com/blog/326258 | 自学1个月拿金奖！北交大学子分享昇腾算子挑战赛秘籍 | 天极网／2024-10-14 | 2026-09-12 | S2 策略：基础解法→性能优化两阶段；实时榜单 | C | verified |
| S21 | https://blog.csdn.net/pz890123/article/details/152697389 | 2025昇腾训练营避坑指南 | CSDN 作者／2026-03-02 | 2026-09-12 | __aicore__ 未定义等入门 CE（结论性） | C | partial |
| S22 | https://www.hiascend.com/app-forum/topic-detail/0269202456643546167 | 【CANN训练营】Erf算子Ascend C开发 | callmedayao-2022／2026-01-21 | 2026-09-12 | 满核原则/UB 充分利用/tilingKey 定制（正面方法论） | C | verified |
| S23 | https://blog.csdn.net/gitblog_00619/article/details/141845484 | CANN/cannbot-skills：rms_norm.asc 直调转自定义算子示例 | cannbot-skills 文档／2026-05-09 | 2026-09-12 | RMSNorm 直调源码形态（3 kernel 类、BF16→FP32 范式） | C（转载；原仓 B） | verified |
| S24 | https://blog.csdn.net/ferriswym/article/details/162241341 | 直调模式 VS 算子框架模式：入口点选择指南 | ferriswym／2026-06-23 | 2026-09-12 | 直调不支持 MIX（hang）等能力边界 | C | verified |
| S25 | https://github.com/mouliangyu/PTOAS/issues/380 | run_ci.sh fails in CANN 9.0.0 env | learning-chip／2026-05-19 | 2026-09-12 | CANN 9.0.0 bisheng 后端崩溃日志 | C | verified |
| S26 | https://blog.csdn.net/gitblog_07213/article/details/151463322 | CANN竞赛作品提交规范 | cann-competitions 转载／2026-05-19 | 2026-09-12 | 作品目录/提交流程规范（复赛阶段参照） | C（转载；原仓 B） | verified |
| L01 | 提交/V001/结果.md（本地） | V001 平台结果（15/15 CE） | 本项目／2026-09 | 2026-09-12 | 本地失败案例 1 | 内部文档 | 已确认 |
| L02 | 提交/V002/结果.md、提交/版本实验记录.md、提交/README.md（本地） | V002 上传异常记录与版本实验 | 本项目／2026-09-11 | 2026-09-12 | 本地失败案例 2 | 内部文档 | 已确认（根因未定论） |

---

## 8. 验收对照

| 验收项 | 要求 | 实际 |
| --- | --- | --- |
| 外部真实案例（含可核查链接） | ≥8 | 22 条（S01–S26，全部附 URL） |
| 本地案例复盘 | 2（V001/V002） | 第 2 节 L-01/L-02，均含"现象→根因→预防措施" |
| 案例结构化 | 每条含现象→根因→预防 | 第 3 节逐条给出，第 5 节汇总为 R1–R11 风险清单 |
| 结论性资料隔离 | 无日志经验贴标 C 级 | 第 4.2 节单列，第 3.7 节表格逐条标注 |
| 范围纪律 | 不研究题面细节、不提交、不修改本地文件、不建多余文件 | 仅新增本报告一个文件 |

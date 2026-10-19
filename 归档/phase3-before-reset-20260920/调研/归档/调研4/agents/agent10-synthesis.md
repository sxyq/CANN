# Agent 10 综合裁决报告：证据审阅、矛盾裁决、反例验证、方案矩阵合并

- 角色：Agent 10（证据审阅 / 矛盾裁决 / 反例验证 / 方案矩阵合并）
- 题目：`AddRmsNormBias`（2026 CANN 挑战赛·西南赛区初赛，CANN 9.0.0，vector，`kernel.asc` 直调）
- 工作机：macOS，**无 CANN 工具链、无昇腾 NPU**。所有结论均为资料核对 / 源码阅读 / CPU 参考计算 / 静态分析，**未声明任何 NPU 编译、运行或精度/性能验证**。未执行 CANNJudge 上传，未消耗提交次数。
- 证据分级（全文统一）：A=官方题面/API/仓库源码；B=官方样例/测试/培训；C=社区文章/论坛/个人仓库；D=仅搜索摘要未核验。

---

## 1. 裁决表（第三部分 11 条矛盾）

> 每条：矛盾描述 / 裁决 / 依据来源与等级 / 是否定案 / 剩余风险

### 1. `DataCopyPadExtParams<T>` 字段顺序
- **矛盾**：旧笔记 `{isPad, paddingValue, leftPadding, rightPadding}` vs Agent02 据 CANN 9.0.0 官方表 `{isPad, leftPadding, rightPadding, paddingValue}`。
- **裁决**：采纳官方顺序 `{isPad, leftPadding, rightPadding, paddingValue}`（A 级）。**V002 用逐字段赋值**（`pad.isPad=...; pad.paddingValue=...; pad.leftPadding=...; pad.rightPadding=...`），对该顺序歧义天然免疫；`源码/op_kernel/add_rms_norm_bias.cpp` 第 115 行聚合初始化 `{true,0,0,0}` 在两种顺序下均为 `isPad=true`+其余全 0，亦安全。
- **依据**：Agent02 §争议点1（A，CANN 9.0.0 文档表 0265）。
- **是否定案**：**是**（字段顺序本身定案；V002 写法免疫）。
- **剩余风险**：极低。真机验证：查 `include/ascendc/.../tensor_api.h` 结构体声明，或跑非全零 `leftPadding/rightPadding` 用例比对。

### 2. UB→GM 搬出 padding 是否污染相邻内存
- **矛盾**：Agent02 称官方 A 级文档「dummy 自动丢弃，不写相邻内存」；旧笔记/社区称 padding 真实写回、覆盖相邻行，列为「真机第一验证项」。
- **裁决**：采官方口径（A 级，文档 0265 原文）——搬出时 UB 侧补的 dummy 在映射到 GM 时被丢弃，**不污染相邻内存**。但此为**静默数据损坏型**风险（不崩溃、难发现），且依赖判题机 CANN 版本/SoC 行为与文档一致。
- **依据**：Agent02 §争议点2（A）；社区相反实测为 C 级（Agent02 判定为未检索到可靠实证）。
- **是否定案**：**官方口径已定案（A）**，但**真机未验证，风险残留**。
- **剩余风险**：**高（静默损坏）**。真机验证：构造 D 非 32B 对齐尾块 + 紧邻有效数据的 GM 缓冲，用非对齐 `DataCopyPad` 搬出后读相邻内存是否被改写。详见 §5 缺口 G5（P0）。

### 3. V001 平台编译失败根因
- **矛盾**：项目记「`TPipe pipe_` 宏命名冲突」；Agent08 推测「`TPipe` 在 9.0.0 作用域可见性问题」。
- **裁决**：V001 根因确实是 `pipe_` 与 9.0.0 头文件中的宏/类型冲突（报错 `unknown type name 'pipe_'` 表明编译器把 `pipe_` 当类型名）；改名 `tpipe` 同时覆盖两种可能。**关键纠正**：V002 平台失败已被诊断为**上传内容截断（首行 `return false;`）**，V002 的 405 行代码**从未被真正编译过**，「改名修复了编译」**没有证据**。
- **依据**：Agent08 §8（B，环境/版本错配高概率根因）；Agent09 案例1（L，本地真实记录）；`提交/V002/结果.md`（L，确认首行截断）。
- **是否定案**：V001 根因**定案（命名冲突）**；V002 修复**未定案（无编译证据）**。
- **剩余风险**：**首个真机动作**：在 CANNJudge 编辑器中全选替换为本地完整 405 行 `V002/kernel.asc` 重新提交，观察 15 点是否进入运行期；若仍 `Compile Error` 才是真正的代码编译问题。

### 4. `ReduceSum` 的 count 上限与 `workLocal` 尺寸
- **矛盾**：官方仅约束「≤UB」；社区/项目给 255 / 4096 / 16320 等口径。
- **裁决与核算**：V002 `work_` 为 `WORK_LEN=1024` 个 float；fp32 tile 下 `calc_len`≤2048，fp16/bf16 下 `calc_len`≤4096（以 fp32 计数）。按社区推导公式 `RoundUp(firstMaxRepeat, 32/typeSize) * (32/typeSize)`（float 时 `firstMaxRepeat = count/64`，`32/typeSize=8`）：
  - count=2048 → `RoundUp(32,8)*8 = 256`
  - count=4096 → `RoundUp(64,8)*8 = 512`
  - 二者均 **< 1024**，故 **`WORK_LEN=1024` 足够**，社区 255/4096/16320 是不同口径（repeat 上限 / 保守安全值 / 255×64 fp32 最大 count），**不直接指向 1024 不足**。
- **依据**：Agent02 §争议点3（A 官方约束 + C/D 社区口径）；本表算式（D 级推导）。
- **是否定案**：**公式推导定案（D 级）**，但官方无显式公式，仍建议真机验证。
- **剩余风险**：低–中。真机验证：A2 上 fp16/bf16、D=4096 单块 `ReduceSum(count=4096, fp32)` 实测正确性与 `work` 缓冲需求。

### 5. 判题 SoC
- **矛盾**：Agent01 抓到题面 API `soc_version: null`、`ddk_version: null`（平台未指定）；项目文档假设 A2 系 / `dav-2201`。
- **裁决**：`dav-2201` 仅为模板 `CMakeLists.txt` 的**编译默认**，非平台约束。若判题机非 A2（910_93/A3 仍映射到 `dav-2201` 安全；但 950/A5 需 `dav-3510`），受影响 API 包括：A2 的 `Add/Mul` bf16 支持矩阵、`ReduceSum` 行为、UB 容量（192KB vs 256KB）、向量核数（`GetCoreNumAiv` 48 vs 40）、`DataCopyPad` 支持范围。
- **依据**：Agent01 §1.10/§6 差异清单（A，题面 API 实测 `soc_version:null`）；Agent08 §4（A，`dav-*` 映射表）。
- **是否定案**：**未定案（证据薄弱，等级 D/⑤）**。
- **剩余风险**：**高（影响多 API 行为）**。必须向组委会/平台确认 SoC；详见 §5 缺口 G1（P0）。

### 6. 入口限定符
- **矛盾**：Agent02 称 `__vector__` 与 `__aicore__` 均合法；项目文档称「纯向量必须 `__vector__`」。
- **裁决**：Agent02 正确。官方修饰符表（A 级）确认 `__vector__` 合法且为直调纯 Vector 核推荐；`__aicore__` 为通用 AI Core 入口（框架模式）。V002 `__global__ __vector__` 合法。
- **依据**：Agent02 §争议点6（A，CANN 8.5.0a002 修饰符表，概念沿用 9.0.0）。
- **是否定案**：**是**。
- **剩余风险**：无（与 SoC 判定无关，修饰符语义稳定）。

### 7. 能否用 `GetReduceRepeatSumSpr`
- **矛盾**：项目文档称其在 9.0.0 有专页；主线用 `ReduceSum + V_S + GetValue(0)`。
- **裁决**：**不需要**。`GetReduceRepeatSumSpr` 为高级变体；基础 `ReduceSum(count)` + `GetValue(0)` 已足够且实现更简单。仅当 `ReduceSum` 内部标量同步成为性能瓶颈时，才作为性能优化候选考虑。
- **依据**：Agent02 §1.9-1.10（A）；Agent07 §6.2（社区实测 ReduceSum 内部含标量同步）。
- **是否定案**：**是（不需要）**。
- **剩余风险**：无。

### 8. BF16 精度判定口径
- **矛盾**：Agent06 实测 bf16 vs 同 dtype golden 失配 0.012%/0.009%（<0.1% tol）通过，但 vs FP64 真值最差单元素相对误差达 16%；主代理 `divpath` 发现 V002 `Muls(y,1/rms)` 与 golden `y/rms` 走不同舍入，bf16 上 1~3 元素差 1~3 ulp（max rel 7.4e-3、abs 9.8e-4）。
- **裁决**：主代理发现**可靠**（纯浮点事实：`y*(1/rms)` ≠ `y/rms` 至多 1 ulp，量化到 bf16 可能在分界翻转）。改成「先除」（`Divs(y, rms)`，与 golden 同链）**确实消除差异**，且为零成本正确性改进（见 §4 独立复现）。**这不改变『isclose+tol 口径下通过』的结论，但降低严格逐元素口径下的失败风险**。
- **依据**：Agent06 §1/§3（A/B）；主代理 `kernel_sim_v002_divpath.py`；本文件 §4 独立复现。
- **是否定案**：**是（发现可靠，建议采纳 Divs）**。
- **剩余风险**：真正的判定口径（是否允许少量元素失配）仍未知 → P0，见 §5 缺口 G2。

### 9. fp32 支持
- **矛盾**：Agent03 称官方 ops-transformer 的 SPLIT_D 路径仅支持 fp16/bf16，fp32 疑似不支持。
- **裁决**：这指**官方仓库现成 SPLIT_D kernel 缺 fp32 分支**，并不意味 Ascend C **不能**做 fp32。本题自研 fp32 完全可行（`ReduceSum` 支持 fp32、`Add/Mul` 支持 float）。仅需自行补 fp32 tile（2048）分支——V002 已做。结论：**不能照抄官方 SPLIT_D 的 fp32 路径（不存在），但自研无技术障碍**。
- **依据**：Agent03 §1.7/§7（A，官方仓库源码）；V002 `TILE_FLOAT=2048`（本地 L）。
- **是否定案**：**是（官方 SPLIT_D 缺 fp32；自研可支持）**。
- **剩余风险**：低（自研已覆盖）。

### 10. 并行核数
- **矛盾**：Agent07 主张 `GetCoreNumAiv()`（910B2=48）；模板 `main.asc` 传 `aclrtGetDeviceInfo(deviceId, ACL_DEV_ATTR_VECTOR_CORE_NUM, &availableCoreNum)`。
- **裁决**：**两者一致**——`ACL_DEV_ATTR_VECTOR_CORE_NUM` 返回的就是向量核数（等于 `GetCoreNumAiv()`）。V002 用 `availableCoreNum` 作为 `blocks` 合理。V002 还做了 `blocks=min(availableCoreNum, outer)` 的钳制，应对 outer 小于核数的情形，合理。
- **依据**：Agent07 §1.2/§2 事实表（A/B）；模板 `main.asc:42`（L）；V002 `run_kernel` 第 410-413 行（L）。
- **是否定案**：**是（一致）**。
- **剩余风险**：取决于 SoC（非 910B2 时核数不同，已随 §5 的 SoC 风险传导）。

### 11. `DataCopyPad` / `DataCopyExtParams` 参数单位
- **矛盾**：`DataCopyExtParams` 各字段单位（byte 还是元素/dataBlock）；V002 用 `params{1, len*sizeof(T), 0, 0, 0}` 是否正确。
- **裁决**：`DataCopyExtParams` 标准语义为 `{blockCount, blockLen(字节), srcStride, dstStride, rsv}`，`blockLen` 为字节。V002 用 `blockCount=1` + 全 0 stride，**无论 stride 单位（字节还是 32B dataBlock）都不影响**（单块、块间间隔=0），且 `blockLen=len*sizeof(T)` 字节 = 单块连续 `len` 元素的字节数，正确。
- **依据**：Agent02 §3 事实表（A，文档 0265）；Agent09 案例14（C，指出 srcStride/dstStride 单位差异——但仅影响 blockCount>1）。
- **是否定案**：**是（V002 参数正确且对该单位歧义免疫）**。
- **剩余风险**：深层「stride 单位」文档歧义（Agent09 引掘金称 src=字节/dst=dataBlock）仍属社区级（C），但**不影响本题**（V002 未用多块带 stride）。

---

## 2. 反例表（第四部分，S1–S14 每方案至少 1 条）

> 等级同全局分级。反例/失败条件按「触发条件 / 后果 / 证据」组织。

| 方案 | 反例或失败条件 | 触发条件 | 后果 | 证据 | 等级 |
| --- | --- | --- | --- | --- | --- |
| **S1 两遍扫描** | 访存翻倍：每行重读 x 与 residual 一次，GM 流量 7·D·S vs 单遍 5·D·S（1.4×） | 大 outer、访存受限形态 | 性能损失 ~28.6% GM 流量 | Agent07 §4（A 吞吐） | A |
| **S2 单遍暂存** | UB 占用增大：整行 `value`(fp32)=D×4；D=32768 fp32 整行 128KB + 5 队列易逼近 192KB 上限（双缓冲更甚） | D 大 + 双缓冲 | `InitBuffer` 报 UB 超限 / 编译失败 | Agent07 §3.2（B） | B |
| **S3 FP32 全中间** | 假定 S3 自动安全而忽略 `sqrtf` 被 fast-math 改写为低精度倒数 | 编译器开启 fast-math | 误差可达 ~1e-3，威胁 bf16 判定 | Agent06 R3（A 标准/风险） | A/D |
| **S4 低精度中间** | `|y|≥256` 时 fp16 平方直接 `Inf` 污染整行 | 任何含 `|y|>255.94` 的 fp16 输入 | 整行 NaN/Inf，判题失败 | Agent06 Q5（A 格式定义）；Agent09 案例8（C） | A |
| **S5 大 tile** | tile 过大 → UB 不够（单 tile value + 5 队列超 192KB）或 ReduceSum count 超 work | TILE>4096 且 fp32 / D=32768 整行 | UB 超限，编译/运行失败 | Agent07 §3.1（B） | B |
| **S6 小 tile** | tile 过小（如 64）→ 每行 tile 数多 → ReduceSum+GetValue 标量同步被放大 | 大 outer + 小 tile | 标量停顿占比高，性能下降 | Agent07 §6.3（C 高可信） | C |
| **S7 按行分配** | outer < 核数时（如 outer=1, D=32768）仅 1 核工作，其余空闲 | outer 极小、D 极大 | 大 D 单核串行，超时风险 | Agent07 形态A（A/B） | A/B |
| **S8 按 tile 分配** | 跨核协作同一行需跨核归约（WholeReduceSum），大幅复杂化且引入同步/正确性风险 | 单行跨多核 | 实现复杂易错，跨核开销可能 > 收益 | Agent03（A 高阶 ReduceSum） | A |
| **S9 DataCopyPad 尾块** | UB→GM dummy 若不丢弃 → 覆盖相邻行（静默损坏） | 判题机行为与文档不符 | 相邻行数据被破坏，难发现 | 社区案例12（C）；官方 A 称丢弃（Agent02） | C/A（争议） |
| **S10 手工尾块** | 手工掩码/Duplicate/GatherMask 边界算错（mask 长度/偏移错误） | D 非 32 倍数且掩码逻辑 bug | 尾块越界/值错 | Agent09 案例15（C） | C |
| **S11 ReduceSum** | 基础 ReduceSum 内部含标量同步（阻塞流水）；count 超 work（已核算安全） | 小 outer 大 D、每行多 tile | 标量停顿占比高 | Agent07 §6.2（C 高可信） | C |
| **S12 手工向量归约** | 手写归约顺序与 golden 不同 → bf16 边界翻转 1~3 ulp（同矛盾#8）；手写掩码易错 | 严格逐元素判定 / 掩码 bug | 失配风险 + 正确性问题 | Agent06 §3；Agent09 | A/C |
| **S13 纯 Ascend C** | 误用官方框架工程（msopgen/`__aicore__` 多文件）当提交物 | 提交形态错误 | 判题无法识别 `run_kernel` 入口 | Agent03 §6（A）；Agent05（A） | A |
| **S14 GPU 迁移** | CUDA/Triton 代码无法在 NPU 编译；warp shuffle/occupancy/L2 等概念在 Ascend 无对应 | 直接移植 GPU 代码 | 不可编译/语义错误 | Agent04 §4-5（C 逻辑确凿） | C |

---

## 3. 误用清单

### 3.1 只适用于 GPU 或其他 Ascend 型号、却被当作 A2 可用的条目
1. **GPU occupancy / L2 命中率 / warp divergence 调优直觉**（Agent04 §5 列了 8 条，已自声明无对应）——若被套到 Ascend 调优属无依据。等级：C（Agent04 已警示）。
2. **310P/310B 的 UB=256KB** vs A2 192KB（Agent07 §2，B 级）——若把 310P 调优（更大 UB 假设）用于 A2，会 UB 超限。等级：B。
3. **950/A5 用 `dav-3510`**——若判题机为 950 而代码按 A2(`dav-2201`) 编译，指令集不兼容（Agent08 §4，A 级）。等级：A。
4. **ROCm AITER `rmsnorm2d_fwd_with_add`**（D≥1024 精度略降、仅 2D）——当本题多 rank/宽 D 参考时可能误导（Agent04 §4 列表7，A/B）。等级：A/B。
5. **GPU 向量化加载 `float4/half8` 与 `ptr%16` 对齐判断**——Ascend 128-bit 向量搬运对齐判断不同，D 非 16 倍数时尾部处理不可直接套（Agent04 §2 行6，C）。等级：C。

### 3.2 被当成官方结论的社区推断（条目 + 正确等级）
1. **「padding 写回污染相邻内存」**——被旧笔记当确证（C），实为官方文档称丢弃（A）。**等级纠错：C→A（反向）**。
2. **「ReduceSum count 上限=4096/255/16320」**——被当硬上限（C/D），实为社区估算，官方仅约束 ≤UB。
3. **「`dav-2201`=A2 平台要求」**——被当官方约束（项目文档），实为模板编译默认，`soc_version:null`（Agent01，A/⑤）。
4. **「`CAST_RND` 是 9.0.0 枚举」**——实为 `CAST_ROUND`/`CAST_RINT`，旧别名（Agent02 §6，A）。
5. **「rsqrt 误差 2^-20」**——被当官方精度，实为社区估计（C）；官方「0 ULP（限定区间）」口径不同（Agent02 §争议点5，A/C）。
6. **「`TPipe` 在 9.0.0 改名/作用域问题」**——被当 V001 根因（Agent08 推测），实为 `pipe_` 命名冲突（V001），且与 V002 无关（V002 是上传截断）。

---

## 4. 独立数值复核实测结果

- **脚本路径**：`/Users/sunyiyang/Desktop/Project/cann/调研/调研2/数值参考脚本/agent10_verify.py`
- **运行命令**：`/usr/bin/python3 agent10_verify.py`
- **环境**：`/usr/bin/python3`（3.9.6）、numpy 2.0.2、ml_dtypes 0.5.4（与主代理同环境，但**不同种子/形状/写法**）
- **声明**：全部为 CPU/numpy 浮点模拟，**非 NPU 实测**；numpy 归约顺序/无 FMA 与昇腾硬件不完全一致，仅验证算法结构与浮点舍入路径。

### 发现 1：V002 算法结构 vs golden 失配率
- 主代理：21 组用例，失配率 0.0000%（`kernel_sim_v002.py`）。
- 本独立复现：13 用例 × 3 种子 = **39 组**（不同 seed 4242/9001/31337，不同形状 incl. (2,3,4,D) 4D、D∈{97,1001,7777,16384,32768}），最差失配率 **0.0000%**，全部 PASS。
- **结论**：复现主代理「0% 失配」结论，**一致**。

### 发现 2：分块累加 vs 整体累加的平方和相对差
- 主代理：≤ **8.9e-8**（`kernel_sim_v002_stress.py`）。
- 本独立复现（用朴素从左到右标量累加模拟硬件归约，对照 numpy pairwise）：worst = **8.6e-6**（fp16/bf16，D=32768）、fp32 ~9e-8。
- **差异说明（如实写）**：本复现 worst 8.6e-6 **略高于**主代理 8.9e-8，原因是本脚本用「朴素顺序累加」建模硬件标量归约（精度低于 numpy pairwise），而归约本身差仍 **≤1e-5**，远低于 fp32 预算 1e-4。两者均确认「分块累加误差可忽略」，结论**一致**。

### 发现 3：除法路径差异 + 改成「先除」是否消除
- 主代理：V002 `Muls(y,1/rms)` vs golden `y/rms`，bf16 上 1~3 元素差 1~3 ulp（max rel 7.41e-3）；改 Divs 预期降到 0。
- 本独立复现：
  - `Muls`（先乘倒数）路径 vs golden：bf16 最多 **1** 元素逐位不等（量级 1 ulp），fp16 最多 **6** 元素；均 isclose 失配 0。
  - 改 `Divs(y, rms)`（先除，与 golden 同链）后：**逐位不等数=0，且与 golden 按字节视图逐位一致（`bit_eq=True`）**。
- **结论**：**复现并确认**主代理发现——`Muls` 路径确实产生 1~3 ulp 量化分界翻转，**改成先除（Divs）后差异降至 0**，且为零成本改进。**主代理结论可靠**。

---

## 5. 证据缺口清单（≥5 条）

| # | 缺什么 | 去哪补 | 怎么验 | Blocker 级别 |
| --- | --- | --- | --- | --- |
| G1 | 判题 SoC 具体型号 | 平台 API `soc_version`（当前 null）/ 组委会 | 登录提交后看判题回传或问组委会 | **P0（B4）** |
| G2 | 判题精度判定口径（是否允许少量元素失配 / tol 取值） | 平台 `verify_result.py` 语义 / 提交后结果页误差列 | 提交一版观察 PASS/FAIL 反推 | **P0** |
| G3 | `ReduceSum` 官方 `workLocal` 尺寸公式 | CANN 9.0.0 头文件 `kernel_operator_vec_reduce_intf.h` | 真机 fp16/bf16 D=4096 单块 ReduceSum 实测 | B / D |
| G4 | `CAST_RINT` 是否 = RNE | CANN 文档 / 真机构造量化分界样本 | 真机核对舍入方向 | R5 / P1 |
| G5 | UB→GM `DataCopyPad` dummy 是否真丢弃 | 真机构造相邻缓冲 | 非对齐尾块搬出后读相邻内存 | **P0（静默损坏）** |
| G6 | 15 测试点具体 shape/dtype/epsilon 配置 | 平台未公开（`visible=0`） | 提交后反推 | B1 |
| G7 | `sqrtf` 在 NPU 映射指令 / fast-math 改写 | 真机 / 编译产物 | 真机比对 sqrt 精度 | R3 |

> Blocker 分级：P0=判题前置（不确认则无法保证 15 点全过）；B=资料缺口；R=精度风险。

---

## 6. 统一方案矩阵

> 固定列同 `方案矩阵.md`。每个单元格结论均带证据等级（A/B/C/D）与来源（代理编号 § 或来源编号）。S1–S14 全部覆盖，另含 2 个性能子变体（S12b、S5b）。

| 方案 | 算法 | 访存次数 | UB 占用 | 精度风险 | 尾块风险 | 性能潜力 | 实现难度 | CANN 兼容风险 | 推荐级别 |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| **S1 两遍扫描** | Pass1 求平方和；Pass2 重读归一化+γ+β | 7·D·S/行（A，Agent07§4） | 低（~108KB fp16，A/B Agent07§3） | 低（S3 保障） | 低（S9 覆盖） | 中（比单遍慢 1.4×） | 低 | 低 | **可作为第二路线** |
| **S2 单遍暂存** | 一次读 x/res，驻留 y 到 UB 再归一化 | 5·D·S/行（A，Agent07§4） | 中（整行 value 128KB+D=32768，B Agent07§3.2） | 低 | 低 | 高（−28.6% GM） | 中（UB 规划） | 低 | **首选** |
| **S3 FP32 全中间** | 全程 FP32，末次 cast 输出 | 同 S1/S2 | 同 | 极低（消除 fp16 溢出/bf16 坍塌，A Agent06Q5/R1） | 低 | — | 低 | 低 | **首选** |
| **S4 低精度中间** | 16-bit 域累加（仅对照） | — | — | **极高**（|y|≥256 Inf 污染，A Agent06Q5） | — | — | — | 低 | **不建议** |
| **S5 大 tile** | tile 尽量大减同步 | 同 | 高（UB 压力，B Agent07§3.1） | 低 | 低 | 高（少同步） | 低 | 低 | **可作为第二路线** |
| **S6 小 tile** | tile 小，UB 占用低 | 同 | 低 | 低 | 低 | 低（同步放大，C Agent07§6.3） | 低 | 低 | **仅作研究参考** |
| **S7 按行分配** | 以行为单位分核，核内多行 | — | — | 低 | 低 | 高（标准） | 低 | 低 | **首选** |
| **S8 按 tile 分配** | 跨核协作同一行（需跨核归约） | — | — | 中（跨核归约顺序，A Agent03） | 低 | 中（复杂度抵消收益） | **高** | 中 | **不建议** |
| **S9 DataCopyPad 尾块** | 用 DataCopyPad 处理非 32B 对齐尾块 | — | — | 低 | **中（dummy 丢弃未真机验，A Agent02/proj 争议）** | — | 低 | 低 | **首选** |
| **S10 手工尾块** | 对齐块+掩码/Duplicate/GatherMask | — | — | 低 | 中（掩码易错，C Agent09） | — | 中 | 低 | **可作为第二路线** |
| **S11 ReduceSum** | 官方 `ReduceSum`（基础 count 版）+ GetValue(0) | — | 中（work=1024 足够，D 级 §1.4） | 低（顺序差 ≤1e-5，A Agent06Q2） | 低 | 中（含标量同步，C Agent07§6.2） | 低 | 低 | **首选** |
| **S12 手工向量归约** | WholeReduceSum/BlockReduceSum 组合 | — | 中 | 中（顺序差 1~3 ulp，A Agent06） | 中 | 高（去标量同步） | **高** | 中 | **可作为第二路线** |
| **S13 纯 Ascend C** | 全部逻辑手写于 `kernel.asc` | — | — | 低 | 低 | — | 中 | 低（唯一合规形态） | **首选** |
| **S14 GPU 迁移** | 借鉴 GPU 算法结构 | — | — | — | — | — | — | **极高（不可编译）** | **仅作研究参考** |

> 子变体（并入对应方案，不单列编号以防与 S1–S14 冲突）：
> - **S12b 跨核归约（WholeReduceSum 跨 block）**：性能优化候选，但本题每行独立、无需跨核，风险/收益不成正比 → 不建议优先。
> - **S5b 大 tile + 双缓冲（BUFFER_NUM=2）**：S5 的增强，性能潜力最高，但 UB 预算需重算 → 第二候选路线内采用。

---

## 7. 推荐路线

### 首版路线（先确保 15 点全过，再谈性能）
- **S13（纯 Ascend C）+ S1（两遍扫描，即 V002 现有结构）+ S3（FP32 全中间）+ S7（按行分配）+ S9/S11（DataCopyPad 尾块 + ReduceSum）+ S13 合规形态**。
- **首版必做修正**：把 V002 的 `Muls(y, 1/rms)` 改为「先除」（`Divs(y, rms)` 或 `Mul` 后 `Div`），消除 bf16 1~3 ulp 差异（§4 已独立验证可降至 0，零成本）。
- 理由：V002 已是正确算法结构（两遍+FP32+分块 ReduceSum+CAST_RINT），仅需先验证其能在真机编译并 15 点全过；先正确后性能（Agent09 案例16/19 强调）。

### 第二候选路线（性能优化，建立在 15 点全过后）
- **S2（单遍暂存）+ S5（大 tile）+ BUFFER_NUM=2 双缓冲 + 消除 GetValue（向量 Sqrt+Reciprocal 广播）+ S9/S10 尾块**。将 GM 流量从 7·D·S 降到 5·D·S（−28.6%），并重叠 MTE/Vector 流水。
- 理由：Agent07 §4/§5 的访存与流水分析，性能潜力最高且 UB 在 D≤32768 内可行（B 级）。

### 性能优化候选路线（按优先级排序，均在 15 点全过后叠加）
1. **BUFFER_NUM=2 双缓冲**（流水重叠，高收益低风险，Agent07 §5）。
2. **单遍暂存 S2**（−28.6% GM 流量，Agent07 §4）。
3. **消除 GetValue 标量读回**（小 D 形态用向量 Sqrt+Reciprocal 广播；大 D 用 BlockReduceSum+WholeReduceSum 组合，Agent07 §6）。
4. **大 tile S5**（减少 ReduceSum+同步次数，受 UB 预算约束）。
5. **D 满足 32B 对齐时 `DataCopy` 替代 `DataCopyPad`**（社区经验，C 级，需真机验证，Agent07 §4.3）。
6. **多 rank 展平 + 行内切 tile**，最大化每核行数（S7）。

---

## 8. 未确认事项汇总
1. **判题 SoC 型号**（§1.5 / G1）：`soc_version:null`，模板默认 `dav-2201` 非平台约束，影响多 API 行为，须组委会确认（P0）。
2. **判题精度判定口径**（§1.8 / G2）：是否允许少量元素失配、`tol` 取值未知，决定 bf16「先除」修正是否必要（P0）。
3. **UB→GM padding 是否真丢弃**（§1.2 / G5）：官方 A 级称丢弃，但静默损坏风险，须真机验证（P0）。
4. **`ReduceSum` workLocal 官方公式**（§1.4 / G3）：按社区公式 1024 足够，但官方无显式公式，fp16/bf16 大 tile 仍建议真机验。
5. **`CAST_RINT` 是否 = RNE**（G4）：影响 bf16 末级舍入，须真机核对。
6. **15 测试点 shape/dtype/epsilon**（G6）：平台未公开，测试矩阵只能按边界枚举推测。
7. **`sqrtf` 指令映射 / fast-math**（G7）：标量 sqrt 行为须真机确认。

---

> 全文简体中文。所有 NPU 侧结论均标注「未在真实 CANN/NPU 验证」。未发现任何文件被创建/修改于 `提交/V00N/` 与 `文档/`。未执行 CANNJudge 上传。

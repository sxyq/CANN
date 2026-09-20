# AddRmsNormBias · 真机验证边界与宿主侧核对笔记（调研2）

> 生成：2026-09-12。本文件汇总本轮调研中"没有真实 CANN/昇腾 NPU 时能做什么、不能做什么"，以及真机到位后的复核清单。
> 环境事实：本机为 macOS，无 CANN 工具链、无昇腾 NPU。判题环境：CANN 9.0.0 + SoC dav-2201（Atlas A2/910B 系，`__CCE_AICORE__==220`）+ vector 核函数直调单文件 `kernel.asc`。

## 一、本轮已完成的非真机验证（口径与证据）

| 项 | 结论 | 手段 | 证据等级 | 出处 |
| --- | --- | --- | --- | --- |
| 题面/接口/规则核对 | 15 测试点、fp32<1e-4、fp16/bf16<1e-3、得分公式、直调签名、dtype 枚举 0/1/2 | CANNJudge 公开页面 + 公开只读 GET API + 前端 JS 逆向 + 本地模板逐字段核对 | A | agent-01 |
| Ascend C API 逐项核对 | 27+3 个 API 签名/支持矩阵/约束；三项定案维持（DataCopyPadExtParams 字段顺序、UB→GM dummy 丢弃、ReduceSum 无 count 硬上限） | 本地缓存官方 9.0.X 页面（正文解码）+ gitcode 官方镜像交叉 | A | agent-02 |
| 官方参考实现源码审阅 | ops-nn add_rms_norm 五种 tiling 模式、cann-samples 直调样例、op-plugin 桥接 | GitHub/GitCode 官方镜像源码全文读取 | A/B | agent-03 |
| GPU 迁移可行性 | 7 个源码级参考实现的机制映射表（warp 归约/shared memory/mask 尾块/双写回等） | 源码静态分析，未编译 | B | agent-04 |
| CPU 参考数值实验 | FP32 累加必须、epsilon 位置红线、bf16 必须 CAST_RINT、NaN/Inf 行为表、测试矩阵 | numpy/ml_dtypes/torch CPU 实验（`/tmp/agent06/exp*.py`） | CPU 参考，不代表 NPU | agent-06 |
| 硬件数字与成本模型 | A2 UB=192KB/48 bank、AIV 40~48、向量吞吐表、非对齐 -21.6%、归约指令成本 | 官方文档/官方样例实测/社区实测，逐条标注 | A/B/C | agent-07 |
| 环境链路与报错案例 | 12 步真机链路、13 个报错案例、Docker/远程 NPU 方案 | 公开网页 + 本地模板只读核对 | A/B/C | agent-08 |
| 失败案例库 | 本地 V001/V002 复盘 + 22 个外部案例（CE/WA/RE/TLE/上传/泛化） | 本地文件 + 公开 Issue/论坛 | 混合 | agent-09 |

**本轮明确没有做的事**：没有执行任何 CANNJudge 上传或提交；没有真实 CANN 编译；没有 NPU 精度或性能测量；没有修改 `源码/`、`提交/`、`文档/` 下任何工程文件（全部只读）。

## 二、CPU 参考实验的适用边界（agent-06）

CPU 实验（numpy RNE cast 语义、IEEE 运算）能证明的：**数学链路**上"FP32 全中间 + CAST_RINT 输出 + epsilon 位于 sqrt(mean+eps)"与本地模板 golden 完全一致（E10 失配率 0）。

CPU 实验不能外推的：
1. A2 上 `Sqrt`/`Rsqrt`/标量除的真实 ulp（官方未见数字承诺；CUDA 同类约 2 ulp，本题容差余量约两个数量级，但需真机确认）。
2. A2 `Cast` fp32→bf16 的 CAST_RINT 是否严格 RNE（含 tie 与饱和行为）——bf16 输出的生死项（RNE 0% vs TRUNC ~40% 失配）。
3. `ReduceSum` 的实际内部累加顺序、count=0/尾块是否有脏数据污染。
4. `DataCopyPad` 尾块搬运与写出的真实内存边界行为。
5. 多核行切分下的确定性与核间不干扰。

## 三、真机复核清单（按优先级排序，含验收方法）

### P0（提交前必须，任何一条不过即阻断提交）

| # | 项 | 验收方法 | 风险来源 |
| --- | --- | --- | --- |
| 1 | 尾块写回边界：D%32≠0（fp16 D=33、bf16 D=67、fp32 D=33/1）时 UB→GM DataCopyPad 不覆盖相邻行 | 构造相邻行放哨兵值的用例（如 outer=2、D=33），写回后校验第 2 行头 4 元素未被污染；再用"相邻行分属不同核"的多核用例复测 | agent-02 定案 vs agent-09 C-WA-01（外部案例：padding 溢出覆盖相邻段） |
| 2 | bf16 输出舍入：CAST_RINT 与 numpy RNE 逐位对齐 | 已知 FP32 中间值 → Cast → 与 CPU RNE 对照；bf16 全 dtype 用例 0 失配 | agent-06 E6（TRUNC 40% 失配） |
| 3 | 宏冲突静态扫描：`grep -nE '\b(BLK\|LOWER\|UPPER\|pipe_\|T)\b' kernel.asc` 为零命中（模板参数名 `T` 需评估改名） | 本地 grep + 首次平台编译结果 | V001 pipe_ 15/15 CE（agent-09 L-01）+ C-CE-01/02（宏随 CANN 小版本漂移） |
| 4 | 上传内容完整性：提交后平台日志回读首行/末行/行数与本地一致 | 上传后在编辑器回读；对照 V002 异常教训 | agent-09 L-02（平台收到 `return false;` 首行） |
| 5 | 全 dtype×全尾块矩阵 0 失配（目标不是压 0.1% 容忍线，是全对） | agent-06 §4 测试矩阵逐格跑 | agent-06（平台 0.1% 容忍是否同款未确认） |

### P1（首版验证轮必须）

| # | 项 | 验收方法 | 风险来源 |
| --- | --- | --- | --- |
| 6 | NaN/Inf 行为逐位对齐 golden | NaN 行、±Inf 行、Inf+(-Inf) 行、全零行用例 | agent-06 §5 行为表 |
| 7 | 确定性：同输入连跑 3 次逐位一致 | 三连跑比对 | 题面硬性要求 |
| 8 | 大 D=32768 多段归约 + 段间合并正确 | fp32/fp16/bf16 各一例 | agent-06（分段误差 CPU 已证安全，内存正确性未证） |
| 9 | 索引 64 位安全：outer×D > 2³¹ 用例不越界 | 构造大 outer 用例（若平台含此规模） | agent-04 C5（vLLM #43390 教训） |
| 10 | 核数自适应：GetBlockNum() 实测值与 tiling 匹配（AIV 40 或 48 两种 SKU 均正常） | 两种核数环境或日志核对 | agent-07（判题 SKU 未确认） |
| 11 | UB 预算：InitBuffer 总量 ≤195584B（192KB−1KB）编译期通过且运行不溢出 | 编译 + EB0000 报错观察 | agent-07（官方模板 InitBuffer 上限 195584）+ agent-09 C-RE-01（248KB 假设崩溃案例） |
| 12 | 首版入口限定符：维持题面模板 `__global__ __vector__`（V002 现状）；如平台报错再回退 `__global__ __aicore__` | 平台编译结果 | agent-02 §三（两写法均合法，__aicore__ 证据链最强） |

### P2（性能验证轮）

| # | 项 | 验收方法 | 风险来源 |
| --- | --- | --- | --- |
| 13 | msprof op 瓶颈判定：`--warm-up=10 --launch-count=5`，读 PipeUtilization/ResourceConflictRatio | 对齐 agent-07 §6 流程 | agent-07（短任务降频污染数据） |
| 14 | 双缓冲按测试点分级：大点开、小点关 | A/B 对照实测 | agent-07（小数据双缓冲反例，A 级指南） |
| 15 | 标量同步成本实测：每行取 rstd vs MERGE_N 多行摊薄 | msprof aiv_scalar_ratio | agent-07 §3.4（绝对时延无官方数字） |
| 16 | Rsqrt 替代 Sqrt+除法的精度+性能双验证 | 15 点全量精度 + msprof | agent-07（Rsqrt 精度未验证）+ agent-09 C-WA-03（等价改写风险） |

## 四、真机环境搭建要点（详见 agent-08）

- 链路：锁内核 → HwHiAiUser 属组 → HDK 26.0.RC1/25.5.2/25.5.1（A2 系包名 `ascend910b-*`，lspci d802 即 910B）→ npu-smi 验证 → toolkit 9.0.0 + 910b-ops 9.0.0（先 toolkit 后 ops）→ `source set_env.sh`（推荐 `latest/bin/setenv.bash`）→ 模板 `./run.sh`。
- CANN 9.0 仿真器仅支持 950PR，**本题无本地仿真捷径**（agent-08）。
- 本地编译选项与判题一致：`--npu-arch=dav-2201`（需 CANN ≥8.3.RC1，9.0.0 按版本序列推断支持，真机以 `bisheng --help` 确认）。
- 3 秒超时语义（`aclrtSynchronizeStreamWithTimeout(stream, 3000)`）：kernel 死等/事件不配对会判超时且可能污染设备状态连坐后续样例（agent-08 步骤 10）。

## 五、提交策略（结合本轮调研）

1. **首提目标拿有效结果**：用最保守正确版本（FP32 全中间、DataCopyPad 尾块、行级均分、无花哨快路径），先确认 15 点全过，再谈性能。
2. **额度不确定**：「每天 50 次提交」无页面依据且被公开统计反证（单队 8 天 558 次，agent-01 §7）；勿按 50/天做规划，但也不要浪费性重交（V001/V002 各烧一次）。
3. **每次提交双闸门**：提交前本地静态核对（宏扫描、行数、入口）；提交后平台日志回读。
4. **性能优化候选只有在 P0-P1 全过后才允许上平台**；每轮只改一个变量（便于归因）。

## 六、与本地两份代码的关系（只读结论）

- `源码/`（msopgen 标准）：两遍扫描正确性优先版，API 用法与 agent-02 核对结论一致；但属标准算子工程，不能直接作为直调提交物（`GET_TILING_DATA_WITH_STRUCT` 在直调工程不可用，A 级约束）。
- `提交/V002/kernel.asc`（直调候选，2955 行多路径）：`tpipe` 已避开 `pipe_` 宏；行基址 uint64_t；`V_S/S_V` 事件配对；UB 规划在 192KB 内——与本轮调研的关键红线一致。未验证项见上表 P0/P1。是否复用 V002 或按推荐路线重构，由 agent-10 的差距对照与用户决策（见 research-report.md 第 11-12 章）。

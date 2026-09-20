# CANN 项目长期备忘（跨会话）

> 本文件是给后续会话用的**索引与不变量**，不是流程事实源。口径冲突时以仓库内文件为准：
> `AGENTS.md`（工作约定）→ `文档/统一探索收敛与线上提交流程.md`（唯一流程事实源）→
> `文档/competition-rules.md` + `文档/problem-add-rms-norm-bias.md`（题面与规则）→
> `调研/总结.md`（R001–R029 路线与实验总记录）→ `提交/版本实验记录.md`（版本索引）。

## 不可违背的不变量

- **计分**：`s_i = 100/(1+log₁.₅(tᵢ/Tᵢ))`，总分 = 15 点均值；`T` 是**实时最优**（登顶即 100），每点 `iterations=5`；性能是唯一计分维度；15 点全过才计分。
- **每日 50 次提交，取当天最后一次成绩**。⇒ 当天最后一发必须是当时最优，不能用未验证版本占坑。
- **判题精度**：`np.isclose(rtol=1e-3, atol=1e-3)` 逐元素 + **失配容忍 0.1%**（判题端口径未完全证实）。本机严格双阈值（绝对且相对同时达标）**比平台严**，会误杀平台能过的版本。
- **本机无 CANN/NPU**：任何 NPU 结论必须来自服务器 3；本地只能静态审阅 + CPU 参考。
- **服务器 3**：`ssh cann-server3`（`hwnput3` / `lelinfeng` / 910B3×8 / 项目内 CANN 9.0 工具链，只读依赖）。空间红线 = **剩余 < 20G**（不是使用率 98%）。项目目录 `/home/data4t2/lelinfeng/cann`。
- **合规**：核心计算必须在 Kernel 内完成；禁止 Host 代算/写死结果；凭据零写入；不删他人数据与公共工具链；**CANNJudge 提交与 GitHub push 必须先经用户确认**。
- **单点修改**：一版只改一个技术变量；改前写变更说明，改后按 编译→最小运行→精度→性能 验证；异常即停并回退到最近可用版本。
- **提交身份**：只提交已推送且绑定 `git_commit + sha256 + 行数 + 字节数` 的版本；禁止"当前最新文件"。唯一 Submission Worker + 2 分钟限流。
- **路线编号不复用**：`R029` = 官方 Tiling 五模式（仓库语义）。外部轨道的宽行 cached-y 线保持独立编号（见下）。

## 关键实现红线（易踩）

- `DataCopyPadExtParams` 声明序 `{isPad, leftPadding, rightPadding, paddingValue}`；**禁止聚合初始化**，必须逐字段赋值。
- 多核按 `k×D×sizeof(T) ≡ 0 (mod 32)` 行块粒度分配，防 32B cache line 跨核踩踏。
- 行基址/偏移强制 `uint64_t`。
- A2 上 bf16 的 `Add/Mul/Muls/Div/ReduceSum` 均不支持 ⇒ Cast→fp32→Cast。
- 归一化**强制 `Divs` 先除**（与 golden 逐位一致）；`Muls(1/rms)` 在「大 outer 小 D」bf16 下最大绝对差 1.562e-02；`Rsqrt` 只能是独立实验过的性能候选。
- 入口与模板逐字对齐 `__global__ __vector__`；提交物是含 `run_kernel` 的 `kernel.asc`（`code_template=npu_kernel_dev`）。
- 本地 `verify_result.py` 阈值只有 1e-3，**会漏判 fp32（应 1e-4）**。

## 两条轨道

- **仓库轨道（H001 正确性优先）**：无官方分。当前执行优先级见 `调研/总结.md` §16.1。
- **外部轨道（ChatGPT《编译并修复》）**：`提交/外部轨道-ChatGPT编译并修复/`，20 个版本；`R029-V003`=官方 28.68、`R029-V004`=官方 **29.01**（15/15），**提交 ID 未绑定**。审查报告在 `调研/归档/外部对话-ChatGPT编译并修复/审查报告.md`。
- 差距量化：D baseline 复算 20.99 → V004 复算 28.99；Case14+15 占总耗时 97.4%；15 点全做到 2×TBest 也只有 36.91 分 ⇒ 冲 70 分需要整体提速 ~3.4 倍。

## 常用入口

```bash
ssh cann-server3                                     # 服务器 3
source /home/data4t2/lelinfeng/cann_game/.toolchains/cann-9.0/cann-9.0.0/set_env.sh
python3 调研/工具/cannjudge.py preflight|poll|problem   # 只读平台接口，无 POST
```

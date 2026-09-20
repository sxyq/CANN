# 本机（Host / CPU）验证记录（调研2 · 2026-09-12）

> **性质**：全部为 **CPU 参考计算（numpy + ml_dtypes）与静态审阅**。
> **本机环境**：macOS（darwin），Python 3.13.12（隔离 venv），numpy 2.5.3 + ml_dtypes。
> **本机没有 CANN 工具链、没有昇腾 NPU、没有 Docker** —— 因此本文件**不记录、也不声称**任何 NPU 编译、NPU 精度或 NPU 性能结果。

---

## 1. 验证范围与不做的事

| 做了 | 没做（且不能做） |
| --- | --- |
| 平台下发模板 8 个文件的逐字核对 | 编译 `kernel.asc`（无 bisheng/ccec） |
| `提交/V002/kernel.asc` 的静态结构审阅 | 运行 `run.sh`（无 NPU） |
| golden 口径确认（`AddRmsNormBias.py`） | 真机精度对比 |
| CPU 参考数值实验（两套独立脚本） | 真机性能测量 |
| 方案/反例的数值链仿真 | 任何 CANNJudge 上传 |

---

## 2. 静态审阅记录

### 2.1 模板核对（A 级）

| 文件 | 关键事实 |
| --- | --- |
| `kernel.asc`（26 行空壳） | `extern "C" void run_kernel(GM_ADDR x, const TensorGroupInfo& info_x, …, int64_t availableCoreNum, aclrtStream stream, float epsilon)`；注释要求 `add_rms_norm_bias_custom<<<blockNum, nullptr, stream>>>(...)`；**文件被 `#include`，不得有 `main()`/`#pragma once`** |
| `main.asc` | `TensorInfo{shape,numDims,dtype}`；dtype 枚举 `0=fp32 1=fp16 2=bf16`；`ACL_DEV_ATTR_VECTOR_CORE_NUM` 取核数；默认用例 FP16 `[1,64]`、`epsilon=1e-5f`；为每个张量 `aclrtMalloc` 128 字节 |
| `CMakeLists.txt` | `find_package(ASC REQUIRED)`；`project(... LANGUAGES ASC CXX)`；默认 `SOC_ARCH="dav-2201"`（可被 `NPU_ARCH` 覆盖）；链接 `tiling_api register platform unified_dlog dl m graph_base`；编译选项 `--npu-arch=${SOC_ARCH}` |
| `run.sh` | `source ${ASCEND_HOME_PATH}/set_env.sh` → `cmake .. && make -j4` → `gen_data.py` → **`timeout 120`** 运行 → `verify_result.py 0` |
| `scripts/AddRmsNormBias.py` | **golden 唯一权威**：FP32 全链路，`y=x+r` → `rms=sqrt(mean(y*y,axis=-1)+eps)` → `out=y/rms*gamma` → `+bias` → 末尾一次 cast。**注意是 `y / rms`** |
| `scripts/verify_result.py` | `np.isclose(rtol=1e-3, atol=1e-3, equal_nan=True)` 逐元素 + 允许失配比例 `tol=1e-3`（0.1%） |
| `scripts/gen_data.py` | 只生成 1 个用例：`uniform(-2,2)` 的 FP16 `[1,64]`，种子 `42` |
| `data_utils.h` | 文件读写工具；`ReadFile` 要求**文件大小必须精确等于** bufferSize |

### 2.2 `提交/V002/kernel.asc` 静态结构（A 级内部快照）

- 规模 **2955 行**；`__global__ __vector__` 启动，成员函数用 `__aicore__`。
- 归约：`AscendC::ReduceSum(...)` + `GetValue(0)` 标量读回（多处）。
- 归一化：`const float invRms = 1.0f / xFp32.GetValue(0);` + `AscendC::Muls(valueTile, valueTile, invRms, valid);` → **乘倒数路径**（与 golden 的除法不同）。
- 分支：`Process*` 变体约 20 个（SmallFp32Batched / WideFp16Cached / …FullRowOutputPipelined 等）。
- 已确认存在 `uint64_t` 行寻址与 `DataCopyPad` 尾块路径。
- **风险**：体量与分支数触发官方记载的 `out of jump/jumpc imm range` 编译风险；`TPipe tpipe` 作为类成员。

---

## 3. CPU 参考数值实验

### 3.1 实验一：主代理独立复核脚本（交叉验证 Agent 06）

- 脚本：`数值参考脚本/main_precision_check.py`
- 日志：`数值参考脚本/main_precision_check.log`；机器可读：`main_precision_check.json`
- golden：直接 `import` 平台下发的 `AddRmsNormBias.py`，保证口径一致
- 种子：`1742`；数据：`x,residual ~ U(-2,2)`，`gamma ~ U(0.8,1.2)`，`bias ~ U(-0.3,0.3)`
- 阈值：fp32 `(1e-4,1e-4)`；fp16/bf16 `(1e-3,1e-3)`（题面口径）

**结果（失配元素 / 总数 = 失配比例）**

| 归一化路径 | fp16 | bf16 | fp32 |
|---|---|---|---|
| `Sqrt` + `Divs`（先除） | **0 / 132144 = 0%**（最大绝对差 **0**） | **0 / 132144 = 0%**（最大绝对差 **0**） | 0 / 132144 = 0%（最大绝对差 2.384e-07） |
| `Sqrt` + `Muls(1/rms)`（乘倒数） | 0 / 132144 = 0%（最大绝对差 **9.766e-04**） | 0 / 132144 = 0%（最大绝对差 0） | 0 / 132144 = 0%（4.768e-07） |
| `Rsqrt` 等价路径 | 0%（**9.766e-04**） | 0%（0） | 0%（4.768e-07） |

- **分块累加（block=1024 / 4096）**：三种 dtype 全部 **0 失配** → 分块不是误差来源。
- **3D/4D（[2,3,4,8]、[4,8,128]、[2,2,2,129]）**：三种 dtype 全部 **0 失配**。
- **反例 · epsilon 外置**（开方之后才加 eps）：bf16 **36 / 65536 = 0.0549%** 失配 → 必须内置。
- **反例 · 低精度中间累加（S4）**：bf16 **64855 / 65536 = 98.96%** 失配 → **证否 S4**。

### 3.2 实验二：`Muls` vs `Divs` 风险边界定向探查

- 脚本：`数值参考脚本/main_muls_probe.log`（内联脚本，探查不同数据尺度 × shape）
- 变量：dtype × shape{`(64,1024)`, `(8,4096)`, **`(8192,64)`**} × scale{0.01, 1, 2, 50}

**结果摘要**

| 路径 | 最大绝对差（全部 36 组配置） | 失配元素 |
| --- | --- | --- |
| **`Divs`** | **0.000e+00 —— 全部 36 组与 golden 逐位一致** | 0 |
| `Muls(1/rms)` | fp16 最高 **1.953e-03**（≈ 2× atol 阈值）；bf16 最高 **1.562e-02**（≈ 15.6× 阈值）；fp32 恒为 4.768e-07（< 1e-4，安全） | bf16 `(8192,64)` 在 scale≥1 时出现 **1–3 个失配元素** |

**关键结论（比 Agent 06 更精确）**：

1. **`Divs` 在所有被测配置下与 golden 逐位一致（最大绝对差恒为 0）**，这是"零风险"路径。
2. **`Muls(1/rms)` 的风险集中在「大 outer + 小 D」形态**（本例 `(8192,64)`），正好是本报告 §7.4 的四种形态之一，也是判题最可能覆盖的形态。
3. `Rsqrt` 路径与 `Muls` 表现相同（都引入额外舍入）。
4. **裁决：首版强制 `Divs` + `Sqrt`，禁止 `Muls(1/rms)` / `Rsqrt`。**

> 与 Agent 06 的差异说明：Agent 06 报 `Muls` 在 bf16 有 1–28 个越界元素，本轮主代理复核在 `U(-2,2)` 下得 0 个、在更广尺度扫描下得 1–3 个。**两者方向一致（Muls 有非零风险、Divs 无风险），具体数值随数据分布变化**。这一差异本身说明：判题端数据分布未知时，`Muls` 的风险不可预测，而 `Divs` 恒定安全。

### 3.3 Agent 06 的独立实验

- 脚本：`数值参考脚本/agent06_precision_matrix.py`（425 行）
- 日志：`agent06_precision_matrix.log`；机器可读：`agent06_results.json`
- 结论（CPU 参考）：`Divs` 逐位一致；分块累加相对误差 ≤3.0e-6；fp16 平方在 `|y|≥256` 溢出为 inf；bf16 全程低精度 50%+ 失配；eps 外置在 bf16 全零行 0.064% 失配。
- 矩阵定义见 `精度测试矩阵.md`。

---

## 4. 本机能复现、真机必须复核的清单

| 项 | 本机结论（CPU 参考） | 真机必须复核的原因 |
| --- | --- | --- |
| 归一化用 `Divs` | 逐位一致，零风险 | 硬件 `Divs` 指令的单次舍入与 CPU 不同 |
| 归约用 FP32 | 三种 dtype 均 0 失配 | 硬件 `ReduceSum` 的累加顺序与 CPU `np.sum` 不同 |
| 分块 ≤ 4096 | 无精度影响 | UB 实际容量与 work buffer 需求未实测 |
| 尾块补零 | 不影响平方和 | `DataCopyPad` 的真实 padding 行为未实测 |
| 多核 32B 踩踏 | **无法在本机验证** | 需要真机 DMA 并发写回 |
| 编译是否通过 | **无法在本机验证** | 需要 CANN 9.0.0 + bisheng/ccec |

---

## 5. 复现方式

```bash
PY=/Users/sunyiyang/.workbuddy/binaries/python/envs/default/bin/python
# 依赖：pip install numpy ml_dtypes（已装：numpy 2.5.3 + ml_dtypes）
cd /Users/sunyiyang/Desktop/Project/cann/调研/调研2/数值参考脚本
$PY main_precision_check.py          # 实验一：全量对比
$PY agent06_precision_matrix.py      # Agent 06 的矩阵实验
```

golden 依赖平台下发模板中的 `scripts/AddRmsNormBias.py`，若模板路径变化需同步修改脚本中的 `TEMPLATE_SCRIPTS`。

---

## 6. 声明

- 本文件所有数值均为 **CPU 参考值**，不代表昇腾 NPU 上的实际结果。
- 未在任何昇腾设备上编译或运行 Ascend C 代码。
- 本轮未执行 CANNJudge 上传，未消耗提交次数。

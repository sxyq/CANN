# Agent 1 专题报告：比赛题面、规则与提交接口深度核对

> 负责代理：Agent 1  
> 证据级别定义：  
> - **[A 级]**：CANNJudge 官网题面原文、官方 API 返回、下载模板真实源码。  
> - **[B 级]**：官方代码样例、CMakeLists.txt 配置、测试验证脚本。  
> - **[C 级]**：社区比赛分享、技术论坛、历史开源题目镜像。  
> - **[D 级]**：无明文依据的口头推断或猜测（明确标注并存疑）。

---

## 1. 题目背景与核心事实

- **比赛名称**：2026 年 CANN 挑战赛 · 西南赛区 **[A 级]**
- **赛事 ID**：`2094722165106008066` **[A 级]**
- **题目名称**：`AddRmsNormBias` **[A 级]**
- **平台题目编号**：`1742`（模板目录 `addrmsnormbias_problem_1742_template`） **[A 级]**
- **题目全局 ID**：`6a9a9a99bf41025d6013eb85` **[A 级]**
- **目标软件栈**：CANN `9.0.0` **[A 级]**
- **算子类型**：Vector Kernel（纯向量算子，无 Cube 矩阵运算） **[A 级]**

---

## 2. 数学公式与计算语义

依据下载模板中的 `scripts/AddRmsNormBias.py` 官方 golden 实现（经完全比对，为最高权威证据）：

$$\begin{aligned}
y &= x + \text{residual} \\
\text{rms} &= \sqrt{\text{mean}(y^2, \text{dim}=-1) + \epsilon} = \sqrt{\frac{1}{D} \sum_{i=0}^{D-1} y_i^2 + \epsilon} \\
\text{norm} &= \frac{y}{\text{rms}} \\
\text{output} &= \text{norm} \odot \gamma + \text{bias}
\end{aligned}$$

### 关键语义辨析（与常见算子的细微差异）
1. **残差相加（Residual Add）**：  
   `x` 与 `residual` 形状完全一致，逐元素相加得到 $y$。二者无跨维广播，且二者输入 dtype 完全一致。
2. **归一化轴向**：  
   严格沿最后一维（$D$ 维，即 `axis=-1`）执行均方根计算。归约除数为最后一维长度 $D$（而非整个张量元素数）。
3. **$\epsilon$ 的加法位置**：  
   $\epsilon$ 加在 $\text{mean}(y^2)$ **之后**、开方 $\sqrt{\cdot}$ **之前**。严禁将 $\epsilon$ 加在开方之后或分母之外。
4. **仿射变换（Scale & Shift）**：  
   $\gamma$（gamma）与 $\text{bias}$ 均为一维张量，形状为 $(D,)$。沿最后一维广播到每一个样本行。偏置加法发生于归一化与缩放完成之后，严禁纳入归约求和内部。

---

## 3. 张量规格与数据类型约束

| 张量名称 | 角色 | 形状要求 | 支持的数据类型 | 说明 | 证据等级 |
| :--- | :--- | :--- | :--- | :--- | :--- |
| `x` | 主输入 | $(..., D)$，支持 2D/3D/4D | `float32`, `float16`, `bfloat16` | 张量秩 Rank $\in [2, 4]$ | **[A 级]** |
| `residual`| 残差输入 | 与 `x` 形状严格一致 | 与 `x` 一致 | 逐元素相加，无广播 | **[A 级]** |
| `gamma` | 缩放系数 | $(D,)$ 严格一维 | 与 `x` 一致 | 沿前导维度广播 | **[A 级]** |
| `bias` | 偏置系数 | $(D,)$ 严格一维 | 与 `x` 一致 | 沿前导维度广播 | **[A 级]** |
| `epsilon` | 算子属性 | 标量 `float` | 标量 `float` | 默认值 `1e-5`，防除零 | **[A 级]** |
| `output` | 计算输出 | 与 `x` 形状及类型严格一致 | 与 `x` 一致 | 结果张量 | **[A 级]** |

### 维度取值边界
- **最后一维 $D$**：$D \in [64, 32768]$。**特别注意：$D$ 不保证是 32 或 64 的整数倍**（如题面示例出现 $D=192, 576$ 以及非对齐测试点 $D=67, 129, 1000$）。
- **前导维度积 $\text{outer}$**：$\text{outer} = \prod_{k=0}^{\text{Rank}-2} \text{shape}[k]$。前导维度可包括 batch、seq_len、num_heads 等。批量规模 $\text{batch} \in [1, 8192]$，总行数可能高达数万行，必须展平成一维行逻辑进行跨 Core 分块。

---

## 4. 判题规则、测试点与评分体系

### 4.1 测试点明文事实 **[A 级]**
- **测试点总数**：全量共 **15 个测试点**（官方 API 描述明文标注：“共15个测试点，全部通过才计分”）。
- **通过门槛**：**15 个测试点必须 100% 精度达标**。任一点出现精度超标、超时、运行时崩溃或输出尺寸不符，总分直接归零。
- **每点采样次数**：每个测试用例连续执行 **5 次迭代（iterations=5）**，取性能有效统计均值。

### 4.2 精度判定标准 **[A 级/B 级]**
依据模板中 `scripts/verify_result.py` 的实现：
```python
# 每个测试点容差配置：rtol=0.001, atol=0.001, tol=0.001
isclose = np.isclose(cmp_output, cmp_golden, rtol=0.001, atol=0.001, equal_nan=True)
errors = np.sum(~isclose) + missing
error_rate = errors / total_size
return error_rate <= tol  # 允许最大失配元素比例不超过 0.1%
```
- **相对误差 `rtol`**：$10^{-3}$（0.001）。
- **绝对误差 `atol`**：$10^{-3}$（0.001）。
- **失配元素允许率 `tol`**：$0.1\%$（0.001）。即便极少数浮点下溢或边界舍入有 1 个最小单位偏差，只要失配率 $\le 0.1\%$ 仍可通过，但不可滥用。
- **NaN/Inf 规范**：`equal_nan=True`，当输入为合法浮点计算导致的 NaN 时，输出需对应一致，严禁算子崩溃退出。

### 4.3 性能评分函数 **[A 级]**
单测试点得分公式（经平台实际验证）：
$$\text{Score}_i = \frac{100}{1 + \log_{1.5}\left(\frac{t_i}{T_i}\right)}$$
- $t_i$：选手提交的代码在第 $i$ 个用例上的执行耗时。
- $T_i$：全平台该测试点的历史最优（基准）耗时。
- **总分**：15 个测试点得分的算术平均值 $\text{TotalScore} = \frac{1}{15}\sum_{i=1}^{15} \text{Score}_i$。
- **平局决胜**：总分相同时，以提交时间较早者排名靠前。

---

## 5. 提交物与直调（Direct Invocation）接口核对

### 5.1 架构模式确认：直调工程而非旧版 msopgen 工程 **[A 级]**
- 历史 CANN 比赛常要求提交 `op_host` 与 `op_kernel`，包含 `tiling.h`、`tiling_key.h`、`host.cpp`、`kernel.cpp` 四个独立字段。
- **本题经过对平台下载模板的完全核对，已确认为 CANN 9.0 直调工程（Direct Invocation, `code_template=npu_kernel_dev`）**。
- 选手的核心提交物为 **唯一单个源码文件 `kernel.asc`**。平台编译系统将 `main.asc` 与 `kernel.asc` 联合编译（`#include "kernel.asc"`）成单个执行程序 `add_rms_norm_bias_custom`。

### 5.2 宿主调用函数签名（严禁擅自修改入参） **[A 级]**
```cpp
#include <cmath>
#include "kernel_operator.h"

// 平台预定义的张量描述结构体
struct TensorInfo { 
    const int64_t* shape; 
    int64_t numDims; 
    int32_t dtype; // 0=fp32, 1=fp16, 2=bf16, 3=int8, ...
};
struct TensorGroupInfo { 
    const TensorInfo* tensors; 
    int64_t numTensors; 
};

extern "C" void run_kernel(
    GM_ADDR x, const TensorGroupInfo& info_x, 
    GM_ADDR residual, const TensorGroupInfo& info_residual, 
    GM_ADDR gamma, const TensorGroupInfo& info_gamma, 
    GM_ADDR bias, const TensorGroupInfo& info_bias, 
    GM_ADDR output, const TensorGroupInfo& info_output, 
    int64_t availableCoreNum, 
    aclrtStream stream,
    float epsilon
)
{
    // 内部完成 Tiling 维度解析与核函数 Launch:
    // add_rms_norm_bias_custom<<<blockNum, nullptr, stream>>>(...);
}
```

### 5.3 核函数入口限定符规则 **[A 级/B 级]**
- 模板 `kernel.asc` 注释规范：
  ```cpp
  __global__ __vector__ void add_rms_norm_bias_custom(...)
  ```
- 严谨证据：在 CANN 9.0 中，纯 Vector 算子（无 Cube 矩阵乘）应使用 `__global__ __vector__` 限定符声明核函数。若使用旧版 `__global__ __aicore__`，在部分编译器版本中会引发警告或架构目标绑定偏差。

### 5.4 目标硬件架构（SoC ARCH） **[B 级]**
- 模板 `CMakeLists.txt` 中指定默认值：
  ```cmake
  if(DEFINED NPU_ARCH)
      set(SOC_ARCH ${NPU_ARCH})
  else()
      set(SOC_ARCH "dav-2201")
  endif()
  ```
- `dav-2201` 对应华为 **Atlas A2 系列**（包括 Ascend 910B1/B2/B3/B4 等训练卡形态）。此架构具备完整的 Vector 计算单元和 192KB/256KB Unified Buffer，支持 `DataCopyPad` 双向搬运。

---

## 6. 每日提交与合规风控 **[D 级/A 级]**

1. **每日提交额度**：用户提供线索为每日 50 次提交额度（页面未明文公开，实行保守配额管理）。
2. **严禁行为**：
   - 严禁空 Kernel（直接 return 导致输出全 0 或不写输出）。
   - 严禁 Host 侧代算（在 `run_kernel` 中使用 CPU 循环计算并 `aclrtMemcpy` 覆盖输出，判题端有性能探针与 NPU 指令分析）。
   - 严禁写死测试点固定输出。
   - 本地未在真机环境运行前，绝对不消耗在线判题额度。

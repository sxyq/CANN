# 代码概要

算子: `row_copy`（依据函数名推定，正式注册名未在可读源码中出现） | 功能: 将二维 `float` 输入逐行复制到输出 | 侧别: Kernel

## 代码脉络

**入口**: `row_copy_kernel`（`submission.asc:4-5`），使用 `extern "C" __global__ __vector__` 声明为 Vector Kernel。调用者、launch 配置和触发条件未包含在本次允许读取的文件中。

**数据流**: 输入 GM（`input`）→ `GlobalTensor<float>` → `VECCALC` 本地 staging buffer → 输出 GM（`output`）。`RowCopyTiling` 提供 `rows`、`cols`。

**计算核心**: `row_copy_kernel`（`submission.asc:4-52`）。`rowStart = GetBlockIdx() * kRowsPerBlock`，每个 block 处理一行；列循环将该行切成不超过 2,048 个 `float` 的块，依次搬入本地张量并写回。

**分支覆盖**:

| 条件 | 位置 | 触发场景 | 处理逻辑 | API |
|---|---|---|---|---|
| `rowStart >= shape->rows` | `submission.asc:10-12` | block 对应行超出行数 | 直接返回，不读写张量 | `GetBlockIdx` |
| `rowElements - col > kMaxElementsPerSegment` | `submission.asc:32-34` | 当前剩余列数超过分段上限 | 取上限；否则处理剩余列，包含尾段 | 无 |

**关键变量流转**:

| 变量 | 来源 | 用途 | 流转路径 |
|---|---|---|---|
| `input`、`output` | Kernel 参数 `GM_ADDR` | 输入和输出 GM 地址 | 参数 → `GlobalTensor` → 数据搬运 |
| `tiling`、`shape` | Kernel 参数；转为 `RowCopyTiling` 指针 | 读取行数和列数 | `tiling` → `shape->rows/cols` |
| `rowStart` | `GetBlockIdx()` 与 `kRowsPerBlock` | 当前 block 的起始行 | block index → GM 基址偏移 |
| `rowsThisBlock` | 常量 `1` | 行循环上界 | 固定处理当前一行 |
| `chunkElements` | 剩余列数与分段上限 | 当前搬运元素数 | 列循环 → `copyParams.blockLen` |
| `staging`、`local` | `TPipe::InitBuffer`、`TBuf::Get` | 暂存输入分段 | GM 输入 → `local[0]` → GM 输出 |

**核心 API**: `AscendC::GetBlockIdx`、`TPipe::InitBuffer`、`TBuf::Get`、`GlobalTensor::SetGlobalBuffer`、`DataCopyPad`、`PipeBarrier<PIPE_ALL>`。

**输出**: 每段写入 `outputGm[row * rowElements + col]`。输入搬入后执行一次 `PIPE_ALL` 屏障，再写回并执行一次屏障；未使用 `EnQue`/`DeQue`。

## 算子业务语义（Kernel 侧）

**数学运算**: `Y[r,c] = X[r,c]` | **输入输出**: 1 个输入、1 个输出，均按二维 `float` 行列数据处理；shape 由 `rows`、`cols` 表示。

**计算模式**: 简单 Vector 分段搬运流水（load → store，无算术变换） | **同步契约**: `PipeBarrier<PIPE_ALL>` 分别隔开输入搬入与输出写回，以及相邻阶段的本地执行。

### 分支业务含义

| 条件 | 位置 | 业务含义 | 处理逻辑 |
|---|---|---|---|
| `rowStart >= shape->rows` | `submission.asc:10-12` | block 没有对应的有效行 | 提前结束 |
| 剩余列数超过分段上限 | `submission.asc:32-34` | 长行需要分段搬运 | 每段至多 2,048 个 `float` |

### 模板参数语义

无模板参数。

## Tiling 业务语义（Tiling 侧）

不适用：当前输入是 Kernel 侧文件。允许读取的 `row_copy_tiling.h` 只声明 `rows`、`cols` 和常量，没有 Host 侧切分、参数约束、Buffer 决策或 TilingKey 生成代码。

## 变量溯源

| 变量 | 声明位置 | 初始化 | 校验/边界处理 | 来源类型 |
|---|---|---|---|---|
| `input`、`output` | `submission.asc:5`，`GM_ADDR` 参数 | Kernel launch 提供 | 本文件未见地址校验 | 外部 Kernel 参数 |
| `tiling` | `submission.asc:5`，`GM_ADDR` 参数 | Kernel launch 提供 | 本文件未见空指针校验 | 外部 Kernel 参数 |
| `shape` | `submission.asc:7-8`，`const __gm__ RowCopyTiling*` | 将 `tiling` 转型 | 读取前未见字段范围校验 | Tiling 参数 |
| `rowStart` | `submission.asc:9`，`uint32_t` | `GetBlockIdx() * kRowsPerBlock` | `submission.asc:10-12` 行数上界保护 | 硬件 block index + 编译期常量 |
| `rowsThisBlock` | `submission.asc:14`，`uint32_t` | 固定为 `1` | 无需从 Host 侧推导 | 编译期固定值 |
| `rowElements` | `submission.asc:15`，`uint32_t` | `shape->cols` | 分段循环以其为上界 | Tiling 参数 |
| `staging`、`local` | `submission.asc:17-19` | 分配 `kScratchRows * kMaxCols * sizeof(float)` 字节后取得本地张量 | 本文件未见设备容量查询 | Kernel 本地资源 |
| `chunkElements` | `submission.asc:32-34`，`uint32_t` | 取剩余元素数与上限中的较小值 | 由分段表达式限制 | 运行时计算 |

## 函数清单

| 函数 | 签名 | 行范围 | 角色 |
|---|---|---:|---|
| `row_copy_kernel` | `extern "C" __global__ __vector__ void row_copy_kernel(GM_ADDR input, GM_ADDR output, GM_ADDR tiling)` | 4-52 | Kernel 入口、搬运核心 |

## API 调用索引

| API | 行号 | 上下文 |
|---|---:|---|
| `AscendC::GetBlockIdx` | 9 | 获取 block index，计算起始行 |
| `TPipe::InitBuffer` | 18 | 为 `staging` 分配本地缓冲区 |
| `TBuf::Get<float>` | 19 | 获取本地 `float` 张量 |
| `GlobalTensor::SetGlobalBuffer` | 23-24 | 设置输入、输出 GM 基址 |
| `AscendC::DataCopyPad` | 45 | 从输入 GM 搬入本地张量，传入 copy 与 pad 参数 |
| `AscendC::PipeBarrier<PIPE_ALL>` | 46 | 输入搬入后的阶段屏障 |
| `AscendC::DataCopyPad` | 47 | 从本地张量写回输出 GM |
| `AscendC::PipeBarrier<PIPE_ALL>` | 48 | 输出写回后的阶段屏障 |

## 常量清单

| 常量 | 值 | 位置 | 用途 |
|---|---:|---|---|
| `kMinCols` | `256` | `row_copy_tiling.h:10` | 头文件定义；本 Kernel 未使用，适用范围未能由当前文件确定 |
| `kMaxCols` | `8192` | `row_copy_tiling.h:11` | 参与 staging buffer 大小计算 |
| `kScratchRows` | `2` | `row_copy_tiling.h:12` | 参与 staging buffer 大小计算 |
| `kRowsPerBlock` | `1` | `row_copy_tiling.h:13` | 计算每个 block 的起始行偏移 |
| `kMaxBlocksPerSegment` | `256` | `submission.asc:27` | 分段的 32-byte block 上限 |
| `kMaxElementsPerSegment` | `256 * 32 / sizeof(float) = 2,048` | `submission.asc:28-29` | 限制每段元素数；对应最多 8,192 字节 |

## 跨文件防御摘要

| 关联文件 | 关键发现 | 位置 | 影响范围 |
|---|---|---|---|
| `support/reference/op_kernel/row_copy_tiling.h` | `RowCopyTiling` 仅含 `uint32_t rows`、`uint32_t cols`；同时定义列、暂存行和每 block 行数常量 | `row_copy_tiling.h:5-13` | Kernel shape 读取、行偏移、缓冲区大小 |
| `kernel_operator.h` | `submission.asc` 直接 include 该头；头文件实体未能在本次可读源码集合中定位 | `submission.asc:1` | AscendC 类型与 API 的声明来源未独立核对 |

## TilingData 值域溯源

| 字段 | Host 侧赋值 | 公式 | 输入参数 | 约束 |
|---|---|---|---|---|
| `rows` | 未在允许读取的文件中找到 Host 赋值 | 未知 | 未知 | Kernel 仅用它做 block 行范围保护 |
| `cols` | 未在允许读取的文件中找到 Host 赋值 | 未知 | 未知 | Kernel 用它作为列循环上界；Host 侧范围未确认 |

## 芯片架构参数

| 参数 | 值 | 来源 | 影响范围 |
|---|---|---|---|
| 每段 32-byte block 上限 | `256` | `submission.asc:26-29` 注释及常量 | 每段最多 `8,192` 字节 |
| `aivNum`、`aicNum` | 未知 | 当前可读文件未提供 | launch 核数及切分无法确定 |
| UB/L1 容量、对齐约束 | 未知 | 当前可读文件未提供；仅可见 `VECCALC` 位置声明 | 不能由本文件推导设备容量或其他对齐条件 |
| 芯片标识 | 注释提及 `dav_c220` | `submission.asc:26` | 代码注释称分段用于符合观察到的二维行边界；此处没有额外规格数据 |

## 代码关联

**上游文件**:

| 文件路径 | 关联方式 | 依据 |
|---|---|---|
| `support/reference/op_kernel/row_copy_tiling.h` | include | `submission.asc:2`；定义 `RowCopyTiling` 和常量 |
| `kernel_operator.h` | include | `submission.asc:1`；SDK 头文件实体未能从本次可读源码集合定位 |

**下游 API**: `GetBlockIdx`、`TPipe`/`TBuf`、`GlobalTensor`、`DataCopyPad`、`PipeBarrier`，调用位置见 API 调用索引。没有读取到 Host 调用链。

## 高性能设计（Kernel 侧）

**流水线模式**: Vector Kernel 内按段顺序搬入和写回；没有 `TQue`、`EnQue`/`DeQue`、双缓冲或 AIC-AIV 协作代码。

| 机制 | 状态 | 设计意图 |
|---|---|---|
| `PipeBarrier<PIPE_ALL>` | 有，每段两处 | 将输入搬入、本地读取和输出写回按阶段隔开 |
| Buffer 管理 | 单个 `TBuf<VECCALC>` | 复用 `local[0]` 处理各列段 |
| 多核切分 | 按行；每 block 一行 | `rowStart = blockIdx * 1`；实际 launch block 数未知 |
| 分段搬运 | 按列；最多 2,048 个 `float` | 每段至多 8,192 字节，末段按剩余列数处理 |

`staging` 分配量为 `2 * 8,192 * sizeof(float) = 65,536` 字节（64 KiB）。代码每次通过 `local[0]` 搬运至多 8,192 字节；设备总容量及剩余容量未提供。

## 未知项

- 正式算子注册名、Kernel 调用者、launch 配置和运行触发条件未在允许读取的文件中出现。
- `rows`、`cols` 的 Host 侧赋值及取值范围未确认；`kMinCols` 在本 Kernel 中未使用。
- `kernel_operator.h` 的具体声明未独立读取，API 行为说明仅依据当前文件中的调用形式和参数。
- 核数、设备内存规格和全局对齐约束未能从允许读取的源码确定。
- 子 Agent 派发接口和本机 CLI 均未提供 `multi_agent_v1`；本概要由主流程按同一指南生成。

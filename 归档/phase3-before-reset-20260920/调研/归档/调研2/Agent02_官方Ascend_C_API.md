# Agent 2 专题报告：官方 Ascend C API 深度梳理与 CANN 9.0 规范

> 负责代理：Agent 2  
> 参考来源：华为昇腾官方文档中心、CANN 9.0.0 手册、Ascend C `kernel_operator.h` 头文件及 API 参考。

---

## 1. 核心数据结构与内存抽象

### 1.1 `GlobalTensor<T>`
- **原型与含义**：代表设备全局内存（Global Memory / HBM / DDR）中的一块连续物理空间。
  ```cpp
  template <typename T> class GlobalTensor;
  ```
- **核心方法**：
  - `void SetGlobalBuffer(__gm__ T* addr, uint32_t size)`：绑定 GM 指针与元素总数。
  - `GlobalTensor<T> operator[](uint32_t offset)`：产生带偏移的子张量引用（注意偏移以元素为单位）。
- **对齐要求**：基础地址通常推荐 32 字节对齐（512 位），在非 32B 连续存储时，普通 `DataCopy` 会产生寻址未对齐异常，需借助 `DataCopyPad` 处理。

### 1.2 `LocalTensor<T>`
- **原型与含义**：代表 AI Core 片上 Unified Buffer (UB) 中的局部张量。
  ```cpp
  template <typename T> class LocalTensor;
  ```
- **核心方法**：
  - `T GetValue(const uint32_t offset) const`：标量单元从 UB 读取单个元素（跨流水线，需注意同步）。
  - `void SetValue(const uint32_t offset, const T value)`：标量单元向 UB 写入单个元素。
  - `LocalTensor<U> ReinterpretCast<U>() const`：重新解释为另一种类型的 LocalTensor（如在复用 UB 空间时使用）。
- **约束**：LocalTensor 内部地址必须为 32 字节对齐。其生命周期由 `TQue` 或 `TBuf` 严格管理。

---

## 2. 管道资源与流水线调度 API

### 2.1 `TPipe`
- **原型与作用**：管理 AI Core 内部的数据传输流水线与 UB 缓冲区生命周期。
  ```cpp
  class TPipe;
  ```
- **核心方法**：
  - `void InitBuffer(TQue<...>& que, uint32_t num, uint32_t len)`：初始化队列与多缓冲空间（双缓冲 `num=2`，单缓冲 `num=1`）。
  - `void InitBuffer(TBuf<...>& buf, uint32_t len)`：初始化单张量共享临时缓冲。
  - `void* AllocTensor(...)` 与 `void FreeTensor(...)`。
- **关键陷阱（V001 编译失败根因）**：  
  在 Ascend C 的特定版本及底层 clang 编译器宏中，`pipe_` 属于内部关键字或宏定义别名。若将类成员变量命名为 `TPipe pipe_;`，编译器会报出：
  ```text
  unknown type name 'pipe_'; did you mean 'pipe_t'?
  cannot use dot operator on a type
  ```
  **规范修复**：必须命名为 `TPipe tpipe;` 或 `TPipe pipe;`，严禁使用末尾带单下划线的 `pipe_`。

### 2.2 `TQue<QuePosition, BUFFER_NUM>`
- **参数含义**：
  - `QuePosition`：队列所处的流水线阶段。例如输入队列 `QuePosition::VECIN`，输出队列 `QuePosition::VECOUT`。
  - `BUFFER_NUM`：缓冲阶数。`1` 为单缓冲阻塞流水，`2` 为 Ping-Pong 双缓冲（重叠搬运与计算）。
- **核心方法**：
  - `LocalTensor<T> AllocTensor<T>()`：从空闲缓冲分配 LocalTensor。
  - `void EnQue(LocalTensor<T>& tensor)`：将填充好的数据放入就绪队列。
  - `LocalTensor<T> DeQue<T>()`：从就绪队列取出可读写数据。
  - `void FreeTensor(LocalTensor<T>& tensor)`：释放缓冲给下一轮流水。

### 2.3 `PipeBarrier<PIPE_V>()` 与 `HardEvent` 硬件同步
- **流水线屏障**：`PipeBarrier<PIPE_V>()` 强制清空 Vector 计算单元的发射队列，确保当前所有矢量指令彻底执行完毕。
- **跨单元同步机制**：
  - Vector 与 Scalar 属于不同的硬件执行部件，二者读写相同片上 UB 时存在数据竞争（RAW/WAR）。
  - `SetFlag<HardEvent::V_S>(event_id)`：由 Vector 单元发射，通知标量单元矢量计算完成。
  - `WaitFlag<HardEvent::V_S>(event_id)`：由 Scalar 单元等待该标记，阻塞后续标量读取，防止读到脏数据。
  - 反向同步：Scalar 向 UB 写入数据后若供 Vector 读取，需配合 `HardEvent::S_V`。

---

## 3. 数据搬运 API

### 3.1 `DataCopy`
- **限制**：要求源地址与目的地址严格 32 字节对齐，且每次搬运长度必须是 32 字节的整数倍（即 $32 / \text{sizeof}(T)$ 个元素）。
- **适用场景**：$D$ 为 32 字节倍数，且行地址连续且对齐。

### 3.2 `DataCopyPad`（非对齐与尾块核心 API）
- **支持硬件**：Atlas A2 训练系列 / Atlas 800I A2 推理系列（即 `dav-2201`）完全支持。
- **搬入原型（GM $\to$ UB）**：
  ```cpp
  template <typename T>
  __aicore__ inline void DataCopyPad(
      const LocalTensor<T>& dst, 
      const GlobalTensor<T>& src, 
      const DataCopyExtParams& dataCopyParams, 
      const DataCopyPadExtParams<T>& padParams
  );
  ```
  - `DataCopyExtParams`：`blockCount=1`，`blockLen = valid_len * sizeof(T)`（精确到实际字节数）。
  - `DataCopyPadExtParams<T>`：`isPad=true`，`leftPadding=0`，`rightPadding = aligned_len - valid_len`，`paddingValue = 0`。
  - **重要特性**：在不足 32 字节时自动补 0，对均方和计算无任何污染。
- **搬出原型（UB $\to$ GM）**：
  ```cpp
  template <typename T>
  __aicore__ inline void DataCopyPad(
      const GlobalTensor<T>& dst, 
      const LocalTensor<T>& src, 
      const DataCopyExtParams& dataCopyParams
  );
  ```
  - **官方语义**：硬件自动丢弃尾部多余 padding 字节，实现任意字节长度的紧凑写回。
  - **真机风控点**：部分早期硬件微码可能存在以 32B burst 突发写回而覆盖相邻行的隐患，首版实现需预留手工尾块方案作为兜底。

---

## 4. 矢量计算 API

### 4.1 算术与仿射运算
```cpp
// 向量加法：dst = src0 + src1
template <typename T>
__aicore__ inline void Add(const LocalTensor<T>& dst, const LocalTensor<T>& src0, const LocalTensor<T>& src1, uint32_t calCount);

// 向量乘法：dst = src0 * src1
template <typename T>
__aicore__ inline void Mul(const LocalTensor<T>& dst, const LocalTensor<T>& src0, const LocalTensor<T>& src1, uint32_t calCount);

// 标量乘向量：dst = src * scalar
template <typename T>
__aicore__ inline void Muls(const LocalTensor<T>& dst, const LocalTensor<T>& src, const T scalar, uint32_t calCount);
```
- **关键约束**：在 Atlas A2 硬件上，Vector 单元内部的物理 ALU 对 `bfloat16_t` 的 `Add`/`Mul` 指令支持受限（硬件不支持原生的 bf16 矢量乘加计算）。**所有数学计算（平方和、求均值、归一化、仿射）必须先 Cast 到 `float` 域下完成**。

### 4.2 类型转换 `Cast`
```cpp
template <typename dst_T, typename src_T>
__aicore__ inline void Cast(
    const LocalTensor<dst_T>& dst, 
    const LocalTensor<src_T>& src, 
    const RoundMode mode, 
    uint32_t calCount
);
```
- **输入提升（半精度 $\to$ 单精度）**：
  - `half` / `bfloat16_t` $\to$ `float`：使用 `RoundMode::CAST_NONE`。这是严格精确无损的数学展开。
- **输出截断（单精度 $\to$ 半精度）**：
  - `float` $\to$ `half` / `bfloat16_t`：使用 `RoundMode::CAST_RINT`（Round to Nearest, ties to Even，四舍六入五成双）。
  - **与 numpy 对齐**：官方 `verify_result.py` 在计算 golden 时使用 numpy 的标准 IEEE-754 默认舍入，即 RNE。使用 `CAST_RINT` 能确保与 Python 端逐 bit 误差最小。

### 4.3 向量归约 `ReduceSum` 与 `GetReduceRepeatSumSpr`
```cpp
template <typename T>
__aicore__ inline void ReduceSum(
    const LocalTensor<T>& dstLocal, 
    const LocalTensor<T>& srcLocal, 
    const LocalTensor<T>& workLocal, 
    const int32_t count
);
```
- **参数规范**：
  - `dstLocal`：存储归约结果的目标 LocalTensor。在 Ascend C 规范中，即使仅输出 1 个标量和，由于 UB 的 32 字节对齐要求，`dstLocal` 必须分配至少 32 字节（例如 `float` 至少分配 8~16 个元素）。
  - `workLocal`：归约计算所需的临时工作区（在 CANN 9.0 部分文档中亦称为 `sharedTmpBuffer`）。官方公式要求容量为：
    $$\text{workLen} \ge \text{RoundUp}(\text{count} / 64, 8) \times 8$$
    对于单块 $4096$ 元素，$\text{workLen} = 512$ 个 float。本设计预留 $1024$ 个 float，空间完全充裕。
  - `count` 实际上限：官方手册指出受 UB 与 repeat 限制，最大 repeat 为 255 次。单次 `ReduceSum` 的 `count` 严禁超过 $4096$（防止出现未知硬件截断）。
- **结果读取的双路线**：
  - **路线 A（标量 GetValue 读取）**：
    ```cpp
    PipeBarrier<PIPE_V>();
    // 发起 V_S 同步，等待 Vector 写入完毕
    float tile_sum = dstLocal.GetValue(0);
    ```
  - **路线 B（ISASI 接口 `GetReduceRepeatSumSpr` 读取）**：
    ```cpp
    PipeBarrier<PIPE_V>();
    float tile_sum = GetReduceRepeatSumSpr<float>();
    ```
    `GetReduceRepeatSumSpr` 直接从硬件专用状态寄存器（SPR）提取最近一次归约完成的累加值，可省去一次显式的 UB 寻址与局部缓存加载。

---

## 5. 多核索引与并行分配 API

- `GetBlockIdx()`：获取当前执行 Core 的物理逻辑编号（从 $0$ 到 $\text{GetBlockNum}() - 1$）。
- `GetBlockNum()`：获取本次算子调用分配的 Vector Core 总数（在直调模板中由 `availableCoreNum` 决定）。
- **多核按行均分算法（防负载倾斜）**：
  ```cpp
  uint32_t block = GetBlockIdx();
  uint32_t blocks = GetBlockNum();
  uint32_t each = outer / blocks;
  uint32_t extra = outer % blocks;
  // 前 extra 个 Core 分配 each + 1 行，其余 Core 分配 each 行
  uint32_t first = block * each + (block < extra ? block : extra);
  uint32_t count = each + (block < extra ? 1 : 0);
  ```
  该算法确保每核处理的样本行数之差不超过 1 行，避免长尾核造成的同步等待。

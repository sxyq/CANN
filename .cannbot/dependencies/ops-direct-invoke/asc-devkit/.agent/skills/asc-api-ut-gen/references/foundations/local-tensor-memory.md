# LocalTensor 内存申请指南

本文档整理了 AscendC 中 LocalTensor 内存申请的关键要点，帮助开发者在编写 UT 时正确管理内存。

---

## 1. 概述

LocalTensor 是 AscendC 中用于表示片上内存（Unified Buffer、L1 等）的张量类型。获取 LocalTensor 需要先通过 `TPipe` 或 `TBufPool` 分配内存给 `TQue` 或 `TBuf`，再从中获取。

---

## 2. 内存分配方式

### 2.1 TPipe::InitBuffer（推荐）

通过 `TPipe` 对象为 `TQue` 或 `TBuf` 分配内存。

#### 为 TQue 分配内存

```cpp
AscendC::TPipe pipe;
AscendC::TQue<AscendC::TPosition::VECOUT, 2> que;

// 分配 2 块内存，每块 128 字节
// num=1 不开启 double buffer；num=2 开启 double buffer
uint8_t num = 2;
uint32_t len = 128;
pipe.InitBuffer(que, num, len);
```

#### 为 TBuf 分配内存

```cpp
AscendC::TPipe pipe;
AscendC::TBuf<AscendC::TPosition::VECCALC> tmpBuf;

uint32_t len = 128;
pipe.InitBuffer(tmpBuf, len);
```

#### 自定义地址分配

```cpp
AscendC::TPipe pipe;
AscendC::TQue<AscendC::TPosition::VECOUT, 1> que;

// 自定义内存地址信息：[起始地址, 长度]
auto addr0 = Std::make_tuple(0, 1024);
auto addr1 = Std::make_tuple(2048, 2048);
auto addr2 = Std::make_tuple(8192, 4096);
pipe.InitBuffer(que, addr0, addr1, addr2);
```

### 2.2 TBufPool::InitBuffer

通过 `TBufPool` 对象分配内存，适用于需要精细控制 Buffer 数量的场景。

```cpp
// TBufPool 分配
// bufIDSize 默认上限为 4，最大为 16
TBufPool<TPosition::VECIN, 8> bufPool;  // 最多 8 个 Buffer
bufPool.InitBuffer(que, num, len);
bufPool.InitBuffer(buf, len);
```

---

## 3. 获取 LocalTensor

### 3.1 从 TQue 获取

```cpp
// 申请 Tensor
LocalTensor<T> tensor = que.AllocTensor<T>();

// 使用后释放
que.EnQue(tensor);  // 先入队
que.DeQue<T>();     // 再出队使用
que.FreeTensor(tensor);  // 最后释放
```

### 3.2 从 TBuf 获取

```cpp
// 直接获取（无需手动释放）
LocalTensor<uint8_t> tmpLocal = tmpBuf.Get<uint8_t>();
```

---

## 4. 重要约束

### 4.1 内存对齐

| 约束项 | 说明 |
|--------|------|
| **32 字节对齐** | `len` 参数非 32Bytes 对齐时，会**自动向上补齐至 32Bytes** |
| **元素对齐** | 确保 count * sizeof(T) 满足对齐要求 |

基础 dtype 的 `sizeof` 与 32B 对齐元素数统一见 [`asc-npu-arch` 架构指南](../../../asc-npu-arch/references/npu-arch-guide.md#基础-dtype-大小与-32b-对齐)；本 guide 只保留 LocalTensor 内存申请相关约束。

### 4.2 Buffer 数量限制

| 限制类型 | 数量限制 |
|----------|----------|
| **TPipe 方式** | 一个 kernel 中所有 Buffer 数量之和**不能超过 64** |
| **TBufPool 方式** | `bufIDSize` 默认上限为 4，最大为 16 |

### 4.3 Double Buffer

```cpp
// num=1: 不开启 double buffer
pipe.InitBuffer(que, 1, len);

// num=2: 开启 double buffer，可提高流水线效率
pipe.InitBuffer(que, 2, len);
```

---

## 5. 生命周期管理

### 5.1 自动释放

```cpp
// InitBuffer 申请的内存会在 TPipe 对象销毁时自动释放
// 无需手动释放
{
    TPipe pipe;
    pipe.InitBuffer(que, num, len);
    // ... 使用 que
}  // pipe 析构时自动释放内存
```

### 5.2 重新分配

```cpp
// 如需重新分配内存，需先调用 Reset
pipe.Reset();
pipe.InitBuffer(que, newNum, newLen);
```

---

## 6. TPosition 位置选择

`TPosition` 决定了 LocalTensor 所在的物理存储位置：

| 位置 | 适用场景 | 说明 |
|------|----------|------|
| **VECIN** | 矢量编程输入 | Vector Core 输入数据 |
| **VECOUT** | 矢量编程输出 | Vector Core 输出数据 |
| **VECCALC** | 矢量计算临时空间 | 用于临时变量，别名 LCM |
| **A1/A2** | 矩阵编程 | A 矩阵 L1 缓冲区 |
| **B1/B2** | 矩阵编程 | B 矩阵 L1 缓冲区 |
| **C1/C2** | 矩阵编程 | C 矩阵 L1 缓冲区 |
| **CO1/CO2** | 矩阵编程 | C 矩阵输出缓冲区 |

**选择原则**：
- 矢量计算 API（Add, Mul 等）：使用 `VECIN`/`VECOUT`/`VECCALC`
- 矩阵计算 API（Mmad 等）：使用 `A1`/`B1`/`C1` 等

---

## 7. UT 编写示例

### 7.1 TmpBuffer 申请示例

```cpp
template <typename T>
class KernelAdd {
public:
    __aicore__ KernelAdd() {}
    __aicore__ void Process(GM_ADDR x, GM_ADDR y, GM_ADDR z, uint32_t dataSize) {
        // 初始化内存管理
        pipe.InitBuffer(inQueueX, 1, dataSize * sizeof(T));
        pipe.InitBuffer(inQueueY, 1, dataSize * sizeof(T));
        pipe.InitBuffer(outQueue, 1, dataSize * sizeof(T));

        // 获取 Global Tensor
        GlobalTensor<T> gmX = GM_ADDR_TO_TENSOR(x, T);
        GlobalTensor<T> gmY = GM_ADDR_TO_TENSOR(y, T);
        GlobalTensor<T> gmZ = GM_ADDR_TO_TENSOR(z, T);

        // 分配 Local Tensor
        LocalTensor<T> xLocal = inQueueX.AllocTensor<T>();
        LocalTensor<T> yLocal = inQueueY.AllocTensor<T>();
        LocalTensor<T> zLocal = outQueue.AllocTensor<T>();

        // 拷入数据
        DataCopy(xLocal, gmX, dataSize);
        DataCopy(yLocal, gmY, dataSize);

        // 计算
        Add(zLocal, xLocal, yLocal, dataSize);

        // 拷出数据
        DataCopy(gmZ, zLocal, dataSize);

        // 释放
        inQueueX.FreeTensor(xLocal);
        inQueueY.FreeTensor(yLocal);
        outQueue.FreeTensor(zLocal);
    }

private:
    TPipe pipe;
    TQue<TPosition::VECIN, 1> inQueueX, inQueueY;
    TQue<TPosition::VECOUT, 1> outQueue;
};
```

### 7.2 需要临时空间的 API

```cpp
template <typename T>
__aicore__ void ProcessWithTmpBuffer(GM_ADDR x, GM_ADDR y, uint32_t dataSize) {
    TPipe pipe;

    // 主数据队列
    TQue<TPosition::VECIN, 1> inQueue;
    TQue<TPosition::VECOUT, 1> outQueue;

    // 临时缓冲区
    TBuf<TPosition::VECCALC> tmpBuf;

    // 根据数据类型确定临时空间大小
    uint32_t tmpSize;
    if constexpr (sizeof(T) == sizeof(half)) {
        tmpSize = dataSize * 8;  // half 类型需要更大临时空间
    } else {
        tmpSize = dataSize * 4;  // float 类型
    }

    pipe.InitBuffer(inQueue, 1, dataSize * sizeof(T));
    pipe.InitBuffer(outQueue, 1, dataSize * sizeof(T));
    pipe.InitBuffer(tmpBuf, tmpSize);

    // 获取临时 LocalTensor
    LocalTensor<uint8_t> tmpLocal = tmpBuf.Get<uint8_t>();

    // 使用 tmpLocal 进行计算...
}
```

---

## 8. 常见问题

LocalTensor / TPipe 的公共排障项统一归档到 [常见问题与解决方案](../troubleshooting/faq.md)：

- [数据对齐与内存大小计算](../troubleshooting/faq.md#8-数据对齐与内存大小计算)
- [LocalTensor / TPipe Buffer 初始化问题](../troubleshooting/faq.md#9-localtensor--tpipe-buffer-初始化问题)
- [TmpBuffer / workspace 大小与分支覆盖](../troubleshooting/faq.md#6-tmpbuffer--workspace-大小与分支覆盖)

---

## 9. 参考文档

- [TPipe::InitBuffer](https://www.hiascend.com/document/detail/zh/canncommercial/850/API/ascendcopapi/atlasascendc_api_07_0110.html)
- [TBufPool::InitBuffer](https://www.hiascend.com/document/detail/zh/canncommercial/850/API/ascendcopapi/atlasascendc_api_07_0125.html)
- [TPosition 枚举](https://www.hiascend.com/document/detail/zh/canncommercial/850/API/ascendcopapi/atlasascendc_api_07_0174.html)

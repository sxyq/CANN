# C++ Tensor编程概述

C++ Tensor编程是Ascend C SIMD编程路径的基础编程接口，基于Tensor对象抽象封装内存地址，提供类型安全、编译期检查的高层次编程能力。开发者通过LocalTensor、GlobalTensor、RegTensor等Tensor对象管理内存，无需直接操作指针地址，搬运计算接口直接操作Tensor对象即可完成数据流转。该路径适用于追求开发效率、类型安全、代码可维护性的算子开发场景，是习惯C++面向对象编程的开发者快速上手NPU算子开发的关键技术路径。本章旨在帮助开发者建立对C++ Tensor编程的系统化认知，理解其Tensor抽象编程范式、核心编程模型与开发方法论。

## 核函数（Kernel）

[核函数（Kernel）](../kernel_function.md)在前序章节已作介绍，C++ Tensor编程在核函数（Kernel）定义与调用上沿用SIMD编程通用规范，无额外语法约束。

## 内存层级

AI Core采用分级存储架构，Cube矩阵计算采用「Global Memory（以下简称为GM） → L1/L0系列Buffer」两级层级服务于矩阵计算，Vector矢量计算传统架构为「Global Memory → UB」两级层级，而[NPU架构版本3510](../../../language_extension/simd_builtin_keywords.md)引入寄存器后构建「GM → UB → Register」三级层级，其中GM存放输入输出数据、Unified Buffer（UB）作为矢量计算中间缓存、Register位于Vector计算单元最内层直接与执行单元交互。

![AI Core存储层次结构](../../../../figures/aicore_mem_2.png)

### Tensor内存抽象

Tensor本质是基于数组的编程抽象，而非简单的内存地址封装，通过数组方式管理内存并执行计算，更匹配SIMD硬件执行模型。其核心思想是将原始内存地址抽象为Tensor对象，实现从“地址操作”到“对象管理”的编程层次提升。

根据芯片分级存储架构，Tensor分为三类：**GlobalTensor**对应GM，存放算子输入/输出数据；**LocalTensor**对应片上内存（UB、L1 Buffer、L0A Buffer/L0B Buffer/L0C Buffer等缓冲区），承载计算过程数据；**RegTensor**对应寄存器，用于寄存器级矢量计算（[NPU架构版本3510](../../../language_extension/simd_builtin_keywords.md)引入）。Tensor体系持续演进：基础Tensor仅携带数据指针和大小信息，需手动传递操作参数；扩展Tensor（Tensor API）引入Layout概念，携带Shape和Stride信息，接口可自动推导搬运长度、计算元素数等参数，大幅降低编程复杂度。该能力引入的接口统一通过`asc::te`命名空间调用（如`asc::te::mmad`）。

#### 基础Tensor与扩展Tensor抽象定义

- 基础Tensor（不带Layout）仅封装数据指针与大小信息，无法表达数据的物理排布方式。开发者在执行搬运和计算时必须手动传入size/count参数、自行计算偏移与步长，面对高维张量时易出错且代码冗长。

    ```cpp
    __global__ __vector__ void add_custom(__gm__ float* x, __gm__ float* y, __gm__ float* z) {
      AscendC::GlobalTensor<float> xGm;
      xGm.SetGlobalBuffer(x + block_idx * blockLength, blockLength);

      AscendC::LocalMemAllocator<AscendC::Hardware::UB> ubAllocator;
      AscendC::LocalTensor<float> xLocal = ubAllocator.Alloc<float, blockLength>();

      // Data copy and computation require manual size parameter
      AscendC::DataCopy(xLocal, xGm, blockLength);
      // yLocal, zLocal的定义、搬运等跟xLocal保持一致，此处省略
      ...
      AscendC::Add(zLocal, xLocal, yLocal, blockLength);
    }
    ```

- 扩展Tensor（Tensor API）封装Shape和Stride信息，接口自动推导操作参数，实现编译期类型安全。开发者无需关注内存布局细节，一行代码即可完成切片、搬运与计算。当前能力赋能于Cube矩阵计算，后续将逐步支持Vector矢量计算。

    ```cpp
    #include "tensor_api/tensor.h"

    using namespace asc::te;

    __global__ __cube__ void matmul_custom(__gm__ half* a, __gm__ half* b, __gm__ half* c) {
        // 实现代码示例，其中m, n, k为单核上GM上左右矩阵的大小, (base_m, k), (k, base_n)为搬运到L1上左右矩阵的大小
        // 扩展Tensor（Tensor API）携带Layout信息，搬运接口自动推导参数
        auto gm_a_tensor = make_tensor(make_mem_ptr(a), make_frame_layout<nd_ext_layout_ptn>(m, k));
        auto gm_b_tensor = make_tensor(make_mem_ptr(b), make_frame_layout<nd_ext_layout_ptn>(k, n));
        ...
        // __cbuf__ 为L1 Buffer的声明修饰符
        __cbuf__ half l1_a_buf[1024];
        __cbuf__ half l1_b_buf[1024];
        auto l1_a_tensor = make_tensor(make_mem_ptr(l1_a_buf), make_frame_layout<nz_layout_ptn>(base_m, K));
        auto l1_b_tensor = make_tensor(make_mem_ptr(l1_b_buf), make_frame_layout<nz_layout_ptn>(k, base_n));
        ...
        // 一行代码完成搬运，无需手动传入size参数
        auto copy_gm_to_l1_atom = make_copy(copy_gm_to_l1{}, gm_to_l1_trait_default{});
        copy(copy_gm_to_l1_atom, l1_a_tensor, gm_a_tensor);
        copy(copy_gm_to_l1_atom, l1_b_tensor, gm_b_tensor);
    }
    ```

### 外部存储

前文的Tensor抽象示例中已展示了GlobalTensor和LocalTensor的基本用法。下面从硬件存储层级的角度，进一步介绍外部存储和内部存储的特性及Tensor管理方式。
GM是昇腾NPU的设备内存，位于AI Core外部，容量大、带宽高但访问延迟较长。用于存储算子的输入数据、输出结果和中间计算数据。GM具有持久性：其中存储的数据持续保留，直到内存空间被释放或应用程序终止。
用户通过Runtime API管理Device侧全局内存。Host侧使用`aclrtMalloc`分配Device内存，通过`aclrtMemcpy`完成Host与Device间的数据拷贝，通过`<<<>>>`拉起核函数（Kernel）执行，最后使用`aclrtFree`释放内存。有关Runtime API的详细信息，可参考[《Runtime运行时API》](https://hiascend.com/document/redirect/CannCommunityRuntimeApi)。

如下示例展示了在Host侧通过Runtime API管理分配Device内存、在Host与Device之间数据拷贝以及核函数（Kernel）的调用过程：
```cpp
// Kernel function definition (using __global__ prefix to indicate Host call, Device execution)
__global__ __vector__ void add_custom(__gm__ uint8_t* x, __gm__ uint8_t* y, __gm__ uint8_t* z)
{
    AscendC::GlobalTensor<float> xGm;
    xGm.SetGlobalBuffer(reinterpret_cast<__gm__ float*>(x) + block_idx * blockLength, blockLength);
    ...
}

// Host side: allocate Device memory, copy data, launch kernel function
std::vector<float> kernel_add(std::vector<float> &x, std::vector<float> &y) {
    uint32_t totalLength = x.size();
    uint32_t byteSize = totalLength * sizeof(float);
    uint8_t *xHost = reinterpret_cast<uint8_t *>(x.data());
    uint8_t *yHost = reinterpret_cast<uint8_t *>(y.data());

    void *xDevice = nullptr;
    void *yDevice = nullptr;
    void *zDevice = nullptr;
    aclrtStream stream = nullptr;

    // Allocate Device memory
    aclrtMalloc(&xDevice, byteSize, ACL_MEM_MALLOC_HUGE_FIRST);
    aclrtMalloc(&yDevice, byteSize, ACL_MEM_MALLOC_HUGE_FIRST);
    aclrtMalloc(&zDevice, byteSize, ACL_MEM_MALLOC_HUGE_FIRST);
    aclrtCreateStream(&stream);

    // Host to Device data copy
    aclrtMemcpy(xDevice, byteSize, xHost, byteSize, ACL_MEMCPY_HOST_TO_DEVICE);
    aclrtMemcpy(yDevice, byteSize, yHost, byteSize, ACL_MEMCPY_HOST_TO_DEVICE);

    // Launch kernel function
    uint32_t numBlocks = 1;
    add_custom<<<numBlocks, 0, stream>>>((uint8_t*)xDevice, (uint8_t*)yDevice, (uint8_t*)zDevice);
    aclrtSynchronizeStream(stream);

    // Device to Host data copy
    std::vector<float> z(totalLength);
    aclrtMemcpy(z.data(), byteSize, zDevice, byteSize, ACL_MEMCPY_DEVICE_TO_HOST);

    // Free memory
    aclrtFree(xDevice);
    aclrtFree(yDevice);
    aclrtFree(zDevice);
    aclrtDestroyStream(stream);

    return z;
}
```
GM作为大容量全局内存，用于Host与Device交互及算子输入输出数据存储，但访问延迟较长，无法满足AI Core低延迟计算需求。所有Cube/Vector计算必须将数据预加载至片上内部存储，下面分UB、寄存器、L1/L0缓冲区介绍内部存储Tensor管理方式。

### 内部存储

AI Core包含三类计算单元：Cube负责矩阵计算、Vector负责矢量计算、Scalar负责标量计算。计算前需将数据从外部存储加载到内部存储，不同存储单元服务不同计算单元：UB支撑矢量计算，L1 Buffer和L0A/L0B Buffer支撑矩阵计算。

#### AIV内部存储之UB

UB是AI Core内主要用于矢量计算的内部存储单元，位于每个Vector Core（AIV）内部。容量较小但带宽高、延迟低，可视为内核执行期间由用户管理的高速缓存。

在C++ Tensor编程中，通过LocalTensor对象管理UB内存，其中对于基础Tensor（不带Layout）而言，主要通过`LocalMemAllocator`指定物理存储位置和内存大小来创建对应的Tensor。LocalMemAllocator是一种线性内存分配器，开发者可以调用Alloc方法进行内存分配，根据调用次序依次向后进行线性分配，但该分配器不提供内存释放以及其它内存管理的能力。

```cpp
__global__ __vector__ void add_custom(__gm__ uint8_t* x, __gm__ uint8_t* y, __gm__ uint8_t* z)
{
    // Basic Tensor: allocate through Allocator
    AscendC::LocalMemAllocator<AscendC::Hardware::UB> ubAllocator;
    AscendC::LocalTensor<half> ubTensor = ubAllocator.Alloc<half, 256>();
}
```

在初始功能开发时，可以使用LocalMemAllocator简化算子编写。如需更精细的内存控制（如规避Bank冲突、内存复用等），可通过LocalTensor直接指定地址进行管理。使用时需合理规划地址分配，确保不超过物理存储上限，并在保证功能正确的前提下进行内存复用。

```cpp
__global__ __vector__ void add_custom(__gm__ uint8_t* x, __gm__ uint8_t* y, __gm__ uint8_t* z)
{
  // 开发者手动指定x, y, z的地址
  uint64_t xAddrPing = 0;
  uint64_t yAddrPing = 1024;
  uint64_t zAddrPing = 2048;

  // 通过构造函数指定物理存储位置和地址、大小等构造LocalTensor
  AscendC::LocalTensor<float> xLocalPing(AscendC::TPosition::VECCALC, xAddrPing, 256);
  AscendC::LocalTensor<float> yLocalPing(AscendC::TPosition::VECCALC, yAddrPing, 256);
  AscendC::LocalTensor<float> zLocalPing(AscendC::TPosition::VECCALC, zAddrPing, 256);
}
```
> 📌 其中AscendC::TPosition::VECCALC为对物理位置的逻辑抽象定义，与物理位置的映射参考文件[逻辑位置和物理存储的映射](../../../../../api/SIMD-API/general_description_and_constraints.md)。

相比基础Tensor，[NPU架构版本3510](../../../language_extension/simd_builtin_keywords.md)引入Layout描述并将其作为Tensor的核心属性，开发者可在Tensor创建时直接关联Shape与Stride信息，简化多维数据的布局管理。通过`make_tensor`，开发者可灵活指定内存地址和Layout布局，更便捷地描述ND、NZ等典型数据排布。

```cpp
// UB buffer declaration
__ubuf__ half ub_buffer[1024];
auto ub_ptr = make_mem_ptr(ub_buffer);
// Extended Tensor: create through make_tensor
auto ub_tensor = make_tensor(ub_ptr, make_layout(make_shape(128, 64)));
```
> 📌 UB地址必须32字节对齐，以匹配硬件总线粒度和SIMD计算单元并行度，非对齐访问将导致运行时错误。

#### AIV内部存储之Register File

[NPU架构版本3510](../../../language_extension/simd_builtin_keywords.md)引入寄存器可编程能力，Vector计算单元的所有计算均需经「GM → UB → Register」三级数据流转，在寄存器上完成矢量计算。C++ Tensor编程通过定义RegTensor方式进行寄存器编程。详见[Reg矢量计算编程](./reg_vector_computation.md)。

> 📌 寄存器可编程能力从[NPU架构版本3510](../../../language_extension/simd_builtin_keywords.md)引入。

#### AIC内部存储之L1 Buffer & L0 Buffer

Cube计算单元专用的缓存为：L0A Buffer存储左矩阵，L0B Buffer存储右矩阵，L0C Buffer存储初始累加值及矩阵计算结果。L1 Buffer用于缓存矩阵计算输入数据。

通过`LocalTensor`对象管理这些存储单元：

| 存储单元 | 作用 | 示例 |
|---------|------|------|
| L1 Buffer | 缓存矩阵计算输入数据 | `LocalTensor<half> l1_buffer;` |
| L0A Buffer | 存储左矩阵 | `LocalTensor<half> l0a_buffer;` |
| L0B Buffer | 存储右矩阵 | `LocalTensor<half> l0b_buffer;` |
| L0C Buffer | 存储计算结果 | `LocalTensor<float> l0c_buffer;` |
| Fixpipe Buffer | 存储量化参数 | `LocalTensor<half> fixpipe_buffer;` |
| BiasTable Buffer | 存储Bias数据 | `LocalTensor<half> bias_buffer;` |

同UB的申请方式，开发者可通过LocalMemAllocator或直接构造LocalTensor来申请L1/L0 Buffer内存。为提升Cube计算效率，L1 Buffer和L0 Buffer采用分形格式。不同产品型号的分形格式存在差异：

| 存储单元 | [NPU架构版本2201](../../../language_extension/simd_builtin_keywords.md) | [NPU架构版本3510](../../../language_extension/simd_builtin_keywords.md) |
|---------|----------------------------|---------------------------|
| L1 Buffer | Nz | Nz |
| L0A Buffer | Zz | Nz |
| L0B Buffer | Zn | Zn |
| L0C Buffer | Nz | Nz |

以`FRACTAL_Nz`为例，内存排布采用大N外部column major（列主序）+ 小z内部row major（行主序）存放：

![FRACTAL_Nz内存排布示意图](../../../../figures/nz_layout.png)

#### Bank冲突

为了提高数据访问的效率和吞吐量，内部存储采用了bank（大小相等的内存模块）结构设计。以UB为例，[NPU架构版本3510](../../../language_extension/simd_builtin_keywords.md)产品上，UB总大小为256KB（256 × 1024字节），包含8个bank group，每个bank group包含2个bank。每个bank大小为16KB，由512行组成，每行长度为32B，采用低位地址交织。[NPU架构版本2201](../../../language_extension/simd_builtin_keywords.md)的UB容量规格请参考[UB bank结构](../../../../operator_practice/simd_operator_optimization/memory_access/avoid_ub_bank_conflict/avoid_bank_conflict_npu_arch_2201.md)。

![UB Bank冲突示意图](../../../../figures/ub_conflict.png)

该架构地址使用低位编址，地址排布如下图所示，每个bank可以独立地进行数据的读写操作，允许多个数据请求同时进行。然而，当多个读写操作试图同时访问同一个bank时，由于硬件资源的限制，这些操作必须排队等待，会导致bank冲突，引起性能下降。

![低位交织地址排布](../../../../figures/low_interleave.png)

每个bank一拍只能完成一读或者一写，每个bank group最多只允许2读0写或者1读1写。根据内存结构，bank冲突主要可以分为以下三种类型：
- 读写冲突：读操作和写操作同时尝试访问同一个bank，或者两个读操作和一个写操作同时尝试访问同一个bank group。
- 写写冲突：多个写操作同时尝试访问同一个bank group。
- 读读冲突：两个读操作访问同一个bank，或两个以上读操作同时尝试访问同一个bank group。

以读写冲突为例，当进行一组`dst = src0 + src1`的矢量加法计算时，如果`src0`的起始地址在0x0000，`dst`的起始地址为0x4000，结合Bank的排布，可以看到当前`src0`和`dst`的地址分别属于同一个bank和同一个bank group，则存在读写冲突。

当遇到Bank冲突时，会引起算子性能的下降，通常可以通过优化计算逻辑和优化分配地址方式来避免冲突的产生，详情请参考[避免Bank冲突](../../../../operator_practice/simd_operator_optimization/memory_access/avoid_ub_bank_conflict/avoid_bank_conflict_npu_arch_3510.md)。

合理规划内存地址可规避Bank冲突、提升访存吞吐，但AI Core内DMA、Vector、Cube多流水异步执行，数据读写存在强依赖。需要配套同步机制管控多流水线执行时序，避免脏读、计算错乱。

## 同步机制

前文介绍了AI Core的多级存储与计算单元架构。由于DMA搬运、矢量计算、矩阵计算等单元均为异步并行执行，数据读写、计算操作存在强依赖，因此必须依靠同步机制管控执行时序，保证程序逻辑正确。在编程场景中，同步是指协调多个执行单元（如线程、进程等）对共享资源的访问顺序和时机，以保障程序执行结果的正确性。若未通过同步机制严格控制执行顺序，会导致计算结果不一致、程序行为不可预测等问题。

AI Core的同步主要分为**核内同步**和**核间同步**。

### 核内同步

AI Core内部的执行单元（如MTE2搬运单元、Vector计算单元等）以异步并行的方式运行，在读写同一存储资源时可能存在数据依赖关系。为确保数据一致性及计算正确性，需通过同步控制协调操作时序。

下图示例描述了一个常见的Vector计算数据流：

1. 通过DMA将数据从GM搬入到Local Memory
2. 进行矢量计算
3. 通过DMA将数据从Local Memory搬出到GM

![Vector计算数据流](../../../../figures/vec_flow.png)

四个执行单元Scalar、Vector、DMA（MTE2）、DMA(MTE3)并行执行，若访问同一片Local Memory，需要同步机制来控制它们的访问时序：保证先搬入Local Memory后再计算，计算完成后再搬出。

![同步顺序示意图](../../../../figures/sync_order.png)

> 📌 提示：上例描述的是主要路径，当`Scalar`执行单元读写GM或者UB时，也需要考虑`Scalar`流水与其他流水的同步。

AI Core上的执行单元分别属于不同的执行流水，同步即是保证不同执行流水间能够正确执行，如下为AI Core内的主要执行流水。

| 流水类型 | 含义 |
|---------|------|
| **PIPE_S** | 标量流水线 |
| **PIPE_V** | 矢量计算流水及部分硬件架构下的L0C Buffer→UB数据搬运流水 |
| **PIPE_M** | 矩阵计算流水 |
| **PIPE_MTE1** | L1 Buffer→L0A Buffer、L1 Buffer→L0B Buffer数据搬运流水 |
| **PIPE_MTE2** | GM→L1 Buffer、GM→UB等数据搬运流水 |
| **PIPE_MTE3** | UB→GM、L1 Buffer→GM数据搬运流水 |
| **PIPE_FIX** | L0C Buffer→GM、L0C Buffer→L1等数据搬运流水 |

为确保流水线间正确执行，AI Core针对不同应用场景提供了多种同步机制，开发者可根据实际需求灵活选择合适的同步方式来实现算子功能。

| 同步类型 | 核心作用及场景说明 | 关键接口 | 实现代码示例 |
|---------|-------------------|----------|-------------|
| **多流水同步（机制一）** | Mutex互斥锁用于核内异步流水间的同步，功能类似传统CPU锁机制。[NPU架构版本3510](../../../language_extension/simd_builtin_keywords.md)新引入，比SetFlag/WaitFlag更简便。 | `Mutex::Lock` & `Mutex::Unlock` | `AscendC::Mutex::Lock<PIPE_MTE2>(mutexId);`
`AscendC::DataCopy(srcLocal, srcGlobal, dataSize);`
`AscendC::Mutex::Unlock<PIPE_MTE2>(mutexId);`
`AscendC::Mutex::Lock<PIPE_V>(mutexId);`
`AscendC::Add(dstLocal, src0Local, src1Local, dataSize);`
`AscendC::Mutex::Unlock<PIPE_V>(mutexId);` |
| **多流水同步（机制二）** | SetFlag与WaitFlag实现两条流水间同步等待。引入Mutex后仍保持兼容，但优先推荐Mutex机制。 | `SetFlag` & `WaitFlag` | `AscendC::DataCopy(srcLocal, srcGlobal, dataSize);`
`AscendC::SetFlag<HardEvent::MTE2_V>(eventId);`
`AscendC::WaitFlag<HardEvent::MTE2_V>(eventId);`
`AscendC::Add(dstLocal, src0Local, src1Local, count);` |
| **单流水同步** | PipeBarrier等待同一流水内前序指令完成，用于同一流水内的前后数据依赖。 | `PipeBarrier` | `AscendC::Add(dstLocal, src0Local, src1Local, count);`
`AscendC::PipeBarrier<PIPE_V>();`
`AscendC::Sub(dstLocal, dstLocal, src0Local, count);` |
| **内存访问顺序控制** | 等待所有前序内存访问完成，用于存在数据依赖或内存依赖的场景。 | `DataSyncBarrier` | `AscendC::DataSyncBarrier<AscendC::MemDsbT::DDR>();` |

### 核间同步

当一个核计算所依赖的数据，需要等待其他相关核的计算全部完成后才能执行时，则需要通过核间同步来确保多核间的时序正确性。

以下图为例，AIC（AI Cube，主要用于Cube计算）需要依赖于AIV（AI Vector，负责Vector计算）的ReduceSum计算结果。由于整体矢量较大，必须拆分为多个部分，由每个AIV分别完成部分计算。每个AIV将其部分计算结果通过原子累加写入GM。AIC需要读取的必须是所有AIV均完成累加后的最终结果。

![核间同步业务场景示例](../../../../figures/inter_core_sync_ex.png)

从上图可以看到，当前AI Core是由AIC核和AIV核组成，其中AIC核主要用于Cube计算，AIV核负责Vector计算。AIC/AIV按group进行划分。一个group内细分block及subblock，block与subblock比例为1:N（N>=1），block表示多少个“主核”，subblock表示一个“主核”带多少个“从核”。

![block和subblock之间关系](../../../../figures/block_rel.png)

算子按计算特征可划分为三类：Cube算子（矩阵计算）、Vector算子（矢量计算）和CV融合算子（矩阵与矢量混合计算）。算子类型决定了其核间同步方式与group配置模式的选择。针对不同算子场景，C++ Tensor编程通过[CrossCoreSetFlag](../../../../../api/SIMD-API/basic_api/sync_control/inter_core_sync/CrossCoreSetFlag_ISASI.md)和[CrossCoreWaitFlag](../../../../../api/SIMD-API/basic_api/sync_control/inter_core_sync/CrossCoreWaitFlag_ISASI.md)两个接口组合实现核间同步，以满足多样化的算子开发需求。

| 核间同步类型 | 场景说明 | 实现代码示例 |
|---|---|---|
| **所有AIC核同步** | 同步所有AIC核，通常用于对Cube算子计算结果进行多核累加操作。 | `if ASCEND_IS_AIC {`
`    AscendC::CrossCoreSetFlag<0x0, PIPE_FIX>(flagID);`
`    AscendC::CrossCoreWaitFlag<0x0>(flagID);`
`}` |
| **所有AIV核同步** | 同步所有AIV核，通常用于对Vector算子计算结果进行多核累加操作。 | `if ASCEND_IS_AIV {`
`    AscendC::CrossCoreSetFlag<0x0, PIPE_MTE3>(flagID);`
`    AscendC::CrossCoreWaitFlag<0x0>(flagID);`
`}` |
| **单个AI Core内两个AIV子核之间同步** | 同步单个AI Core内两个AIV子核，通常用于Vector算子内需要分组进行同步的场景。 | `if ASCEND_IS_AIV {`
`    AscendC::CrossCoreSetFlag<0x1, PIPE_MTE3>(flagID);`
`    AscendC::CrossCoreWaitFlag<0x1>(flagID);`
`}` |
| **单个AI Core内AIC与AIV同步** | 同步单个AI Core内AIC与AIV核，通常用于CV融合算子中Cube与Vector间的交互操作。AIC触发两个AIV同时等待。 | `if ASCEND_IS_AIC {`
`    AscendC::CrossCoreSetFlag<0x2, PIPE_FIX>(0);`
`}`
`if ASCEND_IS_AIV {`
`    AscendC::CrossCoreWaitFlag<0x2>(0);`
`}` |
| **单个AI Core内AIC与AIV同步（[NPU架构版本3510](../../../language_extension/simd_builtin_keywords.md)新引入）** | 同步单个AI Core内AIC与AIV核，通常用于CV融合算子中Cube与Vector间的交互操作。AIV0与AIV1可单独触发AIC等待。 | `if ASCEND_IS_AIC {`
`    AscendC::CrossCoreSetFlag<0x4, PIPE_FIX>(0);`
`    AscendC::CrossCoreSetFlag<0x4, PIPE_FIX>(16);`
`}`
`if ASCEND_IS_AIV {`
`    AscendC::CrossCoreWaitFlag<0x4>(0);`
`}` |

> 📌 基于分离模式（AIC和AIV分离）开发融合算子时，算子逻辑中通常同时包含AIV和AIC的处理逻辑，此时需要使用Ascend C提供的宏ASCEND_IS_AIV/ASCEND_IS_AIC来对AIV和AIC的代码进行隔离。
>
> 上表中`CrossCoreSetFlag`的第一个模板参数为同步目标标识，不同取值对应不同的同步范围：`0x0`表示所有核同步，`0x1`表示单个AI Core内AIV子核间同步，`0x2`表示单个AI Core内AIC与AIV同步（两个AIV同时触发），`0x4`表示单个AI Core内AIC与AIV同步（AIV0与AIV1可单独触发，[NPU架构版本3510](../../../language_extension/simd_builtin_keywords.md)新引入）。

## 算子类型

结合前文介绍的AI Core计算单元、多级内存架构与同步机制，基于硬件能力可开发三类算子：矢量算子、矩阵算子、融合算子。前序章节对硬件架构的介绍中已说明：AI Core包含Vector（矢量计算）和Cube（矩阵计算）两类核心计算单元，Cube计算单元部署于AIC核，Vector计算单元部署于AIV核，每个核均配备独立的Scalar单元，可独立加载各自的代码段。基于此，开发者可实现Vector矢量算子、Cube矩阵算子及CV融合算子。

### Vector矢量计算

Vector矢量计算主要包含基础算术、归约计算、数据类型转换等矢量计算。

在[NPU架构版本2201](../../../language_extension/simd_builtin_keywords.md)上，Vector矢量计算的数据来自于UB（要求32B对齐），C++ Tensor编程在Vector计算单元上提供了通过LocalTensor直接操作计算的接口能力。此类直接操作UB的编程方式称为Memory矢量计算编程。

以Add计算为例，当开发者需要计算4096长度的向量加法，可通过`Add(dstTensor, src0Tensor, src1Tensor, count)`接口直接进行计算。更多内容请参考[Memory矢量计算编程](./memory_vector_computation.md)。

```cpp
// Define Vector operator, marked with __vector__ to execute on Device, suitable for operators with only Vector computation
__global__ __vector__ void add_kernel(__gm__ float* x, __gm__ float* y, __gm__ float* z)
{
    AscendC::LocalMemAllocator<AscendC::Hardware::UB> ubAllocator;
    AscendC::LocalTensor<float> xLocal = ubAllocator.Alloc<float, 4096>();
    AscendC::LocalTensor<float> yLocal = ubAllocator.Alloc<float, 4096>();
    AscendC::LocalTensor<float> zLocal = ubAllocator.Alloc<float, 4096>();

    // 1. Data transfer in: transfer GM data to UB, this implementation is omitted
    // ...
    // 2. Perform calculation for the first 4096 elements via Add interface
    AscendC::Add(zLocal, xLocal, yLocal, 4096);
}
```

[NPU架构版本3510](../../../language_extension/simd_builtin_keywords.md)新一代架构，引入寄存器可编程能力，在传统UB缓存体系的基础上，开放寄存器（Register）可编程能力，构建出「GM → UB → Register」的三级内存层级。新一代架构下的矢量计算都需要从UB加载到Register寄存器后，再利用Register寄存器进行计算操作。可参考[Reg矢量计算编程](./reg_vector_computation.md)中有关Reg矢量计算编程的描述。

> 📌 寄存器可编程能力从[NPU架构版本3510](../../../language_extension/simd_builtin_keywords.md)引入。

### Cube矩阵计算

Cube矩阵计算主要涵盖矩阵乘计算与卷积计算，其数据主要来源于L1 Buffer与L0A/B Buffer。L1 Buffer用于矩阵计算数据缓存，L0A Buffer用于存储左矩阵，L0B Buffer用于存储右矩阵，L0C Buffer用于存储初始累加值及矩阵计算结果。此类基于Cube计算单元直接操作L1/L0A/L0B/L0C Buffer的编程范式称为[Cube矩阵计算编程](./cube_matrix_computation.md)。

以Matmul矩阵乘计算为例，开发者通过操作Cube矩阵计算能力可以完成如下图所示的c = a * b + bias的计算。
- A、B为源操作数，A为左矩阵，形状为[M, K]；B为右矩阵，形状为[K, N]。
- C为目的操作数，存放矩阵乘结果的矩阵，形状为[M, N]。
- bias为矩阵乘偏置，形状为[1, N]。对A*B结果矩阵的每一行都采用该bias进行偏置。

![矩阵乘示意图](../../../../figures/matmul.png)

```cpp
// Define Cube matrix operator, marked with __cube__ to execute on Device Cube core, suitable for operators with only Cube computation
__global__ __cube__ void matmul_kernel(__gm__ float* x, __gm__ float* y, __gm__ float* z)
{
    AscendC::LocalMemAllocator<AscendC::Hardware::L1> l1Allocator;
    AscendC::LocalMemAllocator<AscendC::Hardware::L0A> l0aAllocator;
    AscendC::LocalMemAllocator<AscendC::Hardware::L0B> l0bAllocator;
    AscendC::LocalMemAllocator<AscendC::Hardware::L0C> l0cAllocator;

    AscendC::LocalTensor<half> a_l1_buf = l1Allocator.Alloc<half, BLOCK_M * BLOCK_K>();
    AscendC::LocalTensor<half> b_l1_buf = l1Allocator.Alloc<half, BLOCK_K * BLOCK_N>();
    AscendC::LocalTensor<half> l0a_buf = l0aAllocator.Alloc<half, BLOCK_M * BLOCK_K>();
    AscendC::LocalTensor<half> l0b_buf = l0bAllocator.Alloc<half, BLOCK_K * BLOCK_N>();
    AscendC::LocalTensor<float> l0c_buf = l0cAllocator.Alloc<float, BLOCK_M * BLOCK_N>();

    // GM2L1 and L12L0 data transfer operations
    // ...
    // Execute Mmad, computation size is (BLOCK_M, BLOCK_K, BLOCK_N)
    AscendC::Mmad(l0c_buf, l0a_buf, l0b_buf, {BLOCK_M, BLOCK_K, BLOCK_N, 0, false, false});
    // Matrix data transfer out
}
```

### CV融合计算

CV融合计算程序由多个独立的小算子融合而成，其功能与多个小算子的功能等价，性能方面通常优于独立的小算子。用户可以根据实际业务场景诉求，按照具体算法自由融合向量（Vector）、矩阵（Cube）算子以达到性能上的收益。融合了Cube计算、Vector计算的算子统称为CV融合算子。

比如对于LLM大模型中最核心的一个融合算子Flash Attention，其核心实现如下图。图中的Matmul算子（Cube）、Scale算子（Vector）、Mask算子（Vector）、SoftMax算子（Vector）融合为一个大的算子Flash Attention。

![Flash Attention核心实现](../../../../figures/flash_attn.png)

```cpp
// Define CV fusion operator, marked with __mix__ to execute on Device, suitable for operators combining Cube and Vector computation
__global__ __mix__ void mix_kernel(__gm__ float* x, __gm__ float* y, __gm__ float* z)
{
    if ASCEND_IS_AIV {
        // ...
        AscendC::Add(zLocal, xLocal, yLocal, 4096);
        // ...
    }

    if ASCEND_IS_AIC {
        // ...
        AscendC::Mmad(l0c_buf, l0a_buf, l0b_buf, {BLOCK_M, BLOCK_K, BLOCK_N, 0, false, false});
        // ...
    }
}
```

融合算子在提升性能与充分发挥算力的基础上，通过合并算子减少计算量与内存占用、优化数据流传输、简化代码实现，实现计算效率、资源利用率与代码质量的同步提升。

## 编程约束和限制

- 同步事件由开发者使用`SetFlag/WaitFlag(ISASI)`和`PipeBarrier(ISASI)`手动插入，事件的类型和事件ID由开发者自行管理，但需要注意事件ID不能使用6和7（可能与内部使用的事件ID出现冲突，进而出现未定义行为）。
- 核函数（Kernel）入口处需要开发者手动调用[InitSocState](../../../../../api/SIMD-API/basic_api/tool_interface/system_init/InitSocState.md)接口用来初始化全局状态寄存器。因为全局状态寄存器处于不确定状态，如果不调用该接口，可能导致算子执行过程中出现未定义行为。在TPipe框架编程中，初始化过程由TPipe完成，无需开发者关注。
- 核函数（Kernel）结束前需要开发者手动调用[PipeBarrier<PIPE_ALL>()](../../../../../api/SIMD-API/basic_api/sync_control/intra_core_sync/PipeBarrier_ISASI.md)。
- 部分API内部通过软仿真实现，需占用Ascend C预留的UB空间，开发者使用时需关注目标API是否依赖该预留空间（针对[NPU架构版本2201](../../../language_extension/simd_builtin_keywords.md)，Select等API内部使用了8KB的预留UB空间，用于存储中间数据；针对[NPU架构版本3510](../../../language_extension/simd_builtin_keywords.md)，Exp等API内部使用了2KB的预留UB空间，用于指令兼容或存储中间数据）。若开启--cce-disable-asc-reserved-ubuf编译选项，在对应产品版本下调用相关API会触发编译期报错。具体API范围见[使用预留UB空间的API列表](#section_reserved_ubuf_api)。
- 同时另外一些软仿实现接口内部会占用系统分配的事件ID，如果开发者手动调用`SetFlag/WaitFlag(ISASI)`插入自定义事件ID时会跟接口内部的事件ID产生冲突，所以用户自定义事件ID时，只能支持部分API。具体支持的API列表见[支持的API范围](#section2633193623711)。

## 支持的API范围

<!-- npu="310p" id1 -->
**表1**  针对Atlas 推理系列产品AI Core，支持的API范围

| 接口分类 | 接口名称 |
| --- | --- |
| 基础API > 标量计算 | ScalarGetCountOfValue、ScalarCountLeadingZero、ScalarCast、CountBitsCntSameAsSignBit、ScalarGetSFFValue |
| 基础API > 矢量计算 > 基础算术 | Exp、Ln、Abs、Reciprocal、Sqrt、Rsqrt、Relu、VectorPadding、Add、Sub、Mul、Div、Max、Min、BilinearInterpolation、Adds、Muls、Maxs、Mins、LeakyRelu |
| 基础API > 矢量计算 > 逻辑计算 | Not、And、Or |
| 基础API > 矢量计算 > 复合计算 | Axpy、CastDequant、AddRelu、AddReluCast、AddDeqRelu、SubRelu、SubReluCast、MulAddDst、MulCast、FusedMulAdd、FusedMulAddRelu |
| 基础API > 矢量计算 > 比较与选择 | Compare、Compare（结果存入寄存器）、CompareScalar、GetCmpMask、SetCmpMask、
Select、GatherMask |
| 基础API > 矢量计算 > 类型转换 | Cast |
| 基础API > 矢量计算 > 归约计算 | WholeReduceMax、WholeReduceMin、WholeReduceSum、BlockReduceMax、BlockReduceMin、BlockReduceSum、PairReduceSum、RepeatReduceSum、GetReduceMaxMinCount |
| 基础API > 矢量计算 > 数据转换 | Transpose、TransDataTo5HD |
| 基础API > 矢量计算 > 数据填充 | Duplicate |
| 基础API > 矢量计算 > 排序组合 | ProposalConcat、ProposalExtract、RpSort16、MrgSort4、GetMrgSortResult |
| 基础API > 矢量计算 > 离散与聚合 | Gather、Scatter |
| 基础API > 矢量计算 > 掩码操作 | SetMaskCount、SetMaskNorm、SetVectorMask、ResetMask |
| 基础API > 矢量计算 > 量化设置 | SetDeqScale |
| 基础API > 数据搬运 > DataCopy | 基础数据搬运 |
| 基础API > 同步控制 > 核内同步 | SetFlag/WaitFlag、PipeBarrier |
| 基础API > 缓存控制 | DataCachePreload、DataCacheCleanAndInvalid、ICachePreLoad |
| 基础API > 系统变量访问 | GetBlockNum、GetBlockIdx、GetDataBlockSizeInBytes、GetArchVersion、GetTaskRatio、InitSocState、GetProgramCounter、CheckLocalMemoryIA |
| 基础API > 原子操作 | SetAtomicAdd、DisableDmaAtomic |
| 基础API > 矩阵计算 | InitConstValue、LoadData、SetAippFunctions、LoadImageToLocal、LoadUnzipIndex、LoadDataUnzip、SetLoadDataBoundary、SetLoadDataPaddingValue、Mmad |

<!-- end id1 -->

<!-- npu="910b" id2 -->
**表2**  针对Atlas A2 训练系列产品/Atlas A2 推理系列产品，支持的API范围

| 接口分类 | 接口名称 | 备注 |
| --- | --- | --- |
| 基础API > 标量计算 | GetBitCount、CountLeadingZero、CountBitsCntSameAsSignBit、GetSFFValue、Cast（float转half、int32_t）、Cast（float转bfloat16_t）、Cast（多类型转float） | - |
| 基础API > 矢量计算 > 基础算术 | Exp、Ln、Abs、Reciprocal、Sqrt、Rsqrt、Relu、Add、Sub、Mul、Div、Max、Min、BilinearInterpolation、Adds、Muls、Maxs、Mins、LeakyRelu | - |
| 基础API > 矢量计算 > 逻辑计算 | Not、And、Or、ShiftLeft、ShiftRight | - |
| 基础API > 矢量计算 > 复合计算 | Axpy、CastDequant、AddRelu、AddReluCast、SubRelu、SubReluCast、MulAddDst、MulCast、FusedMulAdd、FusedMulAddRelu | - |
| 基础API > 矢量计算 > 比较与选择 | Compare、Compare（结果存入寄存器）、Compares、GetCmpMask、SetCmpMask、GatherMask | - |
| 基础API > 矢量计算 > 类型转换 | Cast | - |
| 基础API > 矢量计算 > 归约计算 | WholeReduceMax、WholeReduceMin、WholeReduceSum、BlockReduceMax、BlockReduceMin、BlockReduceSum、PairReduceSum、RepeatReduceSum、GetReduceRepeatSumSpr、GetReduceRepeatMaxMinSpr | - |
| 基础API > 矢量计算 > 数据转换 | Transpose、TransDataTo5HD | TransDataTo5HD （不支持dst与src类型为uint64_t的LocalTensor的场景） |
| 基础API > 矢量计算 > 数据填充 | Duplicate、Brcb | - |
| 基础API > 矢量计算 > 排序组合 | Sort32、MrgSort、GetMrgSortResult | - |
| 基础API > 矢量计算 > 离散与聚合 | Gather、Gatherb | - |
| 基础API > 矢量计算 > 掩码操作 | SetMaskCount、SetMaskNorm、SetVectorMask、ResetMask | - |
| 基础API > 矢量计算 > 量化设置 | SetDeqScale | - |
| 基础API > 数据搬运 > DataCopy | 基础数据搬运 | 不支持UB（VECIN/VECCALC/VECOUT）->L1 Buffer（TSCM）通路的数据搬运。 |
| 增强数据搬运 | 不支持UB（VECIN/VECCALC/VECOUT）->L1 Buffer（TSCM）通路的数据搬运。 |  |
| 切片数据搬运 | - |  |
| 随路转换ND2NZ搬运
随路转换NZ2ND搬运
随路量化激活搬运 | 随路转换ND2NZ搬运不支持UB（VECIN/VECCALC/VECOUT）->L1 Buffer（TSCM）通路的数据搬运。 |  |
| 基础API > 数据搬运 | Copy、DataCopyPad、SetPadValue | DataCopyPad不支持UB（VECIN/VECCALC/VECOUT）->L1 Buffer（TSCM）通路的数据搬运。 |
| 基础API > 同步控制 > 核内同步 | SetFlag/WaitFlag、PipeBarrier、DataSyncBarrier | - |
| 基础API > 同步控制 > 核间同步 | CrossCoreSetFlag、CrossCoreWaitFlag | - |
| 基础API > 缓存控制 | DataCachePreload、DataCacheCleanAndInvalid、ICachePreLoad、GetICachePreloadStatus | - |
| 基础API > 系统变量访问 | GetBlockNum、GetBlockIdx、GetDataBlockSizeInBytes、GetArchVersion、GetTaskRatio、InitSocState、GetProgramCounter、GetSubBlockNum、GetSubBlockIdx、GetSystemCycle、
CheckLocalMemoryIA | - |
| 基础API > 原子操作 | SetAtomicAdd、SetAtomicType、DisableDmaAtomic、SetAtomicMax、SetAtomicMin、SetStoreAtomicConfig、GetStoreAtomicConfig | - |
| 基础API > 矩阵计算 | Mmad、MmadWithSparse、SetHF32Mode、SetHF32TransMode、SetMMLayoutTransform、SetFixPipeConfig、SetFixpipeNz2ndFlag、SetFixpipePreQuantFlag、InitConstValue、LoadData、LoadDataWithTranspose、SetAippFunctions、LoadImageToLocal、LoadDataWithSparse、SetFmatrix、SetLoadDataBoundary、SetLoadDataRepeat、SetLoadDataPaddingValue、Fixpipe | - |
| Utils API > C++标准库 > 算法 | max、min、index_sequence | - |
| Utils API > C++标准库 > 容器函数 | tuple、get、make_tuple | - |
| Utils API > C++标准库 > 类型特性 | is_convertible、is_base_of、is_same、enable_if、conditional | - |

<!-- end id2 -->

<!-- npu="A3" id3 -->
**表3**  针对Atlas A3 训练系列产品/Atlas A3 推理系列产品，支持的API范围

| 接口分类 | 接口名称 | 备注 |
| --- | --- | --- |
| 基础API > 标量计算 | GetBitCount、CountLeadingZero、CountBitsCntSameAsSignBit、GetSFFValue、Cast（float转half、int32_t）、Cast（float转bfloat16_t）、Cast（多类型转float） | - |
| 基础API > 矢量计算 > 基础算术 | Exp、Ln、Abs、Reciprocal、Sqrt、Rsqrt、Relu、Add、Sub、Mul、Div、Max、Min、BilinearInterpolation、Adds、Muls、Maxs、Mins、LeakyRelu | - |
| 基础API > 矢量计算 > 逻辑计算 | Not、And、Or、ShiftLeft、ShiftRight | - |
| 基础API > 矢量计算 > 复合计算 | Axpy、CastDequant、AddRelu、AddReluCast、SubRelu、SubReluCast、MulAddDst、MulCast、FusedMulAdd、FusedMulAddRelu | - |
| 基础API > 矢量计算 > 比较与选择 | Compare、Compare（结果存入寄存器）、Compares、GetCmpMask、SetCmpMask、GatherMask | - |
| 基础API > 矢量计算 > 类型转换 | Cast | - |
| 基础API > 矢量计算 > 归约计算 | WholeReduceMax、WholeReduceMin、WholeReduceSum、BlockReduceMax、BlockReduceMin、BlockReduceSum、PairReduceSum、RepeatReduceSum、GetReduceRepeatSumSpr、GetReduceRepeatMaxMinSpr | - |
| 基础API > 矢量计算 > 数据转换 | Transpose、TransDataTo5HD | TransDataTo5HD （不支持dst与src类型为uint64_t的LocalTensor的场景） |
| 基础API > 矢量计算 > 数据填充 | Duplicate、Brcb | - |
| 基础API > 矢量计算 > 排序组合 | Sort32、MrgSort、GetMrgSortResult | - |
| 基础API > 矢量计算 > 离散与聚合 | Gather、Gatherb | - |
| 基础API > 矢量计算 > 掩码操作 | SetMaskCount、SetMaskNorm、SetVectorMask、ResetMask | - |
| 基础API > 矢量计算 > 量化设置 | SetDeqScale | - |
| 基础API > 数据搬运 > DataCopy | 基础数据搬运 | 不支持UB（VECIN/VECCALC/VECOUT）->L1 Buffer（TSCM）通路的数据搬运。 |
| 基础API > 数据搬运 | 增强数据搬运 | 不支持UB（VECIN/VECCALC/VECOUT）->L1 Buffer（TSCM）通路的数据搬运。 |
| 切片数据搬运 | - |  |
| 随路转换ND2NZ搬运
随路转换NZ2ND搬运
随路量化激活搬运 | 随路转换ND2NZ搬运不支持UB（VECIN/VECCALC/VECOUT）->L1 Buffer（TSCM）通路的数据搬运。 |  |
| Copy、DataCopyPad、SetPadValue | DataCopyPad不支持UB（VECIN/VECCALC/VECOUT）->L1 Buffer（TSCM）通路的数据搬运。 |  |
| 基础API > 同步控制 > 核内同步 | SetFlag/WaitFlag、PipeBarrier、DataSyncBarrier | - |
| 基础API > 同步控制 > 核间同步 | CrossCoreSetFlag、CrossCoreWaitFlag | - |
| 基础API > 缓存控制 | DataCachePreload、DataCacheCleanAndInvalid、ICachePreLoad、GetICachePreloadStatus | - |
| 基础API > 系统变量访问 | GetBlockNum、GetBlockIdx、GetDataBlockSizeInBytes、GetArchVersion、GetTaskRatio、InitSocState、GetProgramCounter、GetSubBlockNum、GetSubBlockIdx、GetSystemCycle、
CheckLocalMemoryIA | - |
| 基础API > 原子操作 | SetAtomicAdd、SetAtomicType、DisableDmaAtomic、SetAtomicMax、SetAtomicMin、SetStoreAtomicConfig、GetStoreAtomicConfig | - |
| 基础API > 矩阵计算 | Mmad、MmadWithSparse、SetHF32Mode、SetHF32TransMode、SetMMLayoutTransform、SetFixPipeConfig、SetFixpipeNz2ndFlag、SetFixpipePreQuantFlag、InitConstValue、LoadData、LoadDataWithTranspose、SetAippFunctions、LoadImageToLocal、LoadDataWithSparse、SetFmatrix、SetLoadDataBoundary、SetLoadDataRepeat、SetLoadDataPaddingValue、Fixpipe | - |
| Utils API > C++标准库 > 算法 | max、min、index_sequence | - |
| Utils API > C++标准库 > 容器函数 | tuple、get、make_tuple | - |
| Utils API > C++标准库 > 类型特性 | is_convertible、is_base_of、is_same、enable_if、conditional | - |
| 高阶API > C++标准库 > 类型特性 | is_convertible、is_base_of、is_same、enable_if、conditional | - |
| 高阶API > 模板库函数 > type_traits | is_convertible、is_base_of、is_same、enable_if、conditional | - |

<!-- end id3 -->

<!-- npu="950" id4 -->
**表4**  针对Ascend 950PR/Ascend 950DT，支持的API范围

| 接口分类 | 接口名称 | 备注 |
| --- | --- | --- |
| 基础API > 标量计算 | GetBitCount、CountLeadingZero、CountBitsCntSameAsSignBit、GetSFFValue、Cast（float转half、int32_t）、Cast（float转bfloat16_t）、Cast（多类型转float）、Nop、GetUintDivMagicAndShift、WriteGmBypassDCache、ReadGmBypassDCache | - |
| 基础API > 矢量计算 > 基础算术 | Exp、Ln、Abs、Reciprocal、Sqrt、Rsqrt、Relu、Neg、Add、Sub、Mul、Div、Max、Min、BilinearInterpolation、Prelu、Mull、Adds、Adds（灵活标量位置）、Muls、Muls（灵活标量位置）、Maxs、Maxs（灵活标量位置）、Mins、Mins（灵活标量位置）、Subs、Divs、LeakyRelu | - |
| 基础API > 矢量计算 > 逻辑计算 | Not、And、Or、ShiftLeft（左移位数为Tensor）、ShiftRight（右移位数为Tensor）、Ands、Ors、ShiftLeft、ShiftRight | - |
| 基础API > 矢量计算 > 复合计算 | Axpy、CastDequant、AddRelu、AddReluCast、AddDeqRelu、SubRelu、SubReluCast、MulAddDst、MulCast、FusedMulAdd、MulAddRelu、AbsSub、FusedExpSub、MulsCast | - |
| 基础API > 矢量计算 > 比较与选择 | Compare、Compares、Compares（灵活标量位置）、GetCmpMask、SetCmpMask、GatherMask、Select、Select（灵活标量位置） | Select不支持：tensor高位切分计算->Select模式0和模式2->不传入mask参数接口中的模式0。 |
| 基础API > 矢量计算 > 类型转换 | Cast、Truncate | - |
| 基础API > 矢量计算 > 归约计算 | ReduceMax、ReduceMin、WholeReduceSum、BlockReduceMax、BlockReduceMin、BlockReduceSum、PairReduceSum、RepeatReduceSum、 | - |
| 基础API > 矢量计算 > 数据转换 | Transpose | - |
| 基础API > 矢量计算 > 数据填充 | Duplicate、Brcb、CreateVecIndex | - |
| 基础API > 矢量计算 > 排序组合 | Sort32、MrgSort、GetMrgSortResult | - |
| 基础API > 矢量计算 > 离散与聚合 | Gather、Gatherb、Scatter | - |
| 基础API > 矢量计算 > 掩码操作 | SetMaskCount、SetMaskNorm、SetVectorMask、ResetMask | - |
| 基础API > 矢量计算 > 量化设置 | SetDeqScale | - |
| 基础API > 矢量计算 > 数据重排 | Interleave、DeInterleave | - |
| 基础API > 数据搬运 > DataCopy | 基础数据搬运 | 不支持UB（VECIN/VECCALC/VECOUT）->L1 Buffer（TSCM）通路的数据搬运。 |
| 增强数据搬运 | 不支持UB（VECIN/VECCALC/VECOUT）->L1 Buffer（TSCM）通路的数据搬运。 |  |
| 切片数据搬运 | - |  |
| 随路转换ND2NZ搬运 | 不支持UB（VECIN/VECCALC/VECOUT）->L1 Buffer（TSCM）通路的数据搬运。 |  |
| 随路转换NZ2ND搬运 | - |  |
| 随路量化激活搬运 | - |  |
| 基础API > 数据搬运 | Copy、DataCopyPad、SetPadValue | DataCopyPad不支持UB（VECIN/VECCALC/VECOUT）->L1 Buffer（TSCM）通路的数据搬运。 |
| 基础API > 同步控制 > 核内同步 | SetFlag/WaitFlag、PipeBarrier、DataSyncBarrier | - |
| 基础API > 同步控制 > 核间同步 | CrossCoreSetFlag、CrossCoreWaitFlag、SyncAll | - |
| 基础API > 缓存控制 | DataCachePreload、DataCacheCleanAndInvalid、ICachePreLoad、GetICachePreloadStatus | - |
| 基础API > 系统变量访问 | GetBlockNum、GetBlockIdx、GetDataBlockSizeInBytes、GetArchVersion、InitSocState、GetSpr、ClearSpr、GetProgramCounter、GetSubBlockNum、GetSubBlockIdx、GetSystemCycle、SetCtrlSpr、GetCtrlSpr、ResetCtrlSpr | - |
| 基础API > 原子操作 | SetAtomicAdd、SetAtomicType、DisableDmaAtomic、SetAtomicMax、SetAtomicMin、SetStoreAtomicConfig、GetStoreAtomicConfig、AtomicAdd、AtomicMin、AtomicMax、AtomicCas、AtomicExch | - |
| 基础API > 矩阵计算 | Fill、LoadData、LoadDataWithTranspose、LoadDataWithSparse、SetFmatrix、SetLoadDataBoundary、SetLoadDataRepeat、SetLoadDataPaddingValue、Fixpipe、SetFixPipeConfig、SetFixpipeNz2ndFlag、SetFixpipePreQuantFlag、SetAippFunctions、LoadImageToLocal、Mmad、SetHF32Mode、SetHF32TransMode | - |
| Utils API > C++标准库 > 算法 | max、min、index_sequence | - |
| Utils API > C++标准库 > 容器函数 | tuple、get、make_tuple | - |
| Utils API > C++标准库 > 类型特性 | is_convertible、is_same、enable_if、conditional | - |

<!-- end id4 -->

## 使用预留UB空间的API范围

以下API内部会使用Ascend C预留的UB空间。当开启[--cce-disable-asc-reserved-ubuf](../../../compilation_and_execution/operator_compilation/ai_core_operator_compilation.md#常用的编译选项)编译选项后，在对应产品版本下调用相关API会触发编译期报错。由于同一API使用预留UB空间的情况在不同NPU架构下有差异，开启该编译选项后，需要手动调整API调用方式或替换为不依赖预留UB空间的实现，才能完成兼容性迁移。

>[!CAUTION]注意
> 同一接口若提供多个重载原型，可能只有部分原型会使用UB预留空间。

<!-- npu="A3,910b" id5 -->
**表5**  针对 [NPU架构版本2201](../../../language_extension/simd_builtin_keywords.md#npu-arch)，使用预留UB空间的API范围

| API类别 | API名 |
| --- | --- |
| 基础API | AddDeqRelu、Select |
| 高阶API | Digamma、Lgamma、Power、Sign、LayerNorm、DropOut、SelectWithBytesMask |

<!-- end id5 -->

<!-- npu="950" id6 -->
**表6**  针对 [NPU架构版本3510](../../../language_extension/simd_builtin_keywords.md#npu-arch)，使用预留UB空间的API范围

| API类别 | API名 |
| --- | --- |
| 基础API > Memory数据搬运 | Copy |
| 基础API > 基础算术 | Exp、Ln、Abs、Reciprocal、Sqrt、Rsqrt、Relu、Add、Sub、Mul、Div、Max、Min、Adds、Muls、Maxs、Mins、LeakyRelu、Subs、Divs |
| 基础API > 逻辑计算 | Not、And、Or、ShiftLeft、ShiftRight、Ands、Ors |
| 基础API > 复合计算 | Axpy、CastDequant、CastDeq、AddRelu、AddReluCast、AddDeqRelu、SubRelu、SubReluCast、MulAddDst、MulCast、FusedMulAdd、FusedMulAddRelu、MulAddRelu |
| 基础API > 比较与选择 | Compare、Compares、CompareScalar、Select、GatherMask |
| 基础API > 类型转换 | Cast |
| 基础API > 归约计算 | ReduceRepeat、ReduceDataBlock、ReducePairElem、BlockReduceMax、BlockReduceMin、BlockReduceSum、PairReduceSum、RepeatReduceSum、WholeReduceMax、WholeReduceMin、WholeReduceSum |
| 基础API > 数据填充 | Duplicate |
| 基础API > Kernel-Tiling | GET_TILING_DATA、GET_TILING_DATA_WITH_STRUCT、GET_TILING_DATA_MEMBER |
| Utils API > 调测接口（SIMD VF） | printf、asc_dump_reg、asc_dump、asc_dump_ubuf |
| 高阶API | AscendAntiQuant、AdjustSoftMaxRes、TopK |

<!-- end id6 -->

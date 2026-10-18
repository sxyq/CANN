# 静态Tensor编程

在基于Pipe进行算子开发的方式中，由Pipe（TPipe类）统一管理Device端内存等资源，开发者无需感知内存管理、DoubleBuffer流水、同步等处理，只需要按照计算流编写算子即可，但由此也带来了一些运行时开销（如TPipe创建、InitBuffer等）。

基于以上原因，Ascend C提供了静态Tensor编程方式，相比基于Pipe的编程方式，这种方式避免了TPipe内存管理初始化过程（约数百纳秒），从而减少了运行时开销，更有助于开发者实现极致性能。通过直接构造指定地址和存储位置的LocalTensor，并将其传递给计算、搬运等API进行编程，提供了更高的灵活性。然而，这种编程方式也带来了更高的开发复杂性，需要开发者自行管理DoubleBuffer和同步流水，并且只能使用Ascend C的基础API，而非全部功能。

两种编程方式的对比如下：

![](../../../../figures/op_flow.png)

>[!NOTE]说明
>- 静态Tensor编程的使用约束和限制请参考[使用约束和限制](#section19853161834615)。
>- 本节涉及的完整样例请参考[静态Tensor编程样例](../../../../../../../examples/01_simd_cpp_api/05_best_practices/00_vector_compute/add_high_performance)。

## 编程范式

-   AI Core包括多种内存单元，比如用于矢量计算的Unified Buffer（UB）和用于矩阵计算的L1 Buffer、L0A Buffer、L0B Buffer、L0C Buffer等内存资源。开发者完全自主管理AI Core上的所有内存资源，创建Tensor分配地址时管理内存大小、内存复用关系并确保分配的地址有效性。
-   AI Core包括多种[指令流水类型](../../../../../api/SIMD-API/basic_api/sync_control/intra_core_sync/intra_core_sync_overview.md)，比如Vector/Cube/Scalar计算流水，MTE1、MTE2、MTE3搬运流水等，每条流水并行执行，它们之间的依赖关系通过[同步事件](../../../../../api/SIMD-API/basic_api/sync_control/intra_core_sync/SetFlag_WaitFlag_ISASI.md)来协调。开发者调用Ascend C提供的搬运或者计算类API编写算子并根据数据依赖关系插入对应的同步事件，以达成最优性能。

下图是一个典型矢量算子的示意图，开发者首先根据业务计算量进行数据分块处理，之后根据核内的数据依赖关系完成同步事件的插入：

![](../../../../figures/mem_mgmt_5.png)

## 内存管理

静态Tensor编程方式下，开发者可以使用两种方式创建Tensor：

-   通过[LocalMemAllocator](../../../../../api/SIMD-API/basic_api/resource_management/LocalMemAllocator/LocalMemAllocator.md)指定硬件位置进行Tensor分配。

    LocalMemAllocator是一种线性内存分配器，开发者可以调用Alloc方法进行内存分配，地址分配从0开始，根据调用次序依次向后进行线性分配，LocalMemAllocator只是一个简单的线性分配器，并不提供内存释放以及其它内存管理的能力。在不关注Bank冲突场景或者算子初始功能开发时，可以使用LocalMemAllocator简化算子编写，在后续性能优化时切换到使用LocalTensor进行地址分配的方式。

-   通过[LocalTensor构造函数](../../../../../api/SIMD-API/basic_api/data_structures/LocalTensor/LocalTensor_constructor.md)创建Tensor，极致性能场景推荐使用此方式。

    开发者可以使用LocalTensor构造函数直接指定内存地址，实现内存的完全自主管理（本质上无需申请和释放内存）。使用时，需根据需求合理指定地址（不超过物理存储上限），并在保证功能正确的前提下进行内存复用。如果需要通过规避Bank冲突或者复用内存来获得极致性能时，推荐使用该方式。

```
    // 方式1：使用LocalMemAllocator进行内存分配
    AscendC::LocalMemAllocator<AscendC::Hardware::UB> ubAllocator;
    AscendC::LocalTensor<float> xLocalPing = ubAllocator.Alloc<float, TILE_LENGTH>();
    AscendC::LocalTensor<float> yLocalPing = ubAllocator.Alloc<float, TILE_LENGTH>();
    AscendC::LocalTensor<float> zLocalPing = ubAllocator.Alloc<float, TILE_LENGTH>();

    // 方式2：直接使用LocalTensor构造函数构造Tensor
    AscendC::LocalTensor<float> xLocalPing(AscendC::TPosition::VECCALC, xAddrPing, TILE_LENGTH);
    AscendC::LocalTensor<float> yLocalPing(AscendC::TPosition::VECCALC, yAddrPing, TILE_LENGTH);
    AscendC::LocalTensor<float> zLocalPing(AscendC::TPosition::VECCALC, zAddrPing, TILE_LENGTH);
```

## 同步管理

根据前文介绍的硬件架构，AI Core内部异步并行计算存在多条流水（包括矢量计算、矩阵计算、数据搬入、数据搬出等），多条流水之间存在数据依赖时，需要插入对应的同步事件。静态Tensor编程方式下，开发者使用[SetFlag/WaitFlag\(ISASI\)](../../../../../api/SIMD-API/basic_api/sync_control/intra_core_sync/SetFlag_WaitFlag_ISASI.md)和[PipeBarrier\(ISASI\)](../../../../../api/SIMD-API/basic_api/sync_control/intra_core_sync/PipeBarrier_ISASI.md)手动插入同步，事件的类型和事件ID由开发者自行管理，但需要注意事件ID不能使用6和7（可能与内部使用的事件ID出现冲突，进而出现未定义行为）。另外由于需要使用SetFlag/WaitFlag/PipeBarrier底层同步接口（属于ISASI硬件体系结构相关的接口），无法保证跨硬件版本兼容。

在同步依赖中，根据数据依赖和指令执行关系，存在两种依赖关系，即正向同步（循环内依赖）与反向同步（循环间依赖）：

-   正向同步

    在本次数据搬入和计算之间，插入MTE2\_V（矢量计算流水等待MTE2搬运流水）同步事件，确保数据搬入之后再进行计算；在本次数据计算和搬出之间，插入V\_MTE3（MTE3搬运流水等待矢量计算流水）同步事件，确保数据计算完成后再进行搬出。

-   反向同步

    在上一次的数据计算和本次数据搬入之间，插入V\_MTE2（MTE2搬运流水等待矢量计算流水）同步事件，确保上一次的数据计算完成后，本次的数据再进行搬入。防止本次的数据会覆盖掉上一次未计算完成的数据；在上一次的数据搬出和本次数据计算之间，插入MTE3\_V（矢量计算流水等待MTE3搬运流水）同步事件，确保上一次的数据搬出后，再进行本次数据的计算。防止本次的数据会覆盖掉上一次未搬出的数据。

上述的同步逻辑在使用Pipe编程框架时，框架会使用EnQue/DeQue/AllocTensor/FreeTensor进行封装。您可以通过[TPipe-TQue框架编程原理](../tpipe_tque_programming/tpipe_tque_principles.md)来了解应该如何在使用静态Tensor编程方式时手动进行同步控制。

```
    AscendC::LocalTensor<half> xLocal(AscendC::TPosition::VECCALC, xAddr, MAX_DATA_COPY_LEN);
    AscendC::LocalTensor<half> yLocal(AscendC::TPosition::VECCALC, yAddr, MAX_DATA_COPY_LEN);
    AscendC::LocalTensor<half> zLocal(AscendC::TPosition::VECCALC, zAddr, MAX_DATA_COPY_LEN);

    uint32_t totalBlocks = GetTotalBlocks();

    for (uint32_t loopIdx = 0; loopIdx < totalBlocks; loopIdx++) {
        uint32_t startElement = loopIdx * dataCopyLen;
        uint32_t remainElements = totalElementsPerCore - startElement;
        uint32_t curLen = remainElements > dataCopyLen ? dataCopyLen : remainElements;

        if (loopIdx != 0) {
            AscendC::WaitFlag<AscendC::HardEvent::V_MTE2>(EVENT_ID0);
        }

        AscendC::DataCopy(xLocal, xGm[startElement], curLen);
        AscendC::DataCopy(yLocal, yGm[startElement], curLen);

        AscendC::SetFlag<AscendC::HardEvent::MTE2_V>(EVENT_ID0);
        AscendC::WaitFlag<AscendC::HardEvent::MTE2_V>(EVENT_ID0);

        if (loopIdx != 0) {
            AscendC::WaitFlag<AscendC::HardEvent::MTE3_V>(EVENT_ID0);
        }

        AscendC::Add(zLocal, xLocal, yLocal, curLen);

        if (loopIdx != (totalBlocks - 1)) {
            AscendC::SetFlag<AscendC::HardEvent::V_MTE2>(EVENT_ID0);
        }
        AscendC::SetFlag<AscendC::HardEvent::V_MTE3>(EVENT_ID0);
        AscendC::WaitFlag<AscendC::HardEvent::V_MTE3>(EVENT_ID0);

        AscendC::DataCopy(zGm[startElement], zLocal, curLen);

        if (loopIdx != (totalBlocks - 1)) {
            AscendC::SetFlag<AscendC::HardEvent::MTE3_V>(EVENT_ID0);
        }
    }
```

## 流水优化

在基于TPipe的编程范式中，开发者只需要在InitBuffer时指定buffer数量为2，即可自动开启DoubleBuffer。但是静态Tensor编程方式下，开发者需要手动开启DoubleBuffer，具体示例如下，完整样例请参考[静态Tensor编程样例](../../../../../../../examples/01_simd_cpp_api/05_best_practices/00_vector_compute/add_high_performance)中的DoubleBuffer示例。

```
    // ping
    AscendC::LocalTensor<half> xPing(AscendC::TPosition::VECCALC, xAddrPing, MAX_DATA_COPY_LEN);
    AscendC::LocalTensor<half> yPing(AscendC::TPosition::VECCALC, yAddrPing, MAX_DATA_COPY_LEN);
    AscendC::LocalTensor<half> zPing(AscendC::TPosition::VECCALC, zAddrPing, MAX_DATA_COPY_LEN);
    // pong
    AscendC::LocalTensor<half> xPong(AscendC::TPosition::VECCALC, xAddrPong, MAX_DATA_COPY_LEN);
    AscendC::LocalTensor<half> yPong(AscendC::TPosition::VECCALC, yAddrPong, MAX_DATA_COPY_LEN);
    AscendC::LocalTensor<half> zPong(AscendC::TPosition::VECCALC, zAddrPong, MAX_DATA_COPY_LEN);

    uint32_t totalBlocks = GetTotalBlocks();

    // DoubleBuffer
    AscendC::SetFlag<AscendC::HardEvent::V_MTE2>(EVENT_ID0);
    AscendC::SetFlag<AscendC::HardEvent::V_MTE2>(EVENT_ID1);
    AscendC::SetFlag<AscendC::HardEvent::MTE3_V>(EVENT_ID0);
    AscendC::SetFlag<AscendC::HardEvent::MTE3_V>(EVENT_ID1);

    for (uint32_t loopIdx = 0; loopIdx < totalBlocks; loopIdx++) {
        uint32_t startElement = loopIdx * dataCopyLen;
        uint32_t remainElements = totalElementsPerCore - startElement;
        uint32_t curLen = remainElements > dataCopyLen ? dataCopyLen : remainElements;

        int32_t eventID = ((loopIdx & 1) == 0 ? EVENT_ID0 : EVENT_ID1);
        AscendC::LocalTensor<half>& xLocal = ((loopIdx & 1) == 0 ? xPing : xPong);
        AscendC::LocalTensor<half>& yLocal = ((loopIdx & 1) == 0 ? yPing : yPong);
        AscendC::LocalTensor<half>& zLocal = ((loopIdx & 1) == 0 ? zPing : zPong);

        AscendC::WaitFlag<AscendC::HardEvent::V_MTE2>(eventID);

        AscendC::DataCopy(xLocal, xGm[startElement], curLen);
        AscendC::DataCopy(yLocal, yGm[startElement], curLen);

        AscendC::SetFlag<AscendC::HardEvent::MTE2_V>(eventID);
        AscendC::WaitFlag<AscendC::HardEvent::MTE2_V>(eventID);

        AscendC::WaitFlag<AscendC::HardEvent::MTE3_V>(eventID);

        AscendC::Add(zLocal, xLocal, yLocal, curLen);

        AscendC::SetFlag<AscendC::HardEvent::V_MTE2>(eventID);

        AscendC::SetFlag<AscendC::HardEvent::V_MTE3>(eventID);
        AscendC::WaitFlag<AscendC::HardEvent::V_MTE3>(eventID);

        AscendC::DataCopy(zGm[startElement], zLocal, curLen);

        AscendC::SetFlag<AscendC::HardEvent::MTE3_V>(eventID);
    }

    AscendC::WaitFlag<AscendC::HardEvent::V_MTE2>(EVENT_ID0);
    AscendC::WaitFlag<AscendC::HardEvent::V_MTE2>(EVENT_ID1);
    AscendC::WaitFlag<AscendC::HardEvent::MTE3_V>(EVENT_ID0);
    AscendC::WaitFlag<AscendC::HardEvent::MTE3_V>(EVENT_ID1);
```

以下为不开启DoubleBuffer和开启DoubleBuffer的流水示意图。多数情况下，采用DoubleBuffer能有效提升Vector的时间利用率，缩减算子执行时间，详细内容可参考[DoubleBuffer](../../../../technical_appendix/concepts_and_terms/performance_optimization/double_buffer.md)。

![](../../../../figures/op_flow_6.png)

![](../../../../figures/op_flow_7.png)

## 使用约束和限制

静态Tensor编程方式需要遵循以下约束和限制：

-   开发者不能使用TPipe/TQue/TQueBind/TBufPool等框架接口，和上述框架接口混用可能会出现未定义行为。
-   只能使用部分API。具体支持的API列表见[支持的API范围](#section2633193623711)。因为不在列表范围内的API内部依赖TPipe分配事件ID，可能会和开发者定义的事件ID产生冲突。
-   对于支持的API，仍需关注其内部是否依赖Ascend C预留的UB空间（针对 [NPU架构版本2201](../../../language_extension/simd_builtin_keywords.md#npu-arch)，Select等API内部使用了8KB的预留UB空间，用于存储中间数据；针对 [NPU架构版本3510](../../../language_extension/simd_builtin_keywords.md#npu-arch)，Exp等API内部使用了2KB的预留UB空间，用于指令兼容或存储中间数据）。若开启[--cce-disable-asc-reserved-ubuf](../../../compilation_and_execution/operator_compilation/ai_core_operator_compilation.md#常用的编译选项)编译选项，在对应产品版本下调用相关API会触发编译期报错。具体API范围见[使用预留UB空间的API列表](#section_reserved_ubuf_api)。
-   同步事件需要由开发者使用[SetFlag/WaitFlag\(ISASI\)](../../../../../api/SIMD-API/basic_api/sync_control/intra_core_sync/SetFlag_WaitFlag_ISASI.md)和[PipeBarrier\(ISASI\)](../../../../../api/SIMD-API/basic_api/sync_control/intra_core_sync/PipeBarrier_ISASI.md)手动插入，事件的类型和事件ID由开发者自行管理，但需要注意事件ID不能使用6和7（可能与内部使用的事件ID出现冲突，进而出现未定义行为）。
-   由于需要使用SetFlag/WaitFlag/PipeBarrier底层同步接口（属于ISASI硬件体系结构相关的接口），无法保证算子跨硬件版本兼容。
-   核函数（Kernel）入口处需要开发者手动调用[InitSocState](../../../../../api/SIMD-API/basic_api/tool_interface/system_init/InitSocState.md)接口用来初始化全局状态寄存器。因为全局状态寄存器处于不确定状态，如果不调用该接口，可能导致算子执行过程中出现未定义行为。在TPipe框架编程中，初始化过程由TPipe完成，无需开发者关注。
-   核函数（Kernel）结束前需要开发者手动调用[PipeBarrier<PIPE_ALL>()](../../../../../api/SIMD-API/basic_api/sync_control/intra_core_sync/PipeBarrier_ISASI.md)。

## 支持的API范围

<!-- npu="310p" id1 -->
**表1**  针对Atlas 推理系列产品AI Core，支持的API范围

| 接口分类 | 接口名称 |
| --- | --- |
| 基础API > 标量计算 | GetBitCount、CountLeadingZero、Cast、CountBitsCntSameAsSignBit、GetSFFValue |
| 基础API > 矢量计算 > 基础算术 | Exp、Ln、Abs、Reciprocal、Sqrt、Rsqrt、Relu、VectorPadding、Add、Sub、Mul、Div、Max、Min、BilinearInterpolation、Adds、Muls、Maxs、Mins、LeakyRelu |
| 基础API > 矢量计算 > 逻辑计算 | Not、And、Or |
| 基础API > 矢量计算 > 复合计算 | Axpy、CastDequant、AddRelu、AddReluCast、AddDeqRelu、SubRelu、SubReluCast、MulAddDst、MulCast、FusedMulAdd、MulAddRelu |
| 基础API > 矢量计算 > 比较与选择 | Compare、Compare（结果存入寄存器）、Compares、GetCmpMask、SetCmpMask、
Select、GatherMask |
| 基础API > 矢量计算 > 类型转换 | Cast |
| 基础API > 矢量计算 > 归约计算 | ReduceRepeat、ReduceDataBlock、ReducePairElem、GetReduceRepeatMaxMinSpr |
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
| 基础API > 矩阵计算 | Fill、LoadData、SetAippFunctions、LoadImageToLocal、LoadUnzipIndex、LoadDataUnzip、SetLoadDataBoundary、SetLoadDataPaddingValue、Mmad |

<!-- end id1 -->

<!-- npu="910b" id2 -->
**表2**  针对Atlas A2 训练系列产品/Atlas A2 推理系列产品，支持的API范围

| 接口分类 | 接口名称 | 备注 |
| --- | --- | --- |
| 基础API > 标量计算 | GetBitCount、CountLeadingZero、CountBitsCntSameAsSignBit、GetSFFValue、Cast（float转half、int32_t）、Cast（float转bfloat16_t）、Cast（多类型转float） | - |
| 基础API > 矢量计算 > 基础算术 | Exp、Ln、Abs、Reciprocal、Sqrt、Rsqrt、Relu、Add、Sub、Mul、Div、Max、Min、BilinearInterpolation、Adds、Muls、Maxs、Mins、LeakyRelu | - |
| 基础API > 矢量计算 > 逻辑计算 | Not、And、Or、ShiftLeft、ShiftRight | - |
| 基础API > 矢量计算 > 复合计算 | Axpy、CastDequant、AddRelu、AddReluCast、SubRelu、SubReluCast、MulAddDst、MulCast、FusedMulAdd、MulAddRelu | - |
| 基础API > 矢量计算 > 比较与选择 | Compare、Compare（结果存入寄存器）、Compares、GetCmpMask、SetCmpMask、GatherMask | - |
| 基础API > 矢量计算 > 类型转换 | Cast | - |
| 基础API > 矢量计算 > 归约计算 | ReduceRepeat、ReduceDataBlock、ReducePairElem、GetReduceRepeatSumSpr、GetReduceRepeatMaxMinSpr | - |
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
| 基础API > 矩阵计算 | Mmad、MmadWithSparse、SetHF32Mode、SetHF32TransMode、SetMMRowMajor、SetMMColumnMajor、SetFixPipeConfig、SetFixpipeNz2ndFlag、SetFixpipePreQuantFlag、Fill、LoadData、LoadDataWithTranspose、SetAippFunctions、LoadImageToLocal、LoadDataWithSparse、SetFmatrix、SetLoadDataBoundary、SetLoadDataRepeat、SetLoadDataPaddingValue、Fixpipe | - |
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
| 基础API > 矢量计算 > 复合计算 | Axpy、CastDequant、AddRelu、AddReluCast、SubRelu、SubReluCast、MulAddDst、MulCast、FusedMulAdd、MulAddRelu | - |
| 基础API > 矢量计算 > 比较与选择 | Compare、Compare（结果存入寄存器）、Compares、GetCmpMask、SetCmpMask、GatherMask | - |
| 基础API > 矢量计算 > 类型转换 | Cast | - |
| 基础API > 矢量计算 > 归约计算 | ReduceRepeat、ReduceDataBlock、ReducePairElem、GetReduceRepeatSumSpr、GetReduceRepeatMaxMinSpr | - |
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
| 基础API > 矩阵计算 | Mmad、MmadWithSparse、SetHF32Mode、SetHF32TransMode、SetMMRowMajor、SetMMColumnMajor、SetFixPipeConfig、SetFixpipeNz2ndFlag、SetFixpipePreQuantFlag、Fill、LoadData、LoadDataWithTranspose、SetAippFunctions、LoadImageToLocal、LoadDataWithSparse、SetFmatrix、SetLoadDataBoundary、SetLoadDataRepeat、SetLoadDataPaddingValue、Fixpipe | - |
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
| 基础API > 矢量计算 > 归约计算 | ReduceMax、ReduceMin、ReduceRepeat、ReduceDataBlock、ReducePairElem | - |
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
| 基础API > 复合计算 | Axpy、CastDequant、AddRelu、AddReluCast、AddDeqRelu、SubRelu、SubReluCast、MulAddDst、MulCast、FusedMulAdd、MulAddRelu |
| 基础API > 比较与选择 | Compare、Compares、Select、GatherMask |
| 基础API > 类型转换 | Cast |
| 基础API > 归约计算 | ReduceRepeat、ReduceDataBlock、ReducePairElem |
| 基础API > 数据填充 | Duplicate |
| 基础API > Kernel-Tiling | GET_TILING_DATA、GET_TILING_DATA_WITH_STRUCT、GET_TILING_DATA_MEMBER |
| Utils API > 调测接口（SIMD VF） | printf、asc_dump_reg、asc_dump、asc_dump_ubuf |
| 高阶API | AscendAntiQuant、AdjustSoftMaxRes、TopK |

<!-- end id6 -->

# 避免bank冲突（NPU架构版本2201）

【优先级】高

>[!NOTE]说明
>该性能优化建议适用于如下产品型号：
><!-- npu="A3" id1 -->
>- Atlas A3 训练系列产品/Atlas A3 推理系列产品
><!-- end id1 -->
><!-- npu="910b" id2 -->
>- Atlas A2 训练系列产品/Atlas A2 推理系列产品
><!-- end id2 -->

【描述】为了提高数据访问的效率和吞吐量，Unified Buffer（UB）采用了bank（大小相等的内存模块）结构设计。UB总大小为192K，划分为48个bank。每个bank由128行组成，每行长度为32B。这48个bank进一步组织为16个bank group，每个bank group包含3个bank，例如bank15、bank31和bank47组成一个bank group。

**图1**  bank结构示意图（图中箭头方向表示内存排布的顺序）
![](../../../../figures/bank_layout.png "bank结构示意图（图中箭头方向表示内存排布的顺序）")

每个bank可以独立地进行数据的读写操作，允许多个数据请求同时进行。然而，当多个读写操作试图同时访问同一个bank或bank group时，由于硬件资源的限制，这些操作必须排队等待，会导致bank冲突，引起性能下降。

具体来说，Vector计算单元每拍（一个指令周期）能够从每个bank group中读取或写入一行数据。如果同一个API中的多个操作试图同时访问同一个bank或bank group，Vector计算单元无法在同一个周期内处理所有请求，导致这些请求排队等待。这种排队增加了数据访问的延迟，降低了系统的整体性能。

## bank冲突的典型场景

bank冲突主要可以分为以下三种场景：

-   **读写冲突**：读操作和写操作同时尝试访问同一个bank。
-   **写写冲突**：多个写操作同时尝试访问同一个bank group。
-   **读读冲突**：多个读操作同时尝试访问同一个bank group。

下文给出了一些具体的示例，假设，0x10000地址在bank16上，0x10020在bank17上，0x20020在bank33上，如下图所示：

**图2**  地址分配示意图
![](../../../../figures/addr_alloc.png "地址分配示意图")

-   读写冲突示例

    Vector指令的源操作数src和目的操作数dst同时读写到同一个bank时造成读写冲突，具体分析如下：

    **表1**  读写冲突示例



| 序号 | src地址 | dst地址 | bank | bank group | 结论 |
| --- | --- | --- | --- | --- | --- |
| 示例1 | 0x10020 | 0x10000 | bank_id0 != bank_id1 | bank_group_id0 != bank_group_id1 | src地址和dst地址分别属于bank16和bank17，故无冲突。 |
| 示例2 | 0x10020 | 0x10E20 | bank_id0 == bank_id1 | bank_group_id0 == bank_group_id1 | src地址和dst地址的地址都在bank17，故存在冲突。 |

-   写写冲突示例

    Vector指令目的操作数dst对应的8个DataBlock（block0-block7）同时写到一个bank group时造成写写冲突，具体分析如下：

    **表2**  写写冲突示例



| 序号 | dst地址 | blk_stride | block0_addr | block1_addr | block2_addr | ... | 结论 |
| --- | --- | --- | --- | --- | --- | --- | --- |
| 示例1 | 0x1FE00 | 16 | 0x1FE00 | 0x20000 | 0x20200 | ... | 8个DataBlock均在一个bank group下，故全部冲突，8拍完成一个Repeat的写入。 |
| 示例2 | 0x1FE00 | 8 | 0x1FE00 | 0x1FF00 | 0x20000 | ... | block0和block2在一个bank group，存在冲突，4拍完成一个Repeat的写入。 |

-   读读冲突
    -   Vector指令多个源操作数同时读到同一个bank group时造成读读冲突，具体分析如下：

        **表3**  双src场景读读冲突示例



| 序号 | src0地址 | src1地址 | bank | bank group | 结论 |
| --- | --- | --- | --- | --- | --- |
| 示例1 | 0x10020 | 0x20020 | bank_id0 != bank_id1 | bank_group_id0 == bank_group_id1 | 存在冲突。 |
| 示例2 | 0x10020 | 0x10000 | bank_id0 != bank_id1 | bank_group_id0 != bank_group_id1 | 无冲突。 |

    -   Vector指令某一个源操作数对应的8个DataBlock（block0-block7）读到同一个bank group时造成读读冲突，具体分析如下：

        **表4**  单src场景读读冲突示例



| 序号 | src地址 | blk_stride | block0_addr | block1_addr | block2_addr | ... | 结论 |
| --- | --- | --- | --- | --- | --- | --- | --- |
| 示例1 | 0x1FE00 | 16 | 0x1FE00 | 0x20000 | 0x20200 | ... | 8个DataBlock均在一个bank group下，故全部冲突，8拍完成一个Repeat的读操作。 |
| 示例2 | 0x1FE00 | 8 | 0x1FE00 | 0x1FF00 | 0x20000 | ... | block0和block2在同一个bank group下，存在冲突，4拍完成一个Repeat。 |

>[!NOTE]说明
>通过msOpProf工具可以进行资源冲突占比的相关性能数据采集。
>工具的具体使用方法和资源冲突占比文件性能数据文件说明请参考[《msOpProf用户指南》](https://gitcode.com/Ascend/msopprof/blob/master/docs/zh/user_guide/msopprof_user_guide.md)。

## 如何避免bank冲突

避免bank冲突的方法有两种：**优化计算逻辑**和**优化地址分配**。

-   **优化计算逻辑**

    对一个shape为\(8, 16, 16\)的输入做\(1, 0, 2\)的transpose操作，输出shape为\(16, 8, 16\)。通过将计算逻辑由“跳读，连续写”修改为“连续读，跳写”可避免冲突问题。实现方案对比如下：



| 实现方案 | 原始实现 | 优化实现 |
| --- | --- | --- |
| 实现方法 | 跳读，连续写
    同一Repeat内输入的8个DataBlock都在同一个bank group而发生读读冲突。 | 连续读，跳写
    同一个Repeat内输入的8个DataBlock不在同一个bank group内，避免了读读冲突。 |
| 示意图 |  |  |
| 示例代码 | uint64_t mask = 128;
    UnaryRepeatParams params;
    params.dstBlkStride  = 1;
    params.srcBlkStride = 16;
    for(uint32_t i=0; i<16; i++)   {
        AscendC::Adds(dstLocal[i * 128], srcLocal[i * 16], 0, mask, 1, params);
    } | uint64_t mask = 128;
    UnaryRepeatParams params;
    params.dstBlkStride  = 8;
    params.srcBlkStride = 1;
    for(uint32_t i=0; i<8; i++)   {
        AscendC::Adds(dstLocal[i * 16], srcLocal[i * 256], 0, mask, 2, params);
    } |

-   **优化地址分配**

    实现`8192 × 8192` half ND矩阵到全局紧凑NZ布局的转换，切分后，每个Tile块完成`144 × 128` half数据的转换。通过在内存分配时适当扩大内存，调整单次搬运内相邻Datablock的地址步长，保证在同一个Copy搬运操作内，不同Datablock的写操作不会同时出现在一个bank group内。完整样例可参考[避免bank冲突样例](../../../../../../../examples/01_simd_cpp_api/05_best_practices/04_memory_access/bank_conflict_nd2nz)。
    实现方案对比如下：

    - 原始实现：不做地址优化及搬运步长，Copy时步长为144，各个Tensor的地址分别为：
        - ND矩阵：起始地址0x0，tensor长度为144 * 128 * sizeof(half)字节
        - NZ矩阵：起始地址0x12000，tensor长度为144 * 128 * sizeof(half)字节
        - 在每个Copy搬运操作内，8个Datablock同时写一个bank group，单条Copy从一拍拉长为8拍。

    ![](../../../../figures/mat_pos_69.png)

    - 优化实现：优化地址及搬运步长，为NZ矩阵多申请256字节（即多申请一行UB内存空间），Copy时步长为145，各个Tensor的地址分别为：
        - ND矩阵：起始地址0x0，tensor长度为144 * 128 * sizeof(half)字节
        - NZ矩阵：起始地址0x12000，tensor长度为145 * 128 * sizeof(half)字节
        - NZ矩阵多申请256字节，且将Copy时步长增大1，避免Copy搬运时8个Datablock同时写一个bank group，单条Copy在一拍内完成。

    ![](../../../../figures/mat_pos_70.png)

    - 原始实现示例代码：
        ```cpp
        AscendC::LocalTensor<half> nd(AscendC::TPosition::VECIN, 0, 144 * 128);
        AscendC::LocalTensor<half> nz(AscendC::TPosition::VECOUT, 2 * 144 * 128 * sizeof(half), 144 * 128);
        AscendC::CopyRepeatParams copyParams;
        copyParams.dstStride = 144;
        copyParams.srcStride = 1;
        copyParams.dstRepeatSize = 1;
        copyParams.srcRepeatSize = 128 / 16;

        AscendC::Copy(nz[0], nd[0],
            static_cast<uint64_t>(128), static_cast<uint8_t>(144), copyParams);
        ```
    - 优化实现示例代码：
        ```cpp
        AscendC::LocalTensor<half> nd(AscendC::TPosition::VECIN, 0, 144 * 128);
        AscendC::LocalTensor<half> nz(AscendC::TPosition::VECOUT, 2 * 144 * 128 * sizeof(half), 145 * 128); // 多申请256字节
        AscendC::CopyRepeatParams copyParams;
        copyParams.dstStride = 145;         // 搬运时地址步长增加1
        copyParams.srcStride = 1;
        copyParams.dstRepeatSize = 1;
        copyParams.srcRepeatSize = 128 / 16;

        AscendC::Copy(nz[0], nd[0],
            static_cast<uint64_t>(128), static_cast<uint8_t>(144), copyParams);
        ```

更多的UB Bank冲突场景分析可参考[bank冲突分析样例](../../../../../../../examples/01_simd_cpp_api/05_best_practices/04_memory_access/bank_conflict_ub)。

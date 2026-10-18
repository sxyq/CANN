# 避免bank冲突（NPU架构版本3510）

【优先级】中

>[!NOTE]说明
>该性能优化建议适用于如下产品型号：
><!-- npu="950" id1 -->
>- Ascend 950PR/Ascend 950DT
><!-- end id1 -->

Unified Buffer（UB）缓存总大小为256KB，包含8个bank group，每个bank group包含2个bank。每个bank大小为16KB，由512行组成，每行长度为32B，采用低位地址交织。

UB的地址编码格式：
```
UB_ADDR[18:0] = {BANK_DEPTH[8:0], BANK[0], BG[2:0], BANK_WIDTH[4:0]}
```

**图1**  bank结构示意图（图中箭头方向表示内存排布的顺序）

![](../../../../figures/ub_950.png "950_UB内存结构图")

3510架构使用低位编址，地址排布情况如图所示：

**图2** 低位地址交织排布

![](../../../../figures/low_interleave.png "低位交织地址排布")

每个bank可分为4个subbank，每个subbank宽度为8B，subbank仅在gather/scatter场景被感知，连续读写只感知到bank。

**图3** subbank结构

![](../../../../figures/subbank.png "subbank结构")

## bank冲突的典型场景

同一个bank group单拍支持两次读操作，或者一次读操作和一次写操作；不支持同一个bank group单拍完成两次写操作。根据内存结构，bank冲突主要可以分为以下三种类型：

-   **读写冲突**：读操作和写操作同时尝试访问同一个bank，或者两个读操作和一个写操作同时尝试访问同一个bank group。
-   **写写冲突**：多个写操作同时尝试访问同一个bank group。
-   **读读冲突**：两个读操作同时尝试访问同一个bank，或者两个以上读操作同时尝试访问同一个bank group。

**注：bank冲突的场景与UB的规格密切相关，规格的变化通常会导致bank冲突场景的变化。**

### 基于连续访存的bank冲突分析
以下对连续访存的冲突类型进行解读：

-   读写冲突

    同时读写到同一个bank时造成读写冲突，具体分析如下：

    **表1**  一读一写冲突示例



| 序号 | src地址 | dst地址 | bank | bank group | 结论 |
| --- | --- | --- | --- | --- | --- |
| 示例1 | 0x0000 | 0x4000 | bank_id0 == bank_id1 | bank_group_id0 == bank_group_id1 | src地址和dst地址属于同一bank，存在读写冲突。 |
| 示例2 | 0x0000 | 0x4100 | bank_id0 != bank_id1 | bank_group_id0 == bank_group_id1 | src地址和dst地址属于同一bank group下的不同bank，无冲突。 |

    **表2** 两读一写冲突示例


| 序号 | src0地址 | src1地址 | dst地址 | bank | bank group | 结论 |
| --- | --- | --- | --- | --- | --- | --- |
| 示例1 | 0x0000 | 0x4000 | 0x8000 | bank_id0 == bank_id1
    bank_id0 == bank_id2 | bank_group_id0 == bank_group_id1
    bank_group_id0 == bank_group_id2 | src0地址与src1地址属于同一bank，存在读读冲突。
    src0地址与dst地址属于同一bank，存在读写冲突。
    同一bank group最多支持两读或者一读一写，当前存在冲突。 |
| 示例2 | 0x0000 | 0x0000 | 0x4100 | bank_id0 != bank_id2 | bank_group_id0 == bank_group_id2 | src0和src1起始地址相同，两读被硬件优化为一读。
    src0地址与dst地址不属于同一bank，无读写冲突。 |
| 示例3 | 0x0000 | 0x4100 | 0x8900 | bank_id0 != bank_id1
    bank_id0 != bank_id2
    bank_id1 == bank_id2 | bank_group_id0 == bank_group_id1
    bank_group_id0 == bank_group_id2 | src0地址和src1地址属于同一bank group的不同bank，不产生读读冲突。
    src1地址和dst地址属于同一bank，无冲突。 |

-   写写冲突

    同时写入一个bank group造成写写冲突，具体分析如下：

    **表3**  写写冲突示例


| 序号 | dst0地址 | dst1地址 | bank | bank group | 结论 |
| --- | --- | --- | --- | --- | --- |
| 示例1 | 0x0000 | 0x4100 | bank_id0 != bank_id1 | bank_group_id0 == bank_group_id1 | dst0地址和dst1地址属于同一bank group，故存在冲突。 |

-   读读冲突

    **表4** 读读冲突示例

| 序号 | src0地址 | src1地址 | bank | bank group | 结论 |
| --- | --- | --- | --- | --- | --- |
| 示例1 | 0x0000 | 0x4000 | bank_id0 == bank_id1 | bank_group_id0 == bank_group_id1 | src0地址和src1地址属于同一bank，故存在冲突。 |

### 基于离散访存的bank冲突分析
以下对离散访存下的冲突进行解读：

-   当gather/scatter访存数据分布在不同的subbank中时，不发生冲突。
-   当gather/scatter数据存在同一subbank的同一起始地址时，不发生冲突，因此相同索引值的访存不发生冲突。
-   当gather/scatter数据存在同一subbank的不同起始地址时，发生冲突。
-   gather/scatter会与连续访存指令/MTE搬运指令冲突。

## 如何避免bank冲突

避免bank冲突的方法如下：

- 通过地址重叠减少读指令

  同一个bank group单拍仅支持两次读操作，或者一次读操作和一次写操作。两读一写会超出支持的并行访问组合而引发bank冲突；当两条读指令的起始地址重叠时，硬件会将其优化为一条。

- 离散访存下，将数据分散到不同subbank

  当gather/scatter数据存在同一subbank的不同起始地址时，发生冲突，可通过将数据分散到不同subbank来减少subbank冲突，避免不同起始地址的数据集中在同一subbank下。

- 优化计算逻辑

  对一个数据类型为float，shape为\(8, 64\)的输入每个元素加1。通过将计算逻辑由逐列计算改为逐行计算可避免同一Repeat下的冲突问题，实现方案对比如下：



| 实现方案 | 原始实现 | 优化实现 |
| --- | --- | --- |
| 实现方法 | 逐列计算，同一Repeat内输入的8个DataBlock都在同一个bank group而发生读读冲突。 | 逐行计算，同一个Repeat内输入的8个DataBlock不在同一个bank group内，避免了同一Repeat内的读读冲突。 |
| 示意图 |  |  |

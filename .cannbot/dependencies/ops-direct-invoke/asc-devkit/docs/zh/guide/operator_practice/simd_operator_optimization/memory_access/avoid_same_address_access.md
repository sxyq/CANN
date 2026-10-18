# 避免同地址访问

【优先级】高

>[!NOTE]说明
>该性能优化指导适用于如下产品型号：
><!-- npu="950" id1 -->
>- Ascend 950PR/Ascend 950DT
><!-- end id1 -->
><!-- npu="A3" id2 -->
>- Atlas A3 训练系列产品/Atlas A3 推理系列产品
><!-- end id2 -->
><!-- npu="910b" id3 -->
>- Atlas A2 训练系列产品/Atlas A2 推理系列产品
><!-- end id3 -->

【描述】MTE2、MTE3、Scalar等单元访问Global Memory数据时，其地址请求会按照512字节粒度对齐后进行处理。当同时访问Global Memory的数据，且地址处于连续的512字节范围内时，由于数据一致性的原因，多个请求会被串行处理，进而影响数据搬运效率。

当前算子执行机制保证用户核函数（Kernel）入参（包括Workspace/Tiling）的地址512字节对齐，因此开发者只需要根据地址的偏移量即可判断两个地址是否会落入连续的512字节范围内。

如下图所示，AI Core内的各个核对Global Memory的数据同时发出读写请求，尽管addr0\~addr5是多个不同的地址，但因为落在连续的512字节范围内，被视为同一个地址请求，此时这几个数据请求会被串行处理，数据访问效率会降低。同地址访问的影响受同时访问的核数影响，同地址访问的核数越多时，串行导致的性能劣化越严重。

![](../../../figures/reduce_opt.png)

避免同地址访问的方法主要是**调整数据访问顺序**。下文介绍配套的样例请参考[DataCopy最佳实践样例](../../../../../../examples/01_simd_cpp_api/05_best_practices/04_memory_access/data_copy)中的优化点4。

**调整数据访问顺序**

本节以Atlas A2 训练系列产品/Atlas A2 推理系列产品或Atlas A3 训练系列产品/Atlas A3 推理系列产品上一个形状为 [6144, 512]的half类型输入进行GM到Unified Buffer（UB）搬运这一种场景为例，说明如何通过调整数据访问顺序规避同地址访问冲突，该场景使用48个核参与搬运，即`numBlocks=48`。

为了体现同地址冲突的影响，上述场景关闭L2Cache hint。输入每行数据大小为1024字节（512个half），Tile为 \([128, 64]\)，通过`DataCopyPad`完成GM到UB搬运。每个核都会完整载入一遍输入矩阵，N方向按64列切分，单次搬运数据量为`128 * 64 * 2`字节。原始实现中，所有核按相同的mBlockIdx顺序访问同一输入矩阵，容易在同一时刻访问相同的GM地址段；优化实现中，每个核按 \((mBlockIdx + blockIdx) \% numBlocks\)在块组内轮转访问顺序，降低同地址访问冲突概率。

下文示意图中，M0-M47表示输入矩阵沿M方向切分后的48个分块，T0-T47表示时间轴上的48个搬运时刻。

| 实现方案 | 原始实现 | 优化实现 |
| --- | --- | --- |
| 实现方法 | offsetAddr=false，所有核按相同顺序访问完整输入矩阵，均从同一个mBlockIdx开始搬运，容易产生同地址访问冲突。 | offsetAddr=true，所有核按块组内轮转顺序访问完整输入矩阵，使不同核在同一时刻尽量访问不同的GM地址段。 |
| 示意图 |  |  |
| 示例代码 | uint32_t blockIdx = AscendC::GetBlockIdx();
for (uint32_t mBlockIdx = 0;
     mBlockIdx < fullMBlockCount;
     mBlockIdx++) {
    // 原始实现：所有核使用相同的mBlockIdx。
    uint32_t curMBlockIdx = mBlockIdx;
    uint32_t mStart = curMBlockIdx * singleCoreM;
    DataCopyPad(...) // 搬运逻辑相同

} | uint32_t blockIdx = AscendC::GetBlockIdx();
for (uint32_t mBlockIdx = 0;
     mBlockIdx < fullMBlockCount;
     mBlockIdx++) {
    // 优化实现：在每组numBlocks 个M块内，
    // 按当前核的blockIdx轮转访问顺序。
    uint32_t blockGroupStart =
        (mBlockIdx / numBlocks) * numBlocks;
    uint32_t curMBlockIdx = blockGroupStart +
        (mBlockIdx + blockIdx) % numBlocks;
    uint32_t mStart = curMBlockIdx * singleCoreM;
    DataCopyPad(...) // 搬运逻辑相同
} |
| 访问顺序示例 | 本样例中numBlocks=48。原始实现下，48个核在同一时刻访问相同的curMBlockIdx：T0时刻均访问M0，T1时刻均访问M1，T2时刻均访问M2，依次类推。 | 本样例中numBlocks=48。优化实现下，不同核在同一时刻访问不同的curMBlockIdx：T0时刻block0访问M0、block1访问M1、block46访问M46、block47访问M47；T1时刻分别访问M1、M2、M47、M0。 |

>[!NOTE]说明
>你可以通过执行如下命令行，通过msOpProf工具获取上述示例的性能数据并进行对比。
>```
>msopprof ./demo
>```
>重点关注op\_summary\_\*.csv中的MTE2搬运相关指标。

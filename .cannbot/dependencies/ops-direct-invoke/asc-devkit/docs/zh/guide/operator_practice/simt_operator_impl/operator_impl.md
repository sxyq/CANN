# 算子实现

本章节将以Gather类算子为例，介绍SIMT算子实现的基本流程，如下图所示：

![](../../figures/tik2_vec.png)

-   算子分析与核函数（Kernel）定义：明确算子的输入和输出，分析最大线程数的设置方案。明确算子核函数（Kernel）名、输入输出参数，确定动态参数空间大小，配置最大线程数。
-   Host侧线程切分计算：根据输入数据的shape信息，计算和设置gridDim、blockDim等参数。
-   核函数（Kernel）侧算子实现：实现单线程内的计算逻辑。

下文将对上述步骤进行详细介绍。完整的算子实现请参考[gather算子实现样例](../../../../../examples/03_simt_api/00_introduction/01_gather/general_gather)。

## 算子分析与核函数（Kernel）定义

算子分析具体步骤如下：

1.  明确算子的功能和计算逻辑。

    gather算子的功能为从输入张量中获取指定索引行的数据，即从形状为M \* N的二维张量input中获取指定索引的m行数据，这m行的行索引由输入index指定。算子输出output第i行数据的计算公式为：

    ```cpp
    output[i] = input[index[i]]
    ```

2.  明确算子的输入和输出。
    -   gather算子有两个输入：input与index；输出为output。
    -   本样例中算子输入input的数据类型支持float、half、int32\_t，index的数据类型为uint32\_t，算子输出的数据类型与输入的数据类型相同。
    -   每个线程处理一行数据，需要传入每行数据的长度in\_width以及需要处理的总行数index\_total\_length，以确保尾部线程不会进行无效操作。
    -   本样例中算子无需使用大量临时变量，因此直接将核函数（Kernel）的最大线程数设置为2048。

3.  明确函数名称和参数。
    -   自定义核函数（Kernel）名称，本样例中核函数（Kernel）命名为gather\_custom。
    -   通过分析算子的输入和输出，使用模板参数来支持不同的输入输出数据类型。



| 模板参数名 | 模板参数类型 | 参数定义 |
| --- | --- | --- |
| type_data | typename | 输入输出的数据类型 |
| type_idx | typename | index的数据类型 |

        函数入参定义如下：



| 参数名 | 参数类型 | 参数定义 |
| --- | --- | --- |
| input | type_data* | 输入数据在Global Memory上的内存地址 |
| index | type_idx* | 索引数据在Global Memory上的内存地址 |
| gather_output | type_data* | 输出数据在Global Memory上的内存地址 |
| in_width | uint32_t | 输入数据第二维的长度（列宽） |
| index_total_length | uint32_t | index数据的总长度 |

4.  明确SIMT核函数（Kernel）gridDim、blockDim等动态参数设置方案。
    -   本样例采用均匀切分方案，根据可用核数和最大线程数的限制，计算和调整gridDim（启用的线程块的个数）、blockDim（一个线程块启用的线程个数），同时保证gridDim不超过65535、blockDim不超过最大线程数2048。
    -   本算子实现逻辑中无需使用动态Unified Buffer（UB）空间。

通过以上分析，得到SIMT Gather算子的设计规格如下：

-   算子类型（OpType）：Gather
-   算子输入输出：

    **表1**  Gather算子输入输出规格



| name | shape | data type | format |
| --- | --- | --- | --- |
| input（输入） | (M, N) | float/half/int32_t | ND |
| index（输入） | (m), m < M | uint32_t | ND |
| output（输出） | (m, N) | float/half/int32_t | ND |

-   核函数（Kernel）名称：gather\_custom

核函数（Kernel）定义如下：

```cpp
constexpr uint32_t MAX_THREAD_COUNT = 2048;

template <typename type_data, typename type_idx>
__global__ __launch_bounds__(MAX_THREAD_COUNT) void gather_custom(
    type_data* input,
    type_idx* index,
    type_data* gather_output,
    uint32_t in_width,
    uint32_t index_total_length)
```

>[!NOTE]说明
>在定义核函数（Kernel）时，使用\_\_launch\_bounds\_\_\(MAX\_THREAD\_COUNT\)来指定最大线程数。最大线程数的设置范围为1到2048。设置的最大线程数越大，支持启用的线程越多，性能越好，但每个线程可使用的内部寄存器数量会减少。若未设置，最大线程数默认值为1024。在上述分析中已明确计算不需要过多寄存器，因此设置最大线程数为2048。在实际的算子开发过程中，应根据具体的算子实现来调整该值。

## Host侧线程切分计算

本样例以简单的均匀切分方案介绍如何实现动态切分参数的计算。

1.  设置初始gridDim。

    考虑到如果gridDim设置得比实际AIV核数少，会导致空闲核浪费，因此将初始gridDim设置为当前芯片的实际AIV核数量。AIV数量的获取方法如下所示：

    ```cpp
    uint32_t real_core_num = 0;
    const auto& platformInfoMgr = platform_ascendc::PlatformAscendCManager::GetInstance();
    real_core_num = platformInfoMgr->GetCoreNumAiv();
    blocks_per_grid = real_core_num; // blocks_per_grid为初始gridDim
    ```

2.  计算blockDim。

    根据输入index的长度index\_total\_length、初始gridDim计算一个线程块启用的线程个数blockDim。

    ```cpp
    // threads_per_block为blockDim值
    threads_per_block = (index_total_length + blocks_per_grid - 1) / blocks_per_grid;
    ```

3.  调整blockDim。

    若blockDim超出最大线程数限制，调整blockDim值为最大线程数值。

    ```cpp
    if (threads_per_block > MAX_THREAD_COUNT) {
        threads_per_block = MAX_THREAD_COUNT;
    }
    ```

4.  调整gridDim。

    重新计算gridDim，确保gridDim \* blockDim ≥ index\_total\_length，即确保所有启用的线程能够处理完指定行数的数据。

    ```cpp
    blocks_per_grid = (index_total_length + threads_per_block - 1) / threads_per_block;
    ```

完整的切分计算代码如下：

```cpp
constexpr uint32_t MAX_THREAD_COUNT = 2048;
constexpr uint32_t MAX_BLOCK_COUNT = 65535;

bool block_split(uint32_t index_total_length, uint32_t &blocks_per_grid, uint32_t &threads_per_block) {
    uint32_t real_core_num = 0;
    const auto& platformInfoMgr = platform_ascendc::PlatformAscendCManager::GetInstance();
    if (platformInfoMgr == nullptr) {
        std::cout << "[ERROR] Get platform info failed, please check device status."<< std::endl;
        return false;
    }
    real_core_num = platformInfoMgr->GetCoreNumAiv();
    blocks_per_grid = real_core_num;
    threads_per_block = (index_total_length + blocks_per_grid -1) / blocks_per_grid;
    if (threads_per_block > MAX_THREAD_COUNT) {
        threads_per_block = MAX_THREAD_COUNT;
        blocks_per_grid = (index_total_length + threads_per_block - 1) / threads_per_block;
        if (blocks_per_grid > MAX_BLOCK_COUNT) {
            std::cout << "[ERROR] index_total_length: "<< index_total_length << " can not be bigger than "
                << MAX_THREAD_COUNT * MAX_BLOCK_COUNT<< "."<< std::endl;
            return false;
        }
    }
    return true;
}
```

## 核函数（Kernel）侧算子实现

1.  根据均匀切分算法，获取当前线程的位置偏移量。

    在本算子中，仅使用gridDim、blockDim等线程维度的第一维，因此计算偏移量时只需考虑x维信息。如下代码所示，threadIdx表示线程在其所在线程块内的索引，blockDim表示一个线程块中设置的线程数，而blockIdx表示线程块的索引。

    ```cpp
    // 计算线程索引
    int32_t out_row = blockIdx.x * blockDim.x + threadIdx.x;
    ```

2.  根据线程索引，获取当前线程需要处理数据的行索引，计算对应的输入、输出位置偏移量，实现整行数据的获取采集。

    ```cpp
    uint32_t in_row = index[out_row];
    int input_idx = in_row * in_width;
    int output_idx = out_row * in_width;
    for (int32_t col = 0; col < in_width; col++) {
        gather_output[output_idx] = input[input_idx];
        input_idx += 1;
        output_idx += 1;
    }
    ```

完整的核函数（Kernel）功能代码如下：

```cpp
constexpr uint32_t MAX_THREAD_COUNT = 2048;
constexpr uint32_t MAX_BLOCK_COUNT = 65535;

template <typename type_data, typename type_idx>
__global__ __launch_bounds__(MAX_THREAD_COUNT) void gather_custom(
    type_data* input,
    type_idx* index,
    type_data* gather_output,
    uint32_t in_width,
    uint32_t index_total_length)
{
    // Calculate global thread ID
    int32_t out_row = blockIdx.x * blockDim.x + threadIdx.x;
    // Maps to the row index of output tensor
    if (out_row >= index_total_length) {
        return;
    }
    // Single thread processes entire row (all columns) - enables coalesced memory access
    uint32_t in_row = index[out_row];
    int input_idx = in_row * in_width;
    int output_idx = out_row * in_width;
    for (int32_t col = 0; col < in_width; col++) {
        gather_output[output_idx] = input[input_idx];
        input_idx += 1;
        output_idx += 1;
    }
}
```

## 运行验证

算子核函数（Kernel）程序开发完成后，即可编写Host侧的核函数（Kernel）调用程序，实现从Host侧的APP程序调用算子，进行运行验证。

Host侧的关键代码如下：

```cpp
std::vector<float> gather(std::vector<float>& input, const uint32_t* in_shape, std::vector<uint32_t>& index)
{
    ...
    // 计算切分参数

    uint32_t blocks_per_grid = 0;
    uint32_t threads_per_block = 0;
    block_split(index_total_length, blocks_per_grid, threads_per_block);

    ...
    // 本算子无需动态UB内存
    uint32_t dyn_ubuf_size = 0;  // No need to alloc dynamic memory.

    // 用核函数（Kernel）调用符<<<...>>>调用核函数（Kernel）完成指定的运算

    gather_custom<<<blocks_per_grid, threads_per_block, dyn_ubuf_size, stream>>>(
              input_device, index_device, output_device, in_shape[1], index_total_length);
    ...
}
```

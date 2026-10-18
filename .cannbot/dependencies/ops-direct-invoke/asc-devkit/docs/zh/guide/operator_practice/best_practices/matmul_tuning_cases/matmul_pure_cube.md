# Matmul高阶API开启纯Cube模式

## 案例介绍

本案例呈现了在矩阵乘算子场景中，开启Matmul高阶API的纯Cube模式对算子性能的提升效果。如下图所示，Matmul API默认使用MIX模式，即用户从AIV侧发起消息，通过消息通信框架中转消息后，在AIC侧执行Matmul计算。这套消息处理机制会带来额外的Scalar性能开销。相较于MIX模式，纯Cube模式可以直接跳过消息通信框架，完成Matmul计算，提升算子性能。

**图1**  默认MIX模式的Matmul流程示意图
![](../../../figures/mix_matmul.png "默认MIX模式的Matmul流程示意图")

-   开启纯Cube模式的适用场景

    非融合算子，只有矩阵计算的场景。即相较于MIX模式（包含矩阵计算和矢量计算），没有矢量计算的场景。本案例的算子规格如下：

**表1**  算子用例规格

| 输入 | Shape | Data type | Format |
| --- | --- | --- | --- |
| a | 128, 64 | float16 | ND |
| b | 64, 30720 | float16 | ND |

当前案例使用的AI处理器共24个核，每个核中包含1个AIC核和2个AIV核。

Tiling参数如下：

-   原始shape：M=128, N=30720, K=64。
-   单核shape：
    -   MIX场景：按48个AIV核进行切分，singleCoreM=128，singleCoreN=640，singleCoreK=64。
    -   纯Cube场景：按24个AIC核进行切分，singleCoreM=128，singleCoreN=1280，singleCoreK=64。

-   基本块shape：baseM=128，baseN=256，baseK=64。
-   L1相关Tiling参数：stepM=1，stepN=1，stepKa=4，stepKb=4，depthA1=8，depthB1=8。

## 获取性能数据

使用msOpProf工具获取[算子仿真流水图](../../performance_analysis/collect_performance_data.md#section17259539153513)和[上板Profiling](../../performance_analysis/collect_performance_data.md#section17953123893415)数据。因为纯Cube模式主要优化Scalar流水性能，可以重点分析Scalar的流水情况。

## 分析主要瓶颈点

-   优化前的Profiling数据如下，从C列的aic\_time数据可以看出，多个核中最大算子执行耗时为17.85us。从G列的aic\_scalar\_time数据可以看出，Scalar平均耗时为15.02us，性能瓶颈在Scalar流水。

    ![](../../../figures/zh-cn_image_0000002411013306.png)

-   优化前的流水图如下，由于默认为MIX模式，每次Matmul计算均涉及消息通信框架对消息进行处理，Scalar流水重，性能开销较大，如下图红框所示。

    ![](../../../figures/zh-cn_image_0000002390760922.png)

## 设计优化方案

默认MIX模式下，用户在AIV侧发起消息，通过消息通信框架中转消息后，在AIC侧执行Matmul计算。基于这样的流程，用户使用Matmul高阶API编写算子代码时，可以使用[REGIST\_MATMUL\_OBJ](../../../../api/SIMD-API/adv_api/cube_compute/Matmul_Kernel/REGIST_MATMUL_OBJ.md)宏，无需区分AIV和AIC，但也因这套消息处理机制导致产生了额外的性能开销，如[图1默认MIX模式的Matmul流程示意图](#fig0672118378)所示。

实现默认MIX模式的具体步骤如下：

1.  核函数（Kernel）侧，定义Matmul对象。

    ```
    #include "lib/matmul_intf.h"

    using A_TYPE = AscendC::MatmulType<AscendC::TPosition::GM, CubeFormat::ND, AType>;
    using B_TYPE = AscendC::MatmulType<AscendC::TPosition::GM, CubeFormat::ND, BType>;
    using C_TYPE = AscendC::MatmulType<AscendC::TPosition::GM, CubeFormat::ND, CType>;
    using BIAS_TYPE =  AscendC::MatmulType<AscendC::TPosition::GM, CubeFormat::ND, BiasType>;
    AscendC::Matmul<A_TYPE, B_TYPE, C_TYPE, BIAS_TYPE, CFG_NORM> matmulObj;
    ```

2.  Host侧，Matmul多核Tiling对象调用SetDim接口设置参与运算的核数。

    ```
    auto ascendcPlatform = platform_ascendc::PlatformAscendCManager::GetInstance();
    matmul_tiling::MultiCoreMatmulTiling cubeTiling(*ascendcPlatform);
    int32_t numBlocks = ascendcPlatform->GetCoreNumAiv(); // MIX模式使用GetCoreNumAiv获取AI处理器可用的核数。
    cubeTiling.SetDim(numBlocks);
    ```

3.  调用核函数（Kernel），参考[核函数（Kernel）定义和调用](../../../programming_guide/programming_model/ai_core_simd_programming/kernel_function.md#zh-cn_topic_0000001447989210_section1915102519220)，设置核函数（Kernel）的numBlocks参数配置。

    ```
    matmul_custom_do(ascendcPlatform->GetCoreNumAic(), stream, x1, x2, bias, y, workspaceDevice, tilingDevice); // MIX模式下，启动时，按照AIV和AIC组合启动，numBlocks用于设置启动多少个AI Core。
    ```

在没有矢量计算的算子场景下，可以跳过消息通信框架的机制，开启纯Cube模式完成Matmul计算，减少消息通信的性能开销，提升算子性能。

**图2**  纯Cube模式的Matmul流程示意图
![](../../../figures/cube_matmul.png "纯Cube模式的Matmul流程示意图")

Matmul API开启纯Cube模式的完整样例请参考[纯Cube模式的Matmul样例](../../../../../../examples/01_simd_cpp_api/04_advanced_api/00_matmul/matmul)。开启纯Cube模式的主要步骤如下：

1.  核函数（Kernel）侧，在定义Matmul对象的代码中，包含matmul\_intf.h头文件前设置ASCENDC\_CUBE\_ONLY宏。

    ```
    #define ASCENDC_CUBE_ONLY // 在#include "lib/matmul_intf.h"前，设置ASCENDC_CUBE_ONLY宏
    #include "lib/matmul_intf.h"

    using A_TYPE = AscendC::MatmulType<AscendC::TPosition::GM, CubeFormat::ND, AType>;
    using B_TYPE = AscendC::MatmulType<AscendC::TPosition::GM, CubeFormat::ND, BType>;
    using C_TYPE = AscendC::MatmulType<AscendC::TPosition::GM, CubeFormat::ND, CType>;
    using BIAS_TYPE =  AscendC::MatmulType<AscendC::TPosition::GM, CubeFormat::ND, BiasType>;
    AscendC::Matmul<A_TYPE, B_TYPE, C_TYPE, BIAS_TYPE, CFG_NORM> matmulObj;
    ```

2.  Host侧，Matmul多核Tiling对象调用SetDim接口设置参与运算的核数。

    ```
    auto ascendcPlatform = platform_ascendc::PlatformAscendCManager::GetInstance();
    matmul_tiling::MultiCoreMatmulTiling cubeTiling(*ascendcPlatform);
    int32_t numBlocks = ascendcPlatform->GetCoreNumAic(); // 纯Cube模式使用GetCoreNumAic接口获取AI处理器可用的核数。
    cubeTiling.SetDim(numBlocks);
    ```

3.  调用核函数（Kernel），参考[核函数（Kernel）定义和调用](../../../programming_guide/programming_model/ai_core_simd_programming/kernel_function.md#zh-cn_topic_0000001447989210_section1915102519220)，设置核函数（Kernel）的numBlocks参数配置。

    ```
    matmul_custom_do(ascendcPlatform->GetCoreNumAic(), stream, x1, x2, bias, y, workspaceDevice, tilingDevice); // 仅包含Cube计算的算子，numBlocks用于设置启动多少个AIC。
    ```

4.  核函数（Kernel）侧，核函数（Kernel）实现中增加AIV侧返回分支。

    ```
    extern "C" __global__ __aicore__ void matmul_custom(__gm__ uint8_t* a, __gm__ uint8_t* b,
        __gm__ uint8_t* bias, __gm__ uint8_t* c, __gm__ uint8_t* workspace, __gm__ uint8_t* tilingGm)
    {
        if (g_coreType == AscendC::AIV) { // 纯Cube模式，AIV侧直接return
            return;
        }
        ...
        // 其他代码
    }
    ```

## 验证优化方案性能收益

-   优化后的Profiling数据如下，从C列的aic\_time数据来看，多个核中最大算子执行耗时为11.21us，较优化前的17.85us有较大提升。从G列的aic\_scalar\_time数据来看，Scalar平均耗时从优化前的15.02us降低至5.17us。

    ![](../../../figures/zh-cn_image_0000002411013940.png)

-   优化后的流水图如下。对比优化前的流水图，红框所示位置的Scalar流水明显变稀疏。纯Cube模式相较于MIX模式，减少了对消息通信的处理，优化了整体Scalar性能开销。

    ![](../../../figures/zh-cn_image_0000002377887318.png)

## 总结

在只有矩阵计算，没有矢量计算的场景下，可以考虑开启纯Cube模式，优化Matmul计算中的消息通信性能开销，提升算子性能。

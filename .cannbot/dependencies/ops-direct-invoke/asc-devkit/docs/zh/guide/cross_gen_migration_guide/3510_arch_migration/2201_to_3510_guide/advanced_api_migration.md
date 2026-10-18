# 高阶API迁移指导

Ascend C高阶API基本兼容[NPU架构版本3510](../../../programming_guide/language_extension/simd_builtin_keywords.md#npu-arch)与[NPU架构版本2201](../../../programming_guide/language_extension/simd_builtin_keywords.md#npu-arch)，部分API进行了扩展。当前3510架构不支持卷积计算类高阶API。

## Matmul类高阶API

-   支持的数据类型有变化。

    **表1**  数据类型兼容性情况



| A矩阵 | B矩阵 | Bias矩阵 | C矩阵 | 说明 |
| --- | --- | --- | --- | --- |
| int4b_t | int4b_t | int32_t | int32_t、half | 3510架构不支持。 |
| fp8_e4m3fn_t、fp8_e5m2_t | fp8_e4m3fn_t、fp8_e5m2_t | float、half、bfloat16_t | fp8_e4m3fn_t、half、bfloat16_t、float | 3510架构新增。 |
| hifloat8_t | hifloat8_t | float、half、bfloat16_t | hifloat8_t、half、bfloat16_t、float | 3510架构新增。 |
| float | float | bfloat16_t | float、half、bfloat16_t | 3510架构新增。 |
| bfloat16_t | bfloat16_t | bfloat16_t | float、half、bfloat16_t | 3510架构新增。 |
| half | half | bfloat16_t | float、half、bfloat16_t | 3510架构新增。 |
| int8_t | int8_t | int32_t | bfloat16_t | 3510架构新增。 |

-   不支持4:2稀疏特性。具体兼容方案请参考[4：2结构化稀疏功能](basic_api_migration.md#li69092585134)。

## 其它高阶API

**表2**  数学计算

| AscendC高阶API | 兼容说明 |
| --- | --- |
| Tanh、Asin、Sin、Acos、Cos、Log、Atan、Fmod | 兼容2201架构。
扩展支持算法配置，通过模板参数配置API使用的算法，从而提供高精度、高性能的算法选择。 |
| Sinh、Cosh、Tan、Trunc、Frac、Erf、Erfc、Atanh、Asinh、
Acosh、Floor、Ceil、Round、Axpy、Exp、Lgamma、Digamma、Xor、Cumsum | 兼容2201架构。 |
| Power | 兼容2201架构。
扩展支持uint8_t、int8_t、uint16_t、int16_t、uint32_t、bfloat16_t数据类型。 |
| Sign | 兼容2201架构。
扩展支持int64_t数据类型。 |

**表3**  激活函数

| AscendC高阶API | 兼容说明 |
| --- | --- |
| SoftMax、SimpleSoftMax、SoftmaxFlash、SoftmaxGrad、SoftmaxFlashV2、SoftmaxFlashV3、SoftmaxGradFront、AdjustSoftMaxRes、LogSoftMax、FasterGelu、FasterGeluV2、Gelu、SwiGLU、Silu、Swish、GeGLU、ReFlu、Sigmoid | 兼容2201架构。 |

**表4**  数据归一化

| AscendC高阶API | 兼容说明 |
| --- | --- |
| LayerNormGrad、LayerNormGradBeta、RmsNorm、BatchNorm、DeepNorm、GroupNorm | 兼容2201架构。 |
| Normalize、WelfordUpdate | 兼容2201架构。
扩展支持bfloat16_t数据类型。 |
| LayerNorm | 兼容2201架构。
扩展支持求方差。 |
| WelfordFinalize | 兼容2201架构。
扩展支持算法配置，通过模板参数指定在计算方差时是否使用修正系数。 |

**表5**  量化操作

| AscendC高阶API | 兼容说明 |
| --- | --- |
| AscendQuant | 兼容2201架构。
扩展支持PRE_TOKEN量化、PRE_GROUP量化。
扩展支持从half、bfloat16_t、float类型到fp8_e5m2_t、fp8_e4m3fn_t、hifloat8_t、int8_t类型的量化。
扩展支持从half、bfloat16_t类型到fp4x2_e1m2_t、fp4x2_e2m1_t类型的量化。 |
| AscendDequant | 兼容2201架构。
扩展支持PRE_TOKEN量化、PRE_GROUP量化。
扩展支持从int32_t类型到half、bfloat16_t、float类型反量化。
扩展支持从float类型到half、bfloat16_t、float类型的反量化。 |
| AscendAntiQuant | 兼容2201架构。
扩展支持PRE_TOKEN量化、PRE_GROUP量化。
扩展支持从int8_t、hifloat8_t、fp8_e5m2_t、fp8_e4m3fn_t类型到half、bfloat16_t、float、half类型的伪量化。
扩展支持从fp4x2_e1m2_t、fp4x2_e2m1_t类型到half、bfloat16_t类型的伪量化。 |

**表6**  归约操作

| AscendC高阶API | 兼容说明 |
| --- | --- |
| Sum、Mean、ReduceXorSum、ReduceMean、ReduceAny、ReduceAll、ReduceProd | 兼容2201架构。 |
| ReduceSum | 兼容2201架构。
扩展支持int32_t、uint32_t、int64_t、uint64_t数据类型。 |
| ReduceMax、ReduceMin | 兼容2201架构。
扩展支持int8_t、uint8_t、int16_t、uint16_t、bfloat16_t、int32_t、uint32_t、int64_t、uint64_t数据类型。 |

**表7**  排序操作

| AscendC高阶API | 兼容说明 |
| --- | --- |
| Concat、Extract、GetSortOffset、GetSortLen、MrgSort | 兼容2201架构。 |
| TopK | 兼容2201架构。
使用RADIX_SELECT算法时，扩展支持uint8_t、int8_t、uint16_t、int16_t、uint32_t、int32_t、bfloat16_t、uint64_t、int64_t数据类型。 |
| Sort | 兼容2201架构。
扩展支持算法配置，通过模板参数指定排序算法以及降序升序排序。 |

**表8**  索引计算

| AscendC高阶API | 兼容说明 |
| --- | --- |
| Arange | 兼容2201架构。
扩展支持int64_t数据类型。 |

**表9**  数据过滤

| AscendC高阶API | 兼容说明 |
| --- | --- |
| Select | 兼容2201架构。 |
| DropOut | 兼容2201架构。
扩展支持bfloat16_t数据类型。 |

**表10**  张量变换

| AscendC高阶API | 兼容说明 |
| --- | --- |
| Transpose | 兼容2201架构。
新增支持数据排布转换场景：

- 二维转置或者三维的后两位转置。
- 三维中的第一维和第二维互换。
- 三维中的第一维和第三维互换。
- 使用交织指令对二维ND2NZ转置。 |
| TransData、Pad、UnPad | 兼容2201架构。 |
| BroadCast | 兼容2201架构。
扩展支持动态Shape。
扩展支持int16_t、uint16_t、bfloat16_t、int32_t、uint32_t数据类型。 |
| Fill | 兼容2201架构。
扩展支持uint8_t、int8_t、bfloat16_t、uint64_t、int64_t数据类型。 |

**表11**  Hccl

| AscendC高阶API | 兼容说明 |
| --- | --- |
| Hccl模板参数 | 支持HCCL_SERVER_TYPE_CCU服务端类型。 |
| InitV2、SetCcTilingV2、AllReduce、AllGather、ReduceScatter、AlltoAll、AlltoAllV、Commit、Wait、Finalize | 兼容2201架构。 |
| BatchWrite、iterate、Query、InterHcclGroupSync、GetWindowsInAddr、GetWindowsOutAddr、GetRankId、GetRankDim、QueueBarrier、GetQueueNum | 3510架构暂不支持。 |
| SetReduceType、AlltoAllvWrite | 3510架构新增。 |

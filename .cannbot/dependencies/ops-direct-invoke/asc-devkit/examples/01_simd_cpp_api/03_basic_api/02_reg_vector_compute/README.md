# 向量计算类api样例介绍

## 概述

本路径下包含了与向量计算相关的多个API的样例。每个样例均基于Ascend C的<<<>>>直调方法，支持main函数和kernel函数在同一个cpp文件中实现。

## 算子开发样例

| 目录名称 | 功能描述                                                                                                                     |
| -------------------------------- |-----------------------------------------------------------------------------------------------------|
| [abs](./abs) | 本样例基于Reg编程接口实现Abs运算，Relu/Exp/Sqrt/Ln/Log/Log2/Log10/Neg接口皆可参考该样例                                      |
| [arange](./arange)　　　　　　 | 本样例基于Reg编程接口实现Arange运算，以标量值为起始生成递增/递减索引序列                                      |
| [cast](./cast)　　　　　　　　 | 本样例基于Reg编程接口实现Cast运算，用于RegTensor数据类型转换（位宽大转小/小转大）                              |
| [compare](./compare)　　　　　　 | 本样例基于Reg编程接口实现Compare运算，用于比较两个向量或向量与标量的大小关系                                 |
| [data_relayout](./data_relayout) | 本样例基于Reg编程接口演示Interleave和Pack接口实现数据重排（Interleave交织 + Pack低位提取）                 |
| [duplicate](./duplicate)　　　 | 本样例基于Reg编程接口实现Duplicate运算（标量填充），将标量值填充到向量的每个位置                              |
| [gather](./gather) | 本样例基于Reg编程接口演示Gather接口（源操作数为寄存器）按索引收集元素，使用LoadAlign/StoreAlign的postUpdate模式           |
| [histogram](./histogram)　　　　　　　　　　　　　　　　　　　　| 本样例基于Reg编程接口实现Histogram运算，对输入数据进行直方图统计计算　　　　　  |
| [mergemode](./mergemode) | 本样例基于Reg编程接口综合演示MaskMergeMode::MERGING + postUpdate + UpdateMask + ReduceMax                       |
| [mul](./mul) | 本样例基于Reg编程接口实现Mul运算，Add/Sub/Div/Max/Min/Prelu接口皆可参考该样例                                                 |
| [muls](./muls) | 本样例基于Reg编程接口实现Muls运算，Adds/Maxs/Mins/LeakyRelu接口皆可参考该样例                                                |
| [reduce](./reduce) | 本样例基于Reg编程接口实现Reduce运算（SUM模式），Reduce接口支持SUM/MAX/MIN归约模式                                         |
| [reduce_block](./reduce_block) | 本样例基于Reg编程接口实现ReduceDataBlock运算（SUM模式），对每个DataBlock(32B)内元素归约                      |
| [reduce_pair](./reduce_pair) | 本样例基于Reg编程接口实现PairReduceElem运算（SUM模式），对相邻奇偶元素对进行归约求和                             |
| [select](./select)　　　　　　 | 本样例基于Reg编程接口实现Select运算，根据掩码在两个向量之间选取元素　　　　                                     |
| [squeeze](./squeeze)　　　　　　 | 本样例基于Reg编程接口实现Squeeze运算，将向量压缩为标量　　　　                                               |
| [subnormal](./subnormal)　　　　　　 | 本样例基于Reg编程接口实现Div运算，演示支持subnormal与不支持subnormal计算结果差异                         |
| [trait](./trait)　　　　　　 | 本样例基于Reg编程接口实现Div运算，使用int64_t类型数据，演示单核情况下RegTensor位2倍宽数据处理性能优势　　　　       |
| [truncate](./truncate)　　　　 | 本样例基于Reg编程接口实现Truncate运算，将浮点数截断到整数位                                                   |

# BestPractices样例介绍
## 概述
基于Ascend C的性能优化实践，聚焦于关键算子与内存访问的调优，旨在提升在Ascend平台上的运行效率。

## 样例列表
|  目录名称                                                   |  功能描述                                              |
| ------------------------------------------------------------ | ---------------------------------------------------- |
| [00_vector_compute](./00_vector_compute) | 基于Vector Compute API使用的优化样例，通过<<<>>>直调的实现方式，介绍基于静态Tensor方式编程的性能调优方法。 |
| [01_matrix_compute](./01_matrix_compute) | 基于Matrix Compute API使用的优化样例，介绍Matmul算子的性能调优方法。 |
| [02_reg_compute](./02_reg_compute) | 基于VF函数的性能优化样例，通过<<<>>>直调的实现方式，介绍了VF循环优化、VF指令双发优化、VF连续非对齐场景优化、VF融合优化的方法。 |
| [03_fusion_compute](./03_fusion_compute) | 融合计算相关的性能优化样例，介绍QuantGroupMatmul、Matmul+GELU等Cube-Vector融合算子的高性能实现。 |
| [04_memory_access](./04_memory_access) | 基于搬运类API使用的优化样例，通过<<<>>>直调的实现方式，介绍了减少无效数据搬运、减少搬运指令数量等方法。 |

# Ascend C API兼容性策略

本兼容性说明仅适用于Ascend C算子开发的兼容性迁移指导。总体兼容性策略见[表1](#table18109729593)，兼容性范围不包含编译器BuiltIn API、Ascend C内部实现接口等。文档中涉及的兼容性分为两类：一是功能兼容，包括数据类型兼容、接口原型兼容和常量兼容；二是性能兼容，指对于同等数据量，新架构上执行API耗时不高于旧架构。

若开发者希望在[NPU架构版本3510](../programming_guide/language_extension/simd_builtin_keywords.md#npu-arch)下运行原本在[NPU架构版本2201](../programming_guide/language_extension/simd_builtin_keywords.md#npu-arch)上开发的Ascend C程序，需在3510架构上重新编译并运行，并可能需要根据迁移指导进行代码调整。

**图1**  Ascend C API层次结构
![](../figures/api_hierarchy.png "Ascend-C-API层次结构")

**表1**  Ascend C API兼容策略

| API层级 | 兼容策略 |
| --- | --- |
| 高阶API | 高阶API在同领域内可保证兼容，特定领域扩展特性不保证兼容。 |
| 基础API | 基础API分为可兼容的基础API和ISASI基础API；兼容的API在所有架构上均能兼容；ISASI API为体系架构相关的API，不保证跨架构版本的兼容性，例如CUBE侧的计算接口LoadData、Mmad等。 |
| 语言扩展层C API | SIMD & SIMT C API可分为兼容性API和体系架构相关API；当前SIMD的C API仅支持体系架构相关，暂未支持跨代兼容。 |
| 编译器BuiltIn API | 不保证兼容。 |

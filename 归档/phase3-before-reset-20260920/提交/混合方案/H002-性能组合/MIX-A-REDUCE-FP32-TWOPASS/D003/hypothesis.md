# MIX-A D003

## Hypothesis

D001/D002 的尾块输入搬运把 `DataCopyPadExtParams::rightPadding` 按字节差传入；该参数应按元素数描述对齐后的尾部填充。testcase 2 若命中非 32 字节对齐的行宽，Pass 1 和 Pass 2 的输入复读都可能产生错误的 UB 边界访问。D003 只将 padding 改为 `AlignedElements<T>(validElements) - validElements`，保持 R006 分块归约、R017 FP32 中间、R001 两遍扫描和按行调度不变。

## Scope

- 唯一局部改动：修正 `CopyInPad` 的 `rightPadding` 单位为元素数。
- 不改变 tile 大小、归约工作区、输入复读次数、行分配或输出路径。
- 本候选只要求 server3 CANN compile PASS；不执行本地 NPU correctness、profiling 或性能比较。

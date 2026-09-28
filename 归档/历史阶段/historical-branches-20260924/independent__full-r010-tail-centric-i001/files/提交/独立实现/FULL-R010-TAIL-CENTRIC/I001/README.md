# FULL-R010-TAIL-CENTRIC / I001

本版本只实现题面定义的 AddRmsNormBias Vector 直调路线。每个外层行独立处理，完整 32B 对齐块走 `DataCopy`，最后一个不足对齐的块走 `DataCopyPad`，输出采用同样的分支，避免尾块写入相邻行。

源码按本机约定保存为单一 `kernel.txt` 提交入口。`compile_main.asc` 只用于按题目直调模板提供类型并编译包含该入口的源码；CANNJudge 只使用 `kernel.txt`。

本路线只执行 CANN 9.0 工具链编译和提交前静态核对，不执行 kernel、测试数据、精度、性能或线上提交。

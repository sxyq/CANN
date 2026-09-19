# FULL-R010-TAIL-CENTRIC / I001

本版本只实现题面定义的 AddRmsNormBias Vector 直调路线。每个外层行独立处理，完整 32B 对齐块走 `DataCopy`，最后一个不足对齐的块走 `DataCopyPad`，输出采用同样的分支，避免尾块写入相邻行。

源码按本机约定保存为 `.txt`。server3 编译时将 `op_kernel/tail_centric_kernel.txt` 和 `op_host/tail_centric_main.txt` 原样复制为对应 `.asc` 入口；本机不保留同一源码的 `.asc` 副本。

本路线只执行 CANN 9.0 工具链编译，不执行 kernel、测试数据、精度、性能或线上提交。


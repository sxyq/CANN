# FULL-R024 / I001

本 Route 的技术思想是 CPU 验证矩阵驱动。Kernel 的数据路径固定为：

```text
x + residual
→ 分块平方并沿最后一维累加
→ / D + epsilon
→ sqrt 与倒数
→ 第二遍完成 RMS 缩放、gamma、bias
→ 按输入 dtype 写回
```

`outer` 是除最后一维以外的展平行数，因此同一工程覆盖 2D、3D、4D。每行按 `tileDim` 分块，尾块只搬运有效元素；gamma 和 bias 以最后一维长度传入并在每个分块中复用。

工程入口：

- `op_kernel/addrmsnormbias_kernel.txt`：三种 dtype 的 Ascend C Kernel 入口。
- `op_kernel/addrmsnormbias_tiling.h`：Host/Kernel 共用的 Tiling 数据。
- `op_host/addrmsnormbias_host.txt`：ACL 初始化、参数准备、设备内存搬运和 launch 示例。
- `CMakeLists.txt`：CANN 9.0 Ascend C 目标。
- `compile_server.sh`：在 server3 物化 `.asc` 后执行 CMake 编译。

本机只保存 `.txt` 源码。server3 编译阶段才生成 `.asc` 文件；本 Route 不包含本机测试、NPU 运行或性能脚本。

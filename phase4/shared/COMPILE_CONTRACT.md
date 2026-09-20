# server3 编译要求

## 环境事实

- SSH 主机别名：`cann-server3`。
- 实际设备：Ascend 910B3。
- CANN 路径：`/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002`。
- 默认 toolkit 链接：`/usr/local/Ascend/ascend-toolkit/latest`。
- compiler：`bisheng`。

## 目标

所有 Kernel 只面向：

```text
--npu-soc=Ascend910B3
--npu-arch=dav-2201
```

CMake 工程需要显式设置 `SOC_VERSION=Ascend910B3`，并使用 server3 的 Ascend C 工具链。具体命令由 Child 在自己的 workspace 中建立并记录。

## 编译阶段规则

1. 从空白源码开始。
2. 建立自己的最小 build 文件。
3. 首次编译失败时，只允许处理 syntax、type、include、API signature、CMake/build 和目标平台兼容性。
4. CompileFix 不得改变核心架构，不得引入其它候选代码。
5. Compile PASS 后立即停止源码修改。

Child 完成时必须返回 compile status、compile log 路径和 source path。


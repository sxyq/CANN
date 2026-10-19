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

## 当前迭代要求

1. 在分配给自己的工作区内接续源码，复用已有构建配置。
2. 允许按真实精度、运行和耗时数据修改实现；每版说明主要变化。
3. 分别报告 device compile、submission compile、full link 的实际结果。仅生成 device object 不代表提交文件可用。
4. 线上提交文件必须带可调用的 `run_kernel`，本地完整编译需覆盖该入口和全部合法 dtype。
5. 完成一版后向 Main 返回 source path、revision、compile status、compile log、主要变化和下一实验理由；等待该版判题数据再推进下一版。
6. 构建复用固定 build 路径。完成后仅删除本任务产生、且没有进程使用的中间产物，保留源码、构建配置和日志。

公开题面当前将 `cann_version` 标为 `9.0.0`。这是线上题面字段，与上述 server3 的 `8.5.0.alpha002` 编译环境分开记录；不得把两者视为同一版本。

# Phase4 共同执行要求

## 目标硬件

- 实际设备：Ascend 910B3。
- SoC：`Ascend910B3`。
- Ascend C 架构：`DAV_2201`，编译 target 为 `dav-2201`。
- CANN：server3 上的 `8.5.0.alpha002`。
- 平台配置：20 个 Cube Core、40 个 Vector Core。
- Vector 计算宽度：128B。

## UB 预算

server3 实际配置为：

```text
TOTAL_UB_SIZE        = 192 KiB
TOTAL_VEC_LOCAL_SIZE = 184 KiB
TMP_UB_SIZE          = 8 KiB
```

候选只能把 184 KiB 作为候选 Vector 工作区上限，并从中扣除：

- TQue slots；
- 双缓冲 slots；
- 临时 tensor；
- reduction 临时区；
- 32B 对齐损耗；
- 框架或同步保留空间。

不得把 192 KiB 全部当作候选可用空间。

## 完整正确性范围

每个候选必须有自己的 hot path 和自己的 generic fallback，覆盖全部合法输入域。fallback 只负责简单、正确和完整覆盖，不得承担性能调优，也不得复制其它候选代码。

## 跨核同步

DAV_2201 存在跨核同步能力。C/D 任务必须在实际编译环境确认 API、函数签名、Kernel 类型限制、blockDim 和 workspace 要求，再自行选择实现方式。

任何跨核方案都必须使以下信息一致：

```text
used cores
blockDim
workspace layout
synchronization count
synchronization order
```

不得让未参与调度的 Core 进入等待。

## 隔离

Child 只读取 `phase4/shared/`、自己的 task 文件和自己的 workspace。不得读取归档、Git 历史、其它 task、其它 workspace、control、online 或 audit。


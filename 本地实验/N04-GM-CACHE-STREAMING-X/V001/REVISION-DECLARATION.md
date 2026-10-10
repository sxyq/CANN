# N04 V001 Revision Declaration

ROUTE=N04 GM-CACHE-STREAMING-X
REVISION=V001
DIRECT_PARENT=R31B V011 / Official 45.16
PARENT_SOURCE_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
PARENT_SOURCE_PATH=线上结果/R31B/V011/submission.asc
CANDIDATE_SOURCE_PATH=本地实验/N04-GM-CACHE-STREAMING-X/V001/Candidate.asc

## 单因素变化

只在 `ProcessWideFp32FullCacheRows` 入口，对宽行 FP32 的 `xGm_`、`residualGm_` 调用 `SetL2CacheHint(CACHE_MODE_DISABLE)`。该路径每个输入元素只被读取一次；输出和参数缓存提示保持 V011 默认值。研究目标为大工作集宽行输入流，排除 FP16 `3x20480` 稳态 L2 命中形状。

## 性能假设

大工作集输入流不再占用 L2 分配空间，可能减少对其他可复用数据的驱逐并改善 GM/L2 流式读取时间。若该输入本身受益于 L2，或地址提示带来额外开销，性能可能持平或变慢。算术、Tile 尺寸、DMA 描述符、行间访问顺序、同步和输出完全不变。

## 改动前后

- Parent 函数：`ProcessWideFp32FullCacheRows`，路径 `线上结果/R31B/V011/submission.asc`
- Candidate 函数：同一路径同一函数，仅新增两处 GlobalTensor L2 hint 调用
- Parent SHA-256：`a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- Candidate SHA-256：以 `source-meta.json` 为准
- 路线 HEAD：`1c51a5b2eb9b5b9dffc031c1fbbbc35e9506b49f`
- HEAD 父提交：`c9510be4e4a9da0084433e45442468da1eb75c72`

## RULE_REFRESH_RECEIPT

AGENTS.md、Route Skill、服务器实验规范、本地性能测试规范、Git 工作流程和当前 Route 规则已读取。实际 HEAD 为 `1c51a5b2eb9b5b9dffc031c1fbbbc35e9506b49f`，其父提交为 `c9510be4e4a9da0084433e45442468da1eb75c72`；直接性能对照为 R31B V011 / Official 45.16，源码 SHA-256 为 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`。本路线只研究大工作集 GM/L2 访问策略；不涉及寄存器生命周期、代码布局、Vector 指令并行、参数驻留、DMA 描述符或跨行预取。下一动作：保存当前结果并提交 N04 V001 源码与证据，不推送。

## 构建

- 入口：复用 `本地实验/R31B/V016/support/CMakeLists.txt`、`device.cpp`、`adapter.cpp`、`compile_adapter_v016.asc`、`link_one.sh`；Candidate 以入口所需的 `R31B-V016-WIDE-TILE-SEED_kernel.asc` 文件名放入 N04 专用远端临时目录。
- 环境：`source /usr/local/Ascend/ascend-toolkit/set_env.sh`，未启用 nounset；`ASCEND_HOME_PATH=/usr/local/Ascend/ascend-toolkit/latest`、`ASCEND_CANN_PACKAGE_PATH=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002`、`SOC_VERSION=Ascend910B3`。
- 编译参数：`CMAKE_CXX_FLAGS=-DL2_CACHE_HINT`，使 DAV-2201 的 `SetL2CacheHint(CACHE_MODE_DISABLE)` 地址策略进入编译。
- 结果：`device=PASS`、`submission=PASS`、`full_link=FAIL`。最终链接报 `undefined symbol: g_opSystemRunCfg`；该符号由 `L2_CACHE_HINT` 下的 CANN `L2CacheAlter` 引用，当前 `link_one.sh` 的单对象静态链接没有提供框架配置符号。完整命令和 stderr 保存在 `compile-v001-existing-entry.log`。
- 编译前后设备 6：HBM 使用率 `91%`，AICore `0%`，AIVector `0%`；进程 `3357697 VLLMWorker_TP`，占用 `56358 MB`。未操作该进程。

## 实验结果

COMPILE=DEVICE_PASS; SUBMISSION_PASS; FULL_LINK_FAIL_UNDEFINED_g_opSystemRunCfg; RC=2
CORRECTNESS=NOT_RUN (full_link failed; no executable)
LOCAL_SCORE=NONE
NEXT_ACTION=提交本版本；后续需沿现有 CANN 集成入口解析 g_opSystemRunCfg 的提供方，不在 Candidate 内定义替代全局符号。

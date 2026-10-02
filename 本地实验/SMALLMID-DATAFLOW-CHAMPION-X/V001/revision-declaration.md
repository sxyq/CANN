# SMALLMID-DATAFLOW-CHAMPION-X V001 Revision 声明

## Main 选择确认

- `MAIN_SELECTED`: `YES`
- `SELECTED_HYPOTHESIS`: `SMD-H6-BF16-MID-PARAM-CAST-ONCE`
- `MAIN_SELECTION_EVIDENCE`: `研究/主代理/MAIN-1-W2/campaign-status.md`, receipt commit `2a27be0b` (`origin/main1/champion-exploit`)
- `MAIN_REVIEW`: Main-1 确认 Main-2 `PARAM-RESIDENCY` 的 D>8192 GM cache-policy 路径与本提案 D<=4096 BF16 转换复用不同。

## Revision 字段

- `ROUTE`: `SMALLMID-DATAFLOW-CHAMPION-X`
- `REVISION`: `V001`
- `DIRECT_PARENT`: `R31B V011`
- `PARENT_SOURCE_SHA`: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- `PARENT_SCORE`: `45.16`
- `SINGLE_HYPOTHESIS`: 对 BF16 `ProcessNarrowMidOverlap` 的 `localRows>1` 路径，在进入行循环前将已载入的 gamma/bias 各转换为 FP32 一次，并在本核既有行段内复用。
- `CONTEXT_CLASS`: `OFFICIAL_ANCHORED_EXPLOIT`
- `WHY_NOT_DUPLICATE`: Main-1 已对照 Main-2 PARAM-RESIDENCY；其 D>8192 机制是 GM cache policy，本项针对 D<=4096 的 mid 函数逐行重复 BF16-to-FP32 参数转换。R014/FULL-R014 的参数驻留与数据搬运不等同于消除该处逐行转换；本项也不改变 EPI 逐元素算术表达式或顺序。

## 静态可达性依据

- V011 `run_kernel` 将 `availableCoreNum` 的非正值按 1 处理，再令 `blockCount=min(requestedBlocks,rowCount)` 并截到 `UINT32_MAX`。本地探针使用正的设备 `availableCoreNum=A` 且 leading-dimension 乘积 `rowCount=2*A`；在目标设备范围内原有公式给出 `baseRows=2`、`extraRows=0`、每核 `localRows=2`。不改变 block 数或 row-to-core 归属。
- BF16 目标 D 为 `2049`、`3073`、`4095`。三者均满足 V011 mid dispatch 的 `128<D<=4096`；均大于 low-precision contiguous 上限 `2048`，不会被该分支提前返回。
- BF16 narrow 初始化为 `gammaFp32Buf_`、`biasFp32Buf_` 各分配 `kCacheElems=8192` 个 FP32 元素；两个 buffer 分离，覆盖目标 D。`ProcessNarrowMidOverlap` 返回后才会进入通用参数缓存消费者，因此本 Revision 在该函数中的复用没有同次调用内的其他活跃消费者。

## 单因子限制

只移动 BF16 gamma/bias 的两次 `ToFloat`，从逐行输出阶段移到现有参数 MTE2 等待之后、行循环之前；每行改读 FP32 参数缓冲。保持现有行分配、UB 总量、GM Load、DMA、事件同步、Store 和逐元素算术次序不变。不扩大 tile，不加入第二个性能机制。

`BUILD`: `PASS` (RUN_ID `20261002T151744Z-device1`; record in `BUILD-RESULT.md`; exact candidate SHA verified; compile and link passed)
`CORRECTNESS`: `INCOMPLETE` (attempt 1 exited before main because CANN `aarch64-linux/lib64` was absent from `LD_LIBRARY_PATH`; no kernel case ran; recorded in `local-result.json`)
`EXECUTABLE_IDENTITY`: `PASS` (`159532deffb1ddaf33792c0b7c296c3bcf7cf2f0cc2ea68fbc0e8de7b68e5208`)
`LOCAL_VERDICT`: `NOT_COMPLETE` (Build passed; Correctness attempt 1 was INCOMPLETE before any case ran)
`SOURCE_STATE`: `BUILT_NOT_VALIDATED`
`SOURCE_SHA256`: `a689e5abc03d2770b277812d9a52ae0a1aaf910952b737525e1722f06bdb340a`
`SOURCE_COMMIT`: `9abea741d4d0b40efdc322da5255c463fede481b`
`SERVER_WORK`: device 1 Build 已通过；Correctness 尚未开始，未进行测时。

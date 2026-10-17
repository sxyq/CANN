# N04 V008 进展

## 假设

- 父源码：V006，提交 `0dd0d828`；V007 的首 gamma/bias tile 提前加载不纳入此候选。
- 单一变化：在 `ProcessWideLowPrecision` 的 Pass 2 中，先向备用槽提交下一组 gamma/bias 的 GM→UB 搬运，再等待当前组的 `MTE2_V` 完成。
- 备用槽覆盖前仍等待对应 `V_MTE2` 释放事件；当前参数仍要等自己的 `MTE2_V` 后才由 Vector 读取。
- 机制判断：让下一组异步 MTE2 参数搬运的请求在当前组等待期间即可排队。它和 V005 的 Pass 1 x/residual 提交次序作用于不同数据流，也不复用 V007 的首组参数提前加载。
- 源码提交：`20ba6b60`。

## 编译

- CANN 8.5.0.alpha002，Ascend910B3；现有 paired Runner 与 CMake 入口复用成功。
- CMake 配置：PASS，RC=0；编译/链接：PASS，RC=0。
- 构建产物：`/tmp/cann-n04-dma-pipeline-overlap-20261016/V008/build/paired_runner_v016`。
- 原始记录：[server3-configure.log](server3-configure.log)、[server3-compile.log](server3-compile.log)。配置输出有 `libstdc++.so.6` runtime search-path warning，目标已成功链接。
- 为先完成四组 Correctness 再测 Local，在本路线现有 paired Runner 增加 `--correctness` 单独运行方式；原 `--paired --device ID` 配对采样路径不变，源码中没有 `aclrtResetDevice`。
- Runner 重编第一次因未导出 `CPLUS_INCLUDE_PATH` 失败（RC=2，`vector` 头文件未找到）；沿用本路线此前成功使用的 include 路径重编通过（RC=0）。记录：[server3-runner-compile.log](server3-runner-compile.log)。

## Correctness 与 Local

- 使用 Main 分配的 NPU7 lease `N04-V008-D7-CORR-LOCAL-20261016T102551Z`；未触碰 NPU4。
- 启动前快照 10:37:31Z：Health=OK、HBM 使用率 5%、AICore=0%、设备进程=NONE；Correctness 后、Local 前快照 10:38:43Z 仍为 Health=OK、AICore=0%、设备进程=NONE。
- Correctness：rows=40，FP16/BF16、D=12288/32768 四组全部 PASS，RC=0；原始输出：[server3-v008-correctness-device7-20261016.txt](server3-v008-correctness-device7-20261016.txt)。
- Local：`./paired_runner_v016 --paired --device 7`，四组各 21 对；开始 10:39:23Z，结束 10:39:31Z，RC=0。四组 Correctness 在配对运行中再次 PASS。

| Shape | Parent median (us) | Candidate median (us) | Paired delta median (us) | Delta P10–P90 (us) |
|---|---:|---:|---:|---:|
| FP16 D=12288 | 22.440 | 22.000 | +0.340 | -8.440 to +9.060 |
| FP16 D=32768 | 38.820 | 33.700 | -5.280 | -5.740 to +9.100 |
| BF16 D=12288 | 27.000 | 23.160 | -5.580 | -14.900 to +5.080 |
| BF16 D=32768 | 70.460 | 61.140 | -8.960 | -28.100 to +1.960 |

- 四组 delta 的 P10–P90 均跨零，`LOCAL_SCORE=NONE`，不送 Official。
- Preflight：[server3-before-paired-device7-20261016.txt](server3-before-paired-device7-20261016.txt)；Local 前快照：[server3-before-local-device7-20261016.txt](server3-before-local-device7-20261016.txt)；postflight：[server3-after-paired-device7-20261016.txt](server3-after-paired-device7-20261016.txt)，10:39:50Z Health=OK、HBM 使用率 5%、AICore=0%、设备进程=NONE。
- 配对原始输出：[server3-v008-paired-device7-20261016.txt](server3-v008-paired-device7-20261016.txt)。

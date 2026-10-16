# N04 V012 结果

## 假设与单一变化

- 父版本：V011，源码结果提交 `8fa17527`。
- Candidate：本目录 `Candidate.asc`。
- 目标：FP16、rows=40、D=32768 的宽行 Pass 1/Pass 2 交界。
- 单一变化：最后一个 Pass 1 输入 tile 已提交后，复用已完成的 `rd0` MTE2→V 事件，把 gamma/bias 的首个参数 tile 搬入第三输入槽；该搬运与最后输入 tile 的等待、Vector 计算及后续标量归约并行，Pass 2 直接消费该参数 tile。其余参数仍沿用 V011 的三槽预排，V009 的跨批次首输入预取和 V010 的双输出槽保持不变。
- 与既有版本的区别：V009 预取下一批 x/residual，V010 重叠输出 MTE3，V011 在 Pass 2 内预排参数；V012 将首参数 DMA 前移到 Pass 1 末端。

## 服务器事务

- 主机：`hwnput3`，登录入口：`cann-server3`。最终使用 NPU1；最终运行前 HBM Usage Rate=5%、`No process in device.`，运行后同样为 HBM 5%、`No process in device.`。
- Compile：PASS。首次配置成功但因未带既有 `CPLUS_INCLUDE_PATH` 在编译阶段找不到 `vector`；沿用 V011 的头文件和库路径后，在同一 `build-v012` 完成配置与编译，`CONFIGURE_RC=0`、`BUILD_RC=0`。最终日志：[server3-compile-v012-slotfix-20261017.log](server3-compile-v012-slotfix-20261017.log)。
- Correctness：PASS，四组均通过，Parent/Candidate `failures=0`，`CORRECTNESS_RC=0`。最终日志：[server3-v012-correctness-device1-slotfix-20261017.txt](server3-v012-correctness-device1-slotfix-20261017.txt)。
- Local：PASS，四组各 45 warmup、21 对，计时为 `DEVICE_EVENT_PRIMARY`；配对 device-event 中位差（Candidate−V011）如下。

| Shape / dtype | Parent median (us) | Candidate median (us) | Delta median (us) | Delta P10–P90 (us) |
|---|---:|---:|---:|---:|
| rows=40, D=12288 / FP16 | 17.820 | 18.400 | -1.320 | -7.440–7.540 |
| rows=40, D=32768 / FP16 | 38.980 | 30.720 | -8.320 | -8.680–-7.760 |
| rows=40, D=12288 / BF16 | 33.260 | 31.220 | -2.140 | -3.640–0.120 |
| rows=40, D=32768 / BF16 | 72.960 | 63.280 | -9.600 | -11.940–-2.320 |

- 完整 Local 原始样本：[server3-v012-paired-device1-slotfix-20261017.txt](server3-v012-paired-device1-slotfix-20261017.txt)。
- 早期两次事件顺序尝试在 `fp16-wide-d32768` 停止，均只保留到该 case 开始，返回 `143`；对应原始日志仍保留：[server3-v012-correctness-device1-20261017.txt](server3-v012-correctness-device1-20261017.txt)、[server3-v012-correctness-device1-retry-20261017.txt](server3-v012-correctness-device1-retry-20261017.txt)、[server3-v012-correctness-device1-eventfix-20261017.txt](server3-v012-correctness-device1-eventfix-20261017.txt)。这些进程均为本次事务启动，结束时未调用设备 Reset。
- Official：不执行。

STATUS: COMPILE_PASS_CORRECTNESS_PASS_LOCAL_MEASURED
LOCAL_SCORE: NONE

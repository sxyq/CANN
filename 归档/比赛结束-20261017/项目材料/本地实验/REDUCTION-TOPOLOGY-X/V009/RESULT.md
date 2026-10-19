# N03 REDUCTION-TOPOLOGY-X V009

日期：2026-10-16  
父版本：V008 Candidate `d82d8fc4`  
状态：Compile PASS；Correctness PASS（4/4）；paired Local=NONE（四组 P10–P90 均跨零）。

## 依据

- V008 的 SLOG 原始记录 `V008/device2-slog-envfix-correctness-20261016T133929Z.log` 报告 Vector 访问 UB 地址未对齐；PC 相对偏移为 `0xe18/0xe28`。V008 在 `ProcessWideLowPrecision` 中将 pair partial 写入 FP32 索引 0、2、4…，之后用 `AscendC::Add(..., 1)` 直接读取这些槽。
- V008 `reduceFp32Buf_` 每行分配 `kWideFullYReduceStride=16` 个 FP32。将 partial 槽距改为 8 个 FP32 后，一行可用偏移 0 和 8 两个 32B 对齐槽；每行总分配仍为 64 B。
- V001 也使用 8-float partial 间距，但它为每行设置 `kWideN03ReduceStride=128`，无法直接复用到 V008 的 16-float 行区。V002/V003/V004 的块归约、V005 的双链、V006 的 lane-wise 累加以及 V007/V008 的 pair reduction 均已查阅；没有可直接复用且同时满足本版行区范围的 32B partial 布局。
- 现有 paired Runner 的四个 shape 对应 pairCount 为 FP16 D12288=2、FP16 D32768=4、BF16 D12288=2、BF16 D32768=7。后者按 tileWidth=2560 计算，共 13 个 tile。

## Candidate 变化

- Candidate 从 V008 复制，只改 `ProcessWideLowPrecision` 的 partial 槽布局与合并组织；tile 计算、算术表达式、DataCopyPad、分配及输出路径保持不变。
- 每个 tile pair 的 ReduceSum 目的槽交替使用当前行偏移 0/8 FP32（字节偏移 0/32）；Vector `Duplicate`/`Add` 只访问这两个 32B 对齐槽。每行仍使用原有 16-float UB 区，不增加 TBuf。
- 以四层 FP32 标量 carry stack 保存尚未配对的子树。相邻节点仍按 V008 的平衡二叉顺序合并；奇数尾节点继续向上层传递。该方式覆盖现有 Runner 最多 7 个 pair partial，也保留 V008 对末尾整行 ReduceSum 覆写的删除。

## Compile

- 主机：`hwnput3`（SSH 入口 `cann-server3`）；SoC：Ascend910B3；CANN：`/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002`。
- 复用本 Clone 的 `本地实验/R31B/V016/support/paired/CMakeLists.txt`、`paired_runner_v016.asc`、Runner 源码和 `线上结果/R31B/V011/submission.asc`。远端暂存目录：`/tmp/cann-n03-reduction-topology-v009-20261016`。
- 已 source `/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/script/set_env.sh`，并沿用既有 `CPLUS_INCLUDE_PATH` 配置。
- Configure：`cmake -S /tmp/cann-n03-reduction-topology-v009-20261016/本地实验/R31B/V016/support/paired -B /tmp/cann-n03-reduction-topology-v009-20261016/build -DASCEND_CANN_PACKAGE_PATH=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002 -DR31B_V011_SOURCE_DIR=/tmp/cann-n03-reduction-topology-v009-20261016/线上结果/R31B/V011 -DR31B_V016_SOURCE_DIR=/tmp/cann-n03-reduction-topology-v009-20261016/本地实验/REDUCTION-TOPOLOGY-X/V009`
- Build：`cmake --build /tmp/cann-n03-reduction-topology-v009-20261016/build --target paired_runner_v016 --parallel 1`。
- Configure 和 Build 返回码均为 0；Build 输出 `[100%] Built target paired_runner_v016`。Runner 为 AArch64 ELF，586992 bytes；Ascend object 为 AArch64 ELF，278616 bytes。
- 完整构建输出：`build_out/n03-reduction-topology/compile-v009-server3.log`。本次 Compile 只构建了 Runner；设备验证与测量见下节。

## 设备 Correctness 与 paired Local

- Lease：`N03-V009-D2-CORR-LOCAL-20261016T144504Z`。设备 NPU2，主机 `hwnput3`，CANN 8.5.0.alpha002。
- 14:55:05Z 预采样：Health=OK，HBM 3427/65536 MiB，AICore=0%，NPU2 无运行进程。
- 指定目录 `/tmp/n03-reduction-topology-v009-20261016/build` 于 14:53:15Z 不存在。沿用本文件 Compile 部分记录的实际构建目录 `/tmp/cann-n03-reduction-topology-v009-20261016/build`；其 `paired_runner_v016` 为 AArch64 ELF、586992 bytes，与已记录 Compile 产物一致。
- 执行前 source `/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/script/set_env.sh`；设置 `LD_LIBRARY_PATH=/usr/lib/aarch64-linux-gnu:${LD_LIBRARY_PATH-}` 与 `ASCEND_SLOG_PRINT_TO_STDOUT=1`。命令：`./paired_runner_v016 --paired --device 2`。Runner 输出记录的启动时间为 14:58:02Z，RC=0。
- 四组 Correctness 均 PASS；四组各完成 21 对设备计时。配对差值为 Candidate−Parent：

| Shape | Parent median (us) | Candidate median (us) | 配对差值 median (us) | 配对差值 P10–P90 (us) |
|---|---:|---:|---:|---:|
| FP16 D12288 | 6.860 | 6.680 | -0.040 | [-0.880, 0.640] |
| FP16 D32768 | 10.660 | 10.220 | -0.220 | [-3.300, 0.180] |
| BF16 D12288 | 7.180 | 7.260 | 0.140 | [-2.460, 5.080] |
| BF16 D32768 | 13.920 | 14.100 | 0.260 | [-0.100, 0.720] |

- 四组配对差值区间都跨零，Local 记 NONE。14:58:40Z postflight：Health=OK，HBM 3428/65536 MiB，AICore=0%，NPU2 无运行进程。未 Reset，未触碰其他进程。
- 原始预采样：`N03-V009-D2-CORR-LOCAL-20261016T144504Z-preflight.txt`；Runner stdout/stderr 与 SLOG：`N03-V009-D2-CORR-LOCAL-20261016T144504Z-runner-slog.log`；postflight：`N03-V009-D2-CORR-LOCAL-20261016T144504Z-postflight.txt`。

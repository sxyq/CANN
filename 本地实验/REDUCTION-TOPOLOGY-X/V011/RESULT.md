# N03 REDUCTION-TOPOLOGY-X V011

日期：2026-10-17（服务器 UTC 运行窗口：2026-10-16）  
父版本：V010 Candidate（源码提交 `82dda27b`）  
状态：Compile PASS；Correctness PASS（4/4）；paired Local 完成，四组差值区间均跨零，Local=NONE。

## 假设

V010 已经把连续四个 tile 的平方值先在 Vector lane 内合并，再对每个 block 做一次 `ReduceSum`，但每个 block 结束后仍通过 `SyncVToS`、Scalar carry 和 `SyncSToV` 逐层合并。V011 保留四 tile block 和 lane 内累加，只把 block partial 留在对齐的 UB 槽中，等本行全部 block 完成后再做 Vector-side 二叉树。

目标是移除 block 间的 Scalar 往返依赖，让多个同层 Add 可以在 Vector 阶段连续发出。当前 Runner 的最大形状为 BF16 D32768：tileWidth=2560、tileCount=13、blockCount=4，因此每行四个 32B partial 槽足够覆盖本轮矩阵。

## 源码变化

- Candidate：`本地实验/REDUCTION-TOPOLOGY-X/V011/Candidate.asc`，由 V010 复制后只修改宽行低精度归约段。
- `kWideFullYReduceStride` 从 16 个 FP32 槽增为 32 个 FP32 槽；`kWideN03PartialStride` 保持 8 个 FP32，即每个 partial 起址保持 32B 对齐。
- block partial 从两个交替槽改为按 `blockIndex * kWideN03PartialStride` 写入独立槽；完成 tile 扫描后，按 `treeStride=1,2,...` 执行向量化二叉 Add，结果保留在当前行首槽。
- 删除 V010 block 内的 Scalar carry 数组、逐 block `GetValue` 和对应的 Scalar/Vector 往返。
- tile 宽度、四 tile lane block、FP32 平方、RMS 与输出路径保持不变；没有修改 Runner、CMake 或 Parent。

## 服务器 Compile

- 主机：`hwnput3`；SSH 入口：`cann-server3`；SoC：Ascend910B3。
- 远端目录：`/home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N03/V011`。
- Toolkit：`/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002`；`set_env.sh` 返回 0。
- Candidate SHA256：`d1633619424d55d3dcc31212de5691b06f7495abab12698a0c4757c3ad1ffe61`。
- 复用入口：`本地实验/R31B/V016/support/paired/CMakeLists.txt`、`paired_runner_v016.asc` 和现有 `paired_runner_v016`；没有创建平行 Runner。

首次构建命令：

```text
cmake -S /home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N03/runner -B /home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N03/V011/build -DASCEND_CANN_PACKAGE_PATH=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002 -DR31B_V011_SOURCE_DIR=/home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N03/parent -DR31B_V016_SOURCE_DIR=/home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N03/V011
cmake --build /home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N03/V011/build --target paired_runner_v016 --parallel 1
```

- Configure：PASS，返回码 0。
- 首次 Build：返回码 2；BiSheng 插件阶段报告 `<vector> file not found`，当时 `CPLUS_INCLUDE_PATH` 为空。
- 沿用已有 server3 构建入口补充 `/usr/include/c++/11`、`/usr/include/aarch64-linux-gnu/c++/11`、backward、aarch64 系统头和 CANN HCC 目录后，重新执行同一 Build 命令：PASS，返回码 0。
- 产物：`/home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N03/V011/build/paired_runner_v016`，AArch64 ELF，SHA256 `e18f8c6493f3c8a2b4fc84992816ea3de52921473ae7167fb5eca7bdb9186780`。
- Compile 证据：`本地实验/REDUCTION-TOPOLOGY-X/V011/compile.log`。

## Correctness 与 paired Local

- 设备：NPU7；命令：`./paired_runner_v016 --paired --device 7`。
- 运行前：Health=OK、HBM 使用率 5%（3432/65536 MB）、NPU7 无进程。
- Runner：`warmup_each=45`、`pairs_each=21`；四组 Correctness 全部 PASS，`failures=0`；Runner 返回码 0。
- Runner 日志中的 `binary=V011` 是 Parent，`binary=V016` 是本版本 Candidate；下表已按 Parent/Candidate 展示。

| Shape | Parent median us | Candidate median us | Candidate-Parent delta median us | Delta P10–P90 us | Local 判断 |
|---|---:|---:|---:|---:|---|
| FP16 D12288 | 28.640 | 27.840 | -0.700 | [-19.740, 7.960] | 跨零 |
| FP16 D32768 | 11.800 | 15.380 | -0.320 | [-5.960, 8.900] | 跨零 |
| BF16 D12288 | 12.860 | 13.600 | -0.020 | [-11.700, 11.880] | 跨零 |
| BF16 D32768 | 24.160 | 22.320 | -0.920 | [-7.140, 8.600] | 跨零 |

- Correctness 误差与 V010 相同：FP16 D12288 `max_abs=0.0009765625`、`max_rel=0.0015923567`；FP16 D32768 `0.001953125`、`0.010989011`；BF16 D12288 `0.0078125`、`0.0073529412`；BF16 D32768 `0.0078125`、`0.01754386`。
- 四组差值区间均跨零，`LOCAL_SCORE=NONE`；V011 未形成稳定收益，不做线上提交。
- 运行后：Health=OK、HBM 使用率 5%、NPU7 无进程；未 Reset、未触碰其他进程。
- 原始设备证据：`本地实验/REDUCTION-TOPOLOGY-X/V011/device7-correctness-local-20261017.log`。
- 本轮没有重复此前失败的反汇编命令。

## RESULT

V011 的 Vector-side block partial tree 通过 Compile 和四组 Correctness，但在 NPU7 的 paired Local 中没有稳定收益。BF16 D32768 的配对中位差为 `-0.920 us`，P10–P90 为 `[-7.140, 8.600]`，仍不足以形成 Local 结论。

## NEXT

V012：保持 V011 的 post-sweep Vector tree，单独把 lane block 从 4 tile 改为 2 tile，并为 BF16 D32768 的 7 个 partial 扩展对齐槽区；用同一 `paired_runner_v016` 重新完成四组 Correctness 与 paired Local，区分 block 粒度和 Vector tree 的贡献。

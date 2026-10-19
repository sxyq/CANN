# N03 REDUCTION-TOPOLOGY-X V013

日期：2026-10-17（服务器 UTC 运行窗口：2026-10-16）
父版本：V011 Candidate（源码提交 `fe8495fd`）
状态：Compile PASS；Correctness PASS（4/4）；paired Local 完成，四组差值区间均跨零，`LOCAL_SCORE=NONE`。

## 单一机制

V011 保留四 tile lane block，并在每个 block 结束后把 partial 写入 32B 对齐槽，再做 Vector-side 二叉树。V013 保留 block 粒度、partial 槽布局、树层级和同步顺序，只改变树边的 Vector `Add` 读取宽度：

- Candidate：`本地实验/REDUCTION-TOPOLOGY-X/V013/Candidate.asc`。
- `kWideFullYReduceStride=32`、`kWideN03PartialStride=8`、`kWideN03LaneBlockTiles=4` 均保持 V011。
- tree 中的 `Add(..., 1)` 改为 `Add(..., kWideN03PartialStride)`，一次读取完整 8-FP32 partial 槽；最终 RMS 仍读取每行首槽。
- tile 计算、FP32 平方、RMS、输出路径、Parent 和 paired Runner 均未改动。

## Compile

- 主机：`hwnput3`；SSH 入口：`cann-server3`；SoC：Ascend910B3。
- 远端目录：`/home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N03/V013`。
- Toolkit：`/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002`。
- Candidate SHA256：`8de2702dcf23211d45c58ae53a5ccca9fdb1ee8d6ac3348f9af2703212a40353`。
- 复用入口：现有 `paired_runner_v016`、`runner`、`parent`；没有创建新的 Runner 或执行入口。

命令：

```text
source /usr/local/Ascend/ascend-toolkit/set_env.sh
export CPLUS_INCLUDE_PATH=/usr/include/c++/11:/usr/include/aarch64-linux-gnu/c++/11:/usr/include/c++/11/backward
cmake -S /home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N03/runner -B /home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N03/V013/build -DASCEND_CANN_PACKAGE_PATH=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002 -DR31B_V011_SOURCE_DIR=/home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N03/parent -DR31B_V016_SOURCE_DIR=/home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N03/V013
cmake --build /home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N03/V013/build --target paired_runner_v016 --parallel 1
```

- Configure：返回码 0。
- Build：返回码 0，输出 `[100%] Built target paired_runner_v016`。
- Runner：AArch64 ELF，SHA256 `73d1fdfae22b89104753a2b7d90585073c8a6eba33cc02d3977cbe6819d73386`。
- Compile 原始日志：`本地实验/REDUCTION-TOPOLOGY-X/V013/compile.log`。

首次运行使用顶层 `set_env.sh` 时，动态加载器提示 toolkit `libstdc++.so.6` 缺少 `GLIBCXX_3.4.29`，Runner 未进入设备执行，RC=1。该原始日志保留在 `device7-runtime-env-failure-20261017.log`；随后沿既有成功入口将系统 `libstdc++` 路径置于 `LD_LIBRARY_PATH` 首位，只重跑一次 Runner。

## Correctness 与 paired Local

- 设备：NPU7；运行前 Health=OK、HBM `3434/65536 MB`、无运行中的 N03 Runner。
- 环境：`source /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/script/set_env.sh`，随后 `LD_LIBRARY_PATH=/usr/lib/aarch64-linux-gnu:${LD_LIBRARY_PATH:-}`。
- 命令：`./paired_runner_v016 --paired --device 7`。
- Runner：`warmup_each=45`、`pairs_each=21`；四组 Correctness 全部 PASS，`failures=0`；Runner 返回码 0。
- Runner 中 `binary=V011` 为 Parent，`binary=V016` 为本版本 Candidate。

| Shape | Parent median us | Candidate median us | Candidate-Parent median us | Delta P10-P90 us | Local |
|---|---:|---:|---:|---:|---|
| FP16 D12288 | 8.100 | 7.640 | -0.920 | [-25.120, 24.760] | NONE |
| FP16 D32768 | 20.920 | 20.300 | -0.220 | [-12.100, 6.640] | NONE |
| BF16 D12288 | 20.540 | 20.300 | 0.140 | [-1.020, 16.860] | NONE |
| BF16 D32768 | 25.500 | 26.520 | 1.540 | [-2.260, 23.160] | NONE |

- 四组配对差值区间均跨零，不能形成稳定 Local 收益；BF16 D32768 的 Candidate median 高于 Parent，其余三组中位数略低于 Parent。
- Correctness 误差：FP16 D12288 `max_abs=0.0009765625`、`max_rel=0.0015923567`；FP16 D32768 `0.001953125`、`0.010989011`；BF16 D12288 `0.0078125`、`0.0073529412`；BF16 D32768 `0.0078125`、`0.01754386`。
- 运行后：NPU7 Health=OK、HBM `3434/65536 MB`、AICore=0%、无运行中的 N03 Runner；未 Reset、未触碰其他进程。
- paired 原始日志：`本地实验/REDUCTION-TOPOLOGY-X/V013/device7-correctness-local-20261017.log`。

## RESULT

V013 的 partial 槽读取宽度变化通过 Compile 和四组 Correctness，但 paired Local 未形成稳定收益。当前证据不支持继续扩大该 Vector `Add` 的槽读取宽度变化。

下一实验：回到 V011，选择另一个尚未叠加的单一 Vector tree 依赖组织变化；优先保持 4-tile lane block 和 32B partial 槽不变，并先读取现有代码入口后再决定是否实施。

PUSH=NO；线上提交=NO；共享记录=未修改。

# R31B V016 paired runner 准备

## 当前状态

- 路线与版本：R31B / V016，直接父版本 R31B-V011。
- V016 状态仍为 `NEEDS_ONE_MORE_LOCAL`；当前只保留 V016，不创建 V017。
- paired runner 已在 `cann-server3`（远端主机名 `hwnput3`）完成强制重编译和链接。
- Main review 保持 V016 为 `NEEDS_ONE_MORE_LOCAL`；V011/V016 exact-source runner 编译和链接通过，runner 未执行。
- 当前 server3 没有可用设备：d0-d3 有活动负载，d4-d6 由 Main-2 租用；时序流程排除 d7。等待获准窗口，不创建 V017。
- 本轮没有启动 runner、NPU correctness 或任何时序采样。

## 源码与可执行文件

| 对象 | SHA-256 |
|---|---|
| V011 parent source | `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3` |
| V016 Candidate source | `9f5c353e65a13a740fe97dc7e6415df032d27560831a3ad142c77592b8208eb5` |
| paired wrapper source | `b827960aa2d60e6efd953b30f3ecfcfab17af5111f4d03e5309c4c94c68d1a55` |
| paired runner C++ | `d8a574735bda3e07cbfbc1de3c890a72fb7b437983eae38ea22d0bf8b0bd03bc` |
| paired runner ABI header | `096c0229f5d9e6fa37460808387517280cb0f2e4296e07b8c37f75cd73c89617` |
| paired CMake input | `29eb6e937ca94fbf2877ae22d2ee9a864066f8e5844ea0ccdc356c1ea40a3f3a` |
| remote paired runner ELF | `6e9337a2c1bf4e9c74db56f55d90fd5eb233fdb7f6f4d189265b0a8f80cdd7fc` |

远端 ELF 路径：`/home/data4t2/lelinfeng/phase4-review-repro/R31B-V016/paired-runner-src/build/paired_runner_v016`。构建记录位于 `support/paired/paired-build-v016-success.txt`；旧失败输出 `support/paired/build-v016-attempt1.log` 保留。

此前分开的 V011/V016 probe ELF 身份仍记录在 `local-probe-evidence.txt`；其旧样本为 `LOAD_CONTAMINATED`，不能作为性能结论。正式 kernel 构建与 correctness 证据仍分别保留在 `compile-evidence.txt` 与 `correctness-evidence.txt`。

## Runner 模式

`support/paired/paired_runner.cpp` 的 kernel 类型与调用点均显式使用 `r31b_v011::run_kernel_v011` 和 `r31b_v016::run_kernel_v016`。旧失败日志中的未限定名称属于先前编译输入；当前源码完成重新编译和链接。

Runner 固定 device 4、rows=2，并在一个 ACL process/stream 中执行四个形状。每个形状由 V011 单独进行两组各 31 个 device-event 样本的同一可执行文件资格测试；每个形状均须 PASS。之后才允许执行 21 组相邻 P/C 配对。设备事件时间为主要样本，steady-clock 时间仅作诊断。D2H 与 golden compare 位于该形状的样本之后。

host/device allocation 与 H2D copy 均在 warmup 前完成；每个 kernel 有 10 次完整 stream-sync warmup。原始记录逐对保留 parent、Candidate 的 device/wall 时间与差值；汇总含 median、mean、sample stdev、CV、min/max、max/min、MAD、p10/p90、spread、core CV。日志使用带本地时间戳的独立文件名，不覆盖已有记录。

| 形状 | dtype | width | 同一可执行文件通过要求 |
|---|---|---:|---|
| fp16-tail-d12288 | FP16 | 12288 | 待执行 |
| fp16-wide-d32768 | FP16 | 32768 | 待执行 |
| bf16-tail-d12288 | BF16 | 12288 | 待执行 |
| bf16-wide-d32768 | BF16 | 32768 | 待执行 |

## 未来启动顺序

独立 CMake 输入与构建目录：

```sh
cmake -S /home/data4t2/lelinfeng/phase4-review-repro/R31B-V016/paired-runner-src \
  -B /home/data4t2/lelinfeng/phase4-review-repro/R31B-V016/paired-runner-src/build \
  -DR31B_V011_SOURCE_DIR=/home/data4t2/lelinfeng/phase4-workspaces/R31B \
  -DR31B_V016_SOURCE_DIR=/home/data4t2/lelinfeng/phase4-review-repro/R31B-V016/probe-src
cmake --build /home/data4t2/lelinfeng/phase4-review-repro/R31B-V016/paired-runner-src/build \
  --target paired_runner_v016 --parallel 1
```

本次成功构建使用 `-- -B` 强制重新编译并链接，详细输出路径见 `support/paired/paired-build-v016-success.txt`。构建不会启动可执行文件或调用设备。

只有 Main 授予 MAIN-1 device-4 独占租用并确认可比较的负载窗口后，才可启动同一可执行文件资格测试：

```sh
MAIN_DEVICE_LEASE=R31B R31B_LOAD_WINDOW=CLEAR \
  ./phase4/local/R31B/V016/paired-probe-harness.sh --same-binary
```

只有四种形状的记录均为 PASS，且 parent、Candidate 与 runner SHA-256 完全匹配，才可由同一租用窗口启动 P/C 配对：

```sh
MAIN_DEVICE_LEASE=R31B R31B_LOAD_WINDOW=CLEAR \
R31B_QUALIFICATION_LOG=/absolute/path/to/paired-probe-samebinary-<timestamp>.txt \
  ./phase4/local/R31B/V016/paired-probe-harness.sh --paired
```

launcher 会先读取 canonical device lease 表，再连接 `cann-server3`；环境变量本身不构成租用授权。当前条件未满足，以上命令均未运行。

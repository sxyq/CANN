# N04 V011 结果

## 单一变化

- 父版本：V010。
- Candidate：本目录 `Candidate.asc`。
- 变化：仅对 FP16、rows=40、D=32768 的宽行 Pass 2 gamma/bias 搬运增加第三参数槽，并预排最多两个后续参数 tile；V009 的下一批输入预取与 V010 的输出写回路径保持不变。
- 未创建后续版本，未进行线上提交。

## 服务器事务结果

- Compile：PASS。复用 `support/paired/CMakeLists.txt`、`paired_runner_v016.asc` 和既有 CANN 8.5.0.alpha002；产物：`/home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N04/V010/build-v011/paired_runner_v016`。
- Correctness：PASS，4/4。沿用已有 Runner，在 `LD_LIBRARY_PATH` 中将系统 `/usr/lib/aarch64-linux-gnu` 放到 CANN 工具链库之前后，NPU7 实际执行完成；FP16/BF16 的 D12288/D32768 四组均 `failures=0`。
- Local：已执行，四组各 21 对，`Candidate−V011` 的配对 device-event 中位差分别为 FP16 D12288 `-2.080 us`、FP16 D32768 `-1.520 us`、BF16 D12288 `-2.420 us`、BF16 D32768 `-9.340 us`。四组 P10–P90 分别为 `[-9.000,+3.940]`、`[-20.120,+5.980]`、`[-6.840,-1.560]`、`[-16.680,-0.200] us`；前两组跨零，后两组整体为负，按路线记录保留 `LOCAL_SCORE=NONE`。
- 设备：NPU7 启动前后均无其他运行进程；未调用设备 Reset，未触碰其他进程。
- 原始证据：服务器目标下 `V010/本地实验/R31B/V016/V011/server3-v011-correctness-device7-20261017.txt`、`server3-compile-retry-20261017.log`；本地副本为同目录下对应文件。

STATUS: COMPILE_PASS_CORRECTNESS_PASS_LOCAL_MEASURED
LOCAL_SCORE: NONE

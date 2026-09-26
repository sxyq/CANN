# R31A V021 Main Handoff

## 状态

`NEEDS_ONE_MORE_LOCAL`

停在 Main Review。当前没有 Main 授权给 R31A 的独占设备租约；本轮未查询 NPU、启动 probe 或执行测量。不得创建 V022，也不得提交 CANNJudge。

## 版本与来源

| 项目 | 值 |
|---|---|
| Route / revision | R31A / V021 |
| 分支 / V021 既有证据提交 | `exec/sixlane-20260923-r31a` / `d38328aed787dfce6ad687821223d87db04f7e43` |
| Direct Parent | R31A-V016 |
| Parent source SHA-256 | `dd13093823c885e785a650abff4863827e652eb8607ad0621a96eb31b6764fa0` |
| Parent Official Score | 45.00，15/15 PASS |
| Candidate | `phase4/local/R31A/V021/submission.asc` |
| Candidate source SHA-256 | `4f5bfc319b72d1f0bcfd453bc92e80ac64216719898757292daf1aa1b73b6063` |

V016 的 Online 源、Route 工作区源及 `support/parent_v016_submission.asc` 三者的 SHA-256 相同。V021 的实算 SHA-256 与 `submission.sha256` 相同。V017 不属于 V021 的来源链；`diff.patch` 的直接比较对象是 V016。

## 直接差异与既有验证

Candidate 源仅把 `SyncMTE3ToV()` 从 FP32 CachedRows 输出的 tile 循环体内移到循环之后，位置见 `submission.asc:2053-2055`。记录的单一假设是延后每个 tile 的 MTE3 完成等待，使后续 tile 运算与前一 tile 的输出写入重叠；其他计算路径保持不变。

V021 的编译、链接记录均为 PASS。设备 4 上记录的正确性结果：D=32768 与控制 D=24576 均 PASS，最大绝对误差分别为 `4.83928943e-07`、`3.48268474e-07`。已有两组延迟记录标为 `LOAD_CONTAMINATED`，方向相反，不构成性能结论。

## 配对 runner 源码身份

下表记录此前成功构建时的输入身份；本轮只改 paired runner 正文，Parent 与 Candidate 原始 ASC 源保持不变：

| 输入 | 编译路径 | SHA-256 |
|---|---|---|
| Parent V016 | `support/parent_v016_submission.asc` | `dd13093823c885e785a650abff4863827e652eb8607ad0621a96eb31b6764fa0` |
| Candidate V021 | `../submission.asc` | `4f5bfc319b72d1f0bcfd453bc92e80ac64216719898757292daf1aa1b73b6063` |
| C++ runner | `support/paired_probe.cpp` | `da39bdde59ba89a1bd6925542dabe2b546e3a4553384ea190a528f9fa9d57d9f` |
| Runner body | `support/paired_probe_main.inc` | `70657b535c83564e3140c5bf49aaac7903b24099e15dcbc5acf215db666466ad` |
| Bridge declarations | `support/paired_probe_bridge.h` | `59589d8b22266fa1ee600f1a2294c31cd392dad0836a42a02271d4cdb131246f` |
| Parent ASC wrapper | `support/paired_probe_v016.asc` | `dd45d48c000b77f9fd47f8c9e050a39ad320da13319553f0722aad78e05baecc` |
| Candidate ASC wrapper | `support/paired_probe_v021.asc` | `4ccb96b2a9f5abf6b5f116a1bf2ff3fdff982ae42dbdac9f2f5c39a2e4541df1` |
| Shared host declarations | `support/probe_prelude.inc` | `b4f96bc019bd5c0e3e11846649c1f0466a1dbb629ca4d177908b8791ea6aa5b8` |
| Build rules | `support/CMakeLists.txt` | `e4d450f8c4b6884ea749d3396d7401fcd6d6ced2e1f6a32a0a36921908c225cd` |

此前成功构建所用 runner 正文 SHA-256 为 `70657b535c83564e3140c5bf49aaac7903b24099e15dcbc5acf215db666466ad`。当前正文已按下方协议对照更新，尚未重新构建；此前 runner ELF 及两个模块 ELF 不代表当前正文。

The prior combined `support/paired_probe.asc` has been replaced by separate Parent and Candidate ASC shared-library translation units. Each wrapper renames and compiles its respective original `run_kernel`; the C++ executable uses ordinary `void*` host arguments and C ABI `dlopen`/`dlsym` bridges. This keeps `GM_ADDR` use inside each ASC module and isolates duplicate registration symbols. The generated ASC registration compile receives server3 GCC 11 standard-library paths through `CPATH` and `CPLUS_INCLUDE_PATH` in the CMake compile rule.

The fail-closed bridge returns `false` on `dlopen` or `dlsym` failure and `true` only after calling the resolved entry. Correctness initializes the full device output to quiet NaNs before each version call, synchronizes, copies the whole result back, and rejects any non-finite element. Dispatch failure stops the current warmup or timed loop before that failed call is recorded; completed timed samples are saved with an `INCOMPLETE` block status. `same` mode runs correctness, warmup, and sampling only for its selected version.

Host-only test inputs: `support/paired_probe_host_test.cpp` / `a557c12c301566fcba4cd6ee880effea6f603eddfc9b6da6e981c7c05fd136c2`; success DSO source `support/paired_probe_host_test_entry.cpp` / `4a71cd5310099aa278949baad1b97557ee6e346a9582f7f17dcea50785cc723f`; missing-symbol DSO source `support/paired_probe_host_test_missing_symbol.cpp` / `d753d1e5f812c1b8007f7540fbfd79264b2572f4fbee67036f150a36aa44185e`; host-test CMake source `support/host-tests/CMakeLists.txt` / `1d4449c33467960fa2c466e87c76f7c83b727c043a6e5c13be0ab938f39127ff`.

## Parent/Candidate executable mapping

Build source: `/home/data4t2/lelinfeng/phase4-worktrees/R31A/V021-direct/phase4/local/R31A/V021/support`

Build directory: `/home/data4t2/lelinfeng/phase4-worktrees/R31A/V021-direct/probe-build`
Toolchain target: CANN `8.5.0.alpha002`, `dav-2201`, `Ascend910B3`.

| Role | Original ASC input SHA-256 | Dedicated wrapper and exported bridge | Module ELF SHA-256 |
|---|---|---|---|
| Parent V016 | `dd13093823c885e785a650abff4863827e652eb8607ad0621a96eb31b6764fa0` | `paired_probe_v016.asc`; `r31a_run_kernel_v016` → `r31a_host_entry_v016` | `libr31a_paired_v016.so` / `11d7a2cb60912ce23b05a768c9076abcf2c513ad8b0844cc062d158082a4d5ea` |
| Candidate V021 | `4f5bfc319b72d1f0bcfd453bc92e80ac64216719898757292daf1aa1b73b6063` | `paired_probe_v021.asc`; `r31a_run_kernel_v021` → `r31a_host_entry_v021` | `libr31a_paired_v021.so` / `9bacd8cd72b8db5c6b8a1574ce199c674b18bc2bcf858cfb91ade29f71d24674` |

The shared C++ runner ELF is `r31a_paired_probe`, SHA-256 `f16975ae50d5d904a1829d19f5156da54f05d17a7d88ee8b02a8c92cdade727e`. Its host wrappers load the matching module and call the listed C ABI bridge. GCC 11 host tests exercise bridge load/symbol failure and both version entry calls; they do not execute the runner or call ACL/NPU APIs.

## Build attempts

`support/build-paired-targets-failclosed-cann-server3-20260926.log` records the prior successful build with the earlier runner body. This review's rebuilt ELF identities are recorded below. `readelf` previously confirmed each module exports only its intended `r31a_host_entry_*` bridge symbol. Neither old nor rebuilt runner ELF was invoked.

All prior configure logs remain retained. New configure records are `configure-cann-server3-paired-r31a-failclosed-20260926.log`, `configure-cann-server3-paired-r31a-failclosed-host-tests-20260926.log`, `configure-cann-server3-paired-r31a-failclosed-host-tests-retry2-20260926.log`, and `configure-hosttests-cann-server3-gcc11-20260926.log`.

Every build attempt remains under `support/`:

| Log | Outcome |
|---|---|
| `build-server3-paired.log` | Failed ASC host/device boundary: direct cast from `float*` to `__gm__ uint8_t*` is rejected. |
| `build-cann-server3-paired-r31a-paired_probe-20260925.log` | Failed while the combined ASC translation unit could not resolve Parent `__origin__r31a_kernel_v016` specializations. |
| `build-cann-server3-paired-r31a-paired_probe-20260925-cxx-driver.log` | Failed generated registration compilation because `<vector>` was not found; cceld/objcopy messages followed. |
| `build-cann-server3-paired-r31a-paired_probe-20260925-gcc11-env.log` | Failed after GCC 11 architecture headers entered the ASC compile context; `arm_neon.h` types were unavailable. |
| `build-cann-server3-paired-r31a-paired_probe-20260925-gcc11-stdlib.log` | Failed to resolve Parent `__origin__r31a_kernel_v016` specializations. |
| `build-cann-server3-paired-r31a-paired_probe-20260925-symbol-import.log` | ASC compile failed in CANN headers/language mode; compiler emitted follow-on diagnostics. |
| `build-cann-server3-paired-r31a-paired_probe-20260925-symbol-import-env.log` | Generated registration compile still lacked `<vector>`; later linker/objcopy messages were secondary. |
| `build-cann-server3-paired-r31a-paired_probe-20260925-symbol-import-stdlib.log` | Failed on duplicate device stub symbols when both kernels were combined in one ASC image. |
| `build-cann-server3-paired-r31a-paired_probe-20260925-split.log` | Failed with missing registration `<vector>` and unresolved Parent kernel specializations. |
| `build-cann-server3-paired-r31a-paired_probe-20260925-unique-entry.log` | Partial attempt; log has no final runner target status and is not counted as a successful build. |
| `build-cann-server3-paired-r31a-paired_probe-20260925-void-bridge-dso.log` | Both ASC libraries built; final C++ link returned cceld code 1 without a diagnostic. |
| `build-cann-server3-paired-r31a-paired_probe-20260925-libpath.log` | Both ASC libraries built; final C++ link returned cceld code 1 without a diagnostic. |
| `build-cann-server3-paired-r31a-paired_probe-20260925-aclend.log` | Both ASC libraries built; final C++ link returned cceld code 1 without a diagnostic. |
| `build-cann-server3-paired-r31a-paired_probe-20260925-final-link.log` | PASS: Parent module, Candidate module, and `r31a_paired_probe` linked. |
| `build-cann-server3-paired-r31a-paired_probe-20260925-gmaddr-boundary.log` | PASS: all three targets reported built; runner target was already current after final-link. |
| `build-cann-server3-paired-r31a-failclosed-20260926.log` | Failed full `all` build in unrelated legacy `probe_v016` / `probe_v021` ASC targets; CANN plugin reported missing `ASCEND_HOME_PATH`, followed by header language-mode diagnostics and compiler exit 139. The paired targets are not dependent on these two targets. |
| `build-cann-server3-paired-r31a-failclosed-host-tests-20260926.log` | Paired ASC modules linked; runner compile found two same-mode lambdas dropping the bool dispatch result. Fixed before the passing build. |
| `build-cann-server3-paired-r31a-failclosed-host-tests-retry1-20260926.log` | Paired targets linked; the temporary BiSheng-built host-test executable did not link due to missing C++ runtime symbols. Host tests were moved to a separate GCC 11 CMake project. |
| `build-cann-server3-paired-r31a-failclosed-host-tests-retry2-20260926.log` | PASS for paired runner and temporary host-test targets; no tests were executed by this build. |
| `build-hosttests-cann-server3-gcc11-20260926.log` | PASS: host-only test executable and both test DSOs built with `/usr/bin/g++` 11.4.0. |
| `ctest-host-bridge-cann-server3-gcc11-20260926.log` | PASS: 4/4 tests; missing library, missing V021 symbol, V016 dispatch, and V021 dispatch. |
| `build-paired-targets-failclosed-cann-server3-20260926.log` | PASS for the earlier runner body: explicitly named V016 module, V021 module, and `r31a_paired_probe` targets. |
| `build-paired-protocol-current-cann-server3-20260926.log` | Return code 2: first attempt omitted CANN environment setup; ASC plugin could not find `ASCEND_HOME_PATH`. Retained. |
| `build-paired-protocol-current-cann-server3-env-20260926.log` | Return code 0: after loading `/usr/local/Ascend/ascend-toolkit/set_env.sh`, built `r31a_paired_v016`, `r31a_paired_v021`, and `r31a_paired_probe`. |
| `build-host-bridge-current-cann-server3-gcc11-20260926.log` | Return code 0: GCC 11 host bridge test executable and both test libraries built. |
| `ctest-host-bridge-current-cann-server3-gcc11-20260926.log` | Return code 0: 4/4 pass (missing library, missing symbol, V016 dispatch, V021 dispatch). |

## Timing protocol comparison

当前 runner 正文 SHA-256 为 `c95dec4cfd552020462e509fa3fbbf2dbe3b10162ff719cf735df15b1f08b2e7`，已在本轮 server3 构建中编译。支持入口保持为 `pair DEVICE WIDTH OUTPUT_PREFIX [WARMUPS]` 与 `same DEVICE V016|V021 WIDTH OUTPUT_PREFIX GAP_SECONDS [WARMUPS]`；默认 warmup 为 45，也接受 50..60，以支持较长稳定期重试。形状仍为 FP32、rows=2、blocks=1，width 为 32768 或 24576。

本轮源码已处理的协议差距：

- warmup 每个版本执行 45 次 launch+full-stream-sync；pair 模式交错 Parent/Candidate 的 warmup 次序。
- same-binary 每块 21 个 device-event 样本，共两块；额外写出 `-same-binary.tsv`，记录两块 MAD/median、块间漂移以及协议结果。
- pair 模式改为 4 个统计块，每块 11 组相邻 P/C 样本，组内顺序交替；`-pair-blocks.tsv` 逐块给出 Parent/Candidate 中位数和 `C-P` 差值。`-samples.tsv` 留存全部单次样本；`-jitter.tsv` 留存各块的 median、mean、stdev、CV、min、max、max/min、MAD、p10、p90 和 spread。
- 若 dispatch 在采样中途失败，已完成样本仍落盘；对应块写 `INCOMPLETE`，不报告为完整 P/C 块或 same-binary 通过。
- 单次 device event 为主指标，launch+wait 的 wall time 为诊断项；设备分配只发生在进程初始化，H2D 在 warmup 前完成，计时循环内没有分配或数据拷贝。
- 计时样本先落盘，随后才执行 correctness D2H 与 golden compare。每个被测版本的 correctness 调用前都向 `outputDevice` 写入 NaN sentinel，再验证完整输出；same 模式只调度所选版本的 correctness、warmup 与采样。

仍由 Main 的运行编排负责：paired runner 本身不查询或写入设备租约，也不采集运行前后的 npu-smi、HBM、AICore、进程与时间戳；这些信息须随实际运行记录保存。runner 也没有 Parent-only、PRECHECK-A/B、多进程重复的 window-qualification 模式。`pair` 命令本身不读取 same-binary 结果来决定是否继续，因此每个精确形状必须先对 Direct Parent V016 单独运行 same-binary，确认 MAD/median 与块间漂移均不超过 0.10，再由 Main 确认该形状可测、有独占 MAIN-1 租约，并按需完成 Parent-only window qualification。d7 继续避用。

当前 runner 正文 SHA-256 为 `c95dec4cfd552020462e509fa3fbbf2dbe3b10162ff719cf735df15b1f08b2e7`；Parent source SHA-256 为 `dd13093823c885e785a650abff4863827e652eb8607ad0621a96eb31b6764fa0`；Candidate source SHA-256 为 `4f5bfc319b72d1f0bcfd453bc92e80ac64216719898757292daf1aa1b73b6063`。所有三项服务器源码 SHA 与本地值一致。使用上述四个新日志构建，runner ELF SHA-256 为 `d11fa9aa541e5c75dc38bd509cd515ca44347162741f6b93b7a0b371aecfde11`，Parent module SHA-256 为 `11d7a2cb60912ce23b05a768c9076abcf2c513ad8b0844cc062d158082a4d5ea`，Candidate module SHA-256 为 `9bacd8cd72b8db5c6b8a1574ce199c674b18bc2bcf858cfb91ade29f71d24674`。

新 GCC 11 host bridge suite 覆盖动态库缺失、符号缺失及 V016/V021 dispatch；它编译 `paired_probe.cpp` 的 bridge-only 分支，不编译 timing main include。runner C++ build 已编译当前正文并链接新 ELF。未执行 runner ELF、ACL 注册、NPU correctness、same-binary 或 timing。

The prepared invocations are:

```sh
./r31a_paired_probe same DEVICE V016 32768 PREFIX 60
./r31a_paired_probe same DEVICE V016 24576 PREFIX 60
./r31a_paired_probe pair DEVICE 32768 PREFIX
./r31a_paired_probe pair DEVICE 24576 PREFIX
```

Track-B remains three distinct R31A-only hypotheses in `phase4/research/R31A/next-hypotheses.md`; no new target code-generation evidence was produced in this pass.

## Current handoff boundary

Current protocol-alignment runner build and link: PASS (return code 0); host bridge tests: 4/4 PASS (return code 0). The first build without CANN environment setup failed with return code 2 and remains retained. No V022 or other revision was created. V021 source remains unchanged at `4f5bfc319b72d1f0bcfd453bc92e80ac64216719898757292daf1aa1b73b6063`. No runner execution, ACL/NPU correctness, device query, same-binary, or timing was performed; no eligible MAIN-1 lease is recorded. Earlier V021 correctness records and load-contaminated latency records remain unchanged. All old build logs, including failed attempts, remain retained. No shared control file or other Route was changed.

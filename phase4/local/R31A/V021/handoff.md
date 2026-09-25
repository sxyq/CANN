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

server3 构建目录中的编译输入与本地文件逐项 SHA-256 一致。Parent 与 Candidate 原始 ASC 源保持不变：

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

The prior combined `support/paired_probe.asc` has been replaced by separate Parent and Candidate ASC shared-library translation units. Each wrapper renames and compiles its respective original `run_kernel`; the C++ executable uses ordinary `void*` host arguments and C ABI `dlopen`/`dlsym` bridges. This keeps `GM_ADDR` use inside each ASC module and isolates duplicate registration symbols. The generated ASC registration compile receives server3 GCC 11 standard-library paths through `CPATH` and `CPLUS_INCLUDE_PATH` in the CMake compile rule.

The fail-closed bridge returns `false` on `dlopen` or `dlsym` failure and `true` only after calling the resolved entry. Correctness initializes the full device output to quiet NaNs before each version call, synchronizes, copies the whole result back, and rejects any non-finite element. Dispatch failure stops correctness, warmup, and sampling before a sample is recorded. `same` mode runs correctness, warmup, and sampling only for its selected version.

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

`support/build-paired-targets-failclosed-cann-server3-20260926.log` records successful builds of the Parent module, Candidate module, and final C++ executable using only the paired targets. Final module SHA-256 values remain `11d7a2cb60912ce23b05a768c9076abcf2c513ad8b0844cc062d158082a4d5ea` (V016) and `9bacd8cd72b8db5c6b8a1574ce199c674b18bc2bcf858cfb91ade29f71d24674` (V021); runner SHA-256 is `f16975ae50d5d904a1829d19f5156da54f05d17a7d88ee8b02a8c92cdade727e`. `readelf` confirms the respective modules export only their intended `r31a_host_entry_*` bridge symbol. The runner ELF was not invoked.

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
| `build-paired-targets-failclosed-cann-server3-20260926.log` | PASS: explicitly named V016 module, V021 module, and `r31a_paired_probe` targets. |

## Runner interface and local protocol

The built runner accepts either `pair DEVICE WIDTH OUTPUT_PREFIX` or `same DEVICE V016|V021 WIDTH OUTPUT_PREFIX GAP_SECONDS`. Shapes remain FP32, rows=2, blocks=1; width is 32768 or 24576. The source implements correctness before measurements, 10 warmups, and 21 event-timed samples per block or adjacent pair, writing raw samples and summary tables. GCC 11 host tests verified only dynamic-library and bridge behavior. No ACL registration, NPU correctness, runner execution, or timing was performed for these ELFs.

The next allowed measurement sequence is owned by Main: obtain an active exclusive MAIN-1 lease; use the Direct Parent V016 executable and each exact shape to qualify same-binary noise; proceed only for a shape marked PASS by the shared timing protocol. Avoid d7. The provided invocations are:

```sh
./r31a_paired_probe same DEVICE V016 32768 PREFIX 60
./r31a_paired_probe same DEVICE V016 24576 PREFIX 60
./r31a_paired_probe pair DEVICE 32768 PREFIX
./r31a_paired_probe pair DEVICE 24576 PREFIX
```

## Current handoff boundary

Build, link, and fail-closed host tests: PASS. Route decision: awaiting Main review; no V022 or other revision was created. V021 source remains unchanged at `4f5bfc319b72d1f0bcfd453bc92e80ac64216719898757292daf1aa1b73b6063`. No runner execution, ACL/NPU correctness, or timing was performed; no active MAIN-1 exclusive lease is available. Earlier V021 correctness records and load-contaminated latency records remain unchanged. All build/configure logs, including failed attempts, are retained. No shared control file or other Route was changed.

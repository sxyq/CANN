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
| C++ runner | `support/paired_probe.cpp` | `c0fc544f61f736f44465ee1440bb7c89240c2fa6ffd7573be53c9ee126edf90a` |
| Runner body | `support/paired_probe_main.inc` | `0a9f4e262da816abd9ee3d5f377877627addda492f2d07e9b7f37e707d3626c6` |
| Parent ASC wrapper | `support/paired_probe_v016.asc` | `dd45d48c000b77f9fd47f8c9e050a39ad320da13319553f0722aad78e05baecc` |
| Candidate ASC wrapper | `support/paired_probe_v021.asc` | `4ccb96b2a9f5abf6b5f116a1bf2ff3fdff982ae42dbdac9f2f5c39a2e4541df1` |
| Shared host declarations | `support/probe_prelude.inc` | `b4f96bc019bd5c0e3e11846649c1f0466a1dbb629ca4d177908b8791ea6aa5b8` |
| Build rules | `support/CMakeLists.txt` | `e4d450f8c4b6884ea749d3396d7401fcd6d6ced2e1f6a32a0a36921908c225cd` |

The prior combined `support/paired_probe.asc` has been replaced by separate Parent and Candidate ASC shared-library translation units. Each wrapper renames and compiles its respective original `run_kernel`; the C++ executable uses ordinary `void*` host arguments and C ABI `dlopen`/`dlsym` bridges. This keeps `GM_ADDR` use inside each ASC module and isolates duplicate registration symbols. The generated ASC registration compile receives server3 GCC 11 standard-library paths through `CPATH` and `CPLUS_INCLUDE_PATH` in the CMake compile rule.

## Parent/Candidate executable mapping

Build source: `/home/data4t2/lelinfeng/phase4-worktrees/R31A/V021-direct/phase4/local/R31A/V021/support`

Build directory: `/home/data4t2/lelinfeng/phase4-worktrees/R31A/V021-direct/probe-build`
Toolchain target: CANN `8.5.0.alpha002`, `dav-2201`, `Ascend910B3`.

| Role | Original ASC input SHA-256 | Dedicated wrapper and exported bridge | Module ELF SHA-256 |
|---|---|---|---|
| Parent V016 | `dd13093823c885e785a650abff4863827e652eb8607ad0621a96eb31b6764fa0` | `paired_probe_v016.asc`; `r31a_run_kernel_v016` → `r31a_host_entry_v016` | `libr31a_paired_v016.so` / `11d7a2cb60912ce23b05a768c9076abcf2c513ad8b0844cc062d158082a4d5ea` |
| Candidate V021 | `4f5bfc319b72d1f0bcfd453bc92e80ac64216719898757292daf1aa1b73b6063` | `paired_probe_v021.asc`; `r31a_run_kernel_v021` → `r31a_host_entry_v021` | `libr31a_paired_v021.so` / `9bacd8cd72b8db5c6b8a1574ce199c674b18bc2bcf858cfb91ade29f71d24674` |

The shared C++ runner ELF is `r31a_paired_probe`, SHA-256 `c9a262a438182a096d36b78a0d34c5a31a77bc84934b1605db77f015959575d6`. Its host wrappers load the matching module and call the listed C ABI bridge. These are server3 build outputs; module loading and entry dispatch have not been exercised.

## Build attempts

`support/build-cann-server3-paired-r31a-paired_probe-20260925-final-link.log` records successful links for both ASC shared libraries and the final C++ executable; all three targets reached `Built target`. `support/build-cann-server3-paired-r31a-paired_probe-20260925-gmaddr-boundary.log` also ends with all three targets built. These are build-only results; no ELF was invoked.

All six configure logs are retained and report CMake configure/generate completion: `configure-server3-paired.log`, `configure-cann-server3-paired-r31a-20260925.log`, `configure-cann-server3-paired-r31a-20260925-cxx-driver.log`, `configure-cann-server3-paired-r31a-20260925-gcc11-env.log`, `configure-cann-server3-paired-r31a-20260925-gcc11-stdlib.log`, and `configure-cann-server3-paired-r31a-20260925-symbol-import.log`.

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

## Runner interface and local protocol

The built runner accepts either `pair DEVICE WIDTH OUTPUT_PREFIX` or `same DEVICE V016|V021 WIDTH OUTPUT_PREFIX GAP_SECONDS`. Shapes remain FP32, rows=2, blocks=1; width is 32768 or 24576. The source implements correctness before measurements, 10 warmups, and 21 event-timed samples per block or adjacent pair, writing raw samples and summary tables. This describes the compiled harness only; runtime loading, ACL registration, correctness, and timing have not been tested for these ELFs.

The next allowed measurement sequence is owned by Main: obtain an active exclusive MAIN-1 lease; use the Direct Parent V016 executable and each exact shape to qualify same-binary noise; proceed only for a shape marked PASS by the shared timing protocol. Avoid d7. The provided invocations are:

```sh
./r31a_paired_probe same DEVICE V016 32768 PREFIX 60
./r31a_paired_probe same DEVICE V016 24576 PREFIX 60
./r31a_paired_probe pair DEVICE 32768 PREFIX
./r31a_paired_probe pair DEVICE 24576 PREFIX
```

## Current handoff boundary

Build and link: PASS. Route decision: still awaiting Main review; no V022 or other revision was created. This turn did not execute the runner, ACL, NPU correctness, or timing. No MAIN-1 exclusive lease is active; d0-d6 are occupied and the unified protocol excludes d7. Existing V021 correctness records and load-contaminated historical latency records above remain unchanged. All build/configure logs, including failures, are retained. No shared control file or other Route was changed.

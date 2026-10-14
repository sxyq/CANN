# N01 V022

- Hypothesis: In the wide FP32 tile loop, the `valueTile` local-tensor handle is used only by `Add` and `Mul`, but its declaration currently precedes both input loads and its lexical scope spans `ReduceSum`. Restricting the handle to the Add/Mul region may reduce live address metadata across the reduction and its following synchronization. Arithmetic and instruction order, GM offsets, tile dimensions, and buffer allocation are unchanged.
- Changed function/range: `ProcessWideFp32FullCacheRows`, Candidate lines 2121-2137. `valueTile` is declared after `SyncMTE2ToV()` and scoped through the two existing arithmetic operations only.
- Parent: `线上结果/R31B/V011/submission.asc`; SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- Candidate: `本地实验/N01-REGISTER-LIVE-RANGE-X/V022/Candidate.asc`; SHA256 `2e0d5ecb1b0b2966f2c197e2f686b7d10b2a18a184df56a4cbfed6ee85674052`.
- Device cases that exercise the change: wide FP32 rows, especially `[2, 12288]` as the route target; `[128, 12288]` is a useful same-width diagnostic when the existing runner is available. D=8192 is not the wide-path case.
- CANN 8.5 Full-link: PASS in `/tmp/cann-n01-v022-rW4pnY`; local build log is `logs/build-server3.log`. Parent and Candidate input SHA matched this note; their linked objects and libraries had distinct SHA values.
- Device correctness attempt on d5 used the existing shared runner at 128x12288 FP32. Parent itself returned 1 with `max_abs_error=4.97162247`, `mismatches=1285428`; the runner stopped before Candidate. This same-width Parent failure is also present in the prior N01 route record, so this harness result cannot assess V022.
- Raw failed Parent output, runner identity, device pre-snapshot, and return code are preserved under `logs/correctness-20261014T180815Z/`.
- `CORRECTNESS=INCONCLUSIVE_PARENT_FAIL`; `LOCAL=NONE` and not run. The available runner fixes rows at 128, while the target diagnostic is `[2, 12288]`; no matching existing runner entry was present in this Clone. No performance conclusion is made.
- Next: use the existing project route for the two-row FP32 diagnostic, if available; otherwise retain V022 as compile-only evidence until a valid Parent/Candidate comparison path is available.

## Exact-source runner follow-up (2026-10-15)

- Main located the existing server3 `工具/route-runner/route-runner`, which accepts runtime `--rows`, `--width`, and `--dtype`; it could represent `[2, 12288]` FP32 without changing the comparison shape.
- Inputs were uploaded to the N01-only temporary directory `/tmp/cann-n01-v022-route-runner-RUv5fL/`. The build script verified Parent SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3` and Candidate SHA256 `2e0d5ecb1b0b2966f2c197e2f686b7d10b2a18a184df56a4cbfed6ee85674052`.
- The first build invocation used `dav-c220` for both CCE and NPU architecture arguments and returned 2 with `Unsupported NPU architecture or soc`. The CANN 8.5 mapping is CCE `dav-c220`, NPU `dav-2201`, SoC `Ascend910B3`.
- Follow-up builds with the NPU/SoC arguments separated, the default ASC compile rule restored, and the version-pinned CANN 8.5 environment still returned 2 for both sources during plugin registration:

```text
ASCPLUGIN: Unknown kernelInfo, mangledName: _Z24add_rms_norm_bias_customPhS_S_S_S_mmjff
ld.lld: undefined symbol: __origin__add_rms_norm_bias_custom<float/half/__bf16>
```

- This is a shared-runner integration failure; the existing V022 full-link PASS remains valid. The exact-source runner never linked, so its device Correctness and Local stages were not run. `CORRECTNESS=NONE_FOR_EXACT_SOURCE_RUNNER`; `LOCAL=NONE`.
- No route source or Candidate was changed by these attempts. The temporary server build tree and local CMake patch copy are removed after this record is committed.
- Next: retain V022 as compile-only until an existing exact-source runner can link the two inputs; do not treat the runner plugin error as a Candidate failure or a performance result.

## Runner argument diagnosis (2026-10-15)

- The current server tool checkout is `/home/data4t2/lelinfeng/cann-tooling-runner`, branch `tooling/canonical-route-runner`. Its worktree has staged and unstaged changes in the existing route-runner files and smoke fixtures; Main did not modify them.
- The retained `build-abi-smoke` Make flags pass `--npu-arch=dav-2201`, but the generated ASC compile rule passes `--cce-aicore-arch=dav-2201`. For this server's CANN 8.5 / Ascend910B3 setup, the CCE architecture is `dav-c220` and the NPU architecture is `dav-2201`.
- The current route-runner CMake source uses `RUNNER_ASCENDC_ARCH` for both architecture flags, so one value cannot represent the supported pair. This is a confirmed build-parameter mismatch and a plausible cause of the plugin registration error; it does not prove that it is the only cause. No build was rerun.
- The installed toolkit reports `Version=8.5.T8.0.B060` from `8.5.0.alpha002`, consistent with the recorded N01 Full-link toolchain. The exact-source Runner has not linked Parent or Candidate; Correctness and Local remain unrun.
- Next: separate the existing Runner's CCE and NPU architecture inputs in its source-controlled build entry, then retry the exact-source build with the same Parent/Candidate SHA values. Until that change is available locally and validated, V022 remains compile-only for the existing `w4r07_runner` path.

## Architecture argument trial (2026-10-15)

- Parent and Candidate were transferred to `/tmp/cann-n01-v022-archsplit-BVRYX2/`; remote SHA-256 values matched the identities above.
- The existing `build_server3.sh` ran from the current tooling checkout with `RUNNER_BUILD_DIR` set to this N01 temporary directory. To separate the intended CCE and NPU values without editing the shared checkout, `RUNNER_ASCENDC_ARCH` was passed as `dav-c220 --npu-arch=dav-2201`.
- The generated compile command placed `--cce-aicore-arch=dav-c220 --npu-arch=dav-2201` before `-Xaicore-start`, then emitted `--npu-arch=dav-c220 --npu-arch=dav-2201` again after `-Xhost-end`. Both Parent and Candidate compilation returned 2 with `Unsupported NPU architecture or soc`; no object or linked library was produced.
- This trial did not run Correctness or Local and did not invoke the NPU. It confirms that the current CMake rule emits a device architecture option in the host-option position; an invocation-only override cannot reliably correct the existing rule.
- The shared tooling checkout remains untouched and has pre-existing staged/unstaged changes. Its source is not present in this local repository; editing the server checkout would violate the local-development rule. V022 remains compile-only until the existing Runner source is available for a local change and rebuild.
- The N01-only temporary build directory is removed after this record is committed.

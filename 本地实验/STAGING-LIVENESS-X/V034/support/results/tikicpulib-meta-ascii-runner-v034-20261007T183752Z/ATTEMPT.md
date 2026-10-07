# V034 Route-Local Runner Attempt

- Route/revision: `STAGING-LIVENESS-X / V034`
- Stage: support-only configure/build; no executable, correctness, device, or Local test
- Candidate SHA-256 before and after: `75131108d32bc587c3c289d53ded04a15c5224417c675cb37a46df032e104d33`
- Installed package metadata: `toolkit/ascend_install.info` reports `Install_Path_Param=/usr/local/Ascend/ascend-toolkit`; the installed `tools/tikicpulib/lib/cmake/targets-tikicpulib.cmake` advertises `tools/tikicpulib/lib/include` in the exported tikicpulib include directories.
- Resolved header: `/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/tools/tikicpulib/lib/include/stub_def.h` (exists and was opened by BiSheng).
- Configure result: `PASS` (`CONFIGURE_RC=0`).
- Build result: `FAIL` (`BUILD_RC=2`); no `route_runner` executable was produced.
- Full configure/build output: `configure-build.log`, SHA-256 `c38688138dce9b9734107aa606fc222140130e928dfbc770c4155e0502d9e101`.

## Commands

Configure:

```sh
cmake -S '本地实验/STAGING-LIVENESS-X/V034/support/runner' \
  -B 'route_support/STAGING-LIVENESS-X/V034/build/tikicpulib-meta-ascii-runner-v034-20261007T183752Z' \
  -DENABLE_ASC=OFF \
  -DTIKICPULIB_INCLUDE_DIR='/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/tools/tikicpulib/lib/include' \
  -DEXACT_KERNEL_SOURCE_TU='/home/data4t2/lelinfeng/cann-staging-liveness-x/route_support/STAGING-LIVENESS-X/V034/exact_source_kernel.cpp' \
  -DKERNEL_SOURCE='/home/data4t2/lelinfeng/cann-staging-liveness-x/本地实验/STAGING-LIVENESS-X/V034/src/submission.asc'
```

Build:

```sh
cmake --build \
  'route_support/STAGING-LIVENESS-X/V034/build/tikicpulib-meta-ascii-runner-v034-20261007T183752Z' \
  --target route_runner --parallel 2
```

## Exact blocker

Configure succeeds, and preprocessing reaches the exact V034 source. The template build then compiles the same support TU as `aic_obj` and `aiv_obj` with BiSheng's device compiler. Because the metadata-resolved `tikicpulib` include directory is target-wide, `kernel_operator.h` reaches `stub_def.h` and its `stub_fun.h`; these host-stub definitions conflict with BiSheng's built-in CCE definitions. The log reports, among others:

- `kernel_fp16.h:225`: `half` conflicts with `__cce_half`.
- `kernel_bf16.h:225`: `bfloat16_t` conflicts with `__bf16`.
- `stub_def.h:82-103,121-132`: vector aliases and `int4x2_t` are redefined against CCE vector types.
- `stub_fun.h:14-15`: CRF enumerators are redefined against `cce_aicore_intrinsics.h`.

This is not a missing include path. No include-path retry was made. The evidence needed to resume is a verified way to make `stub_def.h` available only to the generated host-stub compilation while excluding it from the AICore/AIV BiSheng compilation, or an installed compatible host-stub/device-compiler pairing. Until then, runner BUILD/LINK remains failed; do not run device/Candidate tests or Local measurement.

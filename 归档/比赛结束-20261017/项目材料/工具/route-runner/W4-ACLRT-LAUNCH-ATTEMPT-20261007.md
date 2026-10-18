# W4 ACLRT Launch Attempt

Status: `TOOLING_BLOCKED`

Branch: `tooling/canonical-route-runner`

Scope: tooling-owned runner and fixture only. No Candidate/Route worktree, score/dashboard file, or Kernel source was touched.

## Attempt

- CANN: `8.5.T8.0.B060`
- SoC / Ascend C arch: `910B3` / `dav-2201`
- Parent and candidate input: `工具/route-runner/fixtures/fp32_rms_norm_bias_fixture.asc`
- Fixture SHA-256: `fd5fd7c681437d3c9e84a2a2146338173cca9e3500fd9eb8c01ed9ad434cb266`
- Isolated build directory: `/tmp/w4-route-runner-aclrt.WjQFmB/build`
- CMake configuration for the attempt: default `ENABLE_ACLRT_LAUNCH=ON` via an `option(...)` added immediately before `find_package(ASC REQUIRED)`. That one-line edit was removed after the failed attempt.

The build invocation set both `RUNNER_PARENT_SOURCE` and `RUNNER_CANDIDATE_SOURCE` to the fixture above, set both source SHA variables to the fixture SHA, set `RUNNER_ASCENDC_ARCH=dav-2201`, and set `RUNNER_BUILD_DIR` to the isolated directory, then ran:

```bash
bash "${ROOT}/build_server3.sh"
```

The configure/build output was captured in `工具/route-runner/W4-ACLRT-LAUNCH-ATTEMPT-20261007.log` (3,319 bytes; SHA-256 `03d375facb85e4c3c6c510383b137ddcd08214070d7da5f4700538ec84bef176`). It was byte-compared with the original `/tmp` capture.

## Failure

Both ASC wrapper targets failed on the fixture launch at line 144, column 39:

```text
[ERROR] ASCPLUGIN [pid:2012595, tid:2012595] [asc_ast_device_consumer.cpp:300] Unknown kernelInfo, mangledName: tooling_fixture_kernel at /home/data4t2/lelinfeng/cann-tooling-runner/工具/route-runner/fixtures/fp32_rms_norm_bias_fixture.asc:144, col:39, please check log.
[ERROR] ASCPLUGIN [pid:2012594, tid:2012594] [asc_ast_device_consumer.cpp:300] Unknown kernelInfo, mangledName: tooling_fixture_kernel at /home/data4t2/lelinfeng/cann-tooling-runner/工具/route-runner/fixtures/fp32_rms_norm_bias_fixture.asc:144, col:39, please check log.
ld.lld: error: undefined symbol: __origin__tooling_fixture_kernel
```

Failed targets: `route_runner_parent.dir/parent_adapter.asc.o` and `route_runner_candidate.dir/candidate_adapter.asc.o`. `cmake --build` returned 2; the build script returned 1. No `route_runner` executable or candidate shared library was produced. No fixture execution, Route correctness, Local measurement, or Online action was performed. No fallback or repeat attempt was made.

The prior staged and untracked runner work was preserved. The diagnostic commit contains only this attempt record and its raw log.

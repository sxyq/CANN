# TINY V001 Timing Harness Build/Link Record

DATE: `2026-10-03`
STATUS: `BUILD_PASS; LINK_PASS; EXECUTABLE_NOT_RUN`
ROUTE: `TINY-FIXED-OVERHEAD-CHAMPION-X`
REVISION: `V001`
SOURCE_CONTROL_COMMIT: `fe0a750df3c5c439ab52b5c9e70e751e5c3a80a6`
REMOTE_RUN_ID: `timing-build-20261003T132344Z`
REMOTE_ROOT: `/home/data4t2/lelinfeng/cann/server_runs/TINY-FIXED-OVERHEAD-CHAMPION-X/V001/timing-build-20261003T132344Z`

## Source and Module Identity

| Object | Identity |
|---|---|
| Candidate source `submission.asc` | SHA256 `4e8abff76486532cab4be6bc6098ea80c048802d285837ad0afb193ff9712024`; source commit `ab31036f` |
| Direct Parent source `R31B-V011/submission.asc` | SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3` |
| Staged exact Parent copy `source/support/parent_submission.asc` | SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3` |
| Timing runner source | SHA256 `0a2c6f80f170ecc4c0e2e982010cc374ea2b7a0377dcf4f4abdb8202ad4b4bf6` |
| CMake source | SHA256 `ad7742cd18cd094b2d09d4c9d9fbf89c4249a855d2484b895a59e53ee3dd86b0` |
| Generated Parent integration module | SHA256 `30382d11580fd83afbf3f981fb5dffcc7cbc8d599b9ef96fa607caed143cc293` |
| Generated Candidate integration module | SHA256 `433256f20d40c7d52b25fecc4de5b96bd77ccd57ca895faa7cc2ba09e5b03a2c` |
| Compiled ASC object `runner_tiny_v001_timing.asc.o` | SHA256 `f0daaeb65b79300f794faf3ee4aa7ce780479ecf1b812c6efe52b2f94c602c9d` |
| Executable `build/tiny_v001_timing_harness` | SHA256 `f8e2e8a509a6a36fd854d0ff25433999bc00d3b492cc8c40fb25bca74f7f5f6c`; ELF64 AArch64 PIE |

CMake generated the two integration modules under this new run's `build/` directory from the exact source files above, changing only class, device-kernel, and host-wrapper identifiers. The reset-free timing runner and both source identities were staged from the pushed Route worktree. The earlier executable at `timing-prep-20261003/build/tiny_v001_timing_harness` was left untouched; its SHA256 remains `c85ec6ec3a248b565cbc758b56cd98a616faa0fb78cda3b04272cd2c29027131` and it must not be run.

## Environment and Resources

- Host: `hwnput3`; CANN `8.5.0.alpha002`; Bisheng/Clang `15.0.5`; SoC `Ascend910B3`; target `dav-2201`.
- Main preflight at `2026-10-03T13:23:44Z`: d5 HBM `60200/65536 MB`, `5336 MB` free; AICore `44%`; VLLMWorker_TP PID `89602`; project disk `591 GB` free; no TINY build process and no d5 lease.
- Route pre-build read at `2026-10-03T13:29:16Z`: d5 HBM `60201/65536 MB`, AICore `43%`, VLLMWorker_TP PID `89602` using `56678 MB`; project disk `591 GB` free. Other processes and AICore activity were recorded only.
- Post-build disk read at `2026-10-03T13:32:17Z`: project disk `591 GB` free. The old executable SHA remained unchanged.

## Build Commands and Logs

```bash
cd /home/data4t2/lelinfeng/cann/server_runs/TINY-FIXED-OVERHEAD-CHAMPION-X/V001/timing-build-20261003T132344Z
source /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/script/set_env.sh
export CPATH=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/toolkit/toolchain/hcc/aarch64-target-linux-gnu/include:/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/toolkit/toolchain/hcc/aarch64-target-linux-gnu/include/c++/7.3.0:/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/toolkit/toolchain/hcc/aarch64-target-linux-gnu/include/c++/7.3.0/aarch64-target-linux-gnu:/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/toolkit/toolchain/hcc/aarch64-target-linux-gnu/include/c++/7.3.0/backward
cmake -S source/support -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --target tiny_v001_timing_harness --parallel 1 --verbose
```

- Configure log: `logs/cmake-configure.log`
- Build/link log: `logs/build-link.log` (`[100%] Built target tiny_v001_timing_harness`)
- Source, generated-module, object, and executable SHA list: `logs/identity.txt`
- Object/executable file types and ELF header: `logs/module-file.txt`, `logs/executable-elf.txt`
- Pre-build and post-build environment/resource snapshots: `logs/prebuild-environment.txt`, `logs/postbuild-state.txt`
- Build inventory: `logs/build-file-inventory.txt`

CMake emitted a non-fatal runtime search-path warning for `libstdc++.so.6`; the ASC host pass emitted non-fatal `cce_global` attribute warnings. Build and link completed successfully. No executable was launched. No correctness, same-binary, PRECHECK, P/C, or timing work was performed.

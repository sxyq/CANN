# SYNC-TOPOLOGY-CHAMPION-X V001 Build Fix Result

Status: `BUILD=PASS`. Correctness is recorded separately in `CORRECTNESS-RESULT.md`.

## Source and run identity

- Route branch: `w2/m1/sync-topology`
- Candidate source commit: `8e70939563297df675fe46d3045524780dd29c80`
- Candidate path: `本地实验/SYNC-TOPOLOGY-CHAMPION-X/V001/submission.asc`
- Candidate SHA256: `27c853e1cb0c47307899b874338c3aecb912ad5dc125c0d67f6afba76a9117ec`
- Build-fix commit: `29a6f65df278bb44414d5526264d95cc18833bd0`
- RUN_ID: `SYNC-TOPOLOGY-CHAMPION-X-V001-D4-BUILD-FIX-20261002T153544Z`
- Server route worktree, resolved by branch with `git worktree list`: `/home/data4t2/lelinfeng/cann-w2-m1-sync`, HEAD `c0063657a04f4f1c9b746a8377bd820cb26469b4`.

## Compiler paths

- Host: `hwnput3`; SoC: `Ascend 910B3`; `npu-smi`: `25.0.rc1.1`.
- `ASCEND_HOME_PATH=/usr/local/Ascend/ascend-toolkit/latest`, resolving to CANN `8.5.0.alpha002`.
- `bisheng` resolves to `/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/ccec_compiler/bin/bisheng`.
- The `bisheng -E -v` host query selected GCC 12. Its default C++ search entries included `/usr/lib/gcc/aarch64-linux-gnu/12/../../../../include/c++`, which did not provide `vector`.
- Verified installed headers: `/usr/include/c++/11/vector`, `/usr/include/aarch64-linux-gnu/c++/11/bits/c++config.h`, and `/usr/include/c++/11/backward`.
- The CANN linker compiles a generated host registration unit in a child process. The ASC `-Xhost-start` options did not reach that child. The target now launches compilation through `cmake -E env CPATH=...`, carrying the three verified GCC 11 directories into child processes. The earlier `-Xhost-start` include arguments remain in place for the direct ASC host compile.
- An earlier include-only attempt, RUN_ID `SYNC-TOPOLOGY-CHAMPION-X-V001-D4-BUILD-FIX-20261002T152003Z`, still failed with RC `2`; its logs remain under the same V001 `logs/` directory. The successful attempt below uses the child-process `CPATH` launch setting.

## Server resources

- Pre-build snapshot: `2026-10-02T15:42:29Z`; device 4 HBM used `61221/65536 MB`, free `4315 MB`; AICore `0%`.
- Device 4 processes recorded: Python PID `1819590` (`2084 MB`) and `VLLMEngineCor` PID `2999855` (`55664 MB`). Neither was changed.
- Available disk: `628G` at `/home/data4t2`.

## Commands and results

```bash
source /usr/local/Ascend/ascend-toolkit/set_env.sh
# RC=0

printf '%s  %s\n' '27c853e1cb0c47307899b874338c3aecb912ad5dc125c0d67f6afba76a9117ec' "$REMOTE_ROOT/submission.asc" | sha256sum -c
# RC=0

cmake -S "$SUPPORT_DIR" -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Release -DASCEND_CANN_PACKAGE_PATH="$ASCEND_HOME_PATH"
# RC=0

cmake --build "$BUILD_DIR" --target sync_topology_v001_correctness --verbose -j2
# RC=0

sha256sum "$BUILD_DIR/sync_topology_v001_correctness"
# 9e9bb3d578b7d967e2e11538c9dafbac50e3200764007fa1b6a4db4ae5a829f8
```

- Remote source-identity log: `/home/data4t2/lelinfeng/cann/w2/SYNC-TOPOLOGY-CHAMPION-X/V001/logs/SYNC-TOPOLOGY-CHAMPION-X-V001-D4-BUILD-FIX-20261002T153544Z-stage-identity.log`
- Environment log: `/home/data4t2/lelinfeng/cann/w2/SYNC-TOPOLOGY-CHAMPION-X/V001/logs/SYNC-TOPOLOGY-CHAMPION-X-V001-D4-BUILD-FIX-20261002T153544Z-environment.log`
- Resource log: `/home/data4t2/lelinfeng/cann/w2/SYNC-TOPOLOGY-CHAMPION-X/V001/logs/SYNC-TOPOLOGY-CHAMPION-X-V001-D4-BUILD-FIX-20261002T153544Z-pre-build-resource-run.log`
- Configure log: `/home/data4t2/lelinfeng/cann/w2/SYNC-TOPOLOGY-CHAMPION-X/V001/logs/SYNC-TOPOLOGY-CHAMPION-X-V001-D4-BUILD-FIX-20261002T153544Z-configure.log`
- Build/Link log: `/home/data4t2/lelinfeng/cann/w2/SYNC-TOPOLOGY-CHAMPION-X/V001/logs/SYNC-TOPOLOGY-CHAMPION-X-V001-D4-BUILD-FIX-20261002T153544Z-build-link.log`
- Executable identity log: `/home/data4t2/lelinfeng/cann/w2/SYNC-TOPOLOGY-CHAMPION-X/V001/logs/SYNC-TOPOLOGY-CHAMPION-X-V001-D4-BUILD-FIX-20261002T153544Z-executable-identity.log`
- Executable: `/home/data4t2/lelinfeng/cann/w2/SYNC-TOPOLOGY-CHAMPION-X/V001/build-SYNC-TOPOLOGY-CHAMPION-X-V001-D4-BUILD-FIX-20261002T153544Z/sync_topology_v001_correctness`

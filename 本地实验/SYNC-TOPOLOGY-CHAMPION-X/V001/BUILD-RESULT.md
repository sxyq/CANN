# SYNC-TOPOLOGY-CHAMPION-X V001 Build Result

Latest Build/Link: `PASS`, Configure `PASS`, return code `0` in `SYNC-TOPOLOGY-CHAMPION-X-V001-D4-BUILD-FIX-6e650cf5-20261002T231909Z`. The earlier failed attempt remains below as historical evidence.

## Latest Build/Link

- Route HEAD: `6e650cf5224ca0750d9d4886f5ccc428a9edde19`
- Candidate SHA256: `27c853e1cb0c47307899b874338c3aecb912ad5dc125c0d67f6afba76a9117ec`
- Direct Parent: `R31B V011`
- Parent SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- Timing executable SHA256: `3628f9156ab7171e28c0d88a13f6e528f2cf85f138bb412eeab47d35bb24de2f`
- Correctness executable SHA256: `7a8335eec7156cf695cf75281a576a2b403e7df5da24de7fd7c1d8759abc97a7`
- Parent DSO SHA256: `017ab02a4fe0da2c2692ba18c17c89ad27cd928ef66a14bf8035d2c4731ec4ec`
- Candidate DSO SHA256: `1a7172946ceab8786920f0e386d2df11307d58e22d86a8f6da90d11cce3844f4`
- The log has no errors and a small number of `GM_ADDR` ignored-attributes warnings.
- Build/Link log: `/home/data4t2/lelinfeng/cann/w2/SYNC-TOPOLOGY-CHAMPION-X/V001/results/SYNC-TOPOLOGY-CHAMPION-X-V001-D4-BUILD-FIX-6e650cf5-20261002T231909Z/build-link.log`
- Executable identity: `/home/data4t2/lelinfeng/cann/w2/SYNC-TOPOLOGY-CHAMPION-X/V001/results/SYNC-TOPOLOGY-CHAMPION-X-V001-D4-BUILD-FIX-6e650cf5-20261002T231909Z/executable-identity.txt`

## Historical First Build Attempt

### Source identity

- Workstation route branch: `w2/m1/sync-topology`
- Workstation HEAD: `c0063657a04f4f1c9b746a8377bd820cb26469b4`
- Candidate source commit: `8e70939563297df675fe46d3045524780dd29c80`
- Candidate path: `本地实验/SYNC-TOPOLOGY-CHAMPION-X/V001/submission.asc`
- Candidate SHA256: `27c853e1cb0c47307899b874338c3aecb912ad5dc125c0d67f6afba76a9117ec`
- Remote source identity command returned `0`; the remote file matched the expected SHA256.
- Remote route worktree was resolved with `git worktree list --porcelain`: `/home/data4t2/lelinfeng/cann-w2-m1-sync`, branch `w2/m1/sync-topology`, HEAD `c0063657a04f4f1c9b746a8377bd820cb26469b4`.

## Server snapshot

- Time: `2026-10-02T14:32:14Z`; host: `hwnput3`; SoC: `Ascend 910B3`; `npu-smi`: `25.0.rc1.1`.
- Device 4 HBM: `60416/65536 MB` used, `5120 MB` free; AICore: `0%`.
- Active device 4 processes: Python PID `1819590` (`1278 MB`) and `VLLMEngineCor` PID `2999855` (`55664 MB`). Neither was stopped or changed.
- Available disk at `/home/data4t2/lelinfeng/cann`: `629G`.
- `ASCEND_HOME_PATH`: `/usr/local/Ascend/ascend-toolkit/latest`; the configured `bisheng` resolves to `/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/ccec_compiler/bin/bisheng`.

## Commands and results

Attempt: `V001-D4-20261002T143000Z`

```bash
REMOTE_ROOT=/home/data4t2/lelinfeng/cann/w2/SYNC-TOPOLOGY-CHAMPION-X/V001
ATTEMPT_ID=V001-D4-20261002T143000Z

source /usr/local/Ascend/ascend-toolkit/set_env.sh > "$REMOTE_ROOT/logs/${ATTEMPT_ID}-environment.log" 2>&1
# RC=0

printf '%s  %s\n' "27c853e1cb0c47307899b874338c3aecb912ad5dc125c0d67f6afba76a9117ec" "$REMOTE_ROOT/submission.asc" | sha256sum -c > "$REMOTE_ROOT/logs/${ATTEMPT_ID}-source-identity.log" 2>&1
# RC=0

cmake -S "$REMOTE_ROOT/support" -B "$REMOTE_ROOT/build" -DCMAKE_BUILD_TYPE=Release -DASCEND_CANN_PACKAGE_PATH="$ASCEND_HOME_PATH" > "$REMOTE_ROOT/logs/${ATTEMPT_ID}-configure.log" 2>&1
# RC=0

cmake --build "$REMOTE_ROOT/build" --target sync_topology_v001_correctness --verbose -j2 > "$REMOTE_ROOT/logs/${ATTEMPT_ID}-build-link.log" 2>&1
# RC=2
```

- Environment log: `/home/data4t2/lelinfeng/cann/w2/SYNC-TOPOLOGY-CHAMPION-X/V001/logs/V001-D4-20261002T143000Z-environment.log`
- Source identity log: `/home/data4t2/lelinfeng/cann/w2/SYNC-TOPOLOGY-CHAMPION-X/V001/logs/V001-D4-20261002T143000Z-source-identity.log`
- Configure log: `/home/data4t2/lelinfeng/cann/w2/SYNC-TOPOLOGY-CHAMPION-X/V001/logs/V001-D4-20261002T143000Z-configure.log`
- Build/Link log: `/home/data4t2/lelinfeng/cann/w2/SYNC-TOPOLOGY-CHAMPION-X/V001/logs/V001-D4-20261002T143000Z-build-link.log`
- Build log reports `/tmp/asc_plugin_binary_register_code-19bd19.c:3:10: fatal error: 'vector' file not found`; subsequent linker errors follow.
- Executable SHA256: unavailable; `/home/data4t2/lelinfeng/cann/w2/SYNC-TOPOLOGY-CHAMPION-X/V001/build/sync_topology_v001_correctness` was absent.
- Executable identity log was not created. No correctness command or timing command was run.

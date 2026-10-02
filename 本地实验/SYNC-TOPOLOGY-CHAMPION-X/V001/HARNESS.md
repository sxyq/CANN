# SYNC-TOPOLOGY-CHAMPION-X V001 Build / Correctness Harness

Status: the initial Build/Link attempt failed; see `BUILD-RESULT.md`. Correctness and timing have not run.

## Source identity

- Route/revision: `SYNC-TOPOLOGY-CHAMPION-X V001`
- Hypothesis: `H2`, declared in `REVISION-DECLARATION.md`
- Candidate: `submission.asc`
- Expected Candidate SHA256: `27c853e1cb0c47307899b874338c3aecb912ad5dc125c0d67f6afba76a9117ec`
- Target device: `4`
- SoC/toolchain pattern: Ascend 910B3 / CANN direct-invocation CMake pattern, following committed project harnesses.

## Coverage

The runner requests `availableCoreNum=1`; with two rows this launches one block and produces `batchRows=2` on the primary shape.

| Case | Shape | Dtype | Purpose |
|---|---|---|---|
| `h2_primary_wide_path` | `[2, 12288]` | FP16 | H2 target path; three 4096-element tiles |
| `h2_unmodified_path_control` | `[2, 8192]` | FP16 | Exact-width control that does not enter the `D > 8192` wide path |

Each case checks output against an FP32 host reference that models the FP16 rounding points in the V011 implementation. It uses the project floating-point mixed tolerance for FP16: `atol=2^-9`, `rtol=2^-9`, required matched ratio `>=0.99`, and maximum absolute error `<=0.1`. BF16 is intentionally excluded until the primary FP16 direction has stable evidence.

## Server output directory

Use this isolated directory for the eventual assigned server job:

```text
/home/data4t2/lelinfeng/cann/w2/SYNC-TOPOLOGY-CHAMPION-X/V001/
```

The `build/` directory holds CMake output and the executable. Each source-identity, configure, build/link, and correctness command below has its own log in `logs/`. Do not reuse log names across attempts; allocate a new attempt suffix before rerunning.

## Stage files

From the repository root on the workstation, after the route commit is available on the server:

```bash
REMOTE_ROOT=/home/data4t2/lelinfeng/cann/w2/SYNC-TOPOLOGY-CHAMPION-X/V001
ssh cann-server3 "mkdir -p '$REMOTE_ROOT/support' '$REMOTE_ROOT/build' '$REMOTE_ROOT/logs'"
scp 本地实验/SYNC-TOPOLOGY-CHAMPION-X/V001/submission.asc cann-server3:"$REMOTE_ROOT/submission.asc"
scp 本地实验/SYNC-TOPOLOGY-CHAMPION-X/V001/support/CMakeLists.txt cann-server3:"$REMOTE_ROOT/support/CMakeLists.txt"
scp 本地实验/SYNC-TOPOLOGY-CHAMPION-X/V001/support/npu_correctness.asc cann-server3:"$REMOTE_ROOT/support/npu_correctness.asc"
```

## Exact commands

The server's `bisheng` host compiler selects GCC 12, whose default search path omits the installed GCC 11 C++ headers. The CANN linker also compiles a generated host registration unit in a child process that does not inherit the ASC `-Xhost-start` include arguments. The CMake target therefore supplies the verified GCC 11 directories through `CPATH` for the compiler process and its children.

Run these from a server shell only when Main's device/job allocation is active. Set the paths and a fresh attempt ID first:

```bash
REMOTE_ROOT=/home/data4t2/lelinfeng/cann/w2/SYNC-TOPOLOGY-CHAMPION-X/V001
ATTEMPT_ID="V001-D4-$(date -u +%Y%m%dT%H%M%SZ)"
source /usr/local/Ascend/ascend-toolkit/set_env.sh
```

1. Verify the staged source. Log: `$REMOTE_ROOT/logs/${ATTEMPT_ID}-source-identity.log`

```bash
printf '%s  %s\n' '27c853e1cb0c47307899b874338c3aecb912ad5dc125c0d67f6afba76a9117ec' "$REMOTE_ROOT/submission.asc" | sha256sum -c > "$REMOTE_ROOT/logs/${ATTEMPT_ID}-source-identity.log" 2>&1
```

2. Configure the CANN direct-invocation project. Log: `$REMOTE_ROOT/logs/${ATTEMPT_ID}-configure.log`

```bash
cmake -S "$REMOTE_ROOT/support" -B "$REMOTE_ROOT/build" -DCMAKE_BUILD_TYPE=Release -DASCEND_CANN_PACKAGE_PATH="$ASCEND_HOME_PATH" > "$REMOTE_ROOT/logs/${ATTEMPT_ID}-configure.log" 2>&1
```

3. Compile and link the exact Candidate into the correctness executable. Log: `$REMOTE_ROOT/logs/${ATTEMPT_ID}-build-link.log`

```bash
cmake --build "$REMOTE_ROOT/build" --target sync_topology_v001_correctness --verbose -j2 > "$REMOTE_ROOT/logs/${ATTEMPT_ID}-build-link.log" 2>&1
```

4. Run correctness on device 4 only. Log: `$REMOTE_ROOT/logs/${ATTEMPT_ID}-correctness-device4.log`

```bash
LD_LIBRARY_PATH="$ASCEND_HOME_PATH/aarch64-linux/lib64${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}" "$REMOTE_ROOT/build/sync_topology_v001_correctness" > "$REMOTE_ROOT/logs/${ATTEMPT_ID}-correctness-device4.log" 2>&1
```

The runner fixes `kDeviceId=4` and requests one available core for each test case. It prints a case line and a summary with matched ratio, max absolute error, case status, and `timing=NOT_RUN`. A non-zero executable exit is a correctness failure or runtime/setup failure; retain the log unchanged and classify the state separately from performance.

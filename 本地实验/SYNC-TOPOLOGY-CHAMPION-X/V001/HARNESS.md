# SYNC-TOPOLOGY-CHAMPION-X V001 Build / Correctness / Timing Harness

Candidate correctness passed previously; see `CORRECTNESS-RESULT.md`. The latest Build/Link failure is recorded in `BUILD-FAIL-SYNC-TOPOLOGY-CHAMPION-X-V001-D4-BUILD-FIX-7f73e93d-20261002T223305Z.md`: source identities and Configure passed, Build/Link returned 2 because metadata and ACL host declarations were absent from the host compile, and no executable was produced. Local timing has not run. The current harness builds the exact Parent and Candidate ASC source files as separate registered modules, preserving `add_rms_norm_bias_custom` and renaming only each host `run_kernel` wrapper; device lease approval is required only before timing.

## Source identity

- Route/revision: `SYNC-TOPOLOGY-CHAMPION-X V001`
- Hypothesis: `H2`, declared in `REVISION-DECLARATION.md`
- Candidate: `submission.asc`
- Expected Candidate SHA256: `27c853e1cb0c47307899b874338c3aecb912ad5dc125c0d67f6afba76a9117ec`
- Direct Parent: `R31B-V011`
- Expected Parent SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- Existing Candidate correctness executable SHA256: `9e9bb3d578b7d967e2e11538c9dafbac50e3200764007fa1b6a4db4ae5a829f8` (not the timing executable)
- Target device: `4`
- SoC/toolchain pattern: Ascend 910B3 / CANN direct-invocation CMake pattern, following committed project harnesses.

## Coverage

The timing runner is restricted to the approved `[2,12288] FP16` shape. It calls each kernel with `availableCoreNum=1`, producing one block and two rows per block. The correctness executable retains its existing `[2,8192] FP16` control, but that shape is excluded from timing.

| Case | Shape | Dtype | Purpose |
|---|---|---|---|
| `h2_primary_wide_path` | `[2, 12288]` | FP16 | H2 target path; three 4096-element tiles |
| `h2_unmodified_path_control` | `[2, 8192]` | FP16 | Exact-width control that does not enter the `D > 8192` wide path |

Each case checks output against an FP32 host reference that models the FP16 rounding points in the V011 implementation. It uses the project floating-point mixed tolerance for FP16: `atol=2^-9`, `rtol=2^-9`, required matched ratio `>=0.99`, and maximum absolute error `<=0.1`. BF16 is intentionally excluded until the primary FP16 direction has stable evidence.

## Server output directory

Use this isolated directory for a future Main-assigned server job:

```text
/home/data4t2/lelinfeng/cann/w2/SYNC-TOPOLOGY-CHAMPION-X/V001/
```

Keep the timing build in `build-timing/`. Each run gets a new `results/<RUN_ID>/` directory for identity output, stage logs, raw samples, jitter summaries, and device snapshots. Do not reuse a result directory.

## Stage files

Stage the eight support files below from the route worktree for Build/Correctness. The Parent remains the existing root source; do not edit or replace it. A new Main lease is required before starting any timing stage. Candidate/Parent source identities and support-file identities are recorded separately.

```bash
(
set -e
REMOTE_ROOT=/home/data4t2/lelinfeng/cann/w2/SYNC-TOPOLOGY-CHAMPION-X/V001
RUN_ID="SYNC-TOPOLOGY-CHAMPION-X-V001-D4-$(date -u +%Y%m%dT%H%M%SZ)"
RESULT_DIR="$REMOTE_ROOT/results/$RUN_ID"
SUPPORT_FILES="CMakeLists.txt npu_correctness.asc local_abi_shim.h local_tensor_metadata.h timing_runner.cpp runner_main.inc summarize_window.mjs run_parent_window.sh"
(cd 本地实验/SYNC-TOPOLOGY-CHAMPION-X/V001/support && sha256sum $SUPPORT_FILES)
ssh cann-server3 "mkdir -p '$REMOTE_ROOT/support' '$REMOTE_ROOT/results' && test ! -e '$RESULT_DIR' && mkdir '$RESULT_DIR'"
scp 本地实验/SYNC-TOPOLOGY-CHAMPION-X/V001/submission.asc cann-server3:"$REMOTE_ROOT/submission.asc"
scp 本地实验/SYNC-TOPOLOGY-CHAMPION-X/V001/support/CMakeLists.txt cann-server3:"$REMOTE_ROOT/support/CMakeLists.txt"
scp 本地实验/SYNC-TOPOLOGY-CHAMPION-X/V001/support/npu_correctness.asc cann-server3:"$REMOTE_ROOT/support/npu_correctness.asc"
scp 本地实验/SYNC-TOPOLOGY-CHAMPION-X/V001/support/local_abi_shim.h cann-server3:"$REMOTE_ROOT/support/local_abi_shim.h"
scp 本地实验/SYNC-TOPOLOGY-CHAMPION-X/V001/support/local_tensor_metadata.h cann-server3:"$REMOTE_ROOT/support/local_tensor_metadata.h"
scp 本地实验/SYNC-TOPOLOGY-CHAMPION-X/V001/support/timing_runner.cpp cann-server3:"$REMOTE_ROOT/support/timing_runner.cpp"
scp 本地实验/SYNC-TOPOLOGY-CHAMPION-X/V001/support/runner_main.inc cann-server3:"$REMOTE_ROOT/support/runner_main.inc"
scp 本地实验/SYNC-TOPOLOGY-CHAMPION-X/V001/support/summarize_window.mjs cann-server3:"$REMOTE_ROOT/support/summarize_window.mjs"
scp 本地实验/SYNC-TOPOLOGY-CHAMPION-X/V001/support/run_parent_window.sh cann-server3:"$REMOTE_ROOT/support/run_parent_window.sh"
ssh cann-server3 "cd '$REMOTE_ROOT/support' && sha256sum $SUPPORT_FILES | tee '$RESULT_DIR/runner-source-identity.log'"
printf 'RUN_ID=%s\nRESULT_DIR=%s\n' "$RUN_ID" "$RESULT_DIR"
)
```

The exact Parent and Candidate `.asc` files are passed directly to separate `ascendc_library()` targets. Target-specific `ascendc_compile_definitions()` rename only the host `run_kernel` wrappers; the `__global__` `add_rms_norm_bias_custom` entry remains unchanged for ASCPLUGIN registration. The custom `CMAKE_ASC_COMPILE_OBJECT` rule puts `-include local_tensor_metadata.h` inside both the `-Xaicore-start` and `-Xhost-start` sections, so `TensorInfo` and `TensorGroupInfo` reach the ASC parser and host wrapper compiler. It puts `-include local_abi_shim.h` only inside the host section; `<acl/acl.h>` is therefore host-only. Do not inject these repeated `-include` options with `ascendc_compile_options()` or `target_compile_options()`: CMake de-duplicates target options and can leave an include path in the wrong compiler section. `npu_correctness.asc` includes `local_tensor_metadata.h` instead of declaring duplicate structs. Both registered modules use `-Wl,-Bsymbolic`, and the C++ runner calls their host wrappers through the existing `runner_main.inc` implementation.

The staging block prints local and server support-file digests with the same relative filenames and order. The SSH command writes the server output to `$RESULT_DIR/runner-source-identity.log` through remote `tee` and displays it locally. Compare all eight digests before configuring.

Any digest mismatch means stop and do not configure, build, or measure.

## Exact commands

The server's `bisheng` host compiler selects GCC 12, whose default search path omits the installed GCC 11 C++ headers. The CANN linker also compiles a generated host registration unit in a child process that does not inherit the ASC `-Xhost-start` include arguments. The CMake target therefore supplies the verified GCC 11 directories through `CPATH` for the compiler process and its children. The installed CANN `FindASC.cmake` implements `ascendc_library()` by marking each `.asc` source as ASC and attaching its registration/runtime link interface.

The server checkout is `/home/data4t2/lelinfeng/cann-w2-m1-sync`. Verify both source files against the SHA values above before configuring. The prior correctness executable is not accepted as the timing executable because the latter includes both kernel implementations.

Run build commands only after Main has reviewed the local harness. Run any timing command only after Main confirms the live lease and HBM again. In the server shell, reuse the `RUN_ID` printed by the staging block:

```bash
REMOTE_ROOT=/home/data4t2/lelinfeng/cann/w2/SYNC-TOPOLOGY-CHAMPION-X/V001
printf 'Paste RUN_ID printed by staging: '
read -r RUN_ID
RESULT_DIR="$REMOTE_ROOT/results/$RUN_ID"
source /usr/local/Ascend/ascend-toolkit/set_env.sh
test -d "$RESULT_DIR"
```

The result directory was created only after the staging block checked `test ! -e "$RESULT_DIR"`; do not recreate or reuse it. Keep the timing build at the fixed `$REMOTE_ROOT/build-timing` path; the per-run directory holds evidence rather than a build cache.

1. Verify the staged Candidate and the Direct Parent from the route checkout. Log: `$RESULT_DIR/source-identity.log`. The support identities are recorded separately in `$RESULT_DIR/runner-source-identity.log` by the staging step.

```bash
printf '%s  %s\n' \
  '27c853e1cb0c47307899b874338c3aecb912ad5dc125c0d67f6afba76a9117ec' "$REMOTE_ROOT/submission.asc" \
  'a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3' \
  /home/data4t2/lelinfeng/cann-w2-m1-sync/线上结果/R31B/V011/submission.asc \
  | sha256sum -c > "$RESULT_DIR/source-identity.log" 2>&1
```

2. Configure the CANN direct-invocation project. Log: `$RESULT_DIR/configure.log`

```bash
cmake -S "$REMOTE_ROOT/support" -B "$REMOTE_ROOT/build-timing" \
  -DCMAKE_BUILD_TYPE=Release \
  -DASCEND_CANN_PACKAGE_PATH="$ASCEND_HOME_PATH" \
  -DSYNC_PARENT_SOURCE=/home/data4t2/lelinfeng/cann-w2-m1-sync/线上结果/R31B/V011/submission.asc \
  > "$RESULT_DIR/configure.log" 2>&1
```

3. Compile and link both exact-source executables. Log: `$RESULT_DIR/build-link.log`

```bash
cmake --build "$REMOTE_ROOT/build-timing" \
  --target sync_topology_v001_timing sync_topology_v001_correctness --verbose -j2 \
  > "$RESULT_DIR/build-link.log" 2>&1
sha256sum "$REMOTE_ROOT/build-timing/sync_topology_v001_timing" \
  "$REMOTE_ROOT/build-timing/sync_topology_v001_correctness" \
  "$REMOTE_ROOT/build-timing/libsync_topology_v001_parent.so" \
  "$REMOTE_ROOT/build-timing/libsync_topology_v001_candidate.so" \
  > "$RESULT_DIR/executable-identity.txt"
```

4. After Build/Link PASS, immediately run the existing correctness target on device 4. Save the pre/post resource and process snapshots plus its complete output; do not run any timing mode.

```bash
npu-smi info > "$RESULT_DIR/pre-correctness-device.txt" 2>&1
df -h /home/data4t2/lelinfeng/cann > "$RESULT_DIR/pre-correctness-disk.txt" 2>&1
du -sh /home/data4t2/lelinfeng/cann/* > "$RESULT_DIR/pre-correctness-project-usage.txt" 2>&1
ps -eo pid,etime,args > "$RESULT_DIR/pre-correctness-processes.txt"
if LD_LIBRARY_PATH="$ASCEND_HOME_PATH/aarch64-linux/lib64${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}" \
  "$REMOTE_ROOT/build-timing/sync_topology_v001_correctness" \
  > "$RESULT_DIR/correctness.log" 2>&1; then
  correctness_rc=0
else
  correctness_rc=$?
fi
npu-smi info > "$RESULT_DIR/post-correctness-device.txt" 2>&1
df -h /home/data4t2/lelinfeng/cann > "$RESULT_DIR/post-correctness-disk.txt" 2>&1
du -sh /home/data4t2/lelinfeng/cann/* > "$RESULT_DIR/post-correctness-project-usage.txt" 2>&1
ps -eo pid,etime,args > "$RESULT_DIR/post-correctness-processes.txt"
printf 'CORRECTNESS_RC=%s\n' "$correctness_rc" > "$RESULT_DIR/correctness-status.txt"
test "$correctness_rc" -eq 0
```

## Timing sequence

Record a fresh `npu-smi info` snapshot and disk availability before the first qualification stage. The snapshot includes d4 HBM, AICore, and resident process IDs. Do not stop or alter any listed process.

1. Run Parent same-binary shape qualification in one process. This performs 45 synchronized warmups followed by two blocks of 31 device-event samples, writes all samples and jitter, and checks output after timing. Both `MAD/median <= 0.10` and block drift `<= 0.10` must pass. Save stdout to `$RESULT_DIR/same-binary-runner.log`.

```bash
npu-smi info > "$RESULT_DIR/pre-same-binary-device.txt" 2>&1
df -h /home/data4t2/lelinfeng/cann > "$RESULT_DIR/pre-same-binary-disk.txt" 2>&1
du -sh /home/data4t2/lelinfeng/cann/* > "$RESULT_DIR/pre-same-binary-project-usage.txt" 2>&1
"$REMOTE_ROOT/build-timing/sync_topology_v001_timing" \
  --same-binary "$RESULT_DIR/SAME-BINARY" \
  > "$RESULT_DIR/same-binary-runner.log" 2>&1
npu-smi info > "$RESULT_DIR/post-same-binary-device.txt" 2>&1
```

2. Only after same-binary PASS, run the Parent-only window qualification. It uses three fresh processes in PRECHECK-A and three in PRECHECK-B; each process has 45 synchronized warmups and 21 device-event samples. The script records device snapshots before, between, and after the blocks. Summarize the six process medians with the bundled script. Both A and B must meet `CV <= 0.15` and `max/min <= 1.30`.

```bash
export SYNC_RUNNER="$REMOTE_ROOT/build-timing/sync_topology_v001_timing"
export SYNC_RESULT_DIR="$RESULT_DIR/window"
bash "$REMOTE_ROOT/support/run_parent_window.sh"
```

3. Only if the same-binary and both window blocks pass, and Main's new lease remains active, run adjacent interleaved Parent/Candidate pairs. The runner performs 45 alternating synchronized warmups for each binary, then four groups of 11 adjacent device-event pairs. Pair order alternates `P,C` / `C,P`; raw rows include both `device_us` and `wall_us`. Output/jitter paths are `$RESULT_DIR/PAIRED-raw.tsv` and `$RESULT_DIR/PAIRED-jitter.txt`.

```bash
df -h /home/data4t2/lelinfeng/cann > "$RESULT_DIR/pre-paired-disk.txt" 2>&1
npu-smi info > "$RESULT_DIR/pre-paired-device.txt" 2>&1
"$REMOTE_ROOT/build-timing/sync_topology_v001_timing" \
  --paired "$RESULT_DIR/PAIRED" \
  > "$RESULT_DIR/paired-runner.log" 2>&1
npu-smi info > "$RESULT_DIR/post-paired-device.txt" 2>&1
df -h /home/data4t2/lelinfeng/cann > "$RESULT_DIR/post-paired-disk.txt" 2>&1
du -sh /home/data4t2/lelinfeng/cann/* > "$RESULT_DIR/post-paired-project-usage.txt" 2>&1
```

The timing executable does not reset the device. It runs output comparison and D2H only after timed samples. `device_us` is primary; `wall_us` is diagnostic. Keep all raw/jitter files and every pre/post load snapshot. If a qualification stage fails, do not run later timing modes; record the failure, release the new lease through Main, and retain the evidence. If P/C completes, hand off the evidence to Main for the Local verdict, then release the lease.

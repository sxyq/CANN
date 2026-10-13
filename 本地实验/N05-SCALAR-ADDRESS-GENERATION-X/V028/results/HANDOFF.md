# N05 V028 Handoff

V028 source commit is `d6f9606dbc500e2fb0e1d17dd4603a729f3bec38`, based on V027 source commit `56b2940aa688637c0d1260caa8f1ac953ae7f1d4`. Pass-one retained-y UB tile addresses are derived from `batchRow * rowWidth + col`; this differs from the V022 pass-two address change. Parent and Candidate local/remote SHA256 values match, CMakeCache points to distinct input directories, and the saved Runner binary matches the remote build output.

CANN 8.5 configuration and compilation passed. Correctness passed all four cases for Parent and Candidate. One paired Local run completed 4 cases x 21 pairs on d2. Paired-delta medians were -1.860, +0.900, +0.100, and +0.640 us. Every range spans zero, and the BF16 D=32768 range contains a -52.200 us outlier; `LOCAL_SCORE=NONE`.

d2 HBM readings were 62855/65536 MB before Correctness, 62855/65536 MB before Local, and 62856/65536 MB after Local. AICore readings were 13%, 11%, and 14%. Existing Python PID 816979 using 59487 MB remained present and was not modified. No reset occurred. Build, Correctness, Local, identity, and device snapshots are retained under `results/`.

The current window ends with V028. Do not create or run V029 in this window.

## V028 Profile Inventory and Deferred Command (2026-10-12)

PROFILE_INVENTORY: No PipeUtilization CSV, AICore database, or scalar-instruction listing bound to Candidate source SHA-256 `0e0f0a7bbf6417404175818800d07b892702c563acde9710de9a9d82549949d6` was found in the local V028 results or the V028 server run directory. The saved compile output confirms ASC object compilation and Runner linking, but contains no readable device assembly.

SOURCE_IDENTITY: Parent V027 source commit `56b2940aa688637c0d1260caa8f1ac953ae7f1d4`, source SHA-256 `adb2c54017dc1ee120ddb8037939e44512eb63c02d4dd939932d3be2e17e7229`. Candidate V028 source commit `d6f9606dbc500e2fb0e1d17dd4603a729f3bec38`, source SHA-256 `0e0f0a7bbf6417404175818800d07b892702c563acde9710de9a9d82549949d6`. Paired Runner binary SHA-256 `9d19b61a51a0b2c68731aeae57ce0416fa511d4afef8da9b5c5b355fb08aecad`; remote executable: `/home/data4t2/lelinfeng/cann/server_runs/N05-SCALAR-ADDRESS-GENERATION-X-RESET-20261011/V028/0e0f0a7b-v027-parent/build/paired_runner_v001`.

PROFILE_INPUTS: CANN 8.5.0.alpha002, Ascend910B3, device 2. Existing `--paired` Runner executes FP16 rows=16 width=12288 and 32768, then BF16 rows=16 width=12288 and 32768. Each case has 45 warmups per binary and 21 interleaved Parent/Candidate pairs. Runner labels are V011 (actual source V027) and V001 (actual source V028).

RESOURCE_SNAPSHOT: Read-only query found 6.5 GB available on `/home/data4t2`, reported as 100% used. Device 2 reported HBM 63413/65536 MB (96%), AICore 17%, AIVector 14%, and existing Python PID 3954159 using 60042 MB. Other device processes were also present. No profiling process was started and no profile output was written.

PROFILE_METHOD: The project `ops-profiling` standard wrapper runs seven `aic-metrics` groups plus sample-based collection. For a single PipeUtilization pass, use the existing CANN `msprof` invocation pattern below. `PipeUtilization` provides `ai*_scalar_time(us)` and `ai*_scalar_ratio`; the local field guide treats a scalar ratio above 30% as SCALAR Bound. This is aggregate scalar utilization, not an instruction listing or instruction count. Attribution is usable for V028 only if the collected rows retain separate Parent/Candidate kernel entries and shape metadata; otherwise the result cannot establish a Candidate-specific scalar bottleneck.

DEFERRED_COMMAND (not run):

```bash
set -euo pipefail
N05_RUN_ROOT=/home/data4t2/lelinfeng/cann/server_runs/N05-SCALAR-ADDRESS-GENERATION-X-RESET-20261011/V028/0e0f0a7b-v027-parent
. /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/bin/setenv.bash
export LD_PRELOAD=/usr/lib/aarch64-linux-gnu/libstdc++.so.6
export LD_LIBRARY_PATH=/usr/lib/aarch64-linux-gnu:/usr/lib/gcc/aarch64-linux-gnu/11:/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/opp/lib64:/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/tools/ncs/lib64:/usr/local/Ascend/ascend-toolkit/latest/toolkit/tools/adump/lib64/:/usr/local/Ascend/ascend-toolkit/latest/toolkit/tools/profiler/lib64:/usr/local/Ascend/ascend-toolkit/latest/toolkit/tools/profiler/lib64/plugin:/usr/local/Ascend/ascend-toolkit/latest/toolkit/tools/aml/lib64:/usr/local/Ascend/ascend-toolkit/latest/toolkit/tools/aml/lib64/plugin:/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/hccl/lib64:/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/hccl/lib64/plugin/opskernel:/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/hccl/lib64/plugin/nnengine:/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/compiler/lib64:/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/compiler/lib64/plugin/opskernel:/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/compiler/lib64/plugin/nnengine:/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/runtime/lib64:/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/compiler/lib64/stub:/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/hccl/lib64/stub
msprof --output="${N05_RUN_ROOT}/profile-pipeutilization-v028" \
  --ai-core=on --aic-metrics=PipeUtilization --task-time=on --ascendcl=on \
  "${N05_RUN_ROOT}/build/paired_runner_v001" --paired
```

SCALAR_EVIDENCE: NONE. Do not form a scalar-specific optimization hypothesis from the V028 timing spread alone. Revisit only after a source-attributed PipeUtilization result is available.

## Resume Location and Local Evidence (2026-10-13)

LOCAL_WORKTREE: `/Users/sunyiyang/Desktop/Project/cann/worktrees/n05-scalar-address-generation`

BRANCH: `route/n05-scalar-address-generation-x-20261011`; source HEAD before this HANDOFF-only update: `004980c0809c10f977eb98f8e68e033b1a7efa3b`.

REMOTE_INPUT: `/home/data4t2/lelinfeng/cann/server_runs/N05-SCALAR-ADDRESS-GENERATION-X-RESET-20261011/V028/0e0f0a7b-v027-parent`; executable: `build/paired_runner_v001`. Planned output, not created: `profile-pipeutilization-v028` under that run directory. The saved command uses the existing `ops-profiling` CANN `msprof` invocation pattern and collects only `PipeUtilization` over the existing paired Parent/Candidate Runner.

LOCAL_EVIDENCE: Existing V028 paired Local data; 21 pairs per case. Candidate-minus-Parent device-time deltas are:

| Shape | Parent median (us) | Candidate median (us) | Paired delta median (us) | Delta min..max (us) |
|---|---:|---:|---:|---:|
| FP16, rows=16, D=12288 | 18.280 | 12.780 | -1.860 | -19.900..16.220 |
| FP16, rows=16, D=32768 | 19.280 | 20.260 | +0.900 | -20.680..17.480 |
| BF16, rows=16, D=12288 | 13.380 | 12.880 | +0.100 | -21.820..18.900 |
| BF16, rows=16, D=32768 | 30.200 | 30.200 | +0.640 | -52.200..6.020 |

LOCAL_SCORE=NONE; LOCAL_DELTA=NONE. All four paired-delta ranges span zero; directions are mixed and variation is broad. These results do not establish a stable gain.

RESUME_STATE: Ten-hour window ended. Device 2 has shared work; do not launch profiling or another device run. Candidate source remains V028 and was not changed. No V029 was created. The next evidence step remains the deferred PipeUtilization command above; no scalar bottleneck evidence is available yet, so no new optimization hypothesis is formed.

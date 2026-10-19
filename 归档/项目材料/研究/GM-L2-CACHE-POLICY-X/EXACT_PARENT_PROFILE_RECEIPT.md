# EXACT_PARENT_PROFILE_RECEIPT

ROUTE=W5-R04 GM-L2-CACHE-POLICY-X
AGENT_ID=W5-R04-GM-L2-CACHE-POLICY-X
WORKTREE=/home/data4t2/lelinfeng/cann-w5-r04-l2
BRANCH=research/w5-r04-gm-l2-policy
MODE=TRACK-B_RESEARCH_ONLY
CURRENT_STAGE=TARGET_L2_BOTTLENECK_CHECK
CURRENT_PARENT=FROZEN_R31B_V011
PARENT_SOURCE_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
CANDIDATE_EDIT=NO
ONLINE=NO

## Existing exact-parent application

The existing application was found locally; no runner or execution chain was
created. Its parent source and source identity are:

```text
APP=/home/data4t2/lelinfeng/cann/server_runs/W4-R02/V002/build/r02_runner
APP_ARCH=aarch64
APP_SHA256=5952a7e97db1f28b2b06e6050a1fe879cfb2f4ea1c3ed6bf2fb1337bfe2cd8fc
PARENT_LIBRARY=/home/data4t2/lelinfeng/cann/server_runs/W4-R02/V002/build/libr02_parent.so
PARENT_LIBRARY_SHA256=b4947bfd9ef5477ee66729c05f5231b78239718e354d3b3ff48881b0e62c469a
PARENT_SOURCE=/home/data4t2/lelinfeng/cann/server_runs/W4-R02/V002/parent.asc
PARENT_SOURCE_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
```

The existing runner was invoked in its pre-existing `paired-parent` mode:

```text
r02_runner 3 20480 fp16 paired-parent - 45 21 4 <absolute-event.tsv>
```

The event headers identify `SLOT_P_FUNCTION=run_kernel_parent` and
`SLOT_C_FUNCTION=run_kernel_parent`; the linked Candidate slot was not
executed. The W4-R02 source tree and archive were not modified.

## Collection

The installed native `msprof` accepts one metric group per invocation. The
combined `Memory,L2Cache` attempt was retained as a toolchain rejection, then
the same existing parent application was profiled independently:

```text
msprof --ai-core=on --aic-metrics=Memory --task-time=on --ascendcl=on --runtime-api=on --aicpu=off --application="<APP> 3 20480 fp16 paired-parent - 45 21 4 <absolute-event.tsv>"
msprof --ai-core=on --aic-metrics=L2Cache --task-time=on --ascendcl=on --runtime-api=on --aicpu=off --application="<APP> 3 20480 fp16 paired-parent - 45 21 4 <absolute-event.tsv>"
```

```text
DEVICE=3
PARENT_CORRECTNESS=PASS
MEMORY_PROFILE_RC=0
L2CACHE_PROFILE_RC=0
PROFILE_ROWS_EACH=258
WARMUP_ROWS=90
MEASURED_ROWS=168
```

Raw native profile outputs remain under:

```text
研究/GM-L2-CACHE-POLICY-X/profile-existing-parent-memory-l2-20261009/
```

The committed flat evidence files are the corresponding `op_summary` CSVs,
parent-only event TSVs, and collection logs in this route directory.

## L2 observation

The `L2Cache` `op_summary` contains 258 `AI_VECTOR_CORE` rows. After the
runner's 90 warmup rows, the 168 measured rows report:

```text
AIV_R0_READ_HIT_RATE=0.999984989
AIV_R1_READ_HIT_RATE=1.000000000
AIV_WRITE_HIT_RATE=1.000000000
AIV_R0_READ_MISS_ALLOCATE=168
AIV_R1_READ_MISS_ALLOCATE=0
AIV_WRITE_MISS_ALLOCATE=0
```

The cold-start warmup rows contain the expected initial misses, but the
steady-state parent traffic is effectively L2-hit resident. This is not an
actionable steady-state L2 miss bottleneck.

## Memory observation

The `Memory` `op_summary` for the same 168 measured rows reports these
per-task averages:

```text
TASK_DURATION_AVG_US=21.084900
AIV_TIME_AVG_US=16.338000
AIV_MAIN_MEM_READ_BW_AVG_GBPS=0.380804
AIV_MAIN_MEM_WRITE_BW_AVG_GBPS=0.117161
AIV_L2_READ_BW_AVG_GBPS=0
AIV_L2_WRITE_BW_AVG_GBPS=0
AIV_UB_READ_BW_AVG_GBPS=74.7635
AIV_UB_WRITE_BW_AVG_GBPS=63.5531
```

These observations do not establish L2 as the critical bottleneck. They show
high steady-state cache hit rates, no measured L2 bandwidth signal in the
Memory group, and substantially higher UB traffic.

## Gate decision

```text
CAPABILITY=PROVEN_BY_PRIOR_COMPILE_RECEIPT
L2_OBSERVATION=PROFILED
TARGET_L2_BOTTLENECK=NOT_OBSERVED
OFAT_PROPOSAL=NOT_AUTHORIZED
HYPOTHESIS_STATUS=FEASIBILITY_UNPROVEN
BLOCKER=TARGET_L2_BOTTLENECK_NOT_OBSERVED
```

Do not edit the Candidate, change arithmetic/dispatch/UB/DMA/tile/core
ownership/parameter residency/synchronization, or submit Online. Report this
negative target-bottleneck result to Planning/Review; revisit only if an
already-existing exact-parent profile for a separately authorized target
shape supplies a different actionable L2 signal.

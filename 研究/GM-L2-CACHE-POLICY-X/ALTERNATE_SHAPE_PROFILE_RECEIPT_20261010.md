# ALTERNATE_SHAPE_PROFILE_RECEIPT_20261010

```text
ROUTE=W5-R04 GM-L2-CACHE-POLICY-X
AGENT_ID=W5-R04-GM-L2-CACHE-POLICY-X
WORKTREE=/home/data4t2/lelinfeng/cann-w5-r04-l2
BRANCH=research/w5-r04-gm-l2-policy
MODE=TRACK-B_RESEARCH_ONLY
STAGE=TARGET_L2_BOTTLENECK_CHECK
REVISION=NONE (PARENT-ONLY PROFILE; NO CANDIDATE)
CURRENT_PARENT=FROZEN_R31B_V011
PARENT_SOURCE_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
TARGET_SHAPE=rows128,width32768,dtype=fp16
DEVICE=3
TARGET_L2_BOTTLENECK=NOT_OBSERVED
OFAT_PROPOSAL=NOT_AUTHORIZED
HYPOTHESIS_STATUS=FEASIBILITY_UNPROVEN
BLOCKER=TARGET_L2_BOTTLENECK_NOT_OBSERVED
```

## Shape and parent identity

The route's committed exact-parent profile only covers `128x20480 fp16`. The
parent source bounds this operator's width at 32768, so this single alternate
shape uses its maximum supported width to raise the estimated active tensor
footprint from 15,810,560 bytes to 25,296,896 bytes (1.60x, including x,
residual, output, gamma and bias). This is a parent-only observation, not a
performance Candidate or a cache-policy test.

The already-existing application and linked parent library matched the
identities in the earlier receipt before and after collection:

```text
APP=/home/data4t2/lelinfeng/cann/server_runs/W4-R02/V002/build/r02_runner
APP_SHA256=5952a7e97db1f28b2b06e6050a1fe879cfb2f4ea1c3ed6bf2fb1337bfe2cd8fc
PARENT_LIBRARY=/home/data4t2/lelinfeng/cann/server_runs/W4-R02/V002/build/libr02_parent.so
PARENT_LIBRARY_SHA256=b4947bfd9ef5477ee66729c05f5231b78239718e354d3b3ff48881b0e62c469a
PARENT_SOURCE_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
```

No source was copied or modified; no Runner, Candidate, or Revision was
created. All new profile output is in this route's worktree.

## Collection

Native `msprof` accepts one `--aic-metrics` group per invocation. The one
shape-level profile package therefore contains one `Memory` collection and one
`L2Cache` collection, each using the existing parent's parent-only mode:

```text
msprof --output=<route-profile>/msprof-memory-output --ai-core=on --aic-metrics=Memory --task-time=on --ascendcl=on --runtime-api=on --aicpu=off --application="<existing-r02-runner> 3 32768 fp16 paired-parent - 45 21 4 <absolute-memory-event.tsv>"
msprof --output=<route-profile>/msprof-l2cache-output --ai-core=on --aic-metrics=L2Cache --task-time=on --ascendcl=on --runtime-api=on --aicpu=off --application="<existing-r02-runner> 3 32768 fp16 paired-parent - 45 21 4 <absolute-l2cache-event.tsv>"
```

Both returned `PROFILE_RC=0`. The existing harness reported parent correctness
`PASS` for `rows=128,width=32768,dtype=fp16`, with max absolute error
`0.001953125`, zero mismatches and zero nonfinite values. Each metric output
contains 258 op rows: 90 warmup and 168 measured. Each event TSV has 179 lines.

Raw outputs, event TSVs and command logs are retained under:

```text
研究/GM-L2-CACHE-POLICY-X/profile-existing-parent-memory-l2-20261010-128x32768-fp16/
```

The raw CSV SHA256 values are:

```text
Memory op_summary: 80a6c7dbb669b2f4dbf74592f513997a95db6397f0a3b053677edf19dd4de50b
L2Cache op_summary: 35d8e598c19a8828f71beb0ba607643efb34a3068edf5db1017f6fd1e9aa7869
Memory event TSV: 99ecc7757a011ec382dbc9469c9267a6b030846f37299356a5ebc8d17499ae7f
L2Cache event TSV: 538d076636bad1d5c913b20db6ef7450da0e0ee90b52a62598b9d198e242a4df
```

## Target bottleneck result

The 168 measured rows report:

```text
L2Cache R0 read hit rate=0.999999546; miss allocations=10
L2Cache R1 read hit rate=0.999992828; miss allocations=158
L2Cache write hit rate=1.000000000; miss allocations=0
Memory task duration average=33.310262 us
Memory AIV time average=26.158869 us
Main-memory read/write bandwidth average=0.467589/0.116988 GB/s
L2 read/write bandwidth average=0/0 GB/s
UB read/write bandwidth average=74.704690/67.900708 GB/s
```

The wider shape changes the raw miss counts slightly, but hit rates remain
effectively 100%; the Memory group still reports zero L2 bandwidth and much
higher UB traffic. This does not establish an actionable L2 miss/bandwidth
bottleneck and does not authorize an OFAT.

## Load and execution notes

Device 3 pre-profile HBM usage was 3428/65536 MB (5%), AICore/AIVector usage
was 0%, and the device process table reported no process on NPU 3. Between the
two metric collections and after the second collection, HBM remained 5%,
AICore/AIVector remained 0%, and no NPU-3 process was reported. The initial
whole-device snapshot showed external processes on other NPUs (VLLM/Python);
they were recorded only and not modified. Disk availability before collection
was 95 GB. The lease log was not modified; lease metadata was not used as a
stop condition.

One wrapper setup attempt exited 127 before creating this profile directory or
starting the runner because it combined shell `set -u` with CANN environment
loading. The directory was confirmed absent and NPU 3 had no runner process
after that attempt. The successful collections used the existing environment
loading method; no samples from the failed setup attempt exist.

## Decision

```text
BUILD=NOT_RUN_THIS_TURN (existing app/library identities verified)
CORRECTNESS=PASS_PARENT_ONLY (as reported by the existing profiling harness)
LOCAL_CANDIDATE_RESULT=NONE
PROFILE=PASS; NEW_DIFFERENT_SHAPE_DATA=YES
ACTIONABLE_L2_BOTTLENECK=NO
TARGET_L2_BOTTLENECK=NOT_OBSERVED
OFAT_PROPOSAL=NOT_AUTHORIZED
NEXT_ACTION=HOLD; wait for a separately authorized target with new attributable L2 evidence
```

# CACHE_CAPABILITY_RECEIPT

ROUTE=W5-R04 GM-L2-CACHE-POLICY-X
AGENT_ID=W5-R04-GM-L2-CACHE-POLICY-X
WORKTREE=/home/data4t2/lelinfeng/cann-w5-r04-l2
BRANCH=research/w5-r04-gm-l2-policy
MODE=TRACK-B_RESEARCH_ONLY
CURRENT_STAGE=CAPABILITY_GATE
CURRENT_PARENT=FROZEN_R31B_V011
PARENT_SOURCE=归档/历史工作区/R31B/R31B-V011-LP-ROW-PIPELINE_kernel.asc
PARENT_SOURCE_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
ONLINE=NO
CANDIDATE_EDIT=NO

## Environment

- The real local device query reports eight `Ascend 910B3` devices; `npu-smi` is `25.0.rc1.1`.
- After sourcing `/usr/local/Ascend/ascend-toolkit/set_env.sh`, `check_env.sh` passes with `ASCEND_HOME_PATH=/usr/local/Ascend/ascend-toolkit/latest` and CANN `8.5.T8.0.B060` (`version_dir=8.5.0.alpha002`).
- Bisheng is the installed CCE compiler, `clang version 15.0.5`; `npu-smi info -t common -i 2` reports `Aicore Count=20` and `HBM Capacity=65536 MB`.
- The same sourced environment reports runtime NPU architecture `2201`, matching the Ascend910B/A2 mapping and the compile target `dav-2201`.

## Supported Capability Evidence

1. The local Ascend C cache header declares `DataCachePreload` at `/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/compiler/asc/include/basic_api/kernel_operator_cache_intf.h:23-24`. The local SDK document `DataCachePreload.md:11-12,28-56` lists Atlas A2 / Ascend910B support, says the operation loads one GM Cache Line into DCache, and limits consecutive calls to four or fewer.
2. The local `GlobalTensor` header declares `SetL2CacheHint` at `/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/compiler/asc/include/basic_api/kernel_tensor.h:239-240`. Its implementation at `compiler/asc/impl/basic_api/kernel_tensor_impl.h:1489-1498` changes the GM address hint for non-normal modes. `kernel_utils_macros.h:217-242` encodes the `CACHE_MODE_DISABLE` and `CACHE_MODE_NORMAL` forms used by the `dav-2201` path.
3. The local SDK document `SetL2CacheHint.md:11-13,28-65` lists Atlas A2 / Ascend910B support and documents `CACHE_MODE_DISABLE` and `CACHE_MODE_NORMAL`. It says persistent residency is not yet supported on this product family, so `CACHE_MODE_PERSISTENT` is not a legal proposal for this route.
4. The local cache overview `system_cache_overview.md:17,28-37,102-120` distinguishes the relevant paths: MTE2 GM reads use L2; Scalar GM reads use DCache then L2; `DataCachePreload` is a DCache operation. Therefore `DataCachePreload` is not evidence of an MTE2/L2 prefetch policy, while `SetL2CacheHint` is the supported L2 policy surface for `GlobalTensor`/`DataCopy`.

## Exact Direct-Invocation Compile Probe

`cache_capability_probe.asc` includes the unchanged frozen R31B-V011 direct-invocation source and, in the same translation unit, instantiates `GlobalTensor<T>::SetL2CacheHint` and `DataCachePreload` from an `__global__ __vector__` entry point. It does not edit, launch, or replace the parent kernel.

```text
PROBE_SOURCE_SHA256=3ac2f3ef04335abe9808c1a8ee586080c94137b3b80e47ed4be4a43038d9bda9
COMMAND_TARGET=--npu-arch=dav-2201 --npu-soc=Ascend910B3 --cce-aicore-arch=dav-c220
PROBE_RESULT=PASS
PROBE_RC=0
PROBE_OBJECT=/tmp/tmp.KUnY5vBYo7/cache_capability_probe.o
PROBE_OBJECT_SHA256=d9cf84617ca1a03235ff89fa437da5068b4c8ab3726615d0e8164fe8e893926a
PROBE_OBJECT_BYTES=142816
```

The two earlier probe failures were observed during this turn: first a missing dependent-template `template` keyword in the probe, then a missing C++ standard-library include path. After those probe-only fixes, Bisheng compiled the exact parent translation unit plus both cache APIs successfully.

## L2 Observation Evidence

- The installed `msprof op --help` exposes `--aic-metrics=...L2Cache...`; the command also accepts `--kernel-name` for an application profile.
- The local SDK guide `collect_performance_data.md:5-18` documents `msopprof --output=<dir> <application>`, and `performance_tuning.md:57-67` defines `L2Cache.csv` as the L2 hit-rate result and `Memory.csv` as L2/GM bandwidth data.
- A repository search over `本地实验/`, `线上结果/`, and `归档/` found zero existing `L2Cache.csv`/L2 profiling artifacts for this parent. No exact-parent application is present in this route worktree, and creating a runner or execution chain is outside this receipt's scope.

## Exact-Parent Bottleneck Check

The parent initializes `xGm_`, `residualGm_`, `gammaGm_`, `biasGm_`, and `outputGm_` in `R31B-V011-LP-ROW-PIPELINE_kernel.asc:46-54`. Its shared `Load` and `Store` helpers use `DataCopyPad` at `:3367-3383`; the source contains no `SetL2CacheHint`, `DataCachePreload`, `DataCacheCleanAndInvalid`, or `ICachePreLoad` call. This makes the L2 policy surface technically applicable, but it does not prove that L2 misses are on the critical path.

```text
SET_L2_CACHE_HINT=SUPPORTED_AND_COMPILE_VERIFIED
DATA_CACHE_PRELOAD=SUPPORTED_AND_COMPILE_VERIFIED_BUT_DCACHE_ONLY
L2_OBSERVATION=SUPPORTED_BY_INSTALLED_MSOPPROF
TARGET_L2_BOTTLENECK=UNPROVEN_NO_TARGET_PROFILE
OFAT_PROPOSAL=NOT_AUTHORIZED
HYPOTHESIS_STATUS=FEASIBILITY_UNPROVEN
```

## Gate Decision

Capability is proven for a legal `SetL2CacheHint` policy experiment on the Ascend910B3 direct-invocation toolchain. The target bottleneck is not proven because no exact-parent `L2Cache.csv`/`Memory.csv` profile is available and no runner may be created in this research-only turn. Do not edit the Candidate or propose an OFAT until an existing exact-parent application/profile shows an actionable L2 miss or bandwidth bottleneck.

NEXT_ACTION=Obtain or run an already-existing exact-parent direct-invocation application under `msopprof --aic-metrics=L2Cache,Memory`, preserving the parent source and all execution structure; then reassess the bottleneck gate.
BLOCKER=EXACT_PARENT_PROFILE_MISSING; no capability blocker.

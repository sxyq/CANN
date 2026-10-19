# PARAM-RESIDENCY-EPILOGUE-X V009

ROUTE=PARAM-RESIDENCY-EPILOGUE-X
REVISION=V009
PARENT=V006
REFERENCE=R31B-V011
BRANCH=route/new-n05-param-residency-epilogue-x-20261016
CANDIDATE_SOURCE=本地实验/PARAM-RESIDENCY-EPILOGUE-X/V009/R31B-V016-WIDE-TILE-SEED-V009_kernel.asc
RUNNER=本地实验/R31B/V016/support/paired/paired_runner_v016
BUILD_TARGET=paired_runner_v016
PUSH=NO
OFFICIAL=NOT_RUN

## HYPOTHESIS

`ProcessWideLowPrecision` Pass 1 walks `units = batchRows * tileCount`. For each unit, it derives `batchRow` from `u / tileCount`, derives `tile` by multiply/subtract, and multiplies `tile` and `batchBegin + batchRow` to form the current column and row offset. The prefetched unit repeats the same quotient/reconstruction and row-offset calculation. `tileCount` is derived from runtime `rowWidth`.

V009 keeps the existing row-major DMA order, A/B ping-pong choice, event sequence, valid counts, and Vector work. It advances `batchRow`, `tile`, `col`, and `rowOffset` with the stream, including the next-load address. This targets repeated scalar address arithmetic without changing the epilogue or MTE3 schedule.

Source evidence: V006 lines 3125–3150 contain the current and prefetched `u / tileCount` coordinate derivations; V009 lines 3122–3151 use the advancing cursor. V001–V008 contain no cursor version in this Pass 1 loop.

HISTORY_SCOPE=Compared with V001–V008 in this worktree. N05 V028 metadata points to source in another worktree; that source was not read under the assigned-directory scope, so no claim is made about its implementation.

V008's block-count cap applies to low-precision widths above 8,192 through 14,320. D=32,768 does not enter that branch, so its timing delta does not evidence that change. V009 starts from V006 and excludes that launch change; it also excludes V007's deferred Store drain. D=12,288's V008 backslide therefore is not inherited by this candidate.

## CHANGE

- Added V009 source from V006 and changed only Pass 1 scalar coordinate generation in `ProcessWideLowPrecision`.
- Updated the existing paired Runner include and its existing CMake candidate directory to V009.
- Reused `paired_runner_v016`; no new Runner or script was added.

## COMPILE

HOST=cann-server3 (`hwnput3`)
TOOLCHAIN=CANN 8.5.0.alpha002
SOC_TARGET=Ascend910B3 / DAV-C220
REMOTE_TEMP=/tmp/n05-param-residency-epilogue-v009.biWkFE
CONFIGURE_RETURN_CODE=0
BUILD_TARGET=paired_runner_v016
BUILD_RETURN_CODE=0
REMOTE_BINARY=/tmp/n05-param-residency-epilogue-v009.biWkFE/build/paired_runner_v016
REMOTE_BINARY_BYTES=582216
DEVICE_OBJECT_BYTES=271488
CONFIGURE_WARNING=Existing CMake runtime search-path warning involving libstdc++.so.6; configure and build completed.

Evidence: `remote/compile-config.log`, `remote/compile-build.log`. The compiled Runner remains at `REMOTE_BINARY` for the requested follow-up device entry.

## CORRECTNESS AND LOCAL

HOST=cann-server3 (hwnput3)
TOOLCHAIN=CANN 8.5.0.alpha002
SOC_TARGET=Ascend910B3 / DAV-C220
DEVICE_ID=7
LEASE=N05-V009-D7-CORR-LOCAL-20261016T130447Z
PRE_SNAPSHOT_UTC=2026-10-16T13:14:05Z
PRE_STATE=Health:OK; HBM usage rate:5% of 65536 MiB; AICore:0%; no NPU7 process
RUN_COMMAND=`source /usr/local/Ascend/ascend-toolkit/set_env.sh; export LD_LIBRARY_PATH=/usr/lib/aarch64-linux-gnu:${LD_LIBRARY_PATH-}; /tmp/n05-param-residency-epilogue-v009.biWkFE/build/paired_runner_v016 --paired 7`
RUNNER_RC=0
POST_SNAPSHOT_UTC=2026-10-16T13:15:45Z
POST_STATE=Health:OK; HBM usage rate:5% of 65536 MiB; AICore:0%; no NPU7 process

CORRECTNESS=PASS 8/8 (Parent and Candidate passed all four cases)
LOCAL=COMPLETE (4 cases x 21 paired samples; warmup_each=45; device-event timing primary)
LOCAL_SCORE=NONE
OFFICIAL=NOT_RUN

| Case | V011 median (us) | V016 median (us) | Paired delta median (us) | Paired delta P10 (us) | Paired delta P90 (us) |
|---|---:|---:|---:|---:|---:|
| fp16-tail-d12288 | 16.400 | 16.880 | +1.620 | -2.720 | +3.220 |
| fp16-wide-d32768 | 22.080 | 21.760 | -0.320 | -1.180 | +0.060 |
| bf16-tail-d12288 | 18.120 | 18.560 | +0.680 | -6.240 | +5.900 |
| bf16-wide-d32768 | 21.960 | 21.040 | -0.500 | -7.700 | +2.980 |

Paired delta is V016 minus V011; negative favors V016. All four paired-delta P10–P90 intervals cross zero. The two D=12,288 medians are positive and the two D=32,768 medians are negative, so the results are mixed and do not establish a stable Local gain. The fp16-tail case also has large timing outliers (V011 max 35.660 us; V016 max 69.020 us). Keep LOCAL_SCORE=NONE.

The first pre-run snapshot request was malformed and returned 215 before Runner launch; its raw output is retained. The corrected 13:10:51Z and fresh 13:14:05Z samples showed NPU7 healthy and idle. The post-run sample at 13:15:45Z also showed Health OK, 5% HBM usage rate, 0% AICore use, and no NPU7 process. These `npu-smi` samples report HBM usage as a rounded percentage; they do not provide an exact used-MiB value in this output.

Evidence: `remote/pre-npu7-corr-local-snapshot.log`, `remote/pre-npu7-corr-local-snapshot-valid.log`, `remote/pre-npu7-corr-local-snapshot-rerun.log`, `remote/paired-npu7-v009-20261016T131405Z.log`, `remote/paired-npu7-v009-20261016T131405Z.rc`, `remote/post-npu7-corr-local-snapshot-20261016T131405Z.log`. The Runner raw output contains all Parent/Candidate correctness results, 21 interleaved pairs per case, and the per-case summaries.

## COMMIT

SOURCE_COMMIT=2e7cc0a8
PUSH=NO

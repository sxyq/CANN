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

## DEVICE EXECUTION

CORRECTNESS=NOT_RUN_BY_REQUEST
LOCAL_SCORE=NONE
DEVICE_EXECUTION=NOT_RUN_BY_REQUEST
OFFICIAL=NOT_RUN

FOLLOWUP_DEVICE_ENTRY=`source /usr/local/Ascend/ascend-toolkit/set_env.sh; export LD_LIBRARY_PATH=/usr/lib/aarch64-linux-gnu:${LD_LIBRARY_PATH-}; /tmp/n05-param-residency-epilogue-v009.biWkFE/build/paired_runner_v016 --paired 3`

The follow-up `--paired 3` entry runs the existing Parent/Candidate correctness cases and paired local timing. It was not launched in this task.

## COMMIT

SOURCE_COMMIT=2e7cc0a8
PUSH=NO

# PARAM-RESIDENCY-EPILOGUE-X V012

ROUTE=PARAM-RESIDENCY-EPILOGUE-X
REVISION=V012
PARENT=V009
REFERENCE=R31B-V011
BRANCH=route/new-n05-param-residency-epilogue-x-20261016
CANDIDATE_SOURCE=本地实验/PARAM-RESIDENCY-EPILOGUE-X/V012/R31B-V016-WIDE-TILE-SEED-V012_kernel.asc
RUNNER=本地实验/R31B/V016/support/paired/paired_runner_v016
BUILD_TARGET=paired_runner_v016
SOURCE_COMMIT=d7bf82c4
PUSH=NO
OFFICIAL=NOT_RUN

## HYPOTHESIS

V009's wide low-precision Pass 2 loads the next Gamma/Bias tile into the
alternate parameter slot before applying the current tile. The existing
`prel0`/`prel1` release event is emitted once per completed parameter tile,
after all retained rows have consumed that slot. The extra global
`SyncVToMTE2()` immediately before every output store does not protect the
current parameter slot and can be removed while keeping the per-slot release
event, the parameter load order, the output path, and the arithmetic order.

V012 starts from V009 and does not contain V011's two-output-slot FP16 writeback
overlap. This keeps the tested change separate from the N04-overlapping V011
mechanism.

## CHANGE

- Removed only the per-row global `SyncVToMTE2()` immediately before
  `SyncVToMTE3()` in `ProcessWideLowPrecision` Pass 2.
- Kept the existing parameter double-buffer, `prd0`/`prd1` readiness events,
  `prel0`/`prel1` slot release events, MTE3 writeback, row/tile traversal,
  arithmetic, and buffer sizing unchanged.
- Reused the existing `paired_runner_v016`, CMake entry, Runner source, and
  ABI. No new Runner or script was added.

## COMPILE

REMOTE_HOST=cann-server3
REMOTE_HOSTNAME=hwnput3
REMOTE_WORKDIR=/home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N05/V012
TOOLCHAIN=CANN 8.5.0.alpha002
SOC=Ascend910B3
NPU_ARCH=dav-2201
BUILD_TARGET=paired_runner_v016
CONFIGURE_RC=0
COMPILE_RC=0
COMPILE_WARNING=Existing CMake runtime search-path warning involving libstdc++.so.6; Configure and Compile completed.
REMOTE_BINARY=/home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N05/V012/build/paired_runner_v016
REMOTE_BINARY_BYTES=582208

The first Compile attempt returned 2 because the uploaded Candidate had been
renamed to `submission.asc` while the existing Runner included the V012 source
filename. The file was renamed in place to the existing include name, and the
same Configure/Compile command then returned 0. Both attempts remain in the
remote logs.

Evidence: `remote/configure-v012-final.log`,
`remote/compile-v012-final.log`, `remote/compile-v012-final.rc`,
`remote/source-binary-identities-final.txt`.

## CORRECTNESS AND LOCAL

DEVICE_ID=3
RUNNER_COMMAND=`/home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N05/V012/build/paired_runner_v016 --paired 3`
RUNNER_RC=0
METHOD=`existing paired_runner_v016; warmup_each=45; pairs_each=21; device-event timing primary; wall time diagnostic`
CORRECTNESS=PASS_8_OF_8
LOCAL=MEASURED
LOCAL_SCORE=NONE
ONLINE_SUBMIT=NOT_RUN
ONLINE_SUBMIT_OWNER=N06

The existing paired Runner labels the fixed reference binary as `V011` and
the Candidate binary as `V016`; the Candidate source for this run is V012.

| Case | Reference median (us) | V012 median (us) | Paired delta V012-reference (us) | P10 (us) | P90 (us) |
|---|---:|---:|---:|---:|---:|
| fp16-tail-d12288 | 10.640 | 15.760 | -2.860 | -11.380 | 10.440 |
| fp16-wide-d32768 | 16.260 | 17.620 | 4.260 | -5.020 | 9.040 |
| bf16-tail-d12288 | 17.300 | 17.740 | -0.300 | -9.500 | 9.880 |
| bf16-wide-d32768 | 18.560 | 16.580 | -2.400 | -8.060 | 7.780 |

All four cases passed Parent and Candidate correctness. All four paired P10-P90
intervals cross zero. FP16 wide regressed in the central median, BF16 wide was
lower but remained noisy, and the two tail cases were mixed. V012 therefore has
no stable Local gain and is not an Official submission candidate.

## DEVICE AND EVIDENCE

PRE_HBM_USAGE=5% of 65536 MiB
POST_HBM_USAGE=5% of 65536 MiB
PRE_AICORE=0%
POST_AICORE=0%
PRE_AIVECTOR=0%
POST_AIVECTOR=0%
PRE_AICPU=0%
POST_AICPU=0%
OTHER_PROCESS_PRESENT=System process snapshots showed other users' VLLM and npu-smi processes; they were not stopped or modified.
PROCESS_QUERY_NOTE=The attempted `npu-smi info -t proc -i 3` form was rejected by this npu-smi version; the raw error is retained in the pre/post snapshot logs.

Evidence: `remote/paired-v012.stdout.log`,
`remote/paired-v012.stderr.log`, `remote/paired-v012.rc`,
`remote/pre-run-device.txt`, `remote/post-run-device.txt`.

## RESULT

STATUS=COMPILE_PASS_CORRECTNESS_PASS_LOCAL_NONE_OFFICIAL_NOT_RUN
INTERPRETATION=V012 is an independent parameter-slot synchronization change relative to V011's overlapping double-output-slot mechanism. It compiled and preserved correctness, but the single paired Local run is mixed with all central intervals crossing zero; no performance gain is claimed.

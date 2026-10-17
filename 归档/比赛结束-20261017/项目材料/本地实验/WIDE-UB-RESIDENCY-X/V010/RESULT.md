# WIDE-UB-RESIDENCY-X V010

RESEARCH_TRACK: N02 CODE-FOOTPRINT-ICACHE-X
REVISION: V010
PARENT: `本地实验/WIDE-UB-RESIDENCY-X/V007/submission.asc`
PARENT_SOURCE_COMMIT: `bd2e5f7f7c8f475f8057c16211f52959d3b7a33d`
BRANCH: `route/new-n02-wide-ub-residency-x-20261016`

## HYPOTHESIS

The four existing paired cases use widths 12288 and 32768. `Init()` selects
`widePath_` when `rowWidth > kCacheElems` (8192), and `Process()` returns from
the wide dispatch before reaching the mid-size branch. `ProcessNarrowMidOverlap()`
is called only for `128 < rowWidth <= kTileElems` (4096); its implementation
occupies about 120 source lines and is declared `__aicore__ inline`.

If the compiler currently inlines this cold mid-size helper into `Process()`,
outlining it may reduce the dispatch body and instruction-fetch competition for
the wide FP16/BF16 cases, without adding a helper call to those tested paths.
The helper call remains on mid-size rows. This tests cold-path code layout,
distinct from V009, which outlined the selected `ProcessWideLowPrecision()`
helper. The current source and build logs do not establish whether the compiler
inlined `ProcessNarrowMidOverlap()` before this change.

## ONE CHANGE

Add `__attribute__((noinline))` only to the existing
`ProcessNarrowMidOverlap()` declaration. Its body, dispatch conditions,
arithmetic, data movement, synchronization, buffers, tiles, and block assignment
remain unchanged. V010 starts from the V007 source; the failed V009 attribute
on `ProcessWideLowPrecision()` is not included. V008 remains uncompiled and
unrun.

## SOURCE EVIDENCE

- In V007 `submission.asc`, `widePath_` is set for `rowWidth > 8192` at lines
  60-61; `Process()` takes the wide helper and returns at lines 166-172.
- The call to `ProcessNarrowMidOverlap()` is guarded by
  `128 < rowWidth <= 4096` at lines 240-243. Its body is at lines 502-623.
- V007's paired cases are FP16/BF16 at D=12288 and D=32768; all exceed the
  wide-path threshold.
- V010's source difference from V007 is the function attribute on this helper.

## BUILD AND RUNNER

Reuse the existing `paired_runner_v016` target. Its candidate include and
`R31B_V016_SOURCE_DIR` now point to V010; the Parent remains R31B V011. No new
Runner or script was added.

COMPILE: PASS.
COMPILE_CONFIGURE_RETURN_CODE: 0
COMPILE_BUILD_RETURN_CODE: 0
COMPILE_LOG: `本地实验/WIDE-UB-RESIDENCY-X/V010/server-transaction-20261017/compile-rerun-20261016T184718Z-envfix.log`
COMPILE_ENV: CANN `/usr/local/Ascend/ascend-toolkit/set_env.sh`; CPLUS_INCLUDE_PATH recorded in the log.
CORRECTNESS: PASS; Parent and Candidate passed all 4/4 paired cases; runner return code 0.
LOCAL: COMPLETED; 4 cases, 45 warmups, 21 paired samples per case.
LOCAL_LOG: `本地实验/WIDE-UB-RESIDENCY-X/V010/server-transaction-20261017/paired-run-rerun-20261016T184718Z-envfix.log`
HOST: `hwnput3`
SOC_VERSION: `Ascend910B3`
DEVICE_ID: 3
PREFLIGHT_HBM_USAGE: 5%; `npu-smi info -t proc-mem -i 3` reported no process.
POSTFLIGHT_HBM_USAGE: 5%; `npu-smi info -t proc-mem -i 3` reported no process.
DEVICE_RESET: NO

## SERVER EVIDENCE

REMOTE_SOURCE_PATH: `/home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N02/WIDE-UB-RESIDENCY-X/server-txn-20261017/project/本地实验/WIDE-UB-RESIDENCY-X/V010/submission.asc`
CANDIDATE_COMMIT: `c28bb58531b771d247f78db10382eaff1c534273`
SOURCE_SHA256: `156ba3232809cac07dfcec92ffcc1b68456232a6ef9124ca57d5ad5d728ea8b0`
REMOTE_COMPILE_LOG: `/home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N02/WIDE-UB-RESIDENCY-X/server-txn-20261017/V010/compile-rerun-20261016T184718Z-envfix.log`
REMOTE_RUN_LOG: `/home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N02/WIDE-UB-RESIDENCY-X/server-txn-20261017/V010/paired-run-rerun-20261016T184718Z-envfix.log`

## LOCAL SUMMARY

| case | Parent median us | Candidate median us | paired delta median us | P10-P90 us |
|---|---:|---:|---:|---:|
| fp16-tail-d12288 | 7.980 | 7.820 | -0.100 | -2.380..1.440 |
| fp16-wide-d32768 | 11.560 | 11.880 | 0.480 | -0.460..10.900 |
| bf16-tail-d12288 | 12.580 | 7.760 | 0.000 | -10.900..8.460 |
| bf16-wide-d32768 | 17.080 | 22.200 | -1.400 | -9.480..8.080 |

LOCAL_SCORE: NONE
LOCAL_DELTA: NONE; all four P10-P90 intervals cross zero.
ONLINE_RECOMMENDATION: NONE; no stable improvement established.

## PRIOR SERVER ATTEMPT

The first rerun kept the same source and existing runner but omitted the recorded CPLUS_INCLUDE_PATH. Its build failed with `fatal error: 'vector' file not found`; the raw logs remain at:

`/home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N02/WIDE-UB-RESIDENCY-X/server-txn-20261017/V010/compile-rerun-20261016T184237Z.log`

PREVIOUS_N02_STATUS: V007 Compile and Correctness passed; its four Local
P10-P90 intervals crossed zero. V009 Compile passed, Candidate Correctness
failed in the first case, and its timing is not interpreted. V008 remains
uncompiled and unrun.

STATUS: SERVER_VALIDATED_LOCAL_NO_CLEAR_GAIN_ONLINE_WRONG_ANSWER

## Official 服务器事务结果

- 提交使用既有 `工具/cannjudge-submit.mjs`，结果文件：`线上结果/NEW-N02-WIDE-UB-RESIDENCY-X/V010/result.json`。
- 状态：`Wrong Answer`；通过 `9/15`；最大输出误差约 `100%`；平台未列 Official score；Submission ID：`6ad275ec193e507d8ec3087c`。
- 源码身份：本地 SHA256 `156ba3232809cac07dfcec92ffcc1b68456232a6ef9124ca57d5ad5d728ea8b0`，平台返回 `LOCAL_SHA_EQ_REMOTE_SHA`。
- 该线上服务器事务结果不替换本地四组 paired Correctness 事实，并保留为平台全量用例失败证据。
PUSH: NO

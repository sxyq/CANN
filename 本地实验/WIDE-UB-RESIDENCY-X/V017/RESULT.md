# WIDE-UB-RESIDENCY-X V017

ROUTE: WIDE-UB-RESIDENCY-X
RESEARCH_TRACK: N02 CODE-FOOTPRINT-ICACHE-X
REVISION: V017
PARENT_SOURCE: `本地实验/WIDE-UB-RESIDENCY-X/V011/submission.asc`
PARENT_SOURCE_COMMIT: `e591b3ffe1f83f1e47b169be7cb03cda7f77d999`
SOURCE_COMMIT: `c0192c1b1834a9e5f17c5e241d564c13929a698c`
PUSH: NO
SHARED_RECORD: UNCHANGED

## HYPOTHESIS

V017 tests one compiler code-layout hint on the existing wide low-precision
helper. `ProcessWideLowPrecision()` receives `__attribute__((hot))`; its body,
arithmetic, GM transfers, synchronization, UB layout, tile selection, row
assignment, and block count remain unchanged. V017 does not use the V008
`blockCount` change, the V009/V010 `noinline` changes, or the V012-V016 data
movement and residency mechanisms.

## SOURCE CHANGE

Only `本地实验/WIDE-UB-RESIDENCY-X/V017/submission.asc:3085` changes from:

```text
float epsilon)
```

to:

```text
float epsilon) __attribute__((hot))
```

The existing `paired_runner_v016` target and its Parent V011 binding were
reused. No new Runner or script was created.

## SERVER COMPILE

SERVER: `cann-server3` (`hwnput3`)
SOC_VERSION: `Ascend910B3`
SOURCE_PATH: `/home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N02/WIDE-UB-RESIDENCY-X/server-txn-20261017/project/本地实验/WIDE-UB-RESIDENCY-X/V017/submission.asc`
REMOTE_TRANSACTION: `/home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N02/WIDE-UB-RESIDENCY-X/server-txn-20261017/V017`
CANN_ENV: `/usr/local/Ascend/ascend-toolkit/set_env.sh`
RUNNER_BINDING: existing `paired_runner_v016`; temporary V017 binding was used for Compile and then restored to V016.

CONFIGURE_RETURN_CODE: `0`
BUILD_RETURN_CODE: `0`
LDD_RETURN_CODE: `0`
COMPILE: PASS
COMPILE_LOG: `本地实验/WIDE-UB-RESIDENCY-X/V017/compile.log`
REMOTE_COMPILE_LOG: `/home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N02/WIDE-UB-RESIDENCY-X/server-txn-20261017/V017/compile.log`

## DEVICE AVAILABILITY

Correctness and Local were not started. The resource snapshot showed an
existing process on every NPU, so no device was selected and no Runner process
was launched or interrupted. The raw snapshot is preserved at
`本地实验/WIDE-UB-RESIDENCY-X/V017/device-before-run.snapshot`.

| device | HBM usage | existing process | process memory MB |
|---:|---:|---|---:|
| 0 | 59% | VLLMEngineCor | 35520 |
| 1 | 98% | python | 61016 |
| 2 | 97% | python | 60039 |
| 3 | 93% | python | 57691 |
| 4 | 90% | VLLMEngineCor | 55706 |
| 5 | 97% | VLLMWorker_TP plus python | 56360 plus 1920 plus 1920 |
| 6 | 94% | VLLMWorker_TP plus python | 56361 plus 1920 |
| 7 | 95% | python plus python | 57243 plus 1920 |

DEVICE_ID: NONE
RUNNER_STARTED: NO
RUNNER_RETURN_CODE: NONE
CORRECTNESS: NOT_RUN
LOCAL: NOT_RUN
NOT_RUN_REASON: All eight NPUs had existing processes; starting a route on an occupied device could affect other tasks. No process was stopped or changed.
DEVICE_RESET: NO
DEVICE_SNAPSHOT: `本地实验/WIDE-UB-RESIDENCY-X/V017/device-before-run.snapshot`
RUNNER_BINDING_RESTORE_LOG: `本地实验/WIDE-UB-RESIDENCY-X/V017/runner-binding-restore.log`

## FOUR CASE STATUS

No timing or Correctness samples exist because the Runner was not started.

| case | Parent median us | Candidate median us | paired delta median us | P10-P90 us | status |
|---|---:|---:|---:|---:|---|
| fp16-tail-d12288 | NONE | NONE | NONE | NONE | NOT_RUN |
| fp16-wide-d32768 | NONE | NONE | NONE | NONE | NOT_RUN |
| bf16-tail-d12288 | NONE | NONE | NONE | NONE | NOT_RUN |
| bf16-wide-d32768 | NONE | NONE | NONE | NONE | NOT_RUN |

LOCAL_SCORE: NONE
LOCAL_DELTA: NONE
CORRECTNESS_MATRIX: NONE
CANDIDATE_FOR_N06: NO; Correctness and Local evidence are absent.

STATUS: SERVER_COMPILE_PASS_CORRECTNESS_LOCAL_NOT_EXECUTED_RESOURCE_BUSY
PUSH: NO

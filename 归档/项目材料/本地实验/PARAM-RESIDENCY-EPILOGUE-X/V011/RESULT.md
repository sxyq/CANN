# PARAM-RESIDENCY-EPILOGUE-X V011

ROUTE=PARAM-RESIDENCY-EPILOGUE-X
REVISION=V011
PARENT=V009
PARENT_SOURCE_COMMIT=2e7cc0a8
CANDIDATE_SOURCE=本地实验/PARAM-RESIDENCY-EPILOGUE-X/V011/R31B-V016-WIDE-TILE-SEED-V011_kernel.asc
RUNNER=本地实验/R31B/V016/support/paired/paired_runner_v016
BUILD_TARGET=paired_runner_v016
SOURCE_COMMIT=ac615783
PUSH=NO
OFFICIAL=PASS_15_OF_15

## HYPOTHESIS

In V009 `ProcessWideLowPrecision`, wide FP16 uses one output tile and performs `SyncVToMTE3`, `Store`, and `SyncMTE3ToV` for every output tile. This makes the Vector side wait for each MTE3 transfer before it can write the next result. The same kernel already contains an alternating two-output-tile event pattern in `ProcessWideFp16BatchedOutputPipelined`: each slot has a `V_MTE3` ready event and an `MTE3_V` release event, and the release is waited only before that slot is reused.

V011 applies that existing pattern to the wide FP16 epilogue. Two output tiles allow MTE3 to drain one store while Vector forms the next result in the other slot. The arithmetic order remains `ToFloat -> Muls -> FromFloat -> Mul(Gamma) -> Add(Bias) -> Store`; Gamma/Bias load count, parameter values, tile traversal, and BF16 behavior are unchanged. V007 tested a single low-precision output tile for the BF16 path; this candidate tests two slots on the FP16 path.

Source evidence: V009 lines 82–89 allocate one wide FP16 output tile; lines 3325–3399 contain the current output path. The reusable two-slot event ordering is at lines 2916–3071 in the V009 kernel. V007's recorded Local result had mixed small medians and all P10–P90 ranges crossing zero, so it does not establish a stable gain for the single-slot schedule.

## CHANGE

- Copied the V009 kernel snapshot and changed only the wide FP16 output staging in `ProcessWideLowPrecision`.
- Allocated two FP16 output tiles and alternated their use across output stores; each slot is released after its MTE3 transfer completes.
- Counted the two output tiles in the existing wide-row UB sizing call. No math, Gamma/Bias handling, BF16 path, Runner, or build script was changed.

## COMPILE

STATUS=PASS
REMOTE_HOST=cann-server3
REMOTE_WORKDIR=/home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N05/V011
HOST=hwnput3
TOOLCHAIN=CANN 8.5.0.alpha002
SOC=Ascend910B3
NPU_ARCH=dav-2201
DEVICE_ID=3
BUILD_TARGET=paired_runner_v016
CONFIGURE_RC=0
COMPILE_RC=0
REMOTE_CANDIDATE_SOURCE=/home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N05/V011/src/R31B-V016-WIDE-TILE-SEED-V010_kernel.asc
REMOTE_CANDIDATE_SOURCE_SHA256=c6a696776042368844fe50970b94c6b312e71c0087b56cd2f54e29aa2450eec4
COMPILE_LOG=remote/compile-attempt5.log
COMPILE_ENV=remote/compile-attempt5-env.txt
COMPILE_NOTE=Attempts 1-4 retained; attempt4 returned 2 before the CPLUS_INCLUDE_PATH fix, attempt5 returned 0 with the existing server3 set_env.sh and CPLUS_INCLUDE_PATH environment.

## CORRECTNESS AND LOCAL

CORRECTNESS=PASS_4_OF_4
LOCAL=MEASURED
RUNNER_COMMAND=`/home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N05/V011/build/paired_runner_v016 --paired 3`
RUNNER_RC=0
METHOD=`existing paired_runner_v016; rows=2; warmup_each=45; pairs_each=21; device event primary; wall time diagnostic`

| Shape | Parent median us | Candidate median us | Paired delta median us | Candidate minus Parent median |
|---|---:|---:|---:|---:|
| fp16-tail-d12288 | 8.740 | 8.160 | -0.480 | -0.580 us (-6.6%) |
| fp16-wide-d32768 | 10.680 | 8.880 | -1.520 | -1.800 us (-16.9%) |
| bf16-tail-d12288 | 7.480 | 8.040 | -0.380 | +0.560 us (+7.5%) |
| bf16-wide-d32768 | 14.020 | 13.240 | -0.740 | -0.780 us (-5.6%) |

SAMPLE_COUNT=21 pairs per shape
LOCAL_SCORE=FP16_WIDE_CLEAR_SIGNAL
INTERPRETATION=Correctness passed for all four shapes. The changed-domain fp16-wide case shows a clear device-event signal: -1.520 us paired median and -16.9% candidate median. fp16-tail and bf16-wide also improved; bf16-tail candidate median regressed by 0.560 us, so this is a Main review candidate rather than a claim of universal shape improvement.

HBM_USAGE_PRE=5% (npu-smi device 3; exact free HBM not reported by the captured command)
HBM_USAGE_POST=5% (npu-smi device 3; exact free HBM not reported by the captured command)
DEVICE_LOAD_PRE=AICore 0%, AIVector 0%, AICPU 0%, CtrlCPU 1%
DEVICE_LOAD_POST=AICore 0%, AIVector 0%, AICPU 0%, CtrlCPU 17%
SYSTEM_LOAD_PRE=18:50:09, load average 16.23/27.60/29.83
SYSTEM_LOAD_POST=18:50:18, load average 15.82/27.13/29.65
OTHER_PROCESS_PRESENT=Device proc query unsupported; own-process snapshots retained. No N05 runner remained after exit.
RAW_REMOTE_LOGS=/home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N05/V011/logs/
RAW_LOCAL_LOGS=本地实验/PARAM-RESIDENCY-EPILOGUE-X/V011/remote/

## NEXT DEVICE ENTRY

MAIN_HANDOFF=RECOMMEND
ONLINE_SUBMIT=PASS_15_OF_15
ONLINE_SUBMIT_OWNER=Main using the existing online submission script; this Agent did not perform an online submission.
NEXT_CANDIDATE=V011
NEXT_CANDIDATE_SOURCE=本地实验/PARAM-RESIDENCY-EPILOGUE-X/V011/R31B-V016-WIDE-TILE-SEED-V011_kernel.asc

## 2026-10-17 服务器事务结果

- Compile：PASS；首次环境缺失和头文件失败日志保留，沿用现有 `set_env.sh` 与 `CPLUS_INCLUDE_PATH` 后 Compile attempt5 返回 `0`。
- Correctness：PASS，FP16/BF16、D=12288/32768 四组 Parent/Candidate 全部通过。
- Local：四组各 21 对，目标 FP16 D32768 paired median `-1.520 us`，P10-P90 `[-9.540,-0.840] us`；其他三组 paired median 为 `-0.480/-0.380/-0.740 us`，原始服务器事务日志保存在 `/home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N05/V011/logs/paired-run-attempt1.stdout.log`。
- 设备：NPU3，前后无其他 NPU 进程，HBM 使用率约 5%，未执行设备重置。
- Official：`Pass 15/15`，score `41.25`，Submission ID `6ad27379193e507d8ec08770`；结果文件：`线上结果/NEW-N05-PARAM-RESIDENCY-EPILOGUE-X/V011/result.json`。
- 源码身份：本地 SHA256 `c6a696776042368844fe50970b94c6b312e71c0087b56cd2f54e29aa2450eec4`，平台返回 `LOCAL_SHA_EQ_REMOTE_SHA`。

STATUS=COMPILE_PASS_CORRECTNESS_PASS_LOCAL_TARGET_GAIN_OFFICIAL_PASS
OFFICIAL_SCORE=41.25


# W5-R01 CROSSROW-EPI-PROLOGUE-X V002 Local recheck

AGENT_ID: W5-R01/Zeno
REVISION: V002
HYPOTHESIS_ID: H1_MID_EPILOGUE_ISSUE
WORKTREE: /home/data4t2/lelinfeng/cann-w5-r01-crossrow
BRANCH: research/w5-r01-crossrow-epi-prologue

## Identity

PARENT_SOURCE_SHA256: 3698c229dfb6a4b336e7ee0506eb1ceaf898c1c37c937c0af7d3eef71fbc685d
CANDIDATE_SOURCE_SHA256: 426489bfdc63441f2fd179fdff176875b24d815e6e0420343aeed0b0dab7933f
V002_ARTIFACT_MANIFEST: v002-artifact-sha256-20261010.txt
V002_BOUND_PARENT_SO_SHA256: 3ad998adbb7a126f15b7da0a8f8e0a816ca719d92a4dc42aba97d7832ffa4db4
V002_BOUND_CANDIDATE_SO_SHA256: 0ef46469d226225c47ca4c33286b505b48444175095572bc3abf0e8d49c30126
V002_BOUND_RUNNER_SHA256: 9ba784c8748af1c33eeb92cb651203d82a7ffc224294c6ccb8e69b95f8db827f
V002_RUNNER_ABI_SHA256: 0897a04ba82bda39d53e77d548966eb54a6b7ec16c17387dccf26d86cbf577d2

The existing V001 paired backend was temporarily rebound to the two route-owned V002 source files. The temporary metadata labels identify the direct parent as V001-Candidate and the candidate as V002. No new Runner, scorer, kernel change, ABI change, timing-boundary change, shape change, or dispatch change was introduced. The original V001 support files were restored and rebuilt after the measurement; their tracked diff is empty.

## Build

COMMAND: `source /usr/local/Ascend/ascend-toolkit/set_env.sh; export CPLUS_INCLUDE_PATH and CPATH to the HCC C++ include root, target root, and backward; cmake --build 本地实验/CROSSROW-EPI-PROLOGUE-X/V001/support/build --clean-first --parallel 1`
RESULT: PASS (rc=0)
LOG: rebuild-v002-local-20261010.log
RESTORE_COMMAND: same build contract after restoring V001 support files
RESTORE_RESULT: PASS (rc=0)
RESTORE_LOG: restore-v001-after-v002-20261010.log

## Correctness

DEVICE: 3
TARGET: rows=16,width=6144,blocks=8,dtype=fp16
TARGET_COMMAND: `source /usr/local/Ascend/ascend-toolkit/set_env.sh; crossrow_v001_paired_runner --mode correctness --device 3 --rows 16 --width 6144 --blocks 8 --dtype fp16`
TARGET_RESULT: PASS; bit_differences=0; max_abs=0; tolerance_failures=0; nonfinite=0
TARGET_LOG: correctness-v002-clean-20261010-r16-d6144.log
M1_CONTROL: rows=1,width=6144,blocks=1,dtype=fp16
M1_COMMAND: `source /usr/local/Ascend/ascend-toolkit/set_env.sh; crossrow_v001_paired_runner --mode correctness --device 3 --rows 1 --width 6144 --blocks 1 --dtype fp16`
M1_RESULT: PASS; bit_differences=0; max_abs=0; tolerance_failures=0; nonfinite=0
M1_LOG: correctness-v002-clean-20261010-m1-d6144.log

## Local

DEVICE: 3
SHAPE: rows=16,width=6144,blocks=8,dtype=fp16
COMMAND: `source /usr/local/Ascend/ascend-toolkit/set_env.sh; crossrow_v001_paired_runner --mode local --device 3 --rows 16 --width 6144 --blocks 8 --dtype fp16 --warmups 45 --repeats 21 --output local-v002-r16-d6144-w45-20261010.tsv`
WARMUPS: 45
PAIRED_SAMPLES: 21
TIMING: device-event (`aclrtRecordEvent`, stop-event synchronization, `aclrtEventElapsedTime`)
ORDER: alternating PC/CP
RAW: local-v002-r16-d6144-w45-20261010.tsv
RUNNER_LOG: local-v002-r16-d6144-w45-20261010.log

PARENT_MEDIAN_DEVICE_US: 17.580001
CANDIDATE_MEDIAN_DEVICE_US: 17.920000
MEDIAN_PAIRED_DELTA_DEVICE_US: 0.279998
MEDIAN_RATIO_FORMULA: 100 * (median(C) - median(P)) / median(P)
MEDIAN_RATIO_PERCENT: +1.934010%
PAIRED_PERCENT_FORMULA: median_i(100 * (C_i - P_i) / P_i)
PAIRED_PERCENT: +1.592708%
PARENT_P05_P95_DEVICE_US: 7.800000 / 21.919999
CANDIDATE_P05_P95_DEVICE_US: 7.360000 / 23.100000
CANDIDATE_FASTER_SAMPLES: 10/21
CANDIDATE_SLOWER_SAMPLES: 11/21
PARENT_MEDIAN_WALL_US: 67.701000
CANDIDATE_MEDIAN_WALL_US: 67.271000

## Load quality and verdict

HBM_GATE: PASS; helper `free_hbm_mb 3` reported 48496 MB before and after Local, above MIN_FREE_HBM_MB=100
PRE_LOAD: local-v002-r16-d6144-w45-20261010-load-pre-local.txt
POST_LOAD: local-v002-r16-d6144-w45-20261010-load-post.txt
EXTERNAL_PROCESS: PID 1859841 user zhangka+ python3.11 was present on NPU3 in both snapshots, using 14238 MB; target AICore was approximately 11% pre and 13% post
OTHER_DEVICES: npu-smi also reported processes on NPU0/1/2/4/5/6/7
LOAD_QUALITY: CONTAMINATED_EXTERNAL_PROCESS
LOCAL_VERDICT: LOCAL_REJECTED / MEASUREMENT_CONTAMINATED
PROMOTION: NO
NEXT_LOCAL: NEEDS_ONE_MORE_LOCAL only if Planning requests a clean causal measurement

The raw result is real and retained, but the external target-device process and changing system load make it unsuitable for causal acceptance. It does not advance LOCAL_BEST or any Official result. The old V002 polluted raw remains unchanged.

## Dedup and scope

This is a measurement/build identity recheck of the existing V002 H1; the V002 OFAT kernel diff is unchanged. No R31B V011, Champion, other Route source, shared Tooling, shared scheduler, new Runner, scorer, or V003 was used. No new kernel patch was introduced, so no new patch-similarity claim is made; the existing V002 mechanism remains the only tested change.

NEXT_ACTION: keep V002 at contaminated/no-promotion; Planning may authorize at most one clean repeat if a non-contaminated HBM-qualified window is desired. Do not create V003.

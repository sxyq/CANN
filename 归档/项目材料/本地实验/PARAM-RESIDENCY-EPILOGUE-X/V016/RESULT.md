# PARAM-RESIDENCY-EPILOGUE-X V016

ROUTE=PARAM-RESIDENCY-EPILOGUE-X
REVISION=V016
PARENT=V015
REFERENCE=R31B-V011 Official=45.16
PREVIOUS_V014_OFFICIAL=41.64
BRANCH=route/new-n05-param-residency-epilogue-x-20261016
CANDIDATE_SOURCE=本地实验/PARAM-RESIDENCY-EPILOGUE-X/V016/R31B-V016-WIDE-TILE-SEED-V016_kernel.asc
RUNNER=本地实验/R31B/V016/support/paired/paired_runner_v016
BUILD_TARGET=paired_runner_v016
SOURCE_COMMIT=35fd36ecb8e1cc9d1c2b802a5f5fe014835ccdb2
PUSH=NO
OFFICIAL=NOT_RUN

## SEARCH AND MECHANISM

The V001-V015 source and result records were searched before selecting this
candidate. The existing low-precision wide epilogue already contains these
separate mechanisms: parameter-slot event scheduling, retained-y and output
buffer reuse, FP32 parameter promotion, and a BF16 `FusedMulAdd` path. The
FP16 branch in V015 still performed the affine stage as separate FP32 `Mul`
and `Add` operations after the V015 parameter promotion.

V016 changes only that FP16 affine stage to the existing `FusedMulAdd` entry
already used by the BF16 branch and documented by V006. After `Muls` computes
the normalized value, the call evaluates value * Gamma + Bias in the same
FP32 work tensor, followed by the existing `FromFloat` into the existing
FP16 `outputBuf_` tile.

This mechanism is independent of V014 parameter wait placement, V015 FP32
parameter promotion, and retained-y/output-store reuse. It does not alter
parameter loads, `prd`/`prel` events, retained-y layout, output buffer choice,
Store offsets, or the Runner. V013 was not run.

## COMPILE

REMOTE_HOST=cann-server3
REMOTE_HOSTNAME=hwnput3
REMOTE_WORKDIR=/home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N05/V016
TOOLCHAIN=CANN 8.5.0.alpha002
SOC=Ascend910B3
NPU_ARCH=dav-2201
DEVICE_ID=3
BUILD_TARGET=paired_runner_v016
CONFIGURE_RC=0
COMPILE_RC=0
CONFIGURE_COMMAND=`set +u; source /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/script/set_env.sh; set -u; export CPLUS_INCLUDE_PATH=/usr/include/c++/11:/usr/include/aarch64-linux-gnu/c++/11:/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/toolkit/toolchain/hcc/aarch64-target-linux-gnu/include/c++/7.3.0:/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/toolkit/toolchain/hcc/aarch64-target-linux-gnu/include/c++/7.3.0/aarch64-target-linux-gnu:/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/toolkit/toolchain/hcc/aarch64-target-linux-gnu/include/c++/7.3.0/backward; export LD_LIBRARY_PATH=/usr/lib/aarch64-linux-gnu:${LD_LIBRARY_PATH:-}; cmake -S /home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N05/V016/support -B /home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N05/V016/build -DASCEND_CANN_PACKAGE_PATH=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002 -DR31B_V011_SOURCE_DIR=/home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N05/V014/parent -DR31B_V016_SOURCE_DIR=/home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N05/V016/src`
BUILD_COMMAND=`cmake --build /home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N05/V016/build --target paired_runner_v016 --parallel 1`
REMOTE_BINARY=/home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N05/V016/build/paired_runner_v016
REMOTE_BINARY_BYTES=582472
COMPILE_NOTE=Configure and Compile completed. CMake retained the existing libstdc++.so.6 runtime search-path warning.

## CORRECTNESS AND LOCAL

RUNNER_COMMAND=`/home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N05/V016/build/paired_runner_v016 --paired 3`
RUNNER_RC=0
METHOD=`existing paired_runner_v016; warmup_each=45; pairs_each=21; device-event timing primary; wall time diagnostic`
CORRECTNESS=PASS_8_OF_8
LOCAL=MEASURED
SAMPLE_COUNT=21 pairs per case
LOCAL_SCORE=NONE

The existing Runner labels the fixed reference binary as `V011` and the
Candidate binary as `V016`; the Candidate source for this run is V016.

| Case | Parent median us | Candidate median us | Paired delta median us | Paired delta P10 us | Paired delta P90 us |
|---|---:|---:|---:|---:|---:|
| fp16-tail-d12288 | 8.140 | 7.800 | +0.340 | -15.360 | 17.080 |
| fp16-wide-d32768 | 11.940 | 13.080 | +0.960 | -10.060 | 11.360 |
| bf16-tail-d12288 | 16.680 | 7.300 | -3.920 | -12.940 | 13.040 |
| bf16-wide-d32768 | 14.560 | 14.020 | -0.960 | -9.620 | 6.060 |

All four Parent/Candidate correctness cases passed. All four paired P10-P90
intervals cross zero. The two BF16 medians are lower, while both FP16 cases
are mixed or slower; the wide and tail controls are noisy. V016 therefore
does not establish a stable Local gain.

## DEVICE AND EVIDENCE

PRE_HBM_USAGE=5% of 65536 MiB
POST_HBM_USAGE=5% of 65536 MiB
PRE_AICORE=0%
POST_AICORE=0%
PRE_AIVECTOR=0%
POST_AIVECTOR=0%
PRE_TEMPERATURE=33 C
POST_TEMPERATURE=33 C
OTHER_PROCESS_PRESENT=UNKNOWN; `npu-smi info proc -i 3` reported that proc info is unsupported on this device.
DISK_AVAIL_PRE=259G on /home/data4t2
DISK_AVAIL_POST=259G on /home/data4t2
NO_OTHER_PROCESS_MODIFIED=YES

Local evidence directory:
`本地实验/PARAM-RESIDENCY-EPILOGUE-X/V016/remote/`

Evidence files include `preflight-v016.log`, `compile-env-v016.txt`,
`configure-v016.log`, `compile-v016.log`, `pre-run-device-v016.txt`,
`pre-run-processes-v016.txt`, `run-env-v016.txt`, `paired-v016.stdout.log`,
`paired-v016.stderr.log`, `paired-v016.rc`, `post-run-device-v016.txt`,
`post-run-processes-v016.txt`, and `post-run-end-v016.txt`, with individual
return-code files retained alongside the command outputs.

## RESULT

STATUS=COMPILE_PASS_CORRECTNESS_PASS_LOCAL_MEASURED_OFFICIAL_NOT_RUN
ONLINE_SUBMIT=NOT_RUN

## NEXT EXPERIMENT

V016 has no stable Local gain. Before a further version, search the existing
parameter-load and legal-epilogue code again and select a mechanism independent
of V014 wait placement, V015 parameter promotion, V016 affine fusion, and
retained-y/output-store reuse. If no independent entry remains, record that
conclusion and stop without creating a candidate.

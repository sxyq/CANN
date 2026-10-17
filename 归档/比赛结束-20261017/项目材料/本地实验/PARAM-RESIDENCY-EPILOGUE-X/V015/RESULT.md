# PARAM-RESIDENCY-EPILOGUE-X V015

ROUTE=PARAM-RESIDENCY-EPILOGUE-X
REVISION=V015
PARENT=V014
REFERENCE=R31B-V011
BRANCH=route/new-n05-param-residency-epilogue-x-20261016
CANDIDATE_SOURCE=本地实验/PARAM-RESIDENCY-EPILOGUE-X/V015/R31B-V016-WIDE-TILE-SEED-V015_kernel.asc
RUNNER=本地实验/R31B/V016/support/paired/paired_runner_v016
BUILD_TARGET=paired_runner_v016
SOURCE_COMMIT=a83144dc2d70f6ce612f225ea83f12a8f1d77c1a
PUSH=NO
OFFICIAL=NOT_RUN

## SEARCH AND MECHANISM

The existing `ProcessWideLowPrecision` Pass 2 loads Gamma/Bias into the two
parameter slots `gBase` and `bBase`. The wide FP16 path already has independent
`xFp32Buf_` and `residualFp32Buf_` work buffers after Pass 1 reduction, while
the FP16 epilogue previously converted the retained half value to FP16 before
applying native Gamma/Bias.

V015 uses those existing FP32 work buffers for the loaded parameter tile:

- `ToFloat(gammaLocal, biasLocal)` runs once per parameter tile after the
  existing `prd` wait and before the row loop.
- Each retained row performs `Muls`, FP32 `Mul` with Gamma, FP32 `Add` with
  Bias, then one `FromFloat` into the existing FP16 `outputBuf_` tile.
- The `WaitFlag<MTE2_V>(prd)` position, `prel0`/`prel1` release events,
  retained-y layout, output buffer choice, Store offset, and Store ordering
  are unchanged.

This is a parameter promotion and legal epilogue arithmetic change. It does
not use retained-y/output-store reuse and does not repeat the V014 wait
placement change. V013 was not run.

## COMPILE

REMOTE_HOST=cann-server3
REMOTE_HOSTNAME=hwnput3
REMOTE_WORKDIR=/home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N05/V015
TOOLCHAIN=CANN 8.5.0.alpha002
SOC=Ascend910B3
NPU_ARCH=dav-2201
DEVICE_ID=3
BUILD_TARGET=paired_runner_v016
CONFIGURE_RC=0
COMPILE_RC=0
CONFIGURE_COMMAND=`set +u; source /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/script/set_env.sh; set -u; export CPLUS_INCLUDE_PATH=/usr/include/c++/11:/usr/include/aarch64-linux-gnu/c++/11:/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/toolkit/toolchain/hcc/aarch64-target-linux-gnu/include/c++/7.3.0:/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/toolkit/toolchain/hcc/aarch64-target-linux-gnu/include/c++/7.3.0/aarch64-target-linux-gnu:/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/toolkit/toolchain/hcc/aarch64-target-linux-gnu/include/c++/7.3.0/backward; export LD_LIBRARY_PATH=/usr/lib/aarch64-linux-gnu:${LD_LIBRARY_PATH:-}; cmake -S /home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N05/V015/support -B /home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N05/V015/build -DASCEND_CANN_PACKAGE_PATH=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002 -DR31B_V011_SOURCE_DIR=/home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N05/V014/parent -DR31B_V016_SOURCE_DIR=/home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N05/V015/src`
BUILD_COMMAND=`cmake --build /home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N05/V015/build --target paired_runner_v016 --parallel 1`
REMOTE_BINARY=/home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N05/V015/build/paired_runner_v016
REMOTE_BINARY_BYTES=582480
COMPILE_NOTE=Configure and Compile completed. CMake retained the existing libstdc++.so.6 runtime search-path warning.

## CORRECTNESS AND LOCAL

RUNNER_COMMAND=`/home/data4t2/lelinfeng/cann/server_runs/C2C-V2-N05/V015/build/paired_runner_v016 --paired 3`
RUNNER_RC=0
METHOD=`existing paired_runner_v016; warmup_each=45; pairs_each=21; device-event timing primary; wall time diagnostic`
CORRECTNESS=PASS_8_OF_8
LOCAL=MEASURED
SAMPLE_COUNT=21 pairs per case
LOCAL_SCORE=NONE

The existing Runner labels the fixed reference binary as `V011` and the
Candidate binary as `V016`; the Candidate source for this run is V015.

| Case | Parent median us | Candidate median us | Paired delta median us | Paired delta P10 us | Paired delta P90 us |
|---|---:|---:|---:|---:|---:|
| fp16-tail-d12288 | 8.160 | 8.140 | +0.160 | -17.940 | 14.880 |
| fp16-wide-d32768 | 11.640 | 11.900 | +0.860 | -11.920 | 12.380 |
| bf16-tail-d12288 | 7.860 | 20.160 | +0.380 | -3.120 | 14.000 |
| bf16-wide-d32768 | 14.340 | 13.740 | -0.540 | -9.260 | 8.000 |

All four Parent/Candidate correctness cases passed. All four paired P10-P90
intervals cross zero. BF16 D32768 has a small negative paired median, while
the FP16 wide case is positive and both tail cases are noisy. V015 therefore
does not establish a Local gain.

## DEVICE AND EVIDENCE

PRE_HBM_USAGE=5% of 65536 MiB
POST_HBM_USAGE=5% of 65536 MiB
PRE_AICORE=0%
POST_AICORE=0%
PRE_AIVECTOR=0%
POST_AIVECTOR=0%
PRE_TEMPERATURE=30 C
POST_TEMPERATURE=29 C
OTHER_PROCESS_PRESENT=UNKNOWN; `npu-smi info proc -i 3` reported that proc info is unsupported on this device.
DISK_AVAIL_PRE=259G on /home/data4t2
DISK_AVAIL_POST=259G on /home/data4t2
NO_OTHER_PROCESS_MODIFIED=YES

Local evidence directory:
`本地实验/PARAM-RESIDENCY-EPILOGUE-X/V015/remote/`

Evidence files include `preflight-v015.log`, `compile-env-v015.txt`,
`configure-v015.log`, `compile-v015.log`, `pre-run-device-v015.txt`,
`pre-run-processes-v015.txt`, `run-env-v015.txt`, `paired-v015.stdout.log`,
`paired-v015.stderr.log`, `paired-v015.rc`, `post-run-device-v015.txt`,
`post-run-processes-v015.txt`, and `post-run-end-v015.txt`, with individual
return-code files retained alongside the command outputs.

## RESULT

STATUS=COMPILE_PASS_CORRECTNESS_PASS_LOCAL_MEASURED_OFFICIAL_NOT_RUN
ONLINE_SUBMIT=NOT_RUN

## NEXT EXPERIMENT

Do not repeat V015 as an unchanged candidate. Before V016, search the existing
parameter-load and legal-epilogue code again and select a mechanism independent
of V014 wait placement, retained-y/output-store reuse, and this FP16 parameter
promotion. If no such mechanism is found, record that conclusion without
creating a candidate.

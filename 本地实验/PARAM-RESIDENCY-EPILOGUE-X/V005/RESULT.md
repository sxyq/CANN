# PARAM-RESIDENCY-EPILOGUE-X V005

ROUTE=PARAM-RESIDENCY-EPILOGUE-X
REVISION=V005
PARENT=R31B-V011
BRANCH=route/new-n05-param-residency-epilogue-x-20261016
PARENT_SOURCE=线上结果/R31B/V011/submission.asc
PARENT_SOURCE_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
CANDIDATE_SOURCE=本地实验/PARAM-RESIDENCY-EPILOGUE-X/V005/R31B-V016-WIDE-TILE-SEED-V005_kernel.asc
CANDIDATE_SOURCE_SHA256=11f20d8fc1cb3b07d872832f3ec9532aff7c292c57acd4184d9fee64fa285d79

## HYPOTHESIS

In the BF16 wide-row pass 2, Gamma and Bias are widened into the existing FP32 `xFp32` and `residualFp32` tiles before the row loop. The original BF16 Gamma tile is then not read by the output formula. Reuse that tile as the BF16 output destination for every tile and row, removing the separate wide-path output-tile allocation. The existing FP32 values preserve the arithmetic order and parameter values while output is stored.

V004 reused Gamma only for the one-row final BF16 tile and retained the separate output allocation. V005 applies the same proven lifetime to all BF16 tiles and batch rows, and removes that dedicated allocation. FP16, row geometry, retained `y`, reduction, parameter DMA order, and the output formula are unchanged.

## INPUTS AND BUILD

DEVICE=server3 NPU 4, Ascend910B3
CANN=8.5.0.alpha002
RUNNER=existing `paired_runner_v016`
REMOTE_TEMP=/tmp/n05-param-residency-epilogue-v005.85A8gB
CANDIDATE_TRANSFER_IDENTITY=LOCAL_EQUALS_REMOTE; local and remote SHA256 `11f20d8fc1cb3b07d872832f3ec9532aff7c292c57acd4184d9fee64fa285d79`
PARENT_TRANSFER_IDENTITY=LOCAL_EQUALS_REMOTE; parent SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`

CONFIGURE_RC=0
BUILD_RC=0
BUILD_TARGET=paired_runner_v016
CONFIGURE_COMMAND=`cmake -S ${REMOTE_TEMP}/paired -B ${REMOTE_TEMP}/build -DASCEND_CANN_PACKAGE_PATH=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002 -DR31B_V011_SOURCE_DIR=${REMOTE_TEMP}/parent -DR31B_V016_SOURCE_DIR=${REMOTE_TEMP}/candidate`
BUILD_COMMAND=`cmake --build ${REMOTE_TEMP}/build --target paired_runner_v016 --parallel 1`
DEVICE_OBJECT=${REMOTE_TEMP}/build/CMakeFiles/paired_runner_v016.dir/paired_runner_v016.asc.o; size=271360 bytes
RUNNER_BINARY=${REMOTE_TEMP}/build/paired_runner_v016; size=581480 bytes
RUNNER_CANDIDATE_LABEL=V016; include explicitly points to V005 Candidate

Evidence: `remote/compile-config.log`, `remote/compile-build.log`, `remote/source-identities.log`.

## DEVICE OBJECT COMPARISON

V004 was rebuilt with the same CANN 8.5.0.alpha002 package, Ascend910B3 target, CMake project, and paired Runner. The temporary Runner copy included the V004 Candidate. The Runner was not launched.

The first build attempt stopped because the compiler-generated registration source could not locate the system `vector` header. A single rebuild supplied the existing system C++ include directories through `CPLUS_INCLUDE_PATH` and completed successfully.

| Artifact | V004 | V005 | Byte comparison |
|---|---:|---:|---|
| `.asc.o` | 267,672 bytes | 268,480 bytes | different whole objects |
| `.aicore_binary` | 87,584 bytes | 87,336 bytes | different; first differing byte 41 |
| `__aicore_rel_binary` | 91,369 bytes | 91,121 bytes | different; first differing byte 41 |

This confirms the compiled device sections differ between V004 and V005. It does not establish a performance gain. The section extraction sizes and comparisons are recorded in `remote/device-object-compare.log`.

## CORRECTNESS

Parent-only `--qualify-v011`: 4/4 cases PASS.

The existing paired Runner completed two runs. In both runs, Parent and Candidate passed all four cases; Candidate correctness was 4/4 per run (8/8 total), with zero failures.

| Case | Run 1 paired median delta (Candidate − Parent), µs | Run 2 paired median delta, µs |
|---|---:|---:|
| FP16 D=12288 | -0.280 | -0.580 |
| FP16 D=32768 | -0.140 | -0.160 |
| BF16 D=12288 | -0.360 | -0.500 |
| BF16 D=32768 | -0.900 | -1.000 |

Evidence: `remote/qualify-v011.log`, `remote/paired-v005.log`, `remote/paired-v005-a2.log`.

## LOCAL

METHOD=existing paired Runner; 45 warmups and 21 interleaved pairs per case; device-event timing primary; wall-clock diagnostic.
DEVICE=4 Ascend910B3
PRE/POST=HBM 59215/65536 MB used (90%); AICore, AIVector, and HBM bandwidth 0%; health OK
OTHER_PROCESS=VLLMEngineCor PID 2999855 remained present on NPU 4; no process was changed

The four paired median deltas were directionally lower in both runs. However, both runs contain substantial device-event outliers. The untouched FP16 control cases also moved lower, and BF16 paired-delta p90 values were as high as +23.260 us in Run 1 and +43.600 us in Run 2. These samples do not establish a stable Candidate-specific gain.

LOCAL_SCORE=NONE
INTERPRETATION=Correctness passed; two noisy runs show a repeated lower BF16-wide median direction, but dispersion and movement in untouched controls prevent attributing a reliable gain to V005.

Raw samples and device snapshots are retained in `remote/`:

- `pre-local-snapshot.log`
- `pre-paired-device.log`
- `post-paired-device.log`
- `pre-paired-a2-device.log`
- `post-paired-a2-device.log`
- `paired-v005.log`
- `paired-v005-a2.log`

## RESULT

RESULT=COMPILE_PASS_CORRECTNESS_PASS_LOCAL_NONE
OFFICIAL=NOT_SUBMITTED
PUSH=NO
ISOLATION_NOT_ENFORCED=YES
COMMIT=fe97fc2e

NEXT=Keep the measured result and source locally. Do not claim a Local gain; add this distinct, correctness-passing version to the existing Official queue for N06 when that owner is available.

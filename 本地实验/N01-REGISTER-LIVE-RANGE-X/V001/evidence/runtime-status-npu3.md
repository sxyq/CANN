# N01 V001 NPU 3 Runtime Status

ROUTE=N01 REGISTER-LIVE-RANGE-X
REVISION=V001
CAPTURED_UTC=2026-10-10T12:29:40Z; 2026-10-10T12:34:42Z
HOST=server3 / hwnput3
DEVICE=NPU 3; SOC=Ascend910B3; HBM_CAPACITY_MB=65536

## Source And Module Identity

PARENT_SOURCE=线上结果/R31B/V011/submission.asc
PARENT_SOURCE_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
CANDIDATE_SOURCE=本地实验/N01-REGISTER-LIVE-RANGE-X/V001/submission.asc
CANDIDATE_SOURCE_SHA256=c00a2ac61ea722a84a098d9f047ecf39c539e2dfda98d49e2fec910dd063f2dc
REMOTE_PARENT_SOURCE_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
REMOTE_CANDIDATE_SOURCE_SHA256=c00a2ac61ea722a84a098d9f047ecf39c539e2dfda98d49e2fec910dd063f2dc

COMPILE_TOOLKIT=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002
COMPILE_TARGETS=device submission full_link
COMPILE_RESULT=PASS; BUILD_RC=0
DEVICE_MODULE=/home/data4t2/lelinfeng/phase4-review-repro/R31B-V016/N01-V001/build/device.alink
DEVICE_MODULE_SHA256=9d15e8a27eac64f9e5c59865aa1ca2501c2b8d15a82167c371fea16cd4e1e8cd
SUBMISSION_MODULE=/home/data4t2/lelinfeng/phase4-review-repro/R31B-V016/N01-V001/build/submission.alink
SUBMISSION_MODULE_SHA256=586f61f0145b1a32828efbce0c6a26f32ffc4bbdd557873dffe85fc768379661

PAIRED_RUNNER_SOURCE_ASC_SHA256=b827960aa2d60e6efd953b30f3ecfcfab17af5111f4d03e5309c4c94c68d1a55
PAIRED_RUNNER_SOURCE_CPP_SHA256=8ae255cce16bb33e4224d63b1e89127408e36b9db6d7048b341ad440de0dc385
PAIRED_RUNNER_EXECUTABLE=/home/data4t2/lelinfeng/phase4-review-repro/R31B-V016/N01-V001/paired-runner-build/paired_runner_v016
PAIRED_RUNNER_EXECUTABLE_SHA256=b4885a93a959f17f1bb945bfae9c7225bd94275185bec91771fe88f1a3f2474c
PAIRED_RUNNER_SOURCE_MAP=paired_runner_v016.asc includes both R31B-V011-LP-ROW-PIPELINE_kernel.asc and R31B-V016-WIDE-TILE-SEED_kernel.asc
DEVICE_SELECTION_CHANGE=evidence/paired-runner-device3.patch changes kDevice from 4 to 3 in the temporary runner source

## Planned Cases And Execution

SHAPES_AND_DTYPES=rows=2; fp16 width=12288 and 32768; bf16 width=12288 and 32768
RUNNER_PROTOCOL=45 warmups per binary and case; 21 interleaved Parent/Candidate pairs per case
CORRECTNESS=NOT_RUN
LOCAL=NONE
RAW_SAMPLES=NONE; runner was not invoked

The assigned device had an existing python3.11 process. The paired runner executes on the same NPU and calls aclrtResetDevice(device) at shutdown. Its compute workload could also contend with the existing task, so no runner was started and no other device was selected. No process was modified.

## Device Snapshots

At 2026-10-10T12:29:40Z and 2026-10-10T12:34:42Z, npu-smi reported:

NPU_ID=3
HBM_CAPACITY_MB=65536
HBM_USAGE_RATE=26%
AICORE_USAGE_RATE=0%
AIVECTOR_USAGE_RATE=0%
HBM_BANDWIDTH_USAGE_RATE=0%
PROCESS_ID=1859841
PROCESS_NAME=python3.11
PROCESS_MEMORY_MB=14238

The allocation table had no active N01/NPU 3 entry. Its NPU 3 entries were a historical R31A lease marked RELEASED. The live process snapshot remained the controlling device-use fact.

## Paired Runner Build

The first paired-runner build attempt failed because CMake selected the `latest` compiler and could not find `<vector>`. The existing V016 runner was rebuilt with the compiler and include environment aligned to CANN 8.5.0.alpha002; configure and build both returned 0. The executable was built but not run. Full output is in `evidence/compile-server3.log`.

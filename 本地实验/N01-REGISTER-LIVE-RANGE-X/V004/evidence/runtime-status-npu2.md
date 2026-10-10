# N01 V004 NPU 2 Runtime and Compile Status

ROUTE=N01 REGISTER-LIVE-RANGE-X
REVISION=V004
HOST=server3 / hwnput3
DEVICE=NPU 2; SOC=Ascend910B3; HBM_CAPACITY_MB=65536

CANDIDATE_SOURCE=本地实验/N01-REGISTER-LIVE-RANGE-X/V004/submission.asc
CANDIDATE_SOURCE_SHA256=b7fdfc0aff03fffd52f72c55f2ed3b9e61db1ab795de4c0e9684834782f012c1
REMOTE_CANDIDATE=/home/data4t2/lelinfeng/phase4-review-repro/R31B-V016/N01-V004/R31B-V016-WIDE-TILE-SEED_kernel.asc
REMOTE_CANDIDATE_SHA256=b7fdfc0aff03fffd52f72c55f2ed3b9e61db1ab795de4c0e9684834782f012c1
RUNTIME_PARENT=/home/data4t2/lelinfeng/phase4-workspaces/R31B/R31B-V011-LP-ROW-PIPELINE_kernel.asc
RUNTIME_PARENT_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3

BUILD_INPUTS=CMakeLists.txt; device.cpp; adapter.cpp; runtime_probe.cpp from compile_adapter_v016.asc; link_one.sh
BUILD_INPUT_SOURCE=本地实验/R31B/V016/support/
REMOTE_DIR=/home/data4t2/lelinfeng/phase4-review-repro/R31B-V016/N01-V004
TOOLKIT=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002
ENV_INIT=source /usr/local/Ascend/ascend-toolkit/set_env.sh with nounset disabled; explicit V002 CPLUS_INCLUDE_PATH and system-first LD_LIBRARY_PATH

COMPILE_CONFIGURE_RC=0
COMPILE_BUILD_RC=0
COMPILE_TARGETS=device submission full_link
COMPILE_COMMAND=cmake --build /home/data4t2/lelinfeng/phase4-review-repro/R31B-V016/N01-V004/build --target device submission full_link --parallel 2
DEVICE_MODULE=/home/data4t2/lelinfeng/phase4-review-repro/R31B-V016/N01-V004/build/device.alink
DEVICE_MODULE_SHA256=74e18219566986d6daf2307bafe6bbe206fd2b7c11bd91b5fb066068c09871a3
SUBMISSION_MODULE=/home/data4t2/lelinfeng/phase4-review-repro/R31B-V016/N01-V004/build/submission.alink
SUBMISSION_MODULE_SHA256=1904eb438d3616c454c00e3457f91eb65c6896b040944bb51c761a83fa94345c

NPU2_PRE_COMPILE=HBM_USAGE_RATE=5%; AICORE_USAGE_RATE=0%; process query command unsupported by npu-smi; captured 2026-10-10T17:13:40Z
NPU2_POST_COMPILE=HBM_USAGE_RATE=5%; AICORE_USAGE_RATE=0%; process query command unsupported by npu-smi; captured 2026-10-10T17:13:56Z
HOST_LOAD_PRE=37.24 49.90 53.23
HOST_LOAD_POST=45.29 51.23 53.62
DISK_PRE=/dev/nvme1n1p1 3.5T 3.1T 189G 95% /home/data4t2
DISK_POST=/dev/nvme1n1p1 3.5T 3.1T 184G 95% /home/data4t2

CORRECTNESS=PASS
CORRECTNESS_CASES=4/4; PARENT_FAILURES=0; CANDIDATE_FAILURES=0; RUNNER_RC=0
LOCAL=RECORDED_RAW
LOCAL_SCORE=NONE
LOCAL_REASON=HBM usage stayed at 96% with PID 658669 present; device-event timing had large outliers and paired-delta p10-p90 intervals crossed zero for all four shapes.
RUNNER=/home/data4t2/lelinfeng/phase4-review-repro/R31B-V016/N01-V003/paired-runner-build/paired_runner_v016 --paired
RUNNER_SOURCE_SHA256=87da3944c4054eca8e91f62dd6bd90bfed1f50ff6221d83794181a66317b49fa
RUNNER_BINARY_SHA256=3198cc87f3f2d4f59c445e93f46923589fa4e9d934e3d6f4bcbd7c43578687aa
RUNNER_RESET_SCAN=NONE
NPU2_PRE_RUN=2026-10-10T18:42:24Z; HBM_USAGE_RATE=96%; AICORE_USAGE_RATE=0%; AIVECTOR_USAGE_RATE=0%; PROCESS_ID=658669; PROCESS_MEMORY_MB=59977
NPU2_POST_RUN=2026-10-10T18:42:33Z; HBM_USAGE_RATE=96%; AICORE_USAGE_RATE=54%; AIVECTOR_USAGE_RATE=51%; PROCESS_ID=658669; PROCESS_MEMORY_MB=59977
HOST_LOAD_PRE=52.53 53.30 53.59
HOST_LOAD_POST=52.08 53.16 53.55
DISK_RUNTIME=/dev/nvme1n1p1 3.5T 3.2T 143G 96%
RAW_SAMPLES=4 shapes; 21 interleaved pairs per shape; see evidence/correctness-local-server3.log
TEMP_NO_RESET_COPY_BUILD=FAIL; CONFIGURE_RC=0; BUILD_RC=2; no device execution; missing <vector> is preserved in evidence/temp-no-reset-runner-server3.log
ORIGINAL_RESET_RUNNER=NOT_INVOKED
NPU3_USE=NONE
FULL_SOURCE_SIMILARITY=NOT_MEASURED
REMOTE_RAW_LOG=/home/data4t2/lelinfeng/phase4-review-repro/R31B-V016/N01-V004/runtime-v004-npu2-followup-20261010T184224Z.log
EVIDENCE=evidence/compile-server3.log; evidence/runtime-followup-server3.log; evidence/correctness-local-server3.log; evidence/temp-no-reset-runner-server3.log; diff.patch; evidence/dependency-check.md

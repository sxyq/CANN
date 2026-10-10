# N01 V004 NPU 2 Compile Status

ROUTE=N01 REGISTER-LIVE-RANGE-X
REVISION=V004
HOST=server3 / hwnput3
DEVICE=NPU 2; SOC=Ascend910B3; HBM_CAPACITY_MB=65536

CANDIDATE_SOURCE=本地实验/N01-REGISTER-LIVE-RANGE-X/V004/submission.asc
CANDIDATE_SOURCE_SHA256=b7fdfc0aff03fffd52f72c55f2ed3b9e61db1ab795de4c0e9684834782f012c1
REMOTE_CANDIDATE=/home/data4t2/lelinfeng/phase4-review-repro/R31B-V016/N01-V004/R31B-V016-WIDE-TILE-SEED_kernel.asc
REMOTE_CANDIDATE_SHA256=b7fdfc0aff03fffd52f72c55f2ed3b9e61db1ab795de4c0e9684834782f012c1

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

CORRECTNESS=NOT_RUN
LOCAL=NONE
RUNNER=NOT_RUN
ORIGINAL_RESET_RUNNER=NOT_INVOKED
NPU3_USE=NONE
FULL_SOURCE_SIMILARITY=NOT_MEASURED
EVIDENCE=evidence/compile-server3.log; diff.patch; evidence/dependency-check.md

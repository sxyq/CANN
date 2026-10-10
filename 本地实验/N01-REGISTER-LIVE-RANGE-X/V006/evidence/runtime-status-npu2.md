# N01 V006 NPU 2 Runtime and Compile Status

ROUTE=N01 REGISTER-LIVE-RANGE-X
REVISION=V006
HOST=server3 / hwnput3
DEVICE=NPU 2; SOC=Ascend910B3; HBM_CAPACITY_MB=65536

PARENT_SOURCE=本地实验/N01-REGISTER-LIVE-RANGE-X/V005/submission.asc
PARENT_SOURCE_SHA256=07f3c0d291b772385eb47bcad831cd28a063b14c093e9f23a5274cab786d768b
CANDIDATE_SOURCE=本地实验/N01-REGISTER-LIVE-RANGE-X/V006/submission.asc
CANDIDATE_SOURCE_SHA256=85dc7e6b431a1160255fb09243666fda6fadf766034328e25098b181ae573f68
RUNTIME_PARENT=/home/data4t2/lelinfeng/phase4-workspaces/R31B/R31B-V011-LP-ROW-PIPELINE_kernel.asc
RUNTIME_PARENT_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
REMOTE_CANDIDATE=/home/data4t2/lelinfeng/phase4-review-repro/R31B-V016/N01-V006/R31B-V016-WIDE-TILE-SEED_kernel.asc
REMOTE_CANDIDATE_SHA256=85dc7e6b431a1160255fb09243666fda6fadf766034328e25098b181ae573f68

BUILD_INPUTS=CMakeLists.txt; device.cpp; adapter.cpp; runtime_probe.cpp; link_one.sh
BUILD_INPUT_SOURCE=existing /home/data4t2/lelinfeng/phase4-review-repro/R31B-V016/N01-V005 inputs copied into the V006 route directory
REMOTE_DIR=/home/data4t2/lelinfeng/phase4-review-repro/R31B-V016/N01-V006
TOOLKIT=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002
ENV_INIT=source /usr/local/Ascend/ascend-toolkit/set_env.sh with nounset disabled; V002 CPLUS_INCLUDE_PATH and system-first LD_LIBRARY_PATH

COMPILE=PASS
COMPILE_CONFIGURE_RC=0
COMPILE_BUILD_RC=0
COMPILE_TARGETS=device submission full_link
COMPILE_COMMAND=cmake --build /home/data4t2/lelinfeng/phase4-review-repro/R31B-V016/N01-V006/build --target device submission full_link --parallel 2
DEVICE_MODULE=/home/data4t2/lelinfeng/phase4-review-repro/R31B-V016/N01-V006/build/device.alink
DEVICE_MODULE_SHA256=05652b5a78eae7b3d7c7e975bedfdbe7b45ff81c56ad36310d1595f764539a2e
SUBMISSION_MODULE=/home/data4t2/lelinfeng/phase4-review-repro/R31B-V016/N01-V006/build/submission.alink
SUBMISSION_MODULE_SHA256=f29d91abfa9912534fba8e64d828f604c35c61b0b426b4b494a804854a9eafab

RUNNER_SOURCE=/home/data4t2/lelinfeng/phase4-review-repro/R31B-V016/N01-V003/paired-runner-src
RUNNER_BUILD=/home/data4t2/lelinfeng/phase4-review-repro/R31B-V016/N01-V003/paired-runner-build
RUNNER_CONFIGURE_COMMAND=cmake -S /home/data4t2/lelinfeng/phase4-review-repro/R31B-V016/N01-V003/paired-runner-src -B /home/data4t2/lelinfeng/phase4-review-repro/R31B-V016/N01-V003/paired-runner-build -DR31B_V016_SOURCE_DIR=/home/data4t2/lelinfeng/phase4-review-repro/R31B-V016/N01-V006
RUNNER_BUILD_COMMAND=cmake --build /home/data4t2/lelinfeng/phase4-review-repro/R31B-V016/N01-V003/paired-runner-build --target paired_runner_v016 --parallel 1 -- -B
RUNNER_COMMAND=/home/data4t2/lelinfeng/phase4-review-repro/R31B-V016/N01-V003/paired-runner-build/paired_runner_v016 --paired
RUNNER_SOURCE_SHA256=87da3944c4054eca8e91f62dd6bd90bfed1f50ff6221d83794181a66317b49fa
RUNNER_BINARY_SHA256=3198cc87f3f2d4f59c445e93f46923589fa4e9d934e3d6f4bcbd7c43578687aa
RUNNER_CONFIGURE_RC=0
RUNNER_BUILD_RC=0
RESET_SCAN=NONE
ORIGINAL_RESET_RUNNER=NOT_INVOKED

CORRECTNESS=PASS
CORRECTNESS_CASES=4/4; PARENT_FAILURES=0; CANDIDATE_FAILURES=0; RUNNER_RC=0
SHAPES_DTYPES=fp16-tail-d12288; fp16-wide-d32768; bf16-tail-d12288; bf16-wide-d32768
LOCAL=RECORDED_RAW
LOCAL_SCORE=NONE
LOCAL_REASON=HBM remained at 96% with PID 658669 present; AICore/Aivector were nonzero and timing contained large outliers. Every paired-delta p10-p90 interval crossed zero.
LOCAL_STATS=fp16-tail delta_median=-0.560us MAD=7.640us p10=-15.740 p90=11.240; fp16-wide delta_median=-0.520us MAD=2.540us p10=-10.140 p90=11.380; bf16-tail delta_median=8.760us MAD=9.140us p10=-12.380 p90=178.880; bf16-wide delta_median=1.080us MAD=4.320us p10=-5.500 p90=12.520

NPU2_PRE_RUN=2026-10-10T19:58:56Z; HBM_USAGE_RATE=96%; AICORE_USAGE_RATE=68%; AIVECTOR_USAGE_RATE=35%; PROCESS_ID=658669; PROCESS_MEMORY_MB=59977
NPU2_POST_RUN=2026-10-10T19:59:04Z; HBM_USAGE_RATE=96%; AICORE_USAGE_RATE=65%; AIVECTOR_USAGE_RATE=34%; PROCESS_ID=658669; PROCESS_MEMORY_MB=59977
HOST_LOAD_PRE=51.11 50.78 53.08
HOST_LOAD_POST=63.81 53.40 53.90
DISK_RUNTIME=/dev/nvme1n1p1 3.5T 3.2T 134G 97%
NPU3_USE=NONE
FULL_SOURCE_SIMILARITY=NOT_MEASURED
REMOTE_COMPILE_LOG=/home/data4t2/lelinfeng/phase4-review-repro/R31B-V016/N01-V006/compile-v006-server3-20261010T195631Z.log
REMOTE_RUNTIME_LOG=/home/data4t2/lelinfeng/phase4-review-repro/R31B-V016/N01-V006/runtime-v006-npu2-20261010T195830Z.log
EVIDENCE=evidence/compile-server3.log; evidence/correctness-local-server3.log; diff.patch

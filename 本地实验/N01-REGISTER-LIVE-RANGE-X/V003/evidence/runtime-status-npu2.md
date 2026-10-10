# N01 V003 NPU 2 Runtime Status

ROUTE=N01 REGISTER-LIVE-RANGE-X
REVISION=V003
HOST=server3 / hwnput3
DEVICE=NPU 2; SOC=Ascend910B3; HBM_CAPACITY_MB=65536

PARENT_SOURCE=线上结果/R31B/V011/submission.asc
PARENT_SOURCE_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
CANDIDATE_SOURCE=本地实验/N01-REGISTER-LIVE-RANGE-X/V003/submission.asc
CANDIDATE_SOURCE_SHA256=f355ef215f9e717b3207010ade0893fed34c7d6078bd1f2d0e7dbd7c966c1da0
CANDIDATE_MODULE=/home/data4t2/lelinfeng/phase4-review-repro/R31B-V016/N01-V003/build/submission.alink
CANDIDATE_MODULE_SHA256=d039bfb9c16395bd1a6a00d3287b6101ab04ed5f7310bf56d14c5899ba7ef016
CANDIDATE_DEVICE_MODULE=/home/data4t2/lelinfeng/phase4-review-repro/R31B-V016/N01-V003/build/device.alink
CANDIDATE_DEVICE_MODULE_SHA256=3b3a366334d386664cf8308299e8b91cf9ce4b59300ad3db3d38e4379b544ae9

COMPILE=PASS
COMPILE_COMMAND=cmake --build /home/data4t2/lelinfeng/phase4-review-repro/R31B-V016/N01-V003/build --target device submission full_link --parallel 2
COMPILE_BUILD_RC=0
COMPILE_EVIDENCE=evidence/compile-server3.log

RUNNER_BUILD=FAIL
RUNNER_BUILD_COMMAND=cmake --build /home/data4t2/lelinfeng/phase4-review-repro/R31B-V016/N01-V003/paired-runner-build --target paired_runner_v016 --parallel 2
RUNNER_BUILD_RC=2
RUNNER_SOURCE_POLICY=kDevice=2; aclrtResetDevice line removed; RESET_SYMBOL_SCAN=NONE before build
RUNNER_BUILD_REASON=The build command used an incorrect target C++ include path, producing _GLIBCXX20_CONSTEXPR and related standard-header parse errors. No runner process started.

CORRECTNESS=NOT_RUN
LOCAL=NONE
RAW_SAMPLES=NONE
ORIGINAL_RESET_RUNNER=NOT_INVOKED

NPU2_BEFORE_COMPILE=HBM_USAGE_RATE=5%; AICORE_USAGE_RATE=0%; PROCESS=NONE; captured 2026-10-10T15:33:39+00:00
NPU2_AFTER_COMPILE=HBM_USAGE_RATE=5%; AICORE_USAGE_RATE=0%; PROCESS=NONE; captured 2026-10-10T15:33:39+00:00
NPU2_BEFORE_RUNNER_BUILD=HBM_USAGE_RATE=5%; AICORE_USAGE_RATE=0%; PROCESS=NONE; captured 2026-10-10T15:36:52+00:00
NPU2_AFTER_RUNNER_BUILD=HBM_USAGE_RATE=5%; AICORE_USAGE_RATE=0%; PROCESS=NONE; captured 2026-10-10T15:36:52+00:00
HOST_LOAD_COMPILE=53.91 56.53 59.86 before; 58.87 57.34 60.06 after
NO_NPU3_USE=TRUE

The failed runner build did not launch an executable and did not modify any device process. V003 runtime work is deferred to the next action recorded in the declaration.

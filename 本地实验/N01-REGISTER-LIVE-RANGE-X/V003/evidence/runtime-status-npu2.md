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

RUNNER_BUILD_INITIAL=FAIL
RUNNER_BUILD_INITIAL_RC=2
RUNNER_BUILD_INITIAL_REASON=Host-only C++ compilation failed because <vector> was unavailable under the initial include environment; no executable was produced and no NPU run started.
RUNNER_BUILD_RETRY=PASS
RUNNER_BUILD_RETRY_COMMAND=cmake --build /home/data4t2/lelinfeng/phase4-review-repro/R31B-V016/N01-V003/paired-runner-build --target paired_runner_v016 --parallel 1 -- -B
RUNNER_BUILD_RETRY_RC=0
RUNNER_SOURCE_POLICY=kDevice=2; aclrtResetDevice line removed; RESET_SYMBOL_SCAN=NONE before build
RUNNER_BINARY_SHA256=3198cc87f3f2d4f59c445e93f46923589fa4e9d934e3d6f4bcbd7c43578687aa

CORRECTNESS=PASS
CORRECTNESS_CASES=4; PARENT_FAILURES=0; CANDIDATE_FAILURES=0
LOCAL=RECORDED_RAW
LOCAL_SCORE=NONE
RAW_SAMPLES=4 shapes; 21 interleaved pairs per shape; 84 pairs total
ORIGINAL_RESET_RUNNER=NOT_INVOKED

NPU2_BEFORE_COMPILE=HBM_USAGE_RATE=5%; AICORE_USAGE_RATE=0%; PROCESS=NONE; captured 2026-10-10T15:33:39+00:00
NPU2_AFTER_COMPILE=HBM_USAGE_RATE=5%; AICORE_USAGE_RATE=0%; PROCESS=NONE; captured 2026-10-10T15:33:39+00:00
NPU2_BEFORE_RUNNER_BUILD=HBM_USAGE_RATE=5%; AICORE_USAGE_RATE=0%; PROCESS=NONE; captured 2026-10-10T15:36:52+00:00
NPU2_AFTER_RUNNER_BUILD=HBM_USAGE_RATE=5%; AICORE_USAGE_RATE=0%; PROCESS=NONE; captured 2026-10-10T15:36:52+00:00
HOST_LOAD_COMPILE=53.91 56.53 59.86 before; 58.87 57.34 60.06 after
NO_NPU3_USE=TRUE

RUNNER_SOURCE_SHA256=87da3944c4054eca8e91f62dd6bd90bfed1f50ff6221d83794181a66317b49fa
PARENT_MODULE_SHA256=3b3a366334d386664cf8308299e8b91cf9ce4b59300ad3db3d38e4379b544ae9
CANDIDATE_MODULE_SHA256=d039bfb9c16395bd1a6a00d3287b6101ab04ed5f7310bf56d14c5899ba7ef016
RUN_COMMAND=/home/data4t2/lelinfeng/phase4-review-repro/R31B-V016/N01-V003/paired-runner-build/paired_runner_v016 --paired
RUNNER_RC=0; RUNNER_DEVICE=2; RUNNER_PAIRING=Parent/Candidate interleaved; RESET_SYMBOL_SCAN=NONE

LOCAL_SPREAD_MAD_VALIDITY=Raw collection is valid for comparison, but no stable Local score is supported. Spread is max-minus-min from the runner summary. Parent and Candidate device-time spreads are 74.12%-145.36% and 75.87%-121.17% of their respective medians; paired-delta MAD is 1.65x-57.67x the absolute paired-delta median. Every paired-delta p10-p90 interval crosses zero. Keep LOCAL_SCORE=NONE.

| Shape | Parent median/spread/MAD (us) | Candidate median/spread/MAD (us) | Paired delta median/spread/MAD (us) | MAD / abs(delta) | Paired p10-p90 (us) |
|---|---:|---:|---:|---:|---:|
| fp16-tail-d12288 | 17.660 / 21.400 / 2.000 | 17.800 / 19.580 / 4.060 | +0.060 / 21.740 / 3.460 | 57.67x | -10.300 to +3.880 |
| fp16-wide-d32768 | 17.300 / 23.160 / 4.240 | 15.440 / 13.340 / 4.160 | -1.140 / 22.140 / 4.040 | 3.54x | -10.980 to +4.400 |
| bf16-tail-d12288 | 16.360 / 23.780 / 6.760 | 10.580 / 12.820 / 2.820 | -2.540 / 29.080 / 4.180 | 1.65x | -12.540 to +1.640 |
| bf16-wide-d32768 | 19.400 / 14.380 / 3.160 | 17.820 / 13.520 / 3.200 | -1.580 / 19.600 / 3.360 | 2.13x | -8.340 to +7.160 |

NPU2_PRE_RUN=HBM_USAGE_RATE=5%; AICORE_USAGE_RATE=0%; PROCESS=NONE; HOST_LOAD=56.98 53.77 58.70
NPU2_POST_RUN=HBM_USAGE_RATE=5%; AICORE_USAGE_RATE=0%; PROCESS=NONE; HOST_LOAD=54.42 53.29 58.52
FREE_HBM_MB=UNKNOWN; npu-smi output recorded capacity and rounded usage rate only
EVIDENCE_RAW=evidence/correctness-local-server3.log; build attempts=evidence/runner-build-server3.log

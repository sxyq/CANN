# N01 V002 NPU 2 Runtime Status

ROUTE=N01 REGISTER-LIVE-RANGE-X
REVISION=V002
HOST=server3 / hwnput3
DEVICE=NPU 2; SOC=Ascend910B3; HBM_CAPACITY_MB=65536

## Identity

PARENT_SOURCE=本地实验/N01-REGISTER-LIVE-RANGE-X/V001/submission.asc
PARENT_SOURCE_SHA256=c00a2ac61ea722a84a098d9f047ecf39c539e2dfda98d49e2fec910dd063f2dc
CANDIDATE_SOURCE=本地实验/N01-REGISTER-LIVE-RANGE-X/V002/submission.asc
CANDIDATE_SOURCE_SHA256=59ea1096afa7fa72ae7768a8b7fb51389917d6fda28660ee77ca68a9821b3a8d
OFFICIAL_REFERENCE_SOURCE=线上结果/R31B/V011/submission.asc
OFFICIAL_REFERENCE_SOURCE_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
PARENT_MODULE=/home/data4t2/lelinfeng/phase4-review-repro/R31B-V016/N01-V002/build/device.alink
PARENT_MODULE_SHA256=a9807b28de7760e870e9696327b3c87ca549099d8cfbb86bdb68e4cf391030ee
CANDIDATE_MODULE=/home/data4t2/lelinfeng/phase4-review-repro/R31B-V016/N01-V002/build/submission.alink
CANDIDATE_MODULE_SHA256=a512caede5b7a615a673bc18a9aae96774298728eca1c8f8becccfdb43fcaa02
RUNNER=/home/data4t2/lelinfeng/phase4-review-repro/R31B-V016/N01-V002/paired-runner-build/paired_runner_v016
RUNNER_SHA256=56245a48f5ebf0b1c8ed383769f3d11a476debee820a7a396e132764acebe42f
RUNNER_SOURCE_SHA256=87da3944c4054eca8e91f62dd6bd90bfed1f50ff6221d83794181a66317b49fa
RUNNER_SOURCE_POLICY=kDevice=2; aclrtResetDevice line removed; RESET_SYMBOL_SCAN=NONE

## Compile

COMPILE=PASS
CONFIGURE_COMMAND=cmake -S /home/data4t2/lelinfeng/phase4-review-repro/R31B-V016/N01-V002 -B /home/data4t2/lelinfeng/phase4-review-repro/R31B-V016/N01-V002/build -DASCEND_CANN_PACKAGE_PATH=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002 -DR31B_PARENT_SOURCE_DIR=/home/data4t2/lelinfeng/phase4-workspaces/R31B -DCMAKE_BUILD_TYPE=Release
BUILD_COMMAND=cmake --build /home/data4t2/lelinfeng/phase4-review-repro/R31B-V016/N01-V002/build --target device submission full_link --parallel 2
BUILD_RC=0
COMPILE_EVIDENCE=evidence/compile-server3.log

## Correctness And Local

RUN_COMMAND=/home/data4t2/lelinfeng/phase4-review-repro/R31B-V016/N01-V002/paired-runner-build/paired_runner_v016 --paired
RUNNER_ENV_FIX=LD_PRELOAD=/usr/lib/aarch64-linux-gnu/libstdc++.so.6; first attempt failed before aclInit with GLIBCXX_3.4.29 missing from HCC libstdc++.so.6.0.24
CORRECTNESS=PASS; fp16-tail-d12288, fp16-wide-d32768, bf16-tail-d12288, bf16-wide-d32768; Parent and Candidate failures=0 in every case
SHAPE_DTYPE=rows=2; fp16 width=12288/32768; bf16 width=12288/32768
WARMUP=45 per binary and case
SAMPLES=21 Parent/Candidate pairs per case; raw samples are in evidence/correctness-local-server3.log
LOCAL=RECORDED_RAW; LOCAL_SCORE=NONE due one block per case and high dispersion
MEDIANS_US=fp16-tail 8.520/9.300; fp16-wide 11.840/13.380; bf16-tail 15.160/16.420; bf16-wide 15.960/14.800 (Parent/Candidate)
PAIRED_DELTA_MEDIANS_US=fp16-tail +0.320; fp16-wide +0.500; bf16-tail -0.140; bf16-wide -0.380
DEVICE_EVENT_CV=fp16-tail Parent/Candidate 0.63164/0.51744; fp16-wide 0.36352/0.31011; bf16-tail 0.43440/0.45927; bf16-wide 0.21842/0.27520

## Device Snapshots

RUN_PRE_UTC=2026-10-10T14:27:11+00:00
RUN_POST_UTC=2026-10-10T14:27:19+00:00
HBM_PRE=5%; HBM_POST=5%; FREE_HBM_APPROX_MB=62259
AICORE_PRE=0%; AICORE_POST=0%
PROCESS_PRE=NONE
PROCESS_POST=NONE
HOST_LOAD_PRE=64.89 51.88 47.63
HOST_LOAD_POST=59.24 51.10 47.42
NO_OTHER_NPU_PROCESS=TRUE
NO_NPU3_USE=TRUE

The original runner was never invoked. The executed binary was the V002 temporary copy with the reset call removed; it finalized ACL without resetting NPU 2. No process was modified.

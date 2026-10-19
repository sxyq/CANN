# W4-R03 V002 Result

```text
ROUTE = W4-R03
REVISION = V002
DIRECT_PARENT = R31B-V011
SINGLE_CHANGE = When rowCount == 2 * blockCount, assign exactly two consecutive rows per block directly; preserve the quotient/remainder path otherwise.
FOCUS_AXIS = row ownership arithmetic
FOCUS_VALUE = two rows per block fast path
PARENT_SHA256 = a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
CANDIDATE_SHA256 = 673ca80ff59976f4875b243504886103c95cf7ecaee9a7c79dc17dc7bef13de6
V001_CANDIDATE_SHA256 = 9259c38d104fa23046b7b3c95eeeef639e07dca89c2fd95499742afe3566a273
COMPILE = PASS
COMPILE_COMMAND = cmake --build build --target r03_v002_runner --parallel 1 -- -B VERBOSE=1
COMPILE_ENV = CANN 8.5.0.alpha002; CPLUS_INCLUDE_PATH=/usr/include/c++/11:/usr/include/aarch64-linux-gnu/c++/11:/usr/include/c++/11/backward
TOOLCHAIN = CANN 8.5.0.alpha002; bisheng 15.0.5; Ascend910B3 / dav-2201
CORRECTNESS = PASS; Parent and Candidate each match the FP32 CPU reference; outputs match element-for-element.
CORRECTNESS_COMMAND = build/r03_v002_runner --correctness
CORRECTNESS_SHAPE = 80x2056 FP32
CORRECTNESS_ELEMENTS = 164480
CORRECTNESS_MAX_ABS_ERROR = 4.76837158e-7 for both Parent and Candidate
LOCAL_SHAPE = 80x2056 FP32
DEVICE = 0; VECTOR_CORES = 40; BLOCKS = 40
FREE_HBM_BEFORE_LOCAL_MB = 37355 (capacity 65536 MB, usage 43%)
FREE_HBM_AFTER_LOCAL_MB = 37355 (capacity 65536 MB, usage 43%)
LOCAL_PAIRS = 21; WARMUP_PAIRS = 45; ORDER = alternating P,C / C,P
LOCAL_COMMAND = build/r03_v002_runner --paired
PARENT_MEDIAN_EVENT_US = 15.940
CANDIDATE_MEDIAN_EVENT_US = 19.260
PAIRED_DELTA_MEDIAN_US = +2.560
LOCAL_DELTA_PERCENT = +20.828105 (candidate slower; median ratio)
PARENT_EVENT_CV_PERCENT = 67.677036
CANDIDATE_EVENT_CV_PERCENT = 71.794176
PARENT_RAW_EVENT_RANGE_US = 7.180 .. 72.520
CANDIDATE_RAW_EVENT_RANGE_US = 7.360 .. 80.140
ORDER_COUNTS = P,C: 11; C,P: 10
DEVICE_LOAD_BEFORE = AICore 40%; AIVector 23%; HBM bandwidth 29%
DEVICE_LOAD_AFTER = AICore 42%; AIVector 24%; HBM bandwidth 29%
LOAD_NOTE = Busy shared device; load is context only. The post-run SSH command had one temporary hostname-resolution failure; a subsequent read succeeded and is retained in local.log.
LOCAL_VERDICT = OBSERVATION_ONLY; measured median is slower and dispersion is broad.
CURRENT_LOCAL_BEST = NONE
OFFICIAL = NOT_SUBMITTED; hand off to Online Owner for the authorized serial submission flow.
```

## Interpretation

The measured shape has 80 rows and the runner reported 40 vector cores, so the launch uses 40 blocks and each block owns exactly two rows. The Candidate takes the direct `blockIdx * 2` ownership path for that relation and keeps the original balanced quotient/remainder calculation for all other relations. The change removes division, remainder, and uneven-row selection from this case's ownership calculation; it does not alter data movement, arithmetic, tiling, or V001's scratch-buffer change.

The one Local run is an observation, not a stable ranking. Candidate event median was 20.83% slower than Parent and both series had substantial spread. No sample was removed. `support/local.log` preserves all 21 paired raw samples and device snapshots.

## Evidence files

- `submission.asc` — V002 Candidate source.
- `parent.asc` — exact R31B-V011 Parent source.
- `support/compile.log` — first CMake target-name error, missing C++ include on plugin subprocess, and final successful Compile.
- `support/correctness.log` — initial runtime-library mismatch and final successful correctness run.
- `support/local.log` — raw paired event/wall samples, summaries, and device load snapshots.
- `support/CMakeLists.txt`, `support/runner.asc`, `support/runner.cpp`, `support/runner_abi.h` — V002-specific direct-call test harness.

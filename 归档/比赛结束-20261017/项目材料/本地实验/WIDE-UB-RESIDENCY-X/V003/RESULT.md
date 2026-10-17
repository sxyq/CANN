# WIDE-UB-RESIDENCY-X V003

ROUTE: WIDE-UB-RESIDENCY-X
REVISION: V003
PARENT: 线上结果/R31B/V011/submission.asc
PREVIOUS_CANDIDATE: 本地实验/WIDE-UB-RESIDENCY-X/V002/submission.asc
PARENT_LOCAL_COMMIT: fb96a417
BRANCH: route/new-n02-wide-ub-residency-x-20261016

## HYPOTHESIS

V001 recomputed every FP16 wide-row tile in Pass2. V002 retained only the
last tile. A different one-tile compromise is to retain the first FP16
native-half `y = x + residual` tile per batch, consume it before the input
staging slot is needed for later tiles, and recompute all later tiles. This
removes one first-tile GM reload while keeping the UB footprint tile-sized.
BF16 is unchanged. Kernel partitioning, RMS partial reduction order/tree, and
Gamma/Bias parameter residency are unchanged.

## ONE CHANGE

- In the FP16 branch of `ProcessWideLowPrecision`, copy only batch row zero's
  first native-half `y` tile into the existing `outputBuf_` after the Add.
- In Pass2, read that saved tile for `batchRow == 0 && tile == 0`, then reuse
  `outputBuf_` as the normal output/input staging slot for later tiles.
- Remove V002's last-tile FP32 save/read path. No new UB allocation was added;
  BF16, FP32, parameter buffers, block assignment, and reduction code remain
  unchanged.

## INPUTS AND IDENTITY

PARENT_SOURCE: current Clone `线上结果/R31B/V011/submission.asc`
CANDIDATE_SOURCE: current Clone `本地实验/WIDE-UB-RESIDENCY-X/V003/submission.asc`
RUNNER_SOURCE: current Clone `本地实验/R31B/V016/support/paired/paired_runner.cpp`
RUNNER_ABI: current Clone `本地实验/R31B/V016/support/paired/paired_runner_abi.h`
BUILD_BINDING: current Clone `本地实验/R31B/V016/support/paired/paired_runner_v016.asc`
CMAKE: current Clone `本地实验/R31B/V016/support/paired/CMakeLists.txt`
SERVER: `cann-server3`
DEVICE: NPU 4, Ascend910B3
LOAD: paired Runner, rows=2, availableCoreNum=8, warmup_each=45, pairs_each=21
CASES: FP16 D=12288/32768; BF16 D=12288/32768
SOURCE_TRANSFER: server3 `/tmp/wide-ub-residency-x-v003.jBIu1K`
ISOLATION: ISOLATION_NOT_ENFORCED; filesystem and Git access are not
system-enforced at the Agent level. Only the current Clone paths were used.

## COMPILE

COMMAND: `cmake -S /tmp/wide-ub-residency-x-v003.jBIu1K/本地实验/R31B/V016/support/paired -B /tmp/wide-ub-residency-x-v003.jBIu1K/build -DCMAKE_BUILD_TYPE=Release && cmake --build /tmp/wide-ub-residency-x-v003.jBIu1K/build --target paired_runner_v016 --parallel 1 -- -B`
ENVIRONMENT: Ascend 8.5.0.alpha002; `ASCEND_HOME_PATH` set; system C++11
include and system libstdc++ placed before toolkit paths.
RETURN_CODE: 0
RESULT: PASS; log contains `[100%] Built target paired_runner_v016`.
EVIDENCE: `compile.log`.

## CORRECTNESS

COMMAND: existing paired Runner `paired_runner_v016 --paired`
DEVICE: NPU 4, Ascend910B3
FINAL_RETURN_CODE: 0
FINAL_RESULT: PASS; all four cases passed for V011 and V003. The runner labels
the candidate binary `V016` because the existing ABI and executable are reused;
the bound source was V003.
EVIDENCE: `paired.log` contains `failures=0` for both binaries in all cases
and `RUNNER_COMPLETE mode=PAIRED correctness=PASS cases=4`.

## LOCAL

METHOD: existing paired Runner, 45 warmups, 21 alternating pairs per case,
DEVICE_EVENT primary, wall time diagnostic.

| Case | V011 median device us | V003 median device us | Paired median delta us |
|---|---:|---:|---:|
| fp16-tail-d12288 | 6.960 | 7.120 | +0.820 |
| fp16-wide-d32768 | 10.200 | 12.140 | +1.920 |
| bf16-tail-d12288 | 7.180 | 7.080 | -0.140 |
| bf16-wide-d32768 | 13.760 | 13.700 | -0.040 |

INTERPRETATION: The affected FP16 cases are slower at the median, so the
first-tile residency strategy has no positive local result. Raw device
samples contain large outliers; `paired.log` is the source for the reported
medians and raw samples.
EVIDENCE: `paired.log`; device context is in `device.log`.

## RESULT

STATUS: CORRECTNESS_PASS_LOCAL_NEGATIVE
LOCAL_REVISION: NOT_CREATED; real Local result did not support promoting V003.
OFFICIAL: NOT_RUN
PUSH: NO
NEXT: NONE; do not start another revision from this result without a new
instruction.

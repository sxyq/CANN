# WIDE-UB-RESIDENCY-X V002

ROUTE: WIDE-UB-RESIDENCY-X
REVISION: V002
PARENT: 线上结果/R31B/V011/submission.asc
PREVIOUS_CANDIDATE: 本地实验/WIDE-UB-RESIDENCY-X/V001/submission.asc
BRANCH: route/new-n02-wide-ub-residency-x-20261016

## HYPOTHESIS

V001 changed FP16 wide rows from full-row `y` residency to full-pass reload and
recompute. A smaller compromise may reduce one final tile's GM reload while
avoiding the UB cost of retaining a complete row: retain only the last FP16
half `y = x + residual` tile across Pass1 and Pass2. Earlier tiles keep V001's
reload-and-recompute path. BF16 is unchanged. Kernel partitioning, RMS partial
reduction order/tree, and Gamma/Bias parameter residency are unchanged.

## ONE CHANGE

- In the FP16 branch of `ProcessWideLowPrecision`, after the existing Pass1
  row-level reduction and `invRms` calculation, copy the last batch row's last
  native-half `y` tile into the existing `residualFp32Buf_` as FP32.
- In Pass2, use that saved tile only when `batchRow == batchRows - 1` and
  `tile == tileCount - 1`; all other FP16 tiles reload `x/residual` and repeat
  the native-half Add exactly as in V001.
- No new UB allocation was added. `gammaBuf_`/`biasBuf_` remain the parameter
  staging pair in Pass2; BF16 and FP32 paths are unchanged.

## INPUTS AND IDENTITY

PARENT_SOURCE: current Clone `线上结果/R31B/V011/submission.asc`
CANDIDATE_SOURCE: current Clone `本地实验/WIDE-UB-RESIDENCY-X/V002/submission.asc`
RUNNER_SOURCE: current Clone `本地实验/R31B/V016/support/paired/paired_runner.cpp`
RUNNER_ABI: current Clone `本地实验/R31B/V016/support/paired/paired_runner_abi.h`
BUILD_BINDING: current Clone `本地实验/R31B/V016/support/paired/paired_runner_v016.asc`
CMAKE: current Clone `本地实验/R31B/V016/support/paired/CMakeLists.txt`
SERVER: `cann-server3`
DEVICE: NPU 4, Ascend910B3
LOAD: paired Runner, rows=2, availableCoreNum=8, warmup_each=45, pairs_each=21
CASES: FP16 D=12288/32768; BF16 D=12288/32768
SOURCE_TRANSFER: server3 `/tmp/wide-ub-residency-x-v002.0DDvE9`
ISOLATION: ISOLATION_NOT_ENFORCED; filesystem and Git access are not
system-enforced at the Agent level. Only the current Clone paths were used.

## COMPILE

COMMAND: `cmake -S /tmp/wide-ub-residency-x-v002.0DDvE9/本地实验/R31B/V016/support/paired -B /tmp/wide-ub-residency-x-v002.0DDvE9/build -DCMAKE_BUILD_TYPE=Release && cmake --build /tmp/wide-ub-residency-x-v002.0DDvE9/build --target paired_runner_v016 --parallel 1 -- -B`
ENVIRONMENT: Ascend 8.5.0.alpha002; `ASCEND_HOME_PATH` set; system C++11
include and system libstdc++ placed before toolkit paths.
RETURN_CODE: 0
RESULT: PASS; log contains `[100%] Built target paired_runner_v016`.
EVIDENCE: `compile.log`, `compile-rerun.log`, `compile-rerun2.log`,
`compile-final.log`.

## CORRECTNESS

COMMAND: existing paired Runner `paired_runner_v016 --paired`
FINAL_RETURN_CODE: 0
FINAL_RESULT: PASS; all four cases passed for V011 and V002, with
`RUNNER_COMPLETE mode=PAIRED correctness=PASS cases=4`.

The first implementation attempt failed on FP16 D=12288 with 8177 elements;
the retained slot was then moved away from the input staging buffer. The next
attempt failed with 2 elements because the row reduction overwrote the saved
slot. The final implementation saves after the row reduction and passed all
four cases. These failures remain in `paired.log` and `paired-rerun.log`;
`paired-rerun2.log` retains the intermediate 2-element failure and
`paired-final.log` retains the final pass.

## LOCAL

METHOD: existing paired Runner, 45 warmups, 21 alternating pairs per case,
DEVICE_EVENT primary, wall time diagnostic.

| Case | V011 median device us | V002 median device us | Paired median delta us |
|---|---:|---:|---:|
| fp16-tail-d12288 | 6.020 | 6.620 | +0.680 |
| fp16-wide-d32768 | 10.180 | 11.920 | +1.700 |
| bf16-tail-d12288 | 7.860 | 7.280 | -0.320 |
| bf16-wide-d32768 | 14.040 | 14.020 | -0.100 |

INTERPRETATION: The affected FP16 cases are slower at the median, so this
single-tile residency compromise has no positive local result. Raw device
samples contain large outliers, especially in the control paths; the medians
are the reported comparison while magnitude confidence is limited.
EVIDENCE: `paired-final.log`; device context is in `device.log`.

## RESULT

STATUS: CORRECTNESS_PASS_LOCAL_NEGATIVE
OFFICIAL: NOT_RUN
PUSH: NO
NEXT: NONE; do not start V003 from this result without a new instruction.

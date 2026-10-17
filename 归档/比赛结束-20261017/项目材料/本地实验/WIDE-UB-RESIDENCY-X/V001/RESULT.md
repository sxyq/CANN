# WIDE-UB-RESIDENCY-X V001

ROUTE: WIDE-UB-RESIDENCY-X
REVISION: V001
PARENT: 线上结果/R31B/V011/submission.asc
BRANCH: route/new-n02-wide-ub-residency-x-20261016

## HYPOTHESIS

FP16 宽行 Pass1 的 `y = x + residual` 整行 half UB 驻留，可能省下 Pass2 的 GM 重载和重新融合成本，但会占用大块 UB。将它缩为单 tile，并在 Pass2 用同一 native-half 顺序重载 `x/residual` 后重算 `y`，可以直接测量驻留与重算的取舍。BF16 路径保持原样。

## ONE CHANGE

- `gammaBuf_`：FP16 宽行从 `rowWidth * wideFullYRows_` 改为 `tileElems`。
- Pass1：不再把 half `y` 写入整行缓存；仍用原 half Add 结果转 FP32 做平方和。
- Pass2：用 `gammaBuf_` 和 `outputBuf_` 作为输入 tile，重载同一行列的 `x/residual`，执行原 half Add，再进入原缩放、Gamma、Bias、输出路径。
- 为这对输入 tile 增加独立 `MTE2_V` / `V_MTE2` 事件；参数双缓冲、Gamma/Bias 位置、RMS partial 与归约树、blockCount 和 tile 选择未改。

## COMPILE

COMMAND: existing paired CMake target `paired_runner_v016`; server3 temporary build with `ASCEND_HOME_PATH`, `CPATH`, `CPLUS_INCLUDE_PATH`, and system libstdc++ first.
SOURCE_INPUT: current Clone `线上结果/R31B/V011/submission.asc`, current Clone V001 candidate, current Clone paired Runner and ABI.
RESULT: PASS; final log contains `[100%] Built target paired_runner_v016`.
EVIDENCE: `compile.log`; initial environment failures and final successful build are retained.

## CORRECTNESS

COMMAND: `paired_runner_v016 --paired`
DEVICE: NPU 4, Ascend910B3 runner target
SHAPES: FP16 D=12288/32768, BF16 D=12288/32768, rows=2, availableCoreNum=8
RESULT: PASS for V011 and V016 on all four cases; failures=0 in every case.

## LOCAL

METHOD: existing paired Runner, 45 warmups, 21 alternating pairs per case, DEVICE_EVENT primary, wall time diagnostic.

| Case | V011 median device us | V016 median device us | Paired median delta us |
|---|---:|---:|---:|
| fp16-tail-d12288 | 6.220 | 6.780 | +0.560 |
| fp16-wide-d32768 | 10.480 | 13.200 | +1.960 |
| bf16-tail-d12288 | 7.700 | 7.640 | +0.140 |
| bf16-wide-d32768 | 13.880 | 14.600 | +0.140 |

INTERPRETATION: FP16 affected cases are slower at the median. Raw samples contain substantial outliers, so the local window is valid as a negative observation but not a stable magnitude estimate. BF16 is unchanged by the source mechanism and serves as a control path; no gain is promoted.
EVIDENCE: `paired.log` retains the first correctness failure before the event fix; `paired-rerun.log` retains the final all-pass run and raw samples.

## RESULT

STATUS: CORRECTNESS_PASS_LOCAL_NEGATIVE
OFFICIAL: NOT_RUN
PUSH: NO


# N01 V021 Results

ROUTE: N01-REGISTER-LIVE-RANGE-X
REVISION: V021
SOURCE_PARENT_REVISION: V020 (`934e9c99ef531799c4cb9645f45372909bef5248`)
SOURCE_COMMIT: `c50d6f4a7f466fbde2c2b92eb544082cf3a1a0b5`
SOURCE_SHA256: `d4f9994b9eb43e75b454143cbdb3f2b8a2222545d6ad4496fc6022c1682e5134`
SOURCE_PATH: `本地实验/N01-REGISTER-LIVE-RANGE-X/V021/submission.asc`
MEASUREMENT_PARENT: R31B V011
MEASUREMENT_PARENT_SOURCE_SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
DEVICE: d5 / NPU 5 / `hwnput3` / Ascend910B3

## Change

In the wide low-precision row path, the square operation now writes `valueTile * valueTile` to `residualLocal`. The following `ReduceSum` reads that tile and uses `xLocal` as its scratch tile after the loaded `xLocal` values are no longer needed. This tests reuse of a dead FP32 temporary's storage during the reduction. No Scalar address generation, UB allocation, core assignment, arithmetic formula, or DMA ordering changed.

## Build And Device Code

- CANN `8.5.0.alpha002`, Ascend910B3, device 5. Configure, compile, and device-code extraction returned `0`.
- Candidate input: `/home/data4t2/lelinfeng/cann/server_runs/N01-REGISTER-LIVE-RANGE-X-RESET-20261011/V021/c50d6f4a/candidate/R31B-V016-WIDE-TILE-SEED_kernel.asc`; build record identifies its source as V021 with SHA256 above.
- Measurement Parent input: `/home/data4t2/lelinfeng/cann/server_runs/N01-REGISTER-LIVE-RANGE-X-RESET-20261011/V004/e833b161/parent/R31B-V011-LP-ROW-PIPELINE_kernel.asc`, SHA256 above.
- Existing Runner target `paired_runner_v016`; binary SHA256 `bbd673d981aaa371e1d65a288c7b7939b69c2afcffa1a8d9e6e0e0e47e794bbf`.
- Extracted AICore binary SHA256 `fe3c6595abd33f115b03ee03b5b87b8e81be8ee2b7db1ec61d8d084832258323`; it differs from V020. Register count is unknown.
- Runner labels `V001` in Correctness and `V016` in Local are generic slots. Actual Candidate input in both runs is the V021 build above; Local Parent is R31B V011.
- Committed build and device-code evidence: `results/compile-v021.command`, `results/build-v021.log`, `results/device-code-v021.command`, and `results/device-code-v021.log`. The extracted binary remains locally at `results/aicore_binary_v021.bin`; the repository's existing `*.bin` ignore rule excludes it from the commit.

## Correctness

- d5: PASS, 4/4 cases; Runner exit `0`.
- Raw output: `results/correctness-v021.raw.log`.
- Candidate source and Runner identity: `results/correctness-v021.command`.

## Paired Local

The existing Runner completed 21 interleaved pairs per case (84 total) on d5, exit `0`. Times and paired deltas are device-event microseconds; delta is Candidate minus Parent. Every sample is preserved in `results/local-v021.raw.log`.

| Case | Parent median (p10-p90; min-max) | Candidate median (p10-p90; min-max) | Paired delta median (p10-p90; min-max) |
|---|---|---|---|
| FP16 D=12288 | 6.600 (5.540-15.660; 5.500-19.440) | 6.100 (5.720-14.200; 5.540-43.860) | +0.160 (-6.080-+7.460; -13.340-+38.040) |
| FP16 D=32768 | 10.240 (9.700-11.780; 9.600-198.080) | 10.380 (9.760-11.900; 9.640-210.900) | +0.060 (-0.300-+1.060; -188.440-+200.320) |
| BF16 D=12288 | 6.800 (6.340-7.140; 6.280-7.320) | 6.580 (6.200-7.140; 6.020-7.500) | -0.160 (-0.660-+0.420; -0.780-+0.720) |
| BF16 D=32768 | 14.020 (13.580-14.780; 13.320-56.980) | 13.400 (12.800-17.680; 12.720-110.720) | -0.520 (-1.520-+3.380; -43.120-+79.540) |

FP16 D=12288, FP16 D=32768, and BF16 D=32768 contain severe event-time outliers and broad paired-delta tails. BF16 D=12288 is comparatively stable but its -0.160 us paired median is small and the p10-p90 interval crosses zero. `LOCAL_SCORE=NONE`, `LOAD_QUALITY=NOISY`; no gain is claimed and no sample was removed.

## Device State And Evidence

Before sampling on d5: HBM use `91%`, AICore use `0%`, temperature `39 C`; existing `VLLMWorker_TP` PID `3355594` used `56354 MB`. After sampling: HBM use `91%`, AICore use `0%`, temperature `42 C`; the same process remained at `56354 MB`. No other process was changed; the paired Runner exited.

The live pre-run snapshot is `results/pre-local-device-recheck-01.txt`; the post-run snapshot is `results/post-local-device.txt`. The earlier snapshot remains at `results/pre-local-device.txt`.

Full evidence includes `results/local-v021.command`, `results/local-v021.raw.log`, `results/pre-local-device-recheck-01.command`, `results/pre-local-device-recheck-01.txt`, `results/post-local-device.command`, `results/post-local-device.txt`, `results/correctness-v021.command`, and `results/correctness-v021.raw.log`.

LOCAL_SCORE: NONE
NEXT_ACTION: stop at V021; do not create V022 in this task window.

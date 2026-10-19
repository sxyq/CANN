# N01 V020 Results

ROUTE: N01-REGISTER-LIVE-RANGE-X  
REVISION: V020  
SOURCE_PARENT_REVISION: V019 (`405a56772fd2eddb32f416b99805048d7780d87d`)  
SOURCE_COMMIT: `934e9c99ef531799c4cb9645f45372909bef5248`  
SOURCE_SHA256: `2add7f609bb93ffb84bed7040f1526ce02aab2b1049eed14ad12ef867d1cf7dc`  
SOURCE_PATH: `本地实验/N01-REGISTER-LIVE-RANGE-X/V020/submission.asc`  
MEASUREMENT_PARENT: R31B V011  
MEASUREMENT_PARENT_SOURCE_SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`  
DEVICE: d5 / NPU 5 / `hwnput3` / Ascend910B3

## Change

In the FP16 branch of `ProcessWideLowPrecision` Pass 1, the tile `ReduceSum` scratch argument changes from `xFp32` to `residualFp32`, which this branch does not otherwise use before the later row-level reduction. The squared `valueFp32` input and reduction output remain unchanged. Only the scratch tile assignment changes; arithmetic, operation order, barriers, DMA, tile schedule, and core assignment are unchanged.

## Build And Device Code

- CANN `8.5.0.alpha002`; environment setup, configure, compile, and device-code extraction all returned `0`.
- Exact Candidate input: `/home/data4t2/lelinfeng/cann/server_runs/N01-REGISTER-LIVE-RANGE-X-RESET-20261011/V020/934e9c99/candidate/R31B-V016-WIDE-TILE-SEED_kernel.asc`; remote SHA256 matched local source SHA256.
- CMake cache bound Candidate to the V020 path above and Parent to `/home/data4t2/lelinfeng/cann/server_runs/N01-REGISTER-LIVE-RANGE-X-RESET-20261011/V004/e833b161/parent`.
- Existing Runner target `paired_runner_v016`; Runner binary SHA256 `8cc015e8170c21758466d62c95287693a3ae27d2412066341c0547281f923f84`.
- Extracted AICore ELF SHA256 `7b78f9bada9608845bbf0d80776eff70084aba267bfec0492ddaf5d99fb8963e`; it differs from V019, while FP32/FP16/BF16 function sizes and `.text=0x14104` remain unchanged. Register count is unknown.
- Correctness Candidate label `V001` and Local Candidate label `V016` are generic Runner slots; both execute the compiled V020 input. Local Parent slot `V011` is the fixed R31B V011 source.
- Build and device-code evidence: `results/build-v020.log`, `results/device-code-v020.log`, `results/device-code-compare-v019-v020.log`, and `results/aicore_binary_v020.bin`.

## Correctness

- d5: PASS, 4/4 cases; Runner exit `0`.
- Raw output: `results/correctness-v020.raw.log`.
- Exact command and source identities: `results/correctness-v020.command`.

## Paired Local

The existing Runner completed 21 interleaved pairs per case (84 total) on d5. Times and paired deltas are device-event microseconds; delta is Candidate minus Parent. Every raw sample remains in `results/local-v020.raw.log`.

| Case | Parent median (p10-p90; min-max) | Candidate median (p10-p90; min-max) | Paired delta median (p10-p90; min-max) |
|---|---|---|---|
| FP16 D=12288 | 5.800 (5.660-7.140; 5.580-167.920) | 6.120 (5.680-7.480; 5.360-12.780) | +0.140 (-0.180-+0.620; -161.560-+6.980) |
| FP16 D=32768 | 9.860 (9.580-11.520; 9.420-11.760) | 10.120 (9.740-11.500; 9.640-12.060) | +0.180 (-0.040-+0.620; -0.380-+0.740) |
| BF16 D=12288 | 6.760 (6.460-7.640; 6.320-134.380) | 6.680 (6.180-7.380; 6.120-163.240) | -0.200 (-0.600-+0.300; -56.620-+28.860) |
| BF16 D=32768 | 13.800 (13.440-130.300; 13.220-179.620) | 13.180 (12.920-13.540; 12.720-144.440) | -0.600 (-117.580--0.280; -166.240-+131.000) |

Three cases have substantial outliers or broad paired-delta tails. FP16 D=32768 is comparatively stable but Candidate is slightly slower by the paired median. `LOCAL_SCORE=NONE`, `LOAD_QUALITY=NOISY`; no gain is claimed.

## Device State And Evidence

Before Correctness and Local, d5 reported HBM use `91%` and AICore use `0%`. Existing `VLLMWorker_TP` PID `3355594` used `56354 MB` before and after Local; the V020 Runner had exited after sampling. No other process was changed.

A pre-Correctness snapshot briefly showed a non-N01 Runner for N04 on d5. It had exited by the next read; no process was interrupted. Both snapshots are retained in `results/pre-correctness-device.txt` and `results/pre-correctness-device-retry-01.txt`.

Remote run directory: `/home/data4t2/lelinfeng/cann/server_runs/N01-REGISTER-LIVE-RANGE-X-RESET-20261011/V020/934e9c99`.

Evidence includes `results/compile-v020.command`, `results/build-v020.log`, `results/device-code-v020.command`, `results/device-code-v020.log`, `results/device-code-compare-v019-v020.log`, `results/aicore_binary_v020.bin`, `results/paired_runner_v016`, `results/correctness-v020.command`, `results/correctness-v020.raw.log`, `results/pre-build-device.command`, `results/pre-build-device.txt`, both pre-Correctness snapshots and command records, `results/pre-local-device.command`, `results/pre-local-device.txt`, `results/local-v020.command`, `results/local-v020.raw.log`, `results/post-local-device.command`, and `results/post-local-device.txt`.

LOCAL_SCORE: NONE  
NEXT_ACTION: commit and push V020 results, then continue with a distinct single-factor wide-row FP32 temporary lifetime hypothesis as V021 on d5. Do not repeat V020.

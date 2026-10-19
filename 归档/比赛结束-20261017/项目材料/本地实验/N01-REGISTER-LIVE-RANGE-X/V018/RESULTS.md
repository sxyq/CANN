# N01 V018 Results

ROUTE: N01-REGISTER-LIVE-RANGE-X  
REVISION: V018  
SOURCE_PARENT_REVISION: V017 (`b0801b847d1d523fddb4c8e37221b8ab59040e2c`)  
SOURCE_COMMIT: `07f0d3b603f966ebca9b4efc8d9a66087bdee458`  
SOURCE_SHA256: `bc6b799d822b31cd3ee9344e3ad8f77165a7a2826f5593321b645c72781dd0ce`  
SOURCE_PATH: `本地实验/N01-REGISTER-LIVE-RANGE-X/V018/submission.asc`  
MEASUREMENT_PARENT: R31B V011  
MEASUREMENT_PARENT_SOURCE_SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`  
DEVICE: d5 / NPU 5 / `hwnput3` / Ascend910B3

## Change

In the FP16 path of `ProcessWideLowPrecision`, after `xLocal` is written to the cached `yTile`, V018 converts `yTile` to FP32 for the square/reduction path instead of converting `xLocal` again. The hypothesis is that `xLocal` can finish its Vector use earlier. Only the conversion input changes; arithmetic, data movement, synchronization, tile schedule, and core assignment are unchanged. The source diff from V017 is one line.

## Build And Device Code

- CANN `8.5.0.alpha002`; environment setup, configure, compile, and device-code extraction each returned `0`.
- Exact Candidate input: `/home/data4t2/lelinfeng/cann/server_runs/N01-REGISTER-LIVE-RANGE-X-RESET-20261011/V018/07f0d3b6/candidate/R31B-V016-WIDE-TILE-SEED_kernel.asc`; its remote SHA256 matched the local source SHA256.
- Fixed Parent input: `/home/data4t2/lelinfeng/cann/server_runs/N01-REGISTER-LIVE-RANGE-X-RESET-20261011/V004/e833b161/parent/R31B-V011-LP-ROW-PIPELINE_kernel.asc`; source SHA256 is recorded above.
- Existing generic Runner target: `paired_runner_v016`; Runner binary SHA256 `721393ee862cfa48f25c754b02596360271d97baf968819b3befe8779132702e`.
- Extracted `.aicore_binary` SHA256 `b440b1263ec0fdef5904ec94d9b9ad2dd012ceb3be7edbae755afe0e494d9a7c`; it differs from V017. Register count is unavailable, so no register reduction is claimed.
- Runner slot labels do not name this Route revision: Correctness Candidate `V001` and Local Candidate `V016` both execute the compiled V018 source above. Local Parent `V011` is the fixed R31B V011 input.
- Build output: `results/build-v018.log`; extracted device code: `results/device-code-v018.log` and `results/aicore_binary_v018.bin`.

## Correctness

- d5: PASS, 4/4 cases; Runner exit `0`.
- Raw output: `results/correctness-v018.raw.log`.
- Exact command and Candidate/Parent identities: `results/correctness-v018.command`.

## Paired Local

The existing Runner completed 21 interleaved pairs per case (84 pairs total) on d5. Times and paired deltas are device-event microseconds; delta is Candidate minus Parent. All raw samples and outliers are retained in `results/local-v018.raw.log`.

| Case | Parent median (p10-p90; min-max) | Candidate median (p10-p90; min-max) | Paired delta median (p10-p90; min-max) |
|---|---|---|---|
| FP16 D=12288 | 6.180 (5.560-7.460; 5.440-207.760) | 6.140 (5.700-17.600; 5.640-75.760) | +0.100 (-0.520-+0.640; -182.620-+69.120) |
| FP16 D=32768 | 11.680 (9.860-27.000; 9.640-108.400) | 10.380 (9.820-42.880; 9.620-56.440) | -0.160 (-12.540-+25.240; -98.340-+46.660) |
| BF16 D=12288 | 6.800 (6.400-19.700; 6.260-69.960) | 6.540 (6.220-7.480; 6.140-20.160) | -0.280 (-12.400-+0.160; -55.660-+0.280) |
| BF16 D=32768 | 13.720 (13.500-14.140; 13.480-76.140) | 13.200 (12.900-13.480; 12.800-13.540) | -0.600 (-1.080--0.180; -62.640-+0.020) |

All cases contain substantial outliers or broad paired-delta tails. `LOCAL_SCORE=NONE`, `LOAD_QUALITY=NOISY`; no median-only gain is claimed.

## Device State And Evidence

Before Correctness and Local, d5 reported HBM use `91%` and AICore use `0%`. Existing `VLLMWorker_TP` PID `3355594` used `56354 MB` before and after Local; the V018 Runner had exited after sampling. No other process was changed.

Remote run directory: `/home/data4t2/lelinfeng/cann/server_runs/N01-REGISTER-LIVE-RANGE-X-RESET-20261011/V018/07f0d3b6`.

Evidence includes `results/compile-v018.command`, `results/build-v018.log`, `results/device-code-v018.log`, `results/aicore_binary_v018.bin`, `results/paired_runner_v016`, `results/correctness-v018.command`, `results/correctness-v018.raw.log`, `results/pre-build-device.command`, `results/pre-build-device.txt`, `results/pre-correctness-device.command`, `results/pre-correctness-device.txt`, `results/pre-local-device.command`, `results/pre-local-device.txt`, `results/local-v018.command`, `results/local-v018.raw.log`, `results/post-local-device.command`, and `results/post-local-device.txt`.

Initial source staging returned `1` because the remote V018 `candidate/` directory did not exist. That exact error is in `results/stage-v018-attempt-01.log`; the directory was then created under V018 and the uploaded source SHA was confirmed. A local zsh quoting error during the first device-code query is retained in `results/device-code-query-attempt-01.log`; the subsequent remote query completed with exit `0`.

LOCAL_SCORE: NONE  
NEXT_ACTION: after this evidence commit is pushed, continue with a distinct single-factor register-lifetime hypothesis as V019 on d5; do not repeat V018.
